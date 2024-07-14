#include <stdio.h>
//all the libraries needed
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "fasta.h"
#include "LLvNode.h"


//function to be passed in to llFree to delete allocated keys 
void deleteKey(LLvNode *node, void *userdata){
	if (node->key != NULL) {
		free(node->key);
	}

	//free(node->value);

	(void) userdata;
}

//storing a record one at a time
struct LLvNode * storeRecord(struct LLvNode **head, struct LLvNode *tail, struct FASTArecord record){

	//creating and initializing a new node for list
	LLvNode *newNode = llNewNode(strdup(record.description), NULL);

	if(*head == NULL){
		*head = llAppend(*head, newNode);
		tail = newNode;
	} else{
		//appending the new node into head
		tail = llAppend(tail, newNode);
		tail = tail->next;
	
	}

	return tail;
}

//processing records and storing them in a linked list
int processRecord(char *filename, double *timeTaken){

	//opening file
	FILE *file = fopen(filename, "r");
	//if theres an error
	if(file == NULL){
		fprintf(stderr, "Failure opening %s : %s\n", filename, strerror(errno));
		return -1;
	}

	//needed variables to continue
	struct FASTArecord record;
	int lineNum = 0,
		recordNum = 0,
		eofS = 0,
		status;
	clock_t startT, endT;

	//head of linked list to store data
	struct LLvNode * head = NULL;
	struct LLvNode * tail = NULL;

	//recording the start of the clock
	startT = clock();

	//do while loop to go through the file till the end
	do{
		//to keep track of the file working
		if(recordNum % 10000 == 0){
			printf(".");
			fflush(stdout);
		}

		//initialize a record
		fastaInitializeRecord(&record);

		//reading a record in and setting status
		status = fastaReadRecord(file, &record);

		//depending on status
		if(status == 0){
			//end of loop
			eofS = 1;
		} else if (status > 0){
			//if status is reading
			recordNum++;
			lineNum += status;

			//storing the record in node
			tail = storeRecord(&head, tail, record);

			//clearing record
			fastaClearRecord(&record);

		} else {
			//complete error
			fprintf(stderr, "status = %d\n", status);
			fprintf(stderr, "Error: failure at line %d of '%s'\n",
					lineNum, filename);
			return -1;
		}

	} while (!eofS);

	printf(" %d FASTA records\n", recordNum);

	//ending clock
	endT = clock();
	//calculating total time
	(*timeTaken) = ((double) (endT - startT)) / CLOCKS_PER_SEC;

	//freeing the allocated linked list
	llFree(head, deleteKey, NULL);
	//closing file
	fclose(file);

	return recordNum;
}


//for main processing
int keepProcessing (char *filename, long repeatsReq){

	//variables needed
	double timePerIteration, totalTime = 0;
	int minutes, status;

	//loop for repearing process per request of user
	for(long i = 0; i < repeatsReq; i++){
		//processing records
		status = processRecord(filename, &timePerIteration);
		if(status < 0){
			//if error or failure
			return -1;
		}
		//adding up total time
		totalTime += timePerIteration;
	}

	printf("%.3f seconds taken for processing total\n", totalTime);

	//getting average and minutes
	totalTime /= (double) repeatsReq;
	minutes = (int) (totalTime/60);
	totalTime = totalTime - (minutes * 60);
	printf("On average: %d minutes, %.3f second per run\n",
            minutes, totalTime);

	return status;
}



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
