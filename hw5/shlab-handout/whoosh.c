/* This is the main file for the `whoosh` interpreter and the part
   that you modify. */

#include <stdlib.h>
#include <stdio.h>
#include "csapp.h"
#include "ast.h"
#include "fail.h"

static void run_script(script *scr);
static void run_group(script_group *group);
static void run_command(script_command *command);
static void set_var(script_var *var, int new_value);

/* You probably shouldn't change main at all. */

int main(int argc, char **argv) {
  script *scr;
  
  if ((argc != 1) && (argc != 2)) {
    fprintf(stderr, "usage: %s [<script-file>]\n", argv[0]);
    exit(1);
  }

  scr = parse_script_file((argc > 1) ? argv[1] : NULL);

  run_script(scr);

  return 0;
}

static void run_script(script *scr) {
  //printf("Num groups: %d", scr->num_groups);
  if (scr->num_groups == 1) {
    run_group(&scr->groups[0]);
  } else {
    for(int i = 0; i < scr->num_groups; i++)
      run_group(&scr->groups[i]);
  }
}

static void run_group(script_group *group) {
  
  /* You'll have to make run_group do better than this, too */
  /* if (group->repeats != 1) */
  /*   fail("only repeat 1 supported"); */
  if (group->result_to != NULL) 
    fail("setting variables not supported");

  /* if (group->num_commands == 1) { */
  /*   run_command(&group->commands[0]); */
  /* } else { */
  /*   /\* And here *\/ */
  /*   fail("only 1 command supported"); */
  /* } */

  int** fd;
  fd = (int**)malloc((group->repeats * group->num_commands)*sizeof(int*));
  for(int i = 0; i < group->repeats * group->num_commands; i++){
    fd[i] = malloc(2*sizeof(int));
  } 
  
  if(group->mode == GROUP_AND){
    for(int j = 0; j < group->repeats; j++) // Fire off a set for each repeat of the grp
      for(int i = 0; i < group->num_commands; i++){
	//Fire off a new process for each commmand
	if(j == 0 && i == 0){
	  // First process
	  pipe(fd[0]);

	  if(fork() == 0){
	    printf("Redirecting stdout to %d\n", fd[0][1]);
	    dup2(fd[0][1], 1);
	    
	    //close(fd[0][0]);
	    //close(fd[0][1]);
	    run_command(&group->commands[0]);

	    
	  }
	}
	else if(i == group->num_commands-1){
	  // Last Proc
	  if(fork() == 0){
	    printf("Last: Redirecting stdin to %d\n", fd[j+i-1][0]);
	    dup2(fd[j+i-1][0], 0);

	    close(fd[j+i-1][1]);
	    //close(fd[j+i-1][0]);
	    run_command(&group->commands[i]);

	  }
	}
	else{
	  // Intermidiate proc
	  pipe(fd[j+i]);

	  if(fork() == 0){
	    printf("Intermidiate: Redirecting stdin to %d\n", fd[j+i-1][0]);
	    dup2(fd[j+i-1][0], 0); // Grab the tail end of the last pipe
	    printf("Intermidiate: Redirecting stdout to %d\n", fd[j+i][1]);
	    dup2(fd[j+i][1], 1); // Grab the head of the new pipe

	    close(fd[j+i-1][1]);
	    //close(fd[j+i][0]);
	    //close(fd[j+i-1][0]);
	    //close(fd[j+i][1]);
	    run_command(&group->commands[i]);
	  }
	}
      }
  }
  else{
    fail("group_and and group_or not supported");
  }

  for(int i = 0; i < group->repeats * group->num_commands-1; i++){
    //close(fd[i][0]);
    printf("Closing %d\n", fd[i][1]);
    close(fd[i][1]);
  }

  int status;
  int i;
  for(i = 0; i < group->repeats * group->num_commands; i++){
    wait(&status);
  }
}

/* This run_command function is a good start, but note that it runs
   the command as a replacement for the `whoosh` script, instead of
   creating a new process. */

static void run_command(script_command *command) {
  const char **argv;
  int i;

  argv = malloc(sizeof(char *) * (command->num_arguments + 2));
  argv[0] = command->program;
  
  for (i = 0; i < command->num_arguments; i++) {
    if (command->arguments[i].kind == ARGUMENT_LITERAL)
      argv[i+1] = command->arguments[i].u.literal;
    else
      argv[i+1] = command->arguments[i].u.var->value;
  }
  
  argv[command->num_arguments + 1] = NULL;

  Execve(argv[0], (char * const *)argv, environ);

  free(argv);
}

/* You'll likely want to use this set_var function for converting a
   numeric value to a string and installing it as a variable's
   value: */

static void set_var(script_var *var, int new_value) {
  char buffer[32];
  free((void *)var->value);
  snprintf(buffer, sizeof(buffer), "%d", new_value);
  var->value = strdup(buffer);
}
