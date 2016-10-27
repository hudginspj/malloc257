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

typedef long long wordcount;

typedef struct {
  /*Words in entire block, including meta.
   */
  wordcount words : 56; 
  char free : 4; //TODO use one or elimnate bitfield
  char notlast : 2;
} block_meta;

#define META_SIZE sizeof(block_meta)

#define WORD_SIZE 8  //META_SIZE should equal WORD_SIZE


// Iterate through blocks until we find one that's large enough.
// TODO: split block up if it's larger than necessary
block_meta *find_free_block(wordcount size) ;

block_meta *request_space(block_meta* last, wordcount size) ;

// If it's the first ever call, i.e., global_base == NULL, request_space and set global_base.
// Otherwise, if we can find a free block, use it.
// If not, request_space.
void *malloc(size_t size) ;

void *calloc(size_t nelem, size_t elsize) ;

// TODO: maybe do some validation here.
block_meta *get_block_ptr(void *ptr) ;

void free(void *ptr) ;

//void *realloc(void *ptr, size_t size) ;

void analyze( int printall);


#endif // MALLOC_INCLUDED
