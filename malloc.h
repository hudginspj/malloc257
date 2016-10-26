#ifndef MALLOC_INCLUDED
#define MALLOC_INCLUDED

////////////////////////////////////////////////////////////////////////////////
//
//  File          : a1support.h
//  Description   : This is a set of general-purpose utility functions we use
//                  for the 257 assignment #1.
//
//  Author   : Paul Hudgins
//  Last Modified  : 10/5/16

//
// Functional Prototypes

struct block_meta {
  struct block_meta *next;
  size_t size;
  char free;
  int magic;    // For debugging only. TODO: remove this in non-debug mode.
};

struct small_block_meta {
  struct block_meta *next;
  size_t size : 56;
  char free;
};

#define META_SIZE sizeof(struct block_meta)


// Iterate through blocks until we find one that's large enough.
// TODO: split block up if it's larger than necessary
struct block_meta *find_free_block(struct block_meta **last, size_t size) ;

struct block_meta *request_space(struct block_meta* last, size_t size) ;

// If it's the first ever call, i.e., global_base == NULL, request_space and set global_base.
// Otherwise, if we can find a free block, use it.
// If not, request_space.
void *malloc(size_t size) ;

void *calloc(size_t nelem, size_t elsize) ;

// TODO: maybe do some validation here.
struct block_meta *get_block_ptr(void *ptr) ;

void free(void *ptr) ;

void *realloc(void *ptr, size_t size) ;

void analyze( int printall);


#endif // MALLOC_INCLUDED
