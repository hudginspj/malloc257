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
// Don't include stdlb since the names will conflict?



void *global_base = NULL;

//TODO: clean
size_t align(size_t ptr_value) {
    int remainder = ptr_value % WORD_SIZE;
    if (remainder) {
        ptr_value += WORD_SIZE - remainder;
    }
    return ptr_value;
}

wordcount size_to_words(size_t size) {
  return ((size-1)>>3)+1;
}


/*struct block_meta *find_previous(struct block_meta *current) {
  if (current == global_base) {
    return NULL;
  }
  struct block_meta *search = global_base;
  while (search && (search->next != current)) {
    search = search->next;
  }
  return search;
}

void merge_with_next(struct block_meta *block) {
    if (block
      && block->next 
      && block->free 
      && block->next->free) {
        void *end = block->next->size + (void *) (block->next);  //TODO: Check: uses pointers to the metas instead of the blocks because they cancel
        block->size = end - (void *) block;
        block->next = block->next->next;
        block->magic = 44;
    } 
}


int split(struct block_meta *old_block, size_t size) {
    struct block_meta *new_block;
    assert(old_block->size >= size);

    if (old_block->size <= (size_t) align(size + META_SIZE + ALIGNMENT)) {
        old_block->size = size; //TODO Creates inificiency because cannot be used until merge
        return(-1);
    }
    new_block = (struct block_meta*) align((size_t)old_block + size + META_SIZE);
    //new_block--;
    new_block->next = old_block->next;
    new_block->size = (new_block->next)
          ?(size_t) ((void *) new_block->next - (void *) new_block) - META_SIZE
          :old_block->size - size - META_SIZE;
    new_block->free = 1;
    new_block->magic = 11;
    old_block->size = size;
    old_block->next = new_block;
    //old_block->free = 1;
    old_block->magic = 22;
    merge_with_next(new_block);
    return(0);
}*/

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

  //if (best) split(best, size);
  return best;
}

//input: size is the total words in the block, including meta
block_meta *request_space(block_meta* last, wordcount size) {
  if (!last) { // NULL on first request, create an (aligned) terminating block.
    last = (block_meta*) align((size_t)sbrk(0));
    if (brk(last+1) == -1) { //TODO: not thread safe
      return NULL; // brk failed.
    }
    last->words = 0;
    last->notlast = 0;
    last->free = 2; //TODO: debug only
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
  new_last->free = 3; 

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

// TODO: maybe do some validation here.
block_meta *get_block_ptr(void *ptr) {
  return (block_meta*)ptr - 1;
}



/////////////////////////////////////////////////////////////
//Free/Merge



void free(void *ptr) {
  if (!ptr) {
    return;
  }

  block_meta* block_ptr = get_block_ptr(ptr);

  assert(block_ptr->free == 0);
  block_ptr->free = 1;
  //merge_with_next(block_ptr);

  /*struct block_meta *previous =  find_previous(block_ptr);
  if (previous) {
    merge_with_next(previous);
    if (!(previous->next)) {
      block_ptr = previous;
      previous = find_previous(block_ptr);
    }
  }

  if (!block_ptr->notlast) {
    if (previous) {
      brk((void *)previous + previous->size + META_SIZE);  //TODO make this safer
      previous->next = NULL; 
    } else {
      assert(block_ptr = global_base);
      brk(global_base);
      global_base = NULL;
    }
  }*/
}

/*void *realloc(void *ptr, size_t size) {
  if (!ptr) { 
    // NULL ptr. realloc should act like malloc.
    return malloc(size);
  }

  struct block_meta* block_ptr = get_block_ptr(ptr);
  if (block_ptr->size >= size) {
    split(block_ptr, size);
    return ptr;
  }

  // Need to really realloc. Malloc new space and free old space.
  // Then copy old data to new space.
  void *new_ptr;
  new_ptr = malloc(size);
  if (!new_ptr) {
    return NULL; // TODO: set errno on failure.
  }
  memcpy(new_ptr, ptr, block_ptr->size);
  free(ptr);  
  return new_ptr;
}*/


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


void analyze( int printall) { //size_t *total_free, size_t *total_used, size_t *total_gap, int *total_blocks) {
    block_meta *block = global_base;
    size_t total_free = 0;
    size_t total_gap = 0;
    size_t total_used = 0;
    int total_blocks = 0;
    while (1) {  //TODO change to notlast
        total_blocks++;
        size_t size = (block->words -1) * WORD_SIZE;
        char gap = 0;
        if (block->free) {
            total_free += size;
        } else {
            total_used += size;
            if (block->notlast) gap =*(char *)(block+1); //stored by driver function
        }
        total_gap += gap;
        
        if (printall) printf("  Blck #: %d, Loc: %p, Siz: %ld, IsFree: %d, Gap: %d\n, NotLast %d\n",
          total_blocks, block, size, block->free, gap, block->notlast);
        if (!block->notlast) break;
        block += block->words;
    }
    size_t size_of_linkedlist = total_blocks * META_SIZE;
    size_t waste = size_of_linkedlist + total_gap + total_free;
    size_t heap_size = waste + total_used;
    printf("    Num Blocks: %d, Total Used Space: %ld\n",
        total_blocks, total_used);
    printf("    (LEAKS) Size of LList: %ld, Free Space: %ld, Align Gaps: %ld,\n",
        size_of_linkedlist, total_free, total_gap);
    printf("    (EFFICIENCY) Expected Heap Size %ld, Waste: %ld, Effic: %ld%%\n",
        waste + total_used, waste, 
        (heap_size) ? (total_used * 100 / (heap_size)) : 100);

    //MEM leaks defined as size of linked list + frees + gaps
    //WHAT is size when you can't fit another block? makes a leak?
    //Run with alignment of 0 and alignment of 8.
    //Show comparison with original.
}