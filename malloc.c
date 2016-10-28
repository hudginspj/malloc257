////////////////////////////////////////////////////////////////////////////////
//
//  File           : a1support.c
//  Description    : This is a set of general-purpose utility functions we use
//                  for the 257 assignment #1.
//
//   Author        : Paul Hudgins
//   Last Modified : 10/5/16
//

// Include Files
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include "malloc.h"



void *global_base = NULL;

block_meta *get_global_base() {
  return global_base;
}

block_meta *get_block_ptr(void *ptr) {
  return (block_meta*)ptr - 1;
}

/*size_t align(size_t ptr_value) {
    int remainder = ptr_value % WORD_SIZE;
    if (remainder) {
        ptr_value += WORD_SIZE - remainder;
    }
    return ptr_value;
} */

wordcount size_to_words(size_t size) {
  return ((size-1)/WORD_SIZE)+1;
}


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

//check that block is free before calling, do not need to check next
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


void split(block_meta *old_block, wordcount words) {
    block_meta *new_block;
    //assert(old_block->words >= words);
    //assert(old_block->notlast);

    if (old_block->words <= words) {
        return;
    }
    new_block = old_block + words;
    new_block->words = old_block->words - words;
    new_block->free = 1;
    new_block->notlast = 1;

    old_block->words = words;
    merge_with_next(new_block);
}

// Iterate through blocks until we find one that's large enough.
// TODO: split block up if it's larger than necessary
block_meta *find_free_block(wordcount size) {
  block_meta *current = global_base;
  block_meta *best;
  while (current->notlast 
      && !(current->free 
      && current->words >= size)) { 
    current += current->words;
  }
  if (!current->notlast) {
    return current;
  }

  best = current;
  int best_size = best->words;
  while (current->notlast) {
    if (current->free 
      && current->words >= size 
      && (current->words < best_size)) {
        best = current;
        best_size = current->words;
        if (best_size == size) break;
    }
    current += current->words;
  }

  if (best) split(best, size);
  return best;
}

//input: size is the total words in the block, including meta
block_meta *request_space(block_meta* last, wordcount size) {
  if (!last) { // NULL on first request, create an (aligned) terminating block.
    //last = (block_meta*) align((size_t)sbrk(0));
    last = (block_meta*) (8 * size_to_words((size_t)sbrk(0)) );
    if (brk(last+1) == -1) { //TODO: not thread safe
      return NULL; // brk failed.
    }
    last->words = 0;
    last->notlast = 0;
    last->free = 1; //TODO: debug only
  }
  assert(!last->notlast);
  
  assert(sbrk(0) == last+1);
  void *request = sbrk(size * WORD_SIZE);
  if(request == (void *) -1) {
    return NULL;
  }

  block_meta *new_last = last + size;
  new_last->words = 0;
  new_last->notlast = 0;
  new_last->free = 1; 

  last->words = size;
  last->notlast = 1;
  last->free = 0;
  return last;
}

// If it's the first ever call, i.e., global_base == NULL, request_space and set global_base.
// Otherwise, if we can find a free block, use it.
// If not, request_space.
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

void *calloc(size_t nelem, size_t elsize) {
  size_t size = nelem * elsize;
  void *ptr = malloc(size);
  memset(ptr, 0, size);
  return ptr;
}




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

void *realloc(void *ptr, size_t size) {
  if (!ptr) { 
    // NULL ptr. realloc should act like malloc.
    return malloc(size);
  }
  
  wordcount words_needed = size_to_words(size) +1;
  block_meta* block_ptr = get_block_ptr(ptr);
  merge_with_next(block_ptr); //TODO this could be the problem
  if (block_ptr->words >= words_needed) {
    split(block_ptr, words_needed);
    return ptr;
  }

  // Need to really realloc. Malloc new space and free old space.
  // Then copy old data to new space.
  void *new_ptr;
  new_ptr = malloc(size);
  if (!new_ptr) {
    return NULL; // TODO: set errno on failure.
  }
  memcpy(new_ptr, ptr, block_ptr->words * WORD_SIZE);
  free(ptr);  
  return new_ptr;
}


/*void analyze() { //size_t *total_free, size_t *total_used, size_t *total_gap, int *total_blocks) {
    struct block_meta *block = global_base;
    int total_blocks = 0;
    while (block) {
        total_blocks++;
        printf("Size:%ld\n", block->size);
        block = block ->next;
    }
    printf("Sizeof linked list: %d", total_blocks * META_SIZE);
}*/




