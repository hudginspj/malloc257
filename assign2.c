////////////////////////////////////////////////////////////////////////////////
//
//  File           : assign1.c
//  Description    : This is the main source code for for the first assignment
//                   of CMSC257.  
//
//   Author        : Paul Hudgins
//   Last Modified : 10/5/16
//

// Include Files
#include <stdio.h>
#include <assert.h>
#include <stdlib.h> //For ranodom, beware of conflicts
#include <sys/types.h>
#include <unistd.h>

// Project Includes
#include "malloc.h"

//Definitions


// Functions

void store_gap(void *ptr, size_t size) {
	size_t size_allocated = (get_block_ptr(ptr)->words - 1) * WORD_SIZE;
	char * storage_byte = (char *) ptr;
	*storage_byte = (char) (size_allocated - size);
}

void rand_test_malloc(int num_ptrs, void **ptrs, int max_size, int num_calls) {
	int i;
	printf("\nSimulating %d calls to malloc(1 <= n <= %d) using %d pointers:\n",
		num_calls, max_size, num_ptrs);
	for (i = 0; i < num_calls; i++) {
		int index= rand()%num_ptrs;
		free(ptrs[index]);
		ptrs[index] = NULL;

		int size = (rand()%max_size) + 1;
		ptrs[index] = malloc(size);
		store_gap(ptrs[index], size);
	}
}

void rand_test_calloc(int num_ptrs, void **ptrs, int max_elements, int element_size, int num_calls) {
	int i;
	printf("\nSimulating %d calls to calloc(1 <= n <= %d, elsize %d) using %d pointers:\n",
		num_calls, max_elements, element_size, num_ptrs);
	for (i = 0; i < num_calls; i++) {
		int index= rand()%num_ptrs;
		free(ptrs[index]);
		ptrs[index] = NULL;

		int elements = (rand()%max_elements) + 1;
		ptrs[index] = calloc(elements, element_size);
	}
}

void rand_test_realloc(int num_ptrs, void **ptrs, int max_size, int num_calls) {
	int i;
	printf("\nSimulating %d calls to realloc(1 <= n <= %d) using %d pointers:\n",
		num_calls, max_size, num_ptrs);
	for (i = 0; i < num_calls; i++) {
		int index= rand()%num_ptrs;
		//printf("index %d / old_ptr %p", index, ptrs[index]-24);
		int size = (rand()%max_size) + 1;
		ptrs[index] = realloc(ptrs[index], size);
		assert(ptrs[index] != NULL);
		//printf("/ new_ptr %p / size %#x\n", ptrs[index]-24, size);
		//analyze(1);
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
		//printf("index %d / old_ptr %p", index, ptrs[index]-24);

		//printf("/ new_ptr %p / size %#x\n", ptrs[index]-24, size);
		//analyze(1);
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



////////////////////////////////////////////////////////////////////////////////
//
// Function     : main
// Description  : The main function for the CMSC257 assignment #1
//
// Inputs       : argc - the number of command line parameters
//                argv - the parameters
// Outputs      : 0 if successful test, -1 if failure

int main(int argc, char *argv[]) {   
    void * original_sbrk = sbrk(0);
    printf("Original program break: %p\n", original_sbrk);


    /*void *p1 = malloc(103);
    analyze(1);
    void *p2 = malloc(203);
    analyze(1);
    free(p1);
    analyze(1);
    p1 = malloc(15);
    analyze(1);
    void *p3 = malloc(27);
    analyze(1);
    free(p3);
    analyze(1);
    free(p2);
    analyze(1);*/


    
    int num_ptrs = 500;
    void **ptrs = (void**) calloc(num_ptrs, sizeof(void*));

    
    rand_test_calloc(num_ptrs/8, ptrs, 20, 8, 10000);
    analyze(0);
    rand_test_calloc(num_ptrs, ptrs, 24, 1, 10000);
    analyze(0);
    rand_test_calloc(num_ptrs, ptrs, 125, 8, 10000);
    analyze(0);
    
    rand_test_malloc(num_ptrs, ptrs, 24, 10000);
    analyze(1);
    rand_test_free(num_ptrs, ptrs, 100);
    analyze(0);
    rand_test_malloc(num_ptrs, ptrs, 1000, 10000);
    analyze(0);
    rand_test_free(num_ptrs, ptrs, 100);
    analyze(0);
    printf("      sbrk(0)-original: %ld\n", (sbrk(0) - original_sbrk));
    rand_test_malloc(num_ptrs, ptrs, 1000, 10000);
    analyze(0);
    rand_test_free(num_ptrs, ptrs, 450);
    analyze(0);
    printf("      sbrk(0)-original: %ld\n", (sbrk(0) - original_sbrk));
    free_all(num_ptrs, ptrs);
    analyze(0);
	
	
    

    
    rand_test_realloc(num_ptrs, ptrs, 1000, 10000);
    analyze(0);
    rand_test_realloc(num_ptrs, ptrs, 1000, 10000);
    analyze(0);
    rand_test_realloc(num_ptrs, ptrs, 1000, 10000);
    analyze(0);
    printf("      sbrk(0)-original: %ld\n", (sbrk(0) - original_sbrk)); 
	
    free_all(num_ptrs, ptrs);
    analyze(0);
    printf("      sbrk(0)-original: %ld\n", (sbrk(0) - original_sbrk));

    printf("\nFreeing pointer array.\n");
    free(ptrs);
    analyze(1);
    printf("      sbrk(0)-original: %ld\n", (sbrk(0) - original_sbrk));  

    




    // Return successfully
    return(0);
}
//  cd /cygdrive/c/users/pjhud/bash/a2

