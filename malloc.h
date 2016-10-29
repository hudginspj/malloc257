#ifndef MALLOC_INCLUDED
#define MALLOC_INCLUDED

////////////////////////////////////////////////////////////////////////////////
//
//  File          : a1support.h
//  Description   : This is a set of general-purpose utility functions we use
//                  for the 257 assignment #2.
//
//  Author   : Paul Hudgins
//  Last Modified  : 10/28/16

//
// Functional Prototypes


#define WORD_SIZE sizeof(long) 

typedef long wordcount;

typedef struct {
  //Words in entire block, including metadata.
  wordcount words : ((WORD_SIZE*8)-2); 
  char free : 1;
  char notlast : 1;
} block_meta;


block_meta *get_block_ptr(void *ptr) ;

block_meta *get_global_base() ;

void *malloc(size_t size) ;

void free(void *ptr) ;

void *calloc(size_t nelem, size_t elsize) ;

void *realloc(void *ptr, size_t size) ;

#endif // MALLOC_INCLUDED
