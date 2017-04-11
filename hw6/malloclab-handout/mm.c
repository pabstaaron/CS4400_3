/*
 * mm.c - A prototype memory allocator
 * Written By: Aaron Pabst
 *
 * I got as far as implementing a doubly-linked implicit free list that is capable of coalescing adjacent free blocks and giving 
 * unused pages back to the kernal.
 *
 * I fell short of getting to the explicit free list due to the days I spent debugging my initial solution, but I did implement a mechanism for inserting
 * and removing allocations from the free list. It was too buggy to make calls to these functions in my submitted code, however.
 *
 * I also fell short of reducing the header size for the same reason as above.
 * 
 * This code will work with the following trace files from the config.h file
 *     cccp-bal.rep
 *     cp-decl-bal.rep
 *     expr-bal.rep
 *     coalescing-bal.rep
 *     binary-bal.rep
 *     binary2-bal.rep
 *
 * The throughput on all of these traces is very poor due to my using an implicit free list. I also make calls to several functions in 
 * the mm_free function that loop through all or part of the memory structure. Due to time spent debugging, I never was able to improve these
 * functions.
 *
 * As best I can tell, the files that were not listed above were ending in segmentation faults or accesses to unmapped memory regions because of 
 * one or more off-by-one errors in the maintence of information about how many headers there are along with the sizes of the payloads of these headers. 
 * There were also several edge cases were a duplicate header worked its way into the structure.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

#include <errno.h>

/* always use 16-byte alignment */
#define ALIGNMENT 16

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

/* rounds up to the nearest multiple of mem_pagesize() */
#define PAGE_ALIGN(size) (((size) + (mem_pagesize()-1)) & ~(mem_pagesize()-1))

// Factors the necessary header space out of size values
#define WITH_HEADER(size) (size-HDR_SIZE)

/* void *current_avail = NULL; */

unsigned int current_avail_size;
unsigned int numHdrs;

short currCallId; // The next call id to assign
int currId;

void grow(int size);
void writeToFile();
  
typedef struct{
  unsigned int id; // 2
  short callId; // Used to keep track of which call to mem_map this header belongs to, 2 bytes
  void* nxtHdr; // 8
  void* lstHdr; // 8
  unsigned long length; // The length of the memory chunk, 8 bytes
}Header;

Header* top; // The first header in the structure

void coalesce(Header* freed);

void coalesce2(Header* freed);

int insertToList(Header* hdr);

void removeFromList(Header* hdr);

// Pack the alloc bit into 
#define PACK(length, last, alloc) (length | (last << 1) | alloc)

// Extract the allocation bit
#define UNPACK_ALLOC(length) (length & 0x0001)

// Unpack the length field
#define UNPACK_LENGTH(length) (length & 0xfffffffffffffff0)

#define UNPACK_LAST(length) ((length & 0x2) >> 1)  

#define HDR_SIZE sizeof(Header)

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
  //printf("%d\n", HDR_SIZE);
  int l = PAGE_ALIGN(1);
  
  current_avail_size = WITH_HEADER(l); // There is 1 block page to start with
  numHdrs = 1;

  currCallId = 1;
  currId = 1;

  //printf("Sizeof: %d", sizeof(Header));
   
  top = (Header*)mem_map(l);
  top->length = PACK(WITH_HEADER(l), 1, 0);  
  top->nxtHdr = NULL;
  top->lstHdr = NULL;
  top->callId = 0;
  top->id = 0;

  //printf("Top Length - %d\t%d\t%d\n", UNPACK_LENGTH(top->length), UNPACK_LAST(top->length), UNPACK_ALLOC(top->length));
  
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

  //printf("Requested size: %u\tAvail Size: %u\n", newsize, current_avail_size);
  
  if((newsize + HDR_SIZE) > current_avail_size)
    grow(newsize);

  Header* best = NULL;
  Header* curr = top;

  int delta = -1;

  //printf("Starting region search. Num Hdrs: %d\n", numHdrs);
  int i;
  for(i = 0; i < numHdrs; i++){
    //printf("%d:\tLength: %u\tAlloc: %d\tLast: %d\n", i, UNPACK_LENGTH(curr->length), UNPACK_ALLOC(curr->length), UNPACK_LAST(curr->length));
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
      //printf("Jumping to a new Header\n");
    }
    else
      break;
  }   
  //printf("Finished region search\n");
  
  int last =  UNPACK_LAST(best->length);
  
  int old_hdrLength = UNPACK_LENGTH(best->length);
  best->length = PACK(newsize, UNPACK_LAST(best->length), 1); 

  // Install a new header, if necessary
  if(newsize <= WITH_HEADER(old_hdrLength)){ // If there's space for a new header.
    //printf("Installing new header\n");
    // Set up the new header
    Header* newHdr = ((void*)best + newsize + HDR_SIZE);
    newHdr->id = currId;
    currId++;
    
    if(last == 1){
      //printf("Last Header\n");
      best->length = PACK(UNPACK_LENGTH(best->length), 0, UNPACK_ALLOC(best->length));
    }
    
    newHdr->length = PACK((old_hdrLength - WITH_HEADER(newsize) - 2*HDR_SIZE), last, 0);
    newHdr->callId = best->callId;
    
    // Patch in the new header
    if(best->nxtHdr == NULL){
       best->nxtHdr = (void*)newHdr;
    } 
    else{
       void* oldNxt = (void*)best->nxtHdr;
       best->nxtHdr = (void*)newHdr; 
       newHdr->nxtHdr = oldNxt; 
    }

    newHdr->lstHdr = best;
    
    numHdrs++;
    
    if(old_hdrLength >= current_avail_size)
      current_avail_size = WITH_HEADER(current_avail_size);
    //printf("Installed new Header\nLength: %u\tALLOC: %d\tLast: %d\n", UNPACK_LENGTH(newHdr->length), UNPACK_ALLOC(newHdr->length), UNPACK_LAST(newHdr->length));
  }

  if(old_hdrLength >= current_avail_size)
    current_avail_size -= newsize;

  //printf("-Best-\nLength: %d\tAlloc: %d\n", UNPACK_LENGTH(best->length), UNPACK_ALLOC(best->length));
  

  removeFromList(best);

  //writeToFile();
  
  //printf("Returning\n");  
  return (void*)best + HDR_SIZE;
}

/* 
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
  //printf("Free called\n");
  Header* toFree = (Header*)(WITH_HEADER(ptr));
  toFree->length = PACK(UNPACK_LENGTH(toFree->length), UNPACK_LAST(toFree->length), 0);

  if(UNPACK_LENGTH(toFree->length) > current_avail_size)
    current_avail_size = UNPACK_LENGTH(toFree->length);

  insertToList(toFree);
  
  coalesce2(toFree);
  //writeToFile();
} 
 
 
void coalesce(Header* freed){
  
  if(UNPACK_ALLOC(freed->length) != 0)
    printf("Error on freeing!\n");

  if(UNPACK_LAST(freed->length) == 1)
    return;
 
  Header* nxt = (Header*)freed->nxtHdr;

  // If we can coalesce
  if(UNPACK_ALLOC(nxt->length) == 0 && nxt->callId == freed->callId){
    //printf("Coalescing\n");
    int oldFreeLength = UNPACK_LENGTH(freed->length);
    int oldNextLength = UNPACK_LENGTH(nxt->length);
    int newLength = oldFreeLength + oldNextLength + HDR_SIZE;

    short last = UNPACK_LAST(nxt->length); // Copy the last field from the header being absorbed
    //printf("In coal last: %d\n", last);
    
    freed->length = PACK(newLength, last, 0);

    freed->nxtHdr = nxt->nxtHdr;
    if(freed->nxtHdr != NULL){
      //printf("Setting last hdr\n");
      Header* nxt = (Header*)freed->nxtHdr;
      nxt->lstHdr = freed;
      //printf("Done\n");
    }
    
    if(newLength > current_avail_size)
      current_avail_size = newLength;

    if(last == 1)
      freed->nxtHdr = NULL;
    
    numHdrs--;
  }
}

int findLargest(){
  Header* curr = top;
  int largest = UNPACK_LENGTH(top->length);
  int allAlloced = 1;
  
  int i;
  for(i = 0; i < numHdrs; i++){
    if((UNPACK_LENGTH(curr->length) > largest) && UNPACK_ALLOC(curr->length) == 0){
      largest = UNPACK_LENGTH(curr->length);
      allAlloced = 0;
    }

    if(curr->nxtHdr != NULL){
      curr = (Header*)curr->nxtHdr;
    }
  }

  if(allAlloced)
    return 0;
  
  return largest;
}

// Pulls a header from the list
void removeFromList(Header* hdr){
  if(numHdrs == 1)
    return;
  
  if(hdr->lstHdr != NULL && hdr->nxtHdr != NULL){
    Header* nxt = (Header*)hdr->nxtHdr;
    Header* lst = (Header*)hdr->lstHdr;

    nxt->lstHdr = lst;
    lst->nxtHdr = nxt;
    numHdrs--;
    //printf("Removing Header\n");
  }
  else if(top == hdr){
    Header* nxt = (Header*)hdr->nxtHdr;
    top = nxt;
    nxt->lstHdr = NULL;
    numHdrs--;
    //printf("Removing Header\n");
  }
  else if(hdr->nxtHdr == NULL){
    Header* lst = (Header*)hdr->lstHdr;
    lst->length = PACK(UNPACK_LENGTH(lst->length), 1, UNPACK_ALLOC(lst->length));
    //lst->nxtHdr = NULL;
    numHdrs--;
    //printf("Removing Header\n");
  }
  else{
    printf("Not Removing Header\n");
  }
}

int insertToList(Header* hdr){
  // Find the header's callId in list
  // Find correct position in callId
  // Patch in header

  Header* curr = top;
  int foundId = 0;
  int i;
  for(i = 0; i < numHdrs; i++){
    if(curr->callId == hdr->callId){ // Found it
      foundId = 1;
      break;
    }

    if(curr->nxtHdr != NULL)
      curr = (Header*)curr->nxtHdr;
  }

  if(!foundId)
    return -1;

  if(hdr->id < curr->id){
    if(curr->lstHdr != NULL){
      Header* lst = curr->lstHdr;

      lst->nxtHdr = (void*)hdr;
      hdr->nxtHdr = (void*)curr;

      curr->lstHdr = (void*)hdr;
      hdr->lstHdr = (void*)lst;
    }
    else{
      hdr->nxtHdr = (void*)curr;
      curr->lstHdr = (void*)hdr;
      top = hdr;
    }

    if(UNPACK_LENGTH(hdr->length) > current_avail_size)
      current_avail_size = UNPACK_LENGTH(hdr->length);
    
    numHdrs++;
    return 0;
  }
  
  while(curr->id == hdr->id){
    if(curr->nxtHdr != NULL && curr->id < hdr->id && ((Header*)curr->nxtHdr)->id > hdr->id){
      Header* nxt = curr->nxtHdr;

      curr->nxtHdr = (void*)hdr;
      hdr->nxtHdr = (void*)nxt;

      nxt->lstHdr = (void*)hdr;
      hdr->lstHdr = (void*)curr;

      if(UNPACK_LENGTH(hdr->length) > current_avail_size)
	current_avail_size = UNPACK_LENGTH(hdr->length);
      
      numHdrs++;
      return 0;
    }
    else if(curr->nxtHdr == NULL && curr->id < hdr->id){
      // Inserting a new tail
      curr->nxtHdr = (void*)hdr;
      hdr->lstHdr = (void*)curr;

      curr->length = PACK(UNPACK_LENGTH(curr->length), 0, UNPACK_ALLOC(curr->length));
      hdr->length = PACK(UNPACK_LENGTH(curr->length), 1, UNPACK_ALLOC(curr->length));
      hdr->nxtHdr = NULL;

      if(UNPACK_LENGTH(hdr->length) > current_avail_size)
	current_avail_size = UNPACK_LENGTH(hdr->length);
      
      numHdrs++;
      return 0;
    }
    else{
      if(curr->nxtHdr != NULL)
	curr = (Header*)curr->nxtHdr;
    }
  }

  return -1;
}

void giveBack(Header* hdr){
  if(numHdrs == 1)
    return;

  if((UNPACK_LENGTH(hdr->length)+HDR_SIZE) % mem_pagesize() != 0)
    return;
  
  // We can give back the block if any of the below cases apply
  if(hdr->lstHdr != NULL && hdr->nxtHdr != NULL && ((Header*)hdr->lstHdr)->callId != hdr->callId && ((Header*)hdr->nxtHdr)->callId != hdr->callId){
    //printf("Unmapping\n");
    removeFromList(hdr);
    mem_unmap((void*)hdr, UNPACK_LENGTH(hdr->length) + HDR_SIZE);
    current_avail_size = findLargest();
  }
  else if(hdr->lstHdr != NULL && ((Header*)hdr->lstHdr)->callId != hdr->callId && hdr->nxtHdr == NULL){
    //printf("Unmapping\n");
    removeFromList(hdr);
    mem_unmap((void*)hdr, UNPACK_LENGTH(hdr->length) + HDR_SIZE);
    current_avail_size = findLargest();
  }
  else if(hdr->nxtHdr != NULL && ((Header*)hdr->nxtHdr)->callId && hdr->lstHdr == NULL){
    //printf("Unmapping\n");
    removeFromList(hdr);
    mem_unmap((void*)hdr, UNPACK_LENGTH(hdr->length) + HDR_SIZE);
    current_avail_size = findLargest();
  }
} 
 
void coalesce2(Header* freed){
  // 4 cases:
  //  1) Cant coalesce
  //  2) Can coalesce with the previous header
  //  3) Can coalesce with the next header
  //  4) Can coalesce in both directions

  short canFwd = 0;
  short canBwd = 0;
   
  if(freed->nxtHdr != NULL && freed->lstHdr != NULL){ // Valid hdr in both directions
    Header* nxt = (Header*)freed->nxtHdr;
    Header* lst = (Header*)freed->lstHdr;

    if(UNPACK_ALLOC(nxt->length) == 0 && UNPACK_ALLOC(lst->length) == 0 && ((nxt->callId == freed->callId) == lst->callId)){
      canFwd = 1;
      canBwd = 1;
    }
    else if(UNPACK_ALLOC(nxt->length) == 0 && nxt->callId == freed->callId)
      canFwd = 1;
    else if(UNPACK_ALLOC(lst->length) == 0 && lst->callId == freed->callId)
      canBwd = 1;
  }
  else if(freed->nxtHdr != NULL && freed->lstHdr == NULL){ //  Valid hdr in forwards direction
    Header* nxt = (Header*)freed->nxtHdr;
    if(UNPACK_ALLOC(nxt->length) == 0 && nxt->callId == freed->callId)
      canFwd = 1; 
  }
  else if(freed->nxtHdr == NULL && freed->lstHdr != NULL){ // Valid hdr in backwards direction
    Header* lst = (Header*)freed->lstHdr;
    if(UNPACK_ALLOC(lst->length) == 0 && lst->callId == freed->callId)
      canBwd = 1;
  }

  if(canFwd && canBwd){
    Header* nxt = (Header*)freed->nxtHdr;
    Header* lst = (Header*)freed->lstHdr;

    int last = UNPACK_LAST(nxt->length); // Determine if the super will be last

    int nxtLength = UNPACK_LENGTH(nxt->length);
    int freedLength = UNPACK_LENGTH(nxt->length);
    int lstLength = UNPACK_LENGTH(nxt->length);

    // TODO - Will we need to null out headers on if we're at the top/bottom?

    int newLength = nxtLength + freedLength + lstLength + 2*HDR_SIZE;
 
    lst->length = PACK(newLength, last, 0);

    if(newLength > current_avail_size)
      current_avail_size = newLength;
    
    // Fixup pointers
    lst->nxtHdr = nxt->nxtHdr;

    if(lst->nxtHdr != NULL){
      Header* nxtNxt = lst->nxtHdr;
      nxtNxt->lstHdr = (void*)lst;
    }

    numHdrs -= 2;

    // Attempt to give memory back to the kernal
    giveBack(lst);
  }
  else if(canFwd && !canBwd){
    Header* nxt = (Header*)freed->nxtHdr;

    int last = UNPACK_LAST(nxt->length);

    // Calculate new length
    int nxtLength = UNPACK_LENGTH(nxt->length);
    int freedLength = UNPACK_LENGTH(freed->length);

    int newLength = nxtLength + freedLength + HDR_SIZE;

    freed->length = PACK(newLength, last, 0);
    
    if(newLength > current_avail_size)
      current_avail_size = newLength;
    
    // Fixup pointers
    freed->nxtHdr = nxt->nxtHdr;

    if(freed->nxtHdr != NULL){
      Header* nxtNxt = (Header*)nxt->nxtHdr;
      nxtNxt->lstHdr = (void*)freed;
    }

    numHdrs--;

    giveBack(freed);
  }
  else if(!canFwd && canBwd){
    Header* lst = (Header*)freed->lstHdr;

    int last = UNPACK_LAST(freed->length);

    int lstLength = UNPACK_LENGTH(lst->length);
    int freedLength = UNPACK_LENGTH(freed->length);

    int newLength = lstLength + freedLength + HDR_SIZE;
    
    lst->length = PACK(newLength, last, 0);

    if(newLength > current_avail_size)
      current_avail_size = newLength;
    
    //Fixup Pointers
    lst->nxtHdr = freed->nxtHdr;

    if(lst->nxtHdr != NULL){
      Header* nxtNxt = (Header*)lst->nxtHdr;
      nxtNxt->lstHdr = (void*)lst;
    }

    numHdrs--;

    giveBack(lst);
  }
}


// Grows the sturcture by a multiple of a page
void grow(int size){
  //printf("Grow called. Requested Size: %d\n", size);
  int growSize = PAGE_ALIGN(size+HDR_SIZE);

  //printf("growSize: %d\n", growSize);
  Header* newTop = mem_map(growSize);
  newTop->id = currId;
  currId++;
  newTop->length = PACK(WITH_HEADER(growSize), 0, 0);

  //printf("New top length: %d\n", UNPACK_LENGTH(newTop->length));
  
  newTop->nxtHdr = top;
  top->lstHdr = newTop;

  top = newTop;

  newTop->callId = currCallId;
  currCallId++;
    
  current_avail_size = WITH_HEADER(growSize);
  numHdrs++;
  // printf("Called grow\n");
}

void writeToFile(){
  //printf("Writing File. Num Hdrs: %d\n", numHdrs);
  FILE* fp = fopen("heapState.txt", "w+");  

  Header* currHdr = top;

  fprintf(fp, "-Heap State-\nMaximum Avail Size: %d\tNum Hdrs: %d\n", current_avail_size, numHdrs);
  int i;
  for(i = 0; i < numHdrs; i++){
    //printf("%d ", i);
    fprintf(fp, "%u - Curr: %p\t\tLength: %u\t\tAlloc: %u\t\tCallId: %u\t\tIsLast: %u\t\t Last Header: %p\t\t", currHdr->id, currHdr, UNPACK_LENGTH(currHdr->length), UNPACK_ALLOC(currHdr->length), currHdr->callId, UNPACK_LAST(currHdr->length), currHdr->lstHdr);

    if(i == numHdrs-1 && UNPACK_LAST(currHdr->length) != 1){
      //printf("Last header not marked as last\n");
      //exit(1);
    }
    
    if(currHdr->nxtHdr != NULL){
      fprintf(fp, "Nxt: %p\n", currHdr->nxtHdr);
      currHdr = (Header*)currHdr->nxtHdr;
    }
      
  }

  fclose(fp);
}
