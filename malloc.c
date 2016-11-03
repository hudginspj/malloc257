////////////////////////////////////////////////////////////////////////////////
//
//  File           : malloc.c
//  Description    : Implements malloc, calloc, realloc, and free
//
//   Author        : Paul Hudgins
//   Last Modified : 10/28/16
//

// Include Files
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include "malloc.h"


//Global variables

void *global_base = NULL;

//Helper functions

block_meta *get_global_base() {
  return global_base;
}

block_meta *get_block_ptr(void *ptr) {
  return (block_meta*)ptr - 1;
}

wordcount size_to_words(size_t size) {
  return ((size-1)/WORD_SIZE)+1;
}

//Functions

////////////////////////////////////////////////////////////////////////////////
// Description  : Finds the previous block_meta in singly-linked list
//
// Inputs       : current - a pointer to a block_meta
// Outputs      : the previous block_meta, or null if cannot be found
block_meta *find_previous(block_meta *current) {
  if (current == global_base) {
    return NULL;
  }
  block_meta *search = global_base;
  block_meta *next_search = search + search->words;
  if (!next_search->notlast) return NULL;
  while (next_search != current){
    assert(next_search->notlast);
    search = next_search;
    next_search += next_search->words;
  }
  return search;
}

////////////////////////////////////////////////////////////////////////////////
// Description  : Merges a block with the subsequent block, 
//                if the subsequent block is free
//
// Inputs       : block - a pointer to a block_meta
void merge_with_next(block_meta *block) {
    if (block
        && block->notlast ) {
      block_meta *next = block + block->words;
      if (next->free && next->notlast) {
        block->words += next->words;
        block->notlast = next->notlast;
      }
    } 
}

////////////////////////////////////////////////////////////////////////////////
// Description  : Reduces a data block to the required number of words,
//                if possible, and creates a new free block with excess words.
//                The new block will be merged with subsequent free blocks.
//
// Inputs       : old_block - the block to be split
//              : words - the number of words needed for the block,
//                        including metadata
void split(block_meta *old_block, wordcount words) {
    if (old_block->words <= words || !old_block->notlast) {
        return;
    }

    block_meta *new_block;
    new_block = old_block + words;
    new_block->words = old_block->words - words;
    new_block->free = 1;
    new_block->notlast = 1;

    old_block->words = words;
    merge_with_next(new_block);
}

////////////////////////////////////////////////////////////////////////////////
// Description  : Performs best-fit search for a free block
//
// Inputs       : words - the number of words needed for the block,
//                        including metadata
// Outputs      : the smallest block of sufficient size,
//                or the empty terminating block if no block was found
block_meta *find_free_block(wordcount words) {
  block_meta *current = global_base;
  block_meta *best;
  while (current->notlast 
      && !(current->free 
      && current->words >= words)) { 
    current += current->words;
  }

  best = current;
  wordcount best_size = best->words;
  while (current->notlast) {
    if (current->free 
      && current->words >= words 
      && (current->words < best_size)) {
        best = current;
        best_size = current->words;
        if (best_size == words) break;
    }
    current += current->words;
  }

  split(best, words);
  return best;
}

////////////////////////////////////////////////////////////////////////////////
// Description  : Allocates space for a new block at the top of the heap.
//                If global_base is null, also creates an empty terminating block.
//
// Inputs       :  last - the empty terminating block, or null if not intialized
//                 words - the number of words needed for the block,
//                        including metadata
// Outputs      :  the newly allocated block
block_meta *request_space(block_meta* last, wordcount words) {
  if (!last) { // NULL on first request, create an (aligned) terminating block.
    last = (block_meta*) (8 * size_to_words((size_t)sbrk(0)) );
    if (brk(last+1) == -1) { //TODO: not thread safe
      return NULL; // brk failed.
    }
    last->words = 0;
    last->notlast = 0;
    last->free = 1;
  }
  assert(!last->notlast);
  
  assert(sbrk(0) == last+1);
  void *request = sbrk(words * WORD_SIZE);
  if(request == (void *) -1) {
    return NULL;
  }

  block_meta *new_last = last + words;
  new_last->words = 0;
  new_last->notlast = 0;
  new_last->free = 1; 

  last->words = words;
  last->notlast = 1;
  last->free = 0;
  return last;
}


////////////////////////////////////////////////////////////////////////////////
// Description  : Dynamically allocates heap memory
//
// Inputs       :  size - the number of bytes to be allocated
// Outputs      :  a pointer to the beginning of allocated memory
void *malloc(size_t size) {
  block_meta *block;

  if (size <= 0) {
    return NULL;
  }

  wordcount words_needed = size_to_words(size) +1;
  if (!global_base) { // First call.
    block = request_space(NULL, words_needed);
    if (!block) {
      return NULL;
    }
    global_base = block;
  } else {
    block = find_free_block(words_needed);
    if (!block->notlast) { // Failed to find free block.
      block = request_space(block, words_needed);
      if (!block) {
        return NULL;
      }
    } else {      // Found free block
      block->free = 0;
    }
  }
  
  return(block+1);
}

////////////////////////////////////////////////////////////////////////////////
// Description  : Frees a block of heap memory for re-use
//
// Inputs       :  ptr - a pointer to a dynamically allocated block of memory
void free(void *ptr) {
  if (!ptr) {
    return;
  }

  block_meta* block_ptr = get_block_ptr(ptr);

  assert(block_ptr->free == 0);
  block_ptr->free = 1;
  merge_with_next(block_ptr);

  //merge with previous
  block_meta *previous =  find_previous(block_ptr);
  if (previous && previous->free) {
    merge_with_next(previous);
    block_ptr = previous;
  }

  //lower program break if necessary
  block_meta *next = block_ptr + block_ptr->words;
  if (!next->notlast) {
    if (block_ptr != global_base) {
      brk(block_ptr + 1);  //TODO make this safer
      block_ptr->words = 0;
      block_ptr->notlast = 0;
      block_ptr->free = 1; 
    } else {
      brk(global_base);
      global_base = NULL;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Description  : Dynamically allocates heap memory, and initalizes it to all zeros
//
// Inputs       :  size - the number of elements to be accomodated
//                 elsize - the size of each element
// Outputs      :  a pointer to the beginning of allocated memory
void *calloc(size_t nelem, size_t elsize) {
  size_t size = nelem * elsize;
  void *ptr = malloc(size);
  memset(ptr, 0, size);
  return ptr;
}

////////////////////////////////////////////////////////////////////////////////
// Description  : Dynamically resizes a previously allocated block of memory
//
// Inputs       :  ptr - a pointer to the original block of memory
//              :  size - the number of bytes to be allocated
// Outputs      :  a pointer to the beginning of re-allocated memory
void *realloc(void *ptr, size_t size) {
  if (!ptr) { 
    // NULL ptr. realloc should act like malloc.
    return malloc(size);
  }
  
  block_meta* block_ptr = get_block_ptr(ptr);
  merge_with_next(block_ptr);

  //  Check if we can realloc in place
  wordcount words_needed = size_to_words(size) +1;
  if (block_ptr->words >= words_needed) {
    split(block_ptr, words_needed);
    return ptr;
  }

  //  Malloc new space and free old space.
  void *new_ptr;
  new_ptr = malloc(size);
  if (!new_ptr) {
    return NULL; 
  }
  memcpy(new_ptr, ptr, block_ptr->words * WORD_SIZE);
  free(ptr);  
  return new_ptr;
}
