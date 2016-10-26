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

// TODO: align

// sbrk some extra space every time we need it.
// This does no bookkeeping and therefore has no ability to free, realloc, etc.

/*struct block_meta {
  size_t size;
  struct block_meta *next;
  int free;
  int magic;    // For debugging only. TODO: remove this in non-debug mode.
};*/

//#define META_SIZE sizeof(struct block_meta)
#define ALIGNMENT 8

void *global_base = NULL;

size_t align(size_t ptr_value) {
    int remainder = ptr_value % ALIGNMENT;
    if (remainder) {
        ptr_value += ALIGNMENT - remainder;
    }
    return ptr_value;
}


struct block_meta *find_previous(struct block_meta *current) {
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
}

// Iterate through blocks until we find one that's large enough.
// TODO: split block up if it's larger than necessary
struct block_meta *find_free_block(struct block_meta **last, size_t size) {
  struct block_meta *current = global_base;
  struct block_meta *best;
  while (current 
      && !(current->free 
      && current->size >= size)) {
    *last = current;
    current = current->next;
  }
  if (!current) return NULL;
  best = current;
  while (current) {
    if (current->free 
      && current->size >= size 
      && (current->size < best->size)) {
        best = current;
        if (best->size = size) break;
    }
    current = current->next;
  }

  if (best) split(best, size);
  return best;
}


struct block_meta *request_space(struct block_meta* last, size_t size) {
  struct block_meta *block;
  block = (struct block_meta*) align((size_t)sbrk(0)); //TODO: Auto-alignment?
  if (brk((void *)block + size + META_SIZE) == -1) { //TODO: not thread safe
    return NULL; // sbrk failed.
  }
  //void *request = sbrk(size + META_SIZE);
  //assert((void*)block == request); // Not thread safe.
  //if (request == (void*) -1) {
  //  return NULL; // sbrk failed.
  //}
  
  if (last) { // NULL on first request.
    last->next = block;
  }
  block->size = size;
  block->next = NULL;
  block->free = 0;
  block->magic = 12345678;
  return block;
}

// If it's the first ever call, i.e., global_base == NULL, request_space and set global_base.
// Otherwise, if we can find a free block, use it.
// If not, request_space.
void *malloc(size_t size) {
  //printf("malloc %x\n", (int)size);
  struct block_meta *block;
  // TODO: align size?

  if (size <= 0) {
    return NULL;
  }

  if (!global_base) { // First call.
    block = request_space(NULL, size);
    if (!block) {
      return NULL;
    }
    global_base = block;
  } else {
    struct block_meta *last = global_base;
    block = find_free_block(&last, size);
    if (!block) { // Failed to find free block.
      block = request_space(last, size);
      if (!block) {
  return NULL;
      }
    } else {      // Found free block
      // TODO: consider splitting block here.
      block->free = 0;
      block->magic = 77777777;
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
struct block_meta *get_block_ptr(void *ptr) {
  return (struct block_meta*)ptr - 1;
}



/////////////////////////////////////////////////////////////
//Free/Merge



//TODO: lower SBRK
void free(void *ptr) {
  //printf("free %p\n", ptr);
  if (!ptr) {
    return;
  }

  struct block_meta* block_ptr = get_block_ptr(ptr);


  assert(block_ptr->free == 0);
  assert(block_ptr->magic != 55555555);
  block_ptr->free = 1;
  block_ptr->magic = 55555555;  
  merge_with_next(block_ptr);

  struct block_meta *previous =  find_previous(block_ptr);
  if (previous) {
    merge_with_next(previous);
    if (!(previous->next)) {
      block_ptr = previous;
      previous = find_previous(block_ptr);
    }
  }

  if (!block_ptr->next) {
    if (previous) {
      brk((void *)previous + previous->size + META_SIZE);  //TODO make this safer
      previous->next = NULL; 
    } else {
      assert(block_ptr = global_base);
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
}


/*void analyze() { //size_t *total_free, size_t *total_used, size_t *total_gap, int *total_blocks) {
    struct block_meta *block = global_base;
    int total_blocks = 0;
    while (block) {
        total_blocks++;
        block = block ->next;
    }
    printf("Sizeof linked list: %d", total_blocks * META_SIZE);
}*/


void analyze( int printall) { //size_t *total_free, size_t *total_used, size_t *total_gap, int *total_blocks) {
    struct block_meta *block = global_base;
    size_t total_free = 0;
    size_t total_gap = 0;
    size_t total_used = 0;
    int total_blocks = 0;
    size_t gap;
    while (block) {
        total_blocks++;
        if (block->free) {
            total_free += block->size;
        } else {
            total_used += block->size;
        }
        if (printall) printf("  Blck #: %d, Loc: %p, Siz: %ld, IsFree: %d, Mag: %d", total_blocks, block, block->size, block->free, block->magic);

        if (block->next) {
            gap = (void *) block->next - (void *) block - META_SIZE - block->size;
            total_gap += gap;
            if (printall) printf(", Gap(d): %ld", gap);
        }
        if (printall) printf("\n");
        block = block ->next;
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