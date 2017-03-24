#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// pipe  (int* fds)
//
// fds should be an array of 2
// creates a new open "file"
// with fds[1] writing to that file
// and  fds[0] reading from that file


// dup2 (int old_fd, int new_fd)
// 
// sets new_fd so that it refers to the same
// file as old_fd

// Create two children, one to exec reader, one to exec writer
// Set the standard input of reader 
// to the same "file" as the standard output of writer
int main(int argc, char** argv)
{
  if(argc < 3){
    printf("Not enough args!\n");
    exit(1);
  }
  
  char* proca_args[] = {argv[1], NULL};
  char* procb_args[] = {argv[2], NULL};
  char* env[] = {NULL};

  char* procc_args[2];
  if(argc > 3){
    procc_args[0] = argv[3];
  }
  
  int fd[2];
  pipe(fd);
 
  // set up procA
  if (fork() == 0) 
  {
    // redirect stdout to the writing end of the pipe
    dup2(fd[1], 1);
    close(fd[0]);
    execve(proca_args[0], proca_args, env);
  }

  // set up procB
  if (fork() == 0) 
  {    
    // redirect stdin to the reading end of the pipe
    dup2(fd[0], 0);
    close(fd[1]);
    execve(procb_args[0], procb_args, env);
  }

  // set up procC
  if(argc > 3) {
    pipe(fd);

    if(fork() == 0){
      dup2(fd[0], 0);
      close(fd[1]);
      execve(procc_args[0], procc_args, env);
    }
  }
  
  close(fd[0]);
  close(fd[1]);

  // Wait for two children
  int status;
  wait(&status);
  wait(&status);
  if(argc > 3)
    wait(&status);
}
