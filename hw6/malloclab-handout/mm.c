/*
 * mm-naive.c - The least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by allocating a
 * new page as needed.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/* always use 16-byte alignment */
#define ALIGNMENT 16

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

/* rounds up to the nearest multiple of mem_pagesize() */
#define PAGE_ALIGN(size) (((size) + (mem_pagesize()-1)) & ~(mem_pagesize()-1))

/* void *current_avail = NULL; */

int current_avail_size;
int numHdrs;

void grow();

struct Header{
  long length; // The length of the memory chunk, 4 bytes
  short alloc; // 1 If the block is allocated, 0 otherwise. 2 bytes
  void* nxtHdr; // 4
  void* lstHdr; // 4
};

struct Header* top; // The first header in the structure

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
  /* current_avail = NULL; */
  
  current_avail_size = 0; // There is 1 block to start with
  numHdrs = 0;

  top = mem_map(mem_pagesize());
  top->length = 16; // In bytes
  top->alloc = 0;
  
  return 0; 
}

/* 
 * mm_malloc - Allocate a block by using bytes from current_avail,
 *     grabbing a new page if necessary.
 */
void *mm_malloc(size_t size)
{
  int newsize = ALIGN(size);
  
  /* if (current_avail_size < newsize) { */
  /*   current_avail_size = PAGE_ALIGN(newsize); */
  /*   current_avail = mem_map(current_avail_size); */
  /*   if (current_avail == NULL) */
  /*     return NULL; */
  /* } */

  /* p = current_avail; */
  /* current_avail += newsize; */
  /* current_avail_size -= newsize; */

  while(current_avail_size < newsize)
    grow();

  // Traverse the list
  struct Header* currHdr = top;
  int i;
  for(i = 0; i < numHdrs; i++){ // Loop through the headers until we find a block that the data can fit into
    if(currHdr->length >= newsize && currHdr->alloc == 0){ // If we've found a block that is large enough and hasn't been allocated yet

      long oldLength = currHdr->length;
      currHdr->length = newsize;
      currHdr->alloc = 1;

      struct Header* newHdr = currHdr + newsize;
      newHdr->length = oldLength - newsize;
      newHdr->alloc = 0;

      struct Header* nxtHdr_tmp = currHdr->nxtHdr;
      currHdr->nxtHdr = newHdr;
      
      newHdr->nxtHdr = nxtHdr_tmp;
      newHdr->lstHdr = currHdr;

      nxtHdr_tmp->lstHdr = newHdr;

      return newHdr + 16;
    }
    else{
      currHdr = currHdr->nxtHdr; // Jump to the next block
    }
  }
  
  return NULL;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
}
 
// Grows the sturcture by the ratio specified in growRat
void grow(){
  struct Header* currTop = top;
  struct Header* nxtHd = mem_map(mem_pagesize());
  nxtHd->length = current_avail_size;
  nxtHd->alloc = 0;
  nxtHd->nxtHdr = currTop;
  currTop->lstHdr = nxtHd;
  top = nxtHd;

  numHdrs++;
  current_avail_size *= 2;
}

