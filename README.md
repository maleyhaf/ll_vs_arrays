# Hapax Legomena

Program written by Maleyha Fatima for CIS2520 (Data Structures).

## About

This is a tool that tests the time efficiency between:
* double arrays
* linked lists with only a link to the head node
* linked lists with both a link to the head and tail node

## Usuage

To compile, simply run the following commands:
    make

Unfortunately, this program cannot be run without our university's Linux system since it caters to the data files "fasta records" stored there.  


### Examples

For example, the following command line will run the "head and tail" version of the linked list code five times and report on the results:

    $ ./llheadtail -R 5 /home/courses/cis2520/data/uniprot_sprot-100000.fasta
    ........... 100000 FASTA records
    ........... 100000 FASTA records
    ........... 100000 FASTA records
    ........... 100000 FASTA records
    ........... 100000 FASTA records
    1.874 seconds taken for processing total
    On average: 0 minutes, 0.375 second per run
