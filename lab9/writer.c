#include <stdio.h>
#include <stdlib.h>

// write  (int fd, void* src, size_t bytes)

// Write 5 random bytes to stdout, 
// sleep for [0 .. 2] seconds between each
int main()
{
  srand(time(NULL));

  char hi=rand()%256;
  char hi2=rand()%256;
  char hi3=rand()%256;
  char hi4=rand()%256;
  char hi5=rand()%256;
  
  write(1,&hi,1);
  sleep(rand()%3);
  write(1,&hi2,1);
  sleep(rand()%3);
  write(1,&hi3,1);
  sleep(rand()%3);
  write(1,&hi4,1);
  sleep(rand()%3);
  write(1,&hi5,1);

}
