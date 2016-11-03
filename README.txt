CMSC 257
Assignment 2
Paul Hudgins
10/28/16

Basic features:
(1) Alignment: All pointers are aligned to the machine's word size.

(2) Split: Excess space is split off into a new free block during allocation. Split is performed in-place (the heap size will not be increased). Maximum wasted space is 7 bytes.

(3) Merge: The linked list will never contain two consecutive free blocks

(4) Best-fit search: Implemented


Other features:
(1) The program break is lowered so that it is always 8 bytes from the end of the last allocated block. Freeing all pointers will return the program break to its original value.

(2) Metadata size is one machine word (8-bytes on the server, 4 bytes on a 32-bit machine). The metadata stores the number of words in a block, including itself:

#define WORD_SIZE sizeof(long)

typedef long wordcount;

typedef struct {
  //  Words in entire block, including metadata.
  wordcount words : ((WORD_SIZE*8)-2); 
  char free : 1;
  char notlast : 1;
} block_meta;

A pointer is not necessary because of the following property:
(next_block == block + block->words)

(3) Driver function is interactive. 
A number of random calls are performed using an array of pointers. 
Alignment gaps are calculated at the time of allocation, and stored in the allocated space for subsequent analysis.
All forms of memory usage are accounted for, and checked against the program break for correctness.
