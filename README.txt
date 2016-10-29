Basic functionalities:
1. Alignment: All pointers are aligned to the machine's word size.

2. Split: Excess space is split off into a new free block during allocation. Split is performed in-place (the heap size will not be increased). Maximum wasted space is 7 bytes.

3. Merge: The linked list will never contain two consecutive free blocks

4. Best-fit search: Implemented



Other optimizations:
Metadata size is one machine word (8-bytes on the server, 4 bytes on a 32-bit machine). The metadata stores the number of words in
a block, including itself:

#define WORD_SIZE sizeof(long)

typedef long wordcount;

typedef struct {
  //  Words in entire block,  including metadata.
  wordcount words : ((WORD_SIZE*8)-2); 
  char free : 1;
  char notlast : 1;
} block_meta;

A pointer is not necessary because of the property:
block_meta *next_block = block + block->words;



Driver main() function:
Driver function is interactive. 
A number of random calls are performed using an array of pointers. Alignment gaps are calculated at the time of allocation, and stored
in the allocated space for subsequent analysis.

























