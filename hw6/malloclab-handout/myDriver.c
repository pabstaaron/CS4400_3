#include "mm.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// A simple driver application for mm_malloc
int main(int argc, char **argv){
  printf("Begining driver execution\n");

  mm_init();

  int upper = 1000;
  for(int j = 0; j < upper; j++){
    int numInts = upper-j;
  
    int* m = mm_malloc(numInts * sizeof(int)); // Grab an array of 4 ints
    printf("Malloc call complete: %d\n\n", j);
  
    for(int i = 0; i < numInts; i++){
      //printf("%d\n", i+1);
      m[i] = i+1;
    }
  
    /* for(int i = 0; i < numInts; i++){ */
    /*   printf("%d, ", m[i]); */ 
    /* } */
    //printf("\n");

    /* if(j % 2 == 0){ */
    /*   mm_free(m); */
    /*   printf("Free call complete\n"); */
    /* } */
  }
  
  return 0;
}
