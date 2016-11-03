////////////////////////////////////////////////////////////////////////////////
//
//  File           : assign2.c
//  Description    : Contains driver and analysis functions to test malloc.c,
//                   and a main() function which demonstrates them.
//
//   Author        : Paul Hudgins
//   Last Modified : 10/28/16
//

// Include Files
#include <stdio.h>
#include <assert.h>
#include <stdlib.h> //For random, beware of conflicts
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

// Project Includes
#include "malloc.h"

//Global variables

void *original_sbrk;

// Functions

// Calculates the alignment gap of a newly allocated block,
// and stores it in the first byte of that block.
void store_alignment_gap(void *ptr, size_t size) {
	size_t size_allocated = (get_block_ptr(ptr)->words - 1) * WORD_SIZE;
	char * storage_byte = (char *) ptr;
	*storage_byte = (char) (size_allocated - size);
}

void rand_test_malloc(int num_ptrs, void **ptrs, int max_size, int num_calls) {
	int i;
	printf("\nExec %d random calls to free and malloc(1 <= n <= %d) on %d pointers:\n",
		num_calls, max_size, num_ptrs);
	for (i = 0; i < num_calls; i++) {
		int index= rand()%num_ptrs;
		free(ptrs[index]);
		ptrs[index] = NULL;

		int size = (rand()%max_size) + 1;
		ptrs[index] = malloc(size);
		store_alignment_gap(ptrs[index], size);
	}
}

void rand_test_calloc(int num_ptrs, void **ptrs, int max_elements, int element_size, int num_calls) {
	int i;
	printf("\nExec %d random calls to calloc(1 <= n <= %d, elsize %d) on %d ptrs:\n",
		num_calls, max_elements, element_size, num_ptrs);
	for (i = 0; i < num_calls; i++) {
		int index= rand()%num_ptrs;
		free(ptrs[index]);
		ptrs[index] = NULL;

		int elements = (rand()%max_elements) + 1;
		ptrs[index] = calloc(elements, element_size);
		store_alignment_gap(ptrs[index], elements * element_size);
	}
}

void rand_test_realloc(int num_ptrs, void **ptrs, int max_size, int num_calls) {
	int i;
	printf("\nExec %d random calls to realloc(1 <= n <= %d) on %d ptrs:\n",
		num_calls, max_size, num_ptrs);
	for (i = 0; i < num_calls; i++) {
		int index= rand()%num_ptrs;
		int size = (rand()%max_size) + 1;
		ptrs[index] = realloc(ptrs[index], size);
		store_alignment_gap(ptrs[index], size);
	}
}

void rand_test_free(int num_ptrs, void **ptrs, int num_calls) {
	int i;
	printf("\nFreeing %d random pointers out of %d:\n",
	 num_calls, num_ptrs);
	for (i = 0; i < num_calls; i++) {
		int index= rand()%num_ptrs;

		free(ptrs[index]);
		ptrs[index] = NULL;
	}
}

void free_all(int num_ptrs, void **ptrs) {
	printf("\nFreeing all %d pointers.\n", num_ptrs);
	int i;
	for (i = 0; i < num_ptrs; i++) {
		free(ptrs[i]);
		ptrs[i] = NULL;
	}
}


void analyze( int printall) {
    block_meta *block = get_global_base();
    size_t total_free = 0;
    size_t total_gap = 0;
    size_t total_used = 0;
    int total_blocks = 0;
    while (block) {
        total_blocks++;
        if (printall) printf("  Blck: %3d, Loc: %p, ", total_blocks, block);
        
        size_t size = (block->words -1) * WORD_SIZE;
        char gap = 0;
        if (!block->notlast) {
            if (printall) printf("Empty terminating block\n");
            size = 0;
        } else if (block->free) {
            total_free += size;
            if (printall) printf("Siz: %4ld, Free\n", size);
        } else {
            gap =*(char *)(block+1);  //stored by driver function
            size -= gap;
            total_used += size;
            if (printall) printf("Siz: %4ld, Alloc, Align Gap: %d\n", size, gap);
        }
        assert(gap >= 0);
        assert(gap < 8);
        assert(size >= 0);
        total_gap += gap;
        
        if (block->notlast) {
            block += block->words;
        } else {
        	break;
        }
    }
    size_t size_of_linkedlist = total_blocks * WORD_SIZE;
    size_t waste = size_of_linkedlist + total_gap + total_free;
    size_t heap_size = waste + total_used;
    printf("    Num Blocks: %d, Allocated Space: %ld, sbrk(0)-original: %ld\n",
        total_blocks, total_used, (sbrk(0) - original_sbrk));
    printf("    (LEAKS) Total Metadata: %ld, Free Space: %ld, Align Gaps: %ld,\n",
        size_of_linkedlist, total_free, total_gap);
    printf("    (EFFICIENCY) Expected Heap Size %ld, Waste: %ld, Effic: %ld%%\n",
        waste + total_used, waste, 
        (heap_size) ? (total_used * 100 / (heap_size)) : 100);
}

void analyze_with_prompt() {
    analyze(0);
    char response[8];
    printf("Print structure of linked list? [y/n/q]: ");
    fgets(response, 8, stdin);
    if (response[0] == 'y') {
        analyze(1);
    } else if (response[0] == 'q') {
        exit(EXIT_SUCCESS);
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : main
// Description  : The main function for the CMSC257 assignment #2
//
// Inputs       : argc - the number of command line parameters
//                argv - the parameters
// Outputs      : 0 if successful test, -1 if failure

int main(int argc, char *argv[]) {   
    original_sbrk = sbrk(0);
    printf("Original program break: %p\n\n", original_sbrk);

    int num_ptrs = 500;

    printf("Allocating an array to store pointers for subsequent tests:\n");
    void **ptrs = (void**) calloc(num_ptrs+1, sizeof(void*));
    ptrs++;
    analyze(1);

    rand_test_malloc(20, ptrs, 1000, 500);
    analyze_with_prompt();
    rand_test_malloc(num_ptrs, ptrs, 1000, 10000);
    analyze_with_prompt();
    rand_test_free(num_ptrs, ptrs, 450);
    analyze_with_prompt();
    rand_test_malloc(num_ptrs, ptrs, 1000, 10000);
    analyze_with_prompt();

    free_all(num_ptrs, ptrs);
    analyze_with_prompt();

    rand_test_calloc(50, ptrs, 20, 8, 10000);
    analyze_with_prompt();
    rand_test_calloc(num_ptrs, ptrs, 24, 1, 10000);
    analyze_with_prompt();

    free_all(num_ptrs, ptrs);
    analyze_with_prompt();

    rand_test_realloc(50, ptrs, 1000, 1000);
    analyze_with_prompt();
    rand_test_realloc(num_ptrs, ptrs, 1000, 10000);
    analyze_with_prompt();
	
    free_all(num_ptrs, ptrs);
    analyze_with_prompt();

    printf("\nFreeing pointer array.\n");
    free(ptrs-1);
    analyze(1);

    // Return successfully
    return(0);
}
