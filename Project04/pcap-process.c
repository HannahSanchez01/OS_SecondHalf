
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>

#include <search.h> // Kylee
// need to use hcreate, hsearch, and hdestroy

/* for strdup due to C99 */
char * strdup(const char *s);


#include "pcap-process.h"
//#include "ht.c" // 

/* How many packets have we seen? */
uint32_t        gPacketSeenCount;

/* How many total bytes have we seen? */
uint64_t        gPacketSeenBytes;        

/* How many hits have we had? */
uint32_t        gPacketHitCount;

/* How much redundancy have we seen? */
uint64_t        gPacketHitBytes;

/* Our big table for recalling packets */
//struct PacketEntry *    BigTable; 
int BigTable;
int    BigTableSize;
int    BigTableNextToReplace;


/// Kylee // Hash table by Ben Hoyt https://benhoyt.com/writings/hash-table-in-c/
// See LICENSE.txt, ht.c, and ht.h

long hash_function(char* str)//Source: https://www.digitalocean.com/community/tutorials/hash-table-in-c-plus-plus
{
    //printf("%p\n",str);
    long i = 0;

    for (int j = 0; j<strlen(str); j++)
    {
        //printf("%c\n",str[j]);
        i += str[j];
    }
    //printf("%ld\n",strlen(str));

    return i % BigTableSize;
}

void initializeProcessingStats ()
{
    gPacketSeenCount = 0;
    gPacketSeenBytes = 0;        
    gPacketHitCount  = 0;
    gPacketHitBytes  = 0;    
}

char initializeProcessing (int TableSize)
{
    initializeProcessingStats();
	 
	 // BigTable is now a HASH table
	 BigTable = hcreate(TableSize);

	 // Check hash table failure
	 if (BigTable == 0){
	 	 printf("* Error: Error creating the hash table\n");
		 return 0;
	 }

    BigTableSize = TableSize;
    BigTableNextToReplace = 0;
    return 1;
}

void processPacket (struct Packet * pPacket)
{
    uint16_t        PayloadOffset;

    PayloadOffset = 0;


    /* Do a bit of error checking */
    if(pPacket == NULL)
    {
        printf("* Warning: Packet to assess is null - ignoring\n");
        return;
    }

    if(pPacket->Data == NULL)
    {
        printf("* Error: The data block is null - ignoring\n");
        return;
    }

    //printf("STARTFUNC: processPacket (Packet Size %d)\n", pPacket->LengthIncluded);

    /* Step 1: Should we process this packet or ignore it? 
     *    We should ignore it if:
     *      The packet is too small
     *      The packet is not an IP packet
     */

    /* Update our statistics in terms of what was in the file */
    gPacketSeenCount++;
    gPacketSeenBytes += pPacket->LengthIncluded;

    /* Is this an IP packet (Layer 2 - Type / Len == 0x0800)? */

    if(pPacket->LengthIncluded <= MIN_PKT_SIZE)
    {
        discardPacket(pPacket);
        return;
    }

    if((pPacket->Data[12] != 0x08) || (pPacket->Data[13] != 0x00))
    {
        //printf("Not IP - ignoring...\n");
        discardPacket(pPacket);
        return;
    }

    /* Adjust the payload offset to skip the Ethernet header 
        Destination MAC (6 bytes), Source MAC (6 bytes), Type/Len (2 bytes) */
    PayloadOffset += 14;

    /* Step 2: Figure out where the payload starts 
         IP Header - Look at the first byte (Version / Length)
         UDP - 8 bytes 
         TCP - Look inside header */

    if(pPacket->Data[PayloadOffset] != 0x45)
    {
        /* Not an IPv4 packet - skip it since it is IPv6 */
        printf("  Not IPV4 - Ignoring\n");
        discardPacket(pPacket);
        return;
    }
    else
    {
        /* Offset will jump over the IPv4 header eventually (+20 bytes)*/
    }

    /* Is this a UDP packet or a TCP packet? */
    if(pPacket->Data[PayloadOffset + 9] == 6)
    {
        /* TCP */
        uint8_t TCPHdrSize;

        TCPHdrSize = ((uint8_t) pPacket->Data[PayloadOffset+9+12] >> 4) * 4;
        PayloadOffset += 20 + TCPHdrSize;
    }
    else if(pPacket->Data[PayloadOffset+9] == 17)
    {
        /* UDP */

        /* Increment the offset by 28 bytes (20 for IPv4 header, 8 for the UDP header)*/
        PayloadOffset += 28;
    }
    else 
    {
        /* Don't know what this protocol is - probably not helpful */
        discardPacket(pPacket);
        return;
    }

    //printf("  processPacket -> Found an IP packet that is TCP or UDP\n");

    uint16_t    NetPayload;

    NetPayload = pPacket->LengthIncluded - PayloadOffset;

    pPacket->PayloadOffset = PayloadOffset;
    pPacket->PayloadSize = NetPayload;

    /* Step 2: Do any packet payloads match up? */
	 
	 ENTRY entry;
     int k;
     char buf[2048];//Noah: Copy contents of data to buffer for hashing
     memset(&buf,0,2048);
     for(k = 0; k<pPacket->PayloadSize; k++)
     {
        buf[k] = (char)pPacket->Data[k+PayloadOffset];
     }
     long hash = hash_function(buf);
     //printf("%ld\n",hash);
     char str[256];
     sprintf(str, "%ld", hash);
     //printf("%ld\n",strlen(str));
     entry.key = strdup(str);
	 entry.data = pPacket;

	 ENTRY * pointer1 = hsearch( entry, FIND);
	 ENTRY * pointer2;
     struct Packet * pData;
	 
	 if (pointer1 == NULL) // entry not found
	 {
	    // try to add to the hash table
		 pointer2 = hsearch( entry, ENTER);
         //pData = (struct Packet *) pointer2->data;
         //printf("PACKET ENTER TABLE: t=%ld.%08d of %d bytes long (%d on the wire) \n", (long int) pData->TimeCapture.tv_sec, (int) pData->TimeCapture.tv_usec, pData->LengthIncluded, pData->LengthOriginal);

		 if (pointer2 == NULL) // could not add to the hash table
		 {
            //printf("Table full?\n");
            entry.key = "0";
            pointer2 = hsearch(entry,ENTER);//Replace first entry with new hash - eviction policy
	    	//free(entry.key);
		 }
	 }
     else
     {
        pData = (struct Packet *) pointer1->data; //Cast data to pPacket
        //printf("PACKET RETURNED FROM TABLE t=%ld.%08d of %d bytes long (%d on the wire) \n", (long int) pData->TimeCapture.tv_sec, (int) pData->TimeCapture.tv_usec, pData->LengthIncluded, pData->LengthOriginal);
        //printf("PACKET SEARCHED FROM TABLE: t=%ld.%08d of %d bytes long (%d on the wire) \n", (long int) pPacket->TimeCapture.tv_sec, (int) pPacket->TimeCapture.tv_usec, pPacket->LengthIncluded, pPacket->LengthOriginal);
        // do the bytes match up? 
        if(pData->PayloadSize == pPacket->PayloadSize)
        {
            for(k=0; k<pData->PayloadSize; k++)
            {
                if(pData->Data[k+PayloadOffset] != pPacket->Data[k+PayloadOffset])
                {
                    //printf("%c,%c\n",pData->Data[k+PayloadOffset],pPacket->Data[k+PayloadOffset]);
                    // Nope - they are not the same
                    break;
                }
            }
            if(k>=pData->PayloadSize)//Match
            {
                //printf("Match\n");
                gPacketHitCount ++;
                gPacketHitBytes += pPacket -> PayloadSize;
                discardPacket(pPacket);
                return;
            }
        }
     }
}





