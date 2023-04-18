/* pcap-read.c : Parse pcap files to extract packets
 *
 * Adapted from the ScaleBox project of the NetScale lab
 * Code supported in part by the National Science Foundation via
 * CNS-17XXX.
 * 
 * C Port: Adapted in March 2023 for CSE 30341
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>

#include "packet.h"
#include "pcap-read.h"
#include "pcap-process.h"

#define SHOW_DEBUG	0

//Noah
#define STACK_MAX_SIZE 10

pthread_mutex_t StackLock;//Similar idea to milestone 4
pthread_cond_t PushWait = PTHREAD_COND_INITIALIZER;
pthread_cond_t PopWait = PTHREAD_COND_INITIALIZER;

struct Packet * StackItems[STACK_MAX_SIZE];//Queue for producer-consumer

char KeepGoing = 1;//Boolean to determine when producers stop - warns consumers to exit
int  StackSize = 0;

// For threading
int NUM_PRODUCERS =  1;
int NUM_CONSUMERS = 3;

//Noah
char readPcapFile (struct FilePcapInfo * pFileInfo){
	int j;

	//Noah
	pthread_t *     pThreadProducers;
    pthread_t *     pThreadConsumers;

	if(pFileInfo->numThreads != -1)//Threads argument specified else uses optimal #
	{
		NUM_CONSUMERS = pFileInfo->numThreads-1;
	}

    // Allocate space for tracking the threads 
    pThreadProducers = (pthread_t *) malloc(sizeof(pthread_t *) * NUM_PRODUCERS); 
    pThreadConsumers = (pthread_t *) malloc(sizeof(pthread_t *) * NUM_CONSUMERS);
	KeepGoing = 1;//Reset for second iteration of reading file
	
	for(j = 0; j<NUM_PRODUCERS; j++)
	{
		pthread_create(pThreadProducers+j,0,thread_producer,pFileInfo);
	}

	for(j=0; j<NUM_CONSUMERS;j++){
		pthread_create(pThreadConsumers+j,0,thread_consumer,NULL);
	}

	for(j=0; j<NUM_PRODUCERS; j++)
    {
        pthread_join(pThreadProducers[j], NULL);
    }

	for(j=0; j<NUM_CONSUMERS; j++)
    {
        pthread_join(pThreadConsumers[j], NULL);
    }
	 return 1;
}

void * thread_producer(void * pData){
	//Noah - Makes sense for 1 producer but not sure how to make this work with multiple producers due to reading from a file 
	//can't use same file pointer in different threads
	//Maybe optimal # of threads depends on how fast consumers can process the data compared to the speed of the single producer?
	struct FilePcapInfo * pFileInfo = (struct FilePcapInfo *) pData;
	FILE * pTheFile;//File Pointer
	struct Packet * pPacket;

	/* Default is to not flip due to endian-ness issues */
	pFileInfo->EndianFlip = 0;

	/* Reset the counters */
	pFileInfo->Packets = 0;
	pFileInfo->BytesRead = 0;

	//printf("Max Packets %d\n", pFileInfo->MaxPackets);

	/* Open the file and its respective front matter */
	pTheFile = fopen(pFileInfo->FileName, "r");

	/* Read the front matter */
	if(!parsePcapFileStart(pTheFile, pFileInfo))
	{
		printf("* Error: Failed to parse front matter on pcap file %s\n", pFileInfo->FileName);
		return 0;
	}
	while(!feof(pTheFile))//File ending is the end of producing
	{	
    	pthread_mutex_lock(&StackLock);
		while (StackSize >= STACK_MAX_SIZE){ // Wait until there is room to push
	 		pthread_cond_wait(&PushWait, &StackLock);
	 	}
		// Now there is space to push
		pPacket = readNextPacket(pTheFile, pFileInfo);//Do work to get packet
		if(pPacket != NULL){
     		StackItems[StackSize] = pPacket;
     		StackSize++;
			pthread_cond_signal(&PopWait); // if something was trying to pop, signal - at least 1 item on stack     
		}
		if(pFileInfo->MaxPackets != 0)
		{
			if(pFileInfo->Packets >= pFileInfo->MaxPackets)
			{
				KeepGoing = 0;
				pthread_cond_broadcast(&PopWait);//Noah: Call all consumers to check condition
				pthread_mutex_unlock(&StackLock);
				return NULL;
			}
		}
	 	pthread_mutex_unlock(&StackLock);
	}
	pthread_mutex_lock(&StackLock);
	KeepGoing = 0;
	pthread_cond_broadcast(&PopWait);//Noah: Call all consumers to check condition
	pthread_mutex_unlock(&StackLock);
	fclose(pTheFile);
	printf("File processing complete - %s file read containing %d packets with %d bytes of packet data\n", pFileInfo->FileName, pFileInfo->Packets, pFileInfo->BytesRead);
	return NULL;
}

void * thread_consumer(void * pData){
	struct Packet * pPacket;
	while(KeepGoing || StackSize>0){
    	pthread_mutex_lock(&StackLock);
	 	while (StackSize <= 0 && KeepGoing){ //Nothing to pop and there is still data to process
        	pthread_cond_wait(&PopWait, &StackLock);
	 	}
    	if(StackSize>0){//Exit condition and there is something to consume
			//printf("Consumer\n");
        	pPacket = StackItems[StackSize-1]; // Remove
        	StackSize--;
			processPacket(pPacket);//Do the work: Processes packets in pcap-process.c (Producer Null checks already)   
	    	pthread_cond_signal(&PushWait); // signal push because there is room
        	pthread_mutex_unlock(&StackLock);
    	}
    	else//Consumer is just exiting since nothing left
    	{
        	pthread_mutex_unlock(&StackLock);
    	}
	}
	return NULL;
}

char parsePcapFileStart (FILE * pTheFile, struct FilePcapInfo * pFileInfo)
{
	// tcpdump header processing
	//
	//  Reference of file info available at:
	//   http://lists.linux-wlan.com/pipermail/linux-wlan-devel/2003-September/002701.html
	//
	//  Also see:
	//	 http://wiki.wireshark.org/Development/LibpcapFileFormat

	/* Check if FILE pointer is valid */
	if(pTheFile == NULL) 
	{ 
	    printf("* Error (parsePcapFileStart): FILE pointer was NULL\n"); 
		 return 0; 
	}
	
	/* Check if pFileInfo is valid */
	if(pFileInfo == NULL) 
	{ 
		printf("* Error (parsePcapFileStart): F was NULL\n"); 
		return 0; 
	} 

	int					nMagicNum;
	unsigned short		nMajor;
	unsigned short		nMinor;
	unsigned int		nSnapshotLen;
	unsigned int		nMediumType;
	
	// 32 bit magic number
	// 16 bit Major
	// 16 bit Minor
	// Timezone offset (ignore) - 32 bit
	// Timezone accuracy (ignore) - 32 bit
	// Snapshot length - 32 bit
	// Link layer type - 32 bit
	
	fread((char *) &nMagicNum, 4, 1, pTheFile);
	fread((char *) &nMajor, sizeof(unsigned short), 1, pTheFile);
	fread((char *) &nMinor, sizeof(unsigned short), 1, pTheFile);

	/* Determine the endian-ness of this particular machine
	 *   A union allows us to represent a block of machine in different 
     *   ways.  We will also use this in Project 6. 
	 */
	// union {
  	// 	uint32_t a;
  	// 	uint8_t  b[4];
  	// } u;

	/* If we set the 32 bit value to 1
	 *   On a big endian machine, it would be 0x00 00 00 01
	 *   On a little endian machine, it is 0x01 00 00 00 
	 * 
	 */
	//u.a = 1;
  	//char bigEndian = u.b[3];

	if(nMagicNum == 0xa1b2c3d4) {
		pFileInfo->EndianFlip = 0;
	} else if (nMagicNum == 0xd4c3b2a1) {
		pFileInfo->EndianFlip = 1;
		nMajor = endianfixs(nMajor);
		nMinor = endianfixs(nMinor);
	} else {
		printf("Warning: Non-standard magic number at beginning of TCP dump file\n"); 
		printf("   Ignoring processing this file\n");
		return 0;
	}
					
	// Ignore time zone and TZ accuracy
	fseek(pTheFile, 4, SEEK_CUR);
	fseek(pTheFile, 4, SEEK_CUR);
	
	fread((char *) &nSnapshotLen,4,1,pTheFile);
	fread((char *) &nMediumType,4,1,pTheFile);
	
	if(pFileInfo->EndianFlip) 
	{
		/* Normally we can just use ntohol (network to host long) to fix things but since this
		   is a file and not a live protocol, we will need to be intentional about where we 
		   apply endian fixes 
		*/

		nSnapshotLen = endianfixl(nSnapshotLen);
		nMediumType = endianfixl(nMediumType);
	}

	if(SHOW_DEBUG) 
	{
		printf("tcpdump file initial information\n");
		printf("   Major: %d   Minor: %d   Endian Flip: ", nMajor, nMinor);

		if(pFileInfo->EndianFlip)
		{
			printf("Yes\n");
		}
		else
		{
			printf("No\n");
		}

		printf("   Snapshot Len: %d   Medium Type: %d\n", nSnapshotLen, nMediumType);		
	}

	return 1;
}

struct Packet * readNextPacket (FILE * pTheFile, struct FilePcapInfo * pFileInfo)
{
	struct Packet * 	pPacket;

	pPacket = allocatePacket(DEFAULT_READ_BUFFER);

	/* Read the packet from the file
		time_t struct		Seconds, microseconds (each 32 bits)
		Capture Length		32 bits
		Actual Length		32 bits  
	*/

	fread((char *) &(pPacket->TimeCapture.tv_sec), 1, sizeof(uint32_t), pTheFile);
	fread((char *) &(pPacket->TimeCapture.tv_usec), 1, sizeof(uint32_t), pTheFile);
	fread((char *) &(pPacket->LengthIncluded), 1, sizeof(uint32_t), pTheFile);
	fread((char *) &(pPacket->LengthOriginal), 1, sizeof(uint32_t), pTheFile);

	/* Is there an issue with endianness? 
		Do we need to fix it if the file was captured on a big versus small endian machine 
	*/
	if(pFileInfo->EndianFlip) 
	{
		pPacket->TimeCapture.tv_sec = endianfixl(pPacket->TimeCapture.tv_sec);
		pPacket->TimeCapture.tv_usec = endianfixl(pPacket->TimeCapture.tv_usec);

		pPacket->LengthIncluded = endianfixl(pPacket->LengthIncluded);
		pPacket->LengthOriginal = endianfixl(pPacket->LengthOriginal);
	}

	/* Double check that the packet can fit */
	if(pPacket->SizeDataMax < pPacket->LengthIncluded)
	{
		printf("* Warning: Unable to include packet of size %d due it exceeding %d bytes\n", pPacket->LengthIncluded, pPacket->SizeDataMax);
		discardPacket(pPacket);

		/* Skip this packet payload */
		fseek(pTheFile, pPacket->LengthIncluded, SEEK_CUR);
		return NULL;
	}

	/* Read in the actual packet data on a byte-wise basis */
	for(int j=0; j<pPacket->LengthIncluded; j++)
	{
		fread(pPacket->Data+j, sizeof(char), 1, pTheFile);
	}

	pFileInfo->Packets++;
	pFileInfo->BytesRead += pPacket->LengthIncluded;

	if(SHOW_DEBUG)
	{
		printf("Packet %d Info: t=%ld.%08d of %d bytes long (%d on the wire) \n", pFileInfo->Packets-1, (long int) pPacket->TimeCapture.tv_sec, (int) pPacket->TimeCapture.tv_usec, pPacket->LengthIncluded, pPacket->LengthOriginal);
	}

	return pPacket;
}
