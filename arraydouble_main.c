#include <stdio.h>
//all the libraries needed
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "fasta.h"


//copying data of record into a saved array
void copyRecord(FASTArecord *arr, FASTArecord record){
	arr->description = malloc (sizeof(char) * (strlen(record.description)+1));
	arr->sequence = malloc(sizeof(char) * (strlen(record.sequence)+1));

	strcpy(arr->description, record.description);
	strcpy(arr->sequence, record.sequence);
	arr->id = record.id;
}

//to process FASTA records and get time
int processRecord (char *filename, double *timeTaken){

	//opening the file
	FILE *file = fopen(filename, "r");
	//if theres an error
	if (file == NULL){
		fprintf(stderr, "Failure opening %s : %s\n", filename, strerror(errno));
		return -1;
	}

	//if file was a success - needed variables
	struct FASTArecord record;
	int lineNum = 0, 
		recordNum = 0,
		eofS = 0,
		status;
	clock_t startT, endT;

	//array to store records
	struct FASTArecord **storeRecord = malloc(sizeof(struct FASTArecord*));
	size_t totalMemory = sizeof(struct FASTArecord*);
	size_t usedMemory = 0;

	//recording startT before any processing done
	startT = clock();

	//do while loop to go through the file as long as there are no errors
	do {

		//keeping track of records every 10,000 records
		//so we know its processing
		if (recordNum % 10000 == 0){
			printf(".");
			fflush(stdout);
		}

		//initializing a record
		fastaInitializeRecord(&record);

		//reading a record in and setting status
		status = fastaReadRecord(file, &record);
		//depending on the status
		if (status == 0){
			//if read fails or end of file, end loop
			eofS = 1;
		} else if (status > 0){
			//if status is reading
			recordNum++;

			//allocate more space in array if needed by doubling it
			if(totalMemory < sizeof(FASTArecord*)*recordNum){
				totalMemory =  (sizeof(FASTArecord*)*recordNum)*2;
				storeRecord = (struct FASTArecord**) realloc(storeRecord, totalMemory);
			}

			lineNum += status;
			
			//storing the record in array
			storeRecord[recordNum-1] = fastaAllocateRecord();
			copyRecord(storeRecord[recordNum-1], record);	

			//freeing record
			fastaClearRecord(&record);

		} else {
			//complete error
			fprintf(stderr, "status = %d\n", status);
			fprintf(stderr, "Error: failure at line %d of '%s'\n",
					lineNum, filename);
			return -1;
		}

	} while(!eofS);

	//getting the memory waste
	usedMemory = sizeof(FASTArecord*)*recordNum;
	double waste = ((double) usedMemory / totalMemory)*100;

	printf(" %d FASTA records -- %d allocated (%.3f%% waste)\n", recordNum, totalMemory, waste);

	//recording process endT
	endT = clock();

	//calculating total time
	(*timeTaken) = ((double) (endT - startT)) / CLOCKS_PER_SEC;

	//freeing the allocaated array
	for(int i = 0; i < recordNum; i++){
		fastaDeallocateRecord(storeRecord[i]);
	}
	free(storeRecord);

	//closing file
	fclose(file);

	return recordNum;

}

//for main procressing
int keepProcessing (char *filename, long repeatsReq){

	//variables needed
	double timePerIterationInSec,
		totalTimeInSec = 0;
	int minutes, status;
	
	//repeating per request of user
	for(long i = 0; i < repeatsReq; i++){
		//processing records and status
		status = processRecord(filename, &timePerIterationInSec);
		if(status < 0){
			//if error/fail
			return -1;
		}
		//adding up total time
		totalTimeInSec += timePerIterationInSec;
	}

	printf("%.3f seconds taken for processing total\n", totalTimeInSec);

	//getting average
	totalTimeInSec /= (double) repeatsReq;
	//converting to mins
	minutes = (int) (totalTimeInSec / 60);
	//getting rid of extra seconds
	totalTimeInSec = totalTimeInSec - (minutes * 60);
	printf("On average: %d minutes, %.3f second per run\n",
            minutes, totalTimeInSec);

	return status;
}

//help display
void usage(char *progname)
{
	fprintf(stderr, "%s [<OPTIONS>] <file> [ <file> ...]\n", progname);
	fprintf(stderr, "\n");
	fprintf(stderr, "Prints timing of loading and storing FASTA records.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Options: \n");
	fprintf(stderr, "-R <REPEATS> : Number of times to repeat load.\n");
	fprintf(stderr, "             : Time reported will be average time.\n");
	fprintf(stderr, "\n");
}

/**
 * Program mainline
 */
int
main(int argc, char **argv)
{
	int i, recordsProcessed = 0;
	long repeatsReq = 1;

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (argv[i][1] == 'R') {
				if (i >= argc) {
					fprintf(stderr,
							"Error: need argument for repeats requested\n");
					return 1;
				}
				if (sscanf(argv[++i], "%ld", &repeatsReq) != 1) {
					fprintf(stderr,
							"Error: cannot parse repeats requested from '%s'\n",
							argv[i]);
					return 1;
				}
			} else {
				fprintf(stderr,
						"Error: unknown option '%s'\n", argv[i]);
				usage(argv[0]);
			}
		} else {
			recordsProcessed = keepProcessing(argv[i],
					repeatsReq);
			if (recordsProcessed < 0) {
				fprintf(stderr, "Error: Processing '%s' failed -- exitting\n",
						argv[i]);
				return 1;
			}
			printf("%d records processed from '%s'\n",
					recordsProcessed, argv[i]);
		}
	}

	if ( recordsProcessed == 0 ) {
		fprintf(stderr,
				"No data processed -- provide the name of"
				" a file on the command line\n");
		usage(argv[0]);
		return 1;
	}

	return 0;
}
