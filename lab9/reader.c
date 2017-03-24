#include <stdio.h>
#include <time.h>

// Helper to print the hour:min:sec
void print_time();


// read  (int fd, void* dest, size_t bytes)
//
// if EOF has not been reached, waits until data is available
// when data is available, returns number of bytes read
// if EOF reached, returns 0 immediately

// continually read one byte at a time
// print its arrival time, followed by the byte
int main()
{
  char readbyte;
  while (1) {
    int readnum = read(0, &readbyte, 1);
    if (readnum == 0) {
      break;
    } else {
      print_time();
      printf(": %d\n", readbyte);
    }
  }

  printf("reader received EOF\n");

}




void print_time()
{
  time_t t = time(0);
  struct tm* tm;
  tm = localtime(&t);

  printf("%d:%d:%d", tm->tm_hour, tm->tm_min, tm->tm_sec);
}
