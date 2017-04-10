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

#define HDR_SIZE 32

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

/* rounds up to the nearest multiple of mem_pagesize() */
#define PAGE_ALIGN(size) (((size) + (mem_pagesize()-1)) & ~(mem_pagesize()-1))

// Factors the necessary header space out of size values
#define WITH_HEADER(size) (size-HDR_SIZE)

/* void *current_avail = NULL; */

int current_avail_size;
int numHdrs;

short currCallId; // The next call id to assign

void grow(int size);
void defragment();

typedef struct{
  unsigned int length; // The length of the memory chunk, 4 bytes
  short callId; // Used to keep track of which call to mem_map this header belongs to
  void* nxtHdr; // 4
  void* lstHdr; // 4
  long filler;
}Header;

Header* top; // The first header in the structure

void coalesce(Header* freed);

// Pack the alloc bit into 
#define PACK(length, alloc) (length | alloc)

// Extract the allocation bit
#define UNPACK_ALLOC(length) (length & 0x0001)

// Unpack the length field
#define UNPACK_LENGTH(length) (length & 0xfffffffffffffff0)


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{

  int l = PAGE_ALIGN(1);
  
  current_avail_size = WITH_HEADER(l); // There is 1 block page to start with
  numHdrs = 1;

  currCallId = 1;

  //printf("Sizeof: %d", sizeof(Header));
  
  top = (Header*)mem_map(l);
  top->length = PACK(WITH_HEADER(l), 0);  
  top->nxtHdr = NULL;
  top->callId = 0;

  return 0; 
}

/* 
 * mm_malloc - Allocate a block by using bytes from current_avail,
 *     grabbing a new page if necessary.
 */
void *mm_malloc(size_t size)
{
  if(size <= 0)
    return NULL;
  
  int newsize = ALIGN(size); // Ensure that the requested size is on a 16 byte alignment

  if(newsize > current_avail_size)
    grow(newsize);
  
  // Traverse the list
  /*Header* currHdr = top;
  int i;
  for(i = 0; i < numHdrs; i++){ // Loop through the headers until we find a block that the data can fit into
    
    if(UNPACK_LENGTH(currHdr->length) >= newsize && UNPACK_ALLOC(currHdr->length) == 0){ // If we've found a block that is large enough and hasn'
      
      int old_currHdrLength = currHdr->length;
      
      currHdr->length = PACK(newsize, 1);
      
      // Install a new header, if necessary
      if(newsize <= WITH_HEADER(old_currHdrLength)){ // If there's space for a new header.
	
	// Set up the new header
	Header* newHdr = ((void*)currHdr + newsize + 16);
	newHdr->length = PACK((old_currHdrLength - WITH_HEADER(newsize)), 0);
	
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
    }*/

  Header* best = NULL;
  Header* curr = top;

  int delta = -1;

  printf("Starting region search. Requested size: %d\n", newsize);
  int i;
  for(i = 0; i < numHdrs; i++){
    printf("%d:\tLength: %d\tAlloc: %d\n", i, UNPACK_LENGTH(curr->length), UNPACK_ALLOC(curr->length));
    if(UNPACK_LENGTH(curr->length) >= newsize && UNPACK_ALLOC(curr->length) == 0){ // If the allocation will fit in this region
      if(delta == -1){ // This is the first region found that will work
	delta = UNPACK_LENGTH(curr->length) - newsize;
	best = curr;
      }
      else{
	int thisDelta = UNPACK_LENGTH(curr->length) - newsize;
	if(thisDelta < delta)
	  best = curr;
      }
      if(delta == 0)
	break; // No sense in continuing if we've found a best-case scenario
    }
    
    if(curr->nxtHdr != NULL){
      curr = (Header*)curr->nxtHdr;
      //printf("Jumping to a new Heade\n");
    }
  } 
  //printf("Finished region search\n");
  
  int old_hdrLength = UNPACK_LENGTH(best->length);
  best->length = PACK(newsize, 1); 

  // Install a new header, if necessary
  if(newsize <= WITH_HEADER(old_hdrLength)){ // If there's space for a new header.
    //printf("Installing new header\n");
    // Set up the new header
    Header* newHdr = ((void*)best + newsize + HDR_SIZE);
    newHdr->length = PACK((old_hdrLength - WITH_HEADER(newsize) - 2*HDR_SIZE), 0);
	 
    // Patch in the new header
    if(best->nxtHdr == NULL){
       best->nxtHdr = (void*)newHdr;
    } 
    else{
       void* oldNxt = (void*)best->nxtHdr;
       best->nxtHdr = (void*)newHdr;
       newHdr->nxtHdr = oldNxt; 
    }

    numHdrs++;
    
    if(old_hdrLength >= current_avail_size)
      current_avail_size = WITH_HEADER(current_avail_size);
    //printf("Installed new Header\nLength: %d\tALLOC: %d\n", UNPACK_LENGTH(newHdr->length), UNPACK_ALLOC(newHdr->length));
  }

  if(old_hdrLength >= current_avail_size)
    current_avail_size -= newsize;

  //printf("-Best-\nLength: %d\tAlloc: %d\n", UNPACK_LENGTH(best->length), UNPACK_ALLOC(best->length));
  
  //printf("Returning NULL\n");
  return (void*)best + HDR_SIZE;
}

/* 
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
  Header* toFree = (Header*)(WITH_HEADER(ptr));
  toFree->length = PACK(UNPACK_LENGTH(toFree->length), 0);

  if(UNPACK_LENGTH(toFree->length) > current_avail_size)
    current_avail_size = UNPACK_LENGTH(toFree->length);
  
  //printf("Calling coalesce\n");
  coalesce(toFree);
}

// Loops through the structure and merges adjacent empty blocks of the same callId
// Merging two regions that are either still allocated
void defragment(){
  if(top->nxtHdr == NULL)
    return;
  //printf("Defraging sturcture...\n");
  Header* last = top;
  Header* current = top->nxtHdr;
  int i;
  
  for(i = 0; i < numHdrs; i++){
    if((last->callId == current->callId) && (UNPACK_ALLOC(top->length) == 0) && (UNPACK_ALLOC(current->length) == 0)){ // We can merge these blocks
      
      int newLength = UNPACK_LENGTH(last->length) + UNPACK_LENGTH(current->length);
      last->length = PACK(newLength, 0);
      current->nxtHdr = NULL;
      
      if(last->nxtHdr != NULL){
	last->nxtHdr = current->nxtHdr;
      } 
    } 
    else{
      last = current;
    }
    if(current->nxtHdr != NULL)
	current = (Header*)current->nxtHdr;
  }
}

void coalesce(Header* freed){
  
  if(UNPACK_ALLOC(freed->length) != 0)
    printf("Error on freeing!\n");

  if(freed->nxtHdr == NULL)
    return;

  Header* nxt = (Header*)freed->nxtHdr;

  if(UNPACK_ALLOC(nxt->length) == 0 && nxt->callId == freed->callId){
    
    int oldFreeLength = UNPACK_LENGTH(freed->length);
    int oldNextLength = UNPACK_LENGTH(nxt->length);
    int newLength = oldFreeLength + oldNextLength + HDR_SIZE;

    freed->length = PACK(newLength, 0);

    freed->nxtHdr = nxt->nxtHdr;
  }
}

// Grows the sturcture by a page
void grow(int size){
  //printf("Grow called. Requested Size: %d\n", size);
  int growSize = PAGE_ALIGN(size+HDR_SIZE);

  //printf("growSize: %d\n", growSize);
  Header* newTop = mem_map(growSize); 
  newTop->length = PACK(WITH_HEADER(growSize), 0);

  //printf("New top length: %d\n", UNPACK_LENGTH(newTop->length));
  
  newTop->nxtHdr = top;

  top = newTop;

  newTop->callId = currCallId;
  currCallId++;
    
  current_avail_size = WITH_HEADER(growSize);
  numHdrs++;
  // printf("Called grow\n");
}

