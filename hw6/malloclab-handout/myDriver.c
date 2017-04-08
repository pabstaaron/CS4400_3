#include "mm.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// A simple driver application for mm_malloc
int main(int argc, char **argv){
  printf("Begining driver execution\n");

  mm_init();

  
  for(int j = 0; j < 1; j++){
    int numInts = 1000;
  
    int* m = mm_malloc(numInts * sizeof(int)); // Grab an array of 4 ints
    //printf("Malloc call complete\n");
  
    for(int i = 0; i < numInts; i++){
      //printf("%d\n", i+1);
      m[i] = i+1;
    }
  
    /* for(int i = 0; i < numInts; i++){ */
    /*   printf("%d, ", m[i]); */
    /* } */
    //printf("\n");

    //mm_free(m);
  }
  
  return 0;
}
