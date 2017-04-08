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

// Factors the necessary header space out of size values
#define WITH_HEADER(size) (size-16)

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

// Pack the alloc bit into 
#define PACK(length, alloc) (length | alloc)

// Extract the allocation bit
#define UNPACK_ALLOC(length) (length & 0x0001)

// Unpack the length field
#define UNPACK_LENGTH(length) (length & 0xfff0)


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{

  int l = PAGE_ALIGN(1);
  
  current_avail_size = WITH_HEADER(l); // There is 1 block page to start with
  numHdrs = 1;

  top = (Header*)mem_map(l);
  top->length = PACK(WITH_HEADER(l), 0);  
  top->nxtHdr = NULL;

  return 0; 
}

/* 
 * mm_malloc - Allocate a block by using bytes from current_avail,
 *     grabbing a new page if necessary.
 */
void *mm_malloc(size_t size)
{
  int newsize = ALIGN(size); // Ensure that the requested size is on a 16 byte alignment

  if(newsize > WITH_HEADER(current_avail_size))
    grow(newsize);
  
  // Traverse the list
  Header* currHdr = top;
  int i;
  for(i = 0; i < numHdrs; i++){ // Loop through the headers until we find a block that the data can fit into
    printf("Length: %d\tAlloc: %d\n", UNPACK_LENGTH(currHdr->length), UNPACK_ALLOC(currHdr->alloc));
    if(UNPACK_LENGTH(currHdr->length) >= newsize && UNPACK_ALLOC(currHdr->length) == 0){ // If we've found a block that is large enough and hasn'
      
      int old_currHdrLength = currHdr->length;
      
      currHdr->length = PACK(newsize, 1);
      printf("newsize: %d\tcurrHdrLength: %d\n", newsize, currHdr->length);
      
      // Install a new header, if necessary
      if(newsize <= WITH_HEADER(old_currHdrLength)){ // If there's space for a new header.
	
	// Set up the new header
	Header* newHdr = ((void*)currHdr + newsize + 16);
	newHdr->length = PACK((old_currHdrLength - WITH_HEADER(newsize)), 0);
	printf("newHdr Length: %d\n", newHdr->length);
	
	// Patch in the new header
	if(currHdr->nxtHdr == NULL){
	  currHdr->nxtHdr = (void*)newHdr;
	} 
	else{
	  void* oldNxt = (void*)currHdr->nxtHdr;
	  currHdr->nxtHdr = (void*)newHdr;
	  newHdr->nxtHdr = oldNxt; 
	}

	numHdrs++;

	current_avail_size = WITH_HEADER(current_avail_size);
      }

      current_avail_size -= newsize;
      return ((void*)currHdr) + 16;
    }
    else{
      currHdr = (Header*)currHdr->nxtHdr; // Jump to the next block
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
  toFree->length = PACK(UNPACK_LENGTH(toFree->length), 0);
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
    newTop->length = PACK((top->length + growSize), 0);
    current_avail_size = WITH_HEADER(growSize);
    top = newTop;
  }
  else{
    //printf("In alloced grow\n");
    Header* newTop = mem_map(growSize);
    newTop->length = PACK(WITH_HEADER(growSize), 0);

    newTop->nxtHdr = top;

    top = newTop;

    current_avail_size = WITH_HEADER(growSize);
    numHdrs++;
  }
}

