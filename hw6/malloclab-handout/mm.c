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

void grow(int size);

typedef struct{
  unsigned int length; // The length of the memory chunk, 4 bytes
  short alloc; // 1 If the block is allocated, 0 otherwise. 2 bytes
  void* nxtHdr; // 4
  //void* lstHdr; // 4
}Header;

Header* top; // The first header in the structure

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
  /* current_avail = NULL; */
  //printf("Stepping into mm_init\n");

  int l = PAGE_ALIGN(1);
  
  current_avail_size = l-32; // There is 1 block page to start with
  numHdrs = 1;

  top = (Header*)mem_map(l);
  top->length = l-16; // In bytes
  top->alloc = 0;  
  top->nxtHdr = NULL;
  //top->lstHdr = NULL; 

  //printf("Init top\n");
  
  /* Header* bottom = ((void*)top + top->length); */
  /* printf("Head/tail delta: %d\n", (void*)bottom-(void*)top); */
  /* bottom->alloc = 0; */
  /* bottom->length = 0; */
  /* bottom->nxtHdr = 0; */

  //top->nxtHdr = bottom;
  
  int lengthTest = top->length;
  
  //printf("Sizeof: %d\n", current_avail_size);
  
  //printf("Finished mm_init\n");
  return 0; 
}

/* 
 * mm_malloc - Allocate a block by using bytes from current_avail,
 *     grabbing a new page if necessary.
 */
void *mm_malloc(size_t size)
{
  //printf("Stepping into mm_malloc\n");
  int newsize = ALIGN(size);
  //printf("Requested block size: %d\n", newsize);

  if(newsize > current_avail_size - 16)
    grow(newsize);
  //printf("Made it past grow\n");
  
  // Traverse the list
  Header* currHdr = top;
  int i;
  for(i = 0; i < numHdrs; i++){ // Loop through the headers until we find a block that the data can fit into
    //printf("Length of current header: %d. \tAlloc: %d.\n", currHdr->length, currHdr->alloc);
    if(currHdr->length >= newsize && currHdr->alloc == 0){ // If we've found a block that is large enough and hasn't been allocated yet
      //printf("Found a header that fits!\n");
      
      int old_currHdrLength = currHdr->length;
      
      currHdr->length = newsize; // seg fault here!
      //printf("Finished adjusting currHdr\n");

      currHdr->alloc = 1;
      //printf("Altered alloc\n");
      
      // Install a new header, if necessary
      if(newsize <= old_currHdrLength - 16){ // If there's space for a new header. Make sure to account for the header
	// Set up the new header
	//printf("CurrHdr Val: %p\n", currHdr);
	Header* newHdr = ((void*)currHdr + newsize + 16);
	//printf("Pntr delta: %d\n", (void*)newHdr-(void*)currHdr);
	
	//printf("Generated pointer to new header\n");
	newHdr->length = old_currHdrLength - newsize - 16;
	//printf("Next blk size: %d\n", newHdr->length);
	
	// Patch in the new header
	if(currHdr->nxtHdr == NULL){
	  currHdr->nxtHdr = (void*)newHdr;
	} 
	else{
	  void* oldNxt = (void*)currHdr->nxtHdr;
	  currHdr->nxtHdr = (void*)newHdr;
	  newHdr->nxtHdr = oldNxt; 
	  //((struct Header*)oldNxt)->lstHdr = newHdr;
	}

	//newHdr->lstHdr = currHdr;
	newHdr->alloc = 0;  
	numHdrs++;

	//printf("New header Installed!\nLength %d\tAlloc: %d\n", newHdr->length, newHdr->alloc);
	current_avail_size -= 16;
      }

      //printf("Returning!\n");
      current_avail_size -= newsize;
      //printf("Current avail size: %d\n", current_avail_size);
      return ((void*)currHdr) + 16;
    }
    else{
      //printf("Found a block that won't fit, moving on...\n");
      //Header* old = currHdr;
      currHdr = (Header*)currHdr->nxtHdr; // Jump to the next block
      //printf("currHdr->currHdr delta: %d\n", (void*)currHdr - (void*)old);
    }
  }

  
  //printf("Returning NULL\n");
  return NULL;
}

/* 
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
  Header* toFree = (Header*)ptr; 
  toFree->alloc = 0;
}
 
// Grows the sturcture by a page
void grow(int size){
  // Might want to be a bit more sophisticated here.
  // Only pop up a pagesize if the requested size is going to be a tight squeeze.
  int growSize = PAGE_ALIGN(size+16);
  
  if(top->alloc == 0){ // We need to insert more space after top
    //printf("In unalloc grow\n");
    
    Header* newTop = mem_map(growSize);
    //printf("Succesfully grabbed more memory!\n");
    
    newTop->nxtHdr = top->nxtHdr;
    //printf("Set up newTop->nxtHdr\n");
    //((struct Header*)top->nxtHdr)->lstHdr = newTop;
    newTop->length = top->length + growSize;
    //printf("Set up length\n");
    current_avail_size = growSize - 16;
    //printf("Set up curr size\n");
    top = newTop;
    //printf("Installed new Top\n");
    top->alloc = 0;
  }
  else{
    //printf("In alloced grow\n");
    Header* newTop = mem_map(growSize);
    newTop->alloc = 0;
    newTop->length = growSize - 16;

    newTop->nxtHdr = top;
    //top->lstHdr = newTop;

    top = newTop;

    current_avail_size = growSize - 16;
    //printf("%d\n", current_avail_size);
    numHdrs++;
    //printf("Finished alloc\n");
  }
}

