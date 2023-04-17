/* main.c : Main file for redextract */

#include <stdio.h>
#include <stdlib.h>

/* for strdup due to C99 */
char * strdup(const char *s);

#include <string.h>
#include <search.h>
#include <pthread.h>

#include "pcap-read.h"
#include "pcap-read.c"
#include "pcap-process.h"

// For threading
#define NUM_PRODUCERS 1
#define NUM_CONSUMERS 1


// moved noah's contributions from pcap-read to main
char readPcapFile (struct FilePcapInfo * pFileInfo){
	int j;

	//Noah
	pthread_t *     pThreadProducers;
    pthread_t *     pThreadConsumers;

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


int main (int argc, char *argv[])
{
	 
    if(argc < 2 || argc > 4 || argc == 3) 
    {
        printf("Usage: redextract FileX\n");
        printf("       redextract FileX\n");
        printf("  FileList        List of pcap files to process\n");
        printf("    or\n");
        printf("  FileName        Single file to process (if ending with .pcap)\n");
        printf("\n");
        printf("Optional Arguments:\n");
        /* You should handle this argument but make this a lower priority when 
           writing this code to handle this 
         */
        printf("  -threads N       Number of threads to use (2 to 8)\n");
        /* Note that you do not need to handle this argument in your code */
        printf("  -window  W       Window of bytes for partial matching (64 to 512)\n");
        printf("       If not specified, the optimal setting will be used\n");
        return -1;
    }
	 else if ( argc == 4 ){ 
	 	  if (strcmp( "-threads", argv[2]) != 0)
		  {
		      printf(" -threads is the only valid flag\n");
				return -1;
		  }
	 	  int num_threads = atoi(argv[3]);
		  if (num_threads < 2 || num_threads > 8){
            printf("  -threads N       Number of threads to use (2 to 8)\n");
				return -1;
		  }
	 }
		

    printf("MAIN: Initializing the table for redundancy extraction\n");
    initializeProcessing(DEFAULT_TABLE_SIZE);
    printf("MAIN: Initializing the table for redundancy extraction ... done\n");

    /* Note that the code as provided below does only the following 
     *
     * - Reads in a single file twice
     * - Reads in the file one packet at a time
     * - Process the packets for redundancy extraction one packet at a time
     * - Displays the end results
     */

    struct FilePcapInfo     theInfo;

    theInfo.FileName = strdup(argv[1]);
    theInfo.EndianFlip = 0;
    theInfo.BytesRead = 0;
    theInfo.Packets = 0;
    theInfo.MaxPackets = 5;

    //Hannah
    if (theInfo.FileName[strlen(theInfo.FileName)-1] != 'p'){
        FILE *fp = fopen(theInfo.FileName, "r");
        char buf[256];
        while(1){
            fgets(buf, 256, fp);
            if (feof(fp)){
                break;
            }
            buf[strcspn(buf, "\n")] = 0;
            theInfo.FileName = strdup(buf);
            printf("MAIN: Attempting to read in the file named %s\n", theInfo.FileName);
            readPcapFile(&theInfo);
				free(theInfo.FileName);

           // printf("MAIN: Attempting to read in the file named %s again\n", theInfo.FileName);
            //readPcapFile(&theInfo);
				
				/*
            printf("Summarizing the processed entries\n");
            tallyProcessing();
				*/
        }
    }
    else{
        printf("MAIN: Attempting to read in the file named %s\n", theInfo.FileName);
        readPcapFile(&theInfo);
		  free(theInfo.FileName);

       // printf("MAIN: Attempting to read in the file named %s again\n", theInfo.FileName);
        //readPcapFile(&theInfo);
			
		  /*
        printf("Summarizing the processed entries\n");
        tallyProcessing();
		  */
    }

	 hdestroy();

    /* Output the statistics */

    printf("Parsing of file %s complete\n", argv[1]);

    printf("  Total Packets Parsed:    %d\n", gPacketSeenCount);
    printf("  Total Bytes   Parsed:    %lu\n", (unsigned long) gPacketSeenBytes);
    printf("  Total Packets Duplicate: %d\n", gPacketHitCount);
    printf("  Total Bytes   Duplicate: %lu\n", (unsigned long) gPacketHitBytes);

    float fPct;

    fPct = (float) gPacketHitBytes / (float) gPacketSeenBytes * 100.0;

    printf("  Total Duplicate Percent: %6.2f%%\n", fPct);


    return 0;
}
