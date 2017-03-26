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

  if (group->num_commands == 1) {
    run_command(&group->commands[0]);
  } 
  /* } else { */
  /*   /\* And here *\/ */
  /*   fail("only 1 command supported"); */
  /* } */

  /* int** fd; */
  /* fd = (int**)malloc((group->repeats * group->num_commands)*sizeof(int*)); */
  /* for(int i = 0; i < group->repeats * group->num_commands; i++){ */
  /*   fd[i] = malloc(2*sizeof(int)); */
  /* }  */
  int i, j;

  int fda[2], fdb[2];
  int inOut = 0; // 0 if pipe input, 1 otherwise 
  
  if(group->mode == GROUP_AND || group->mode == GROUP_OR || group->mode == GROUP_SINGLE){
    for(j = 0; j < group->repeats; j++){ // Fire off a set for each repeat of the grp
      inOut = 0;
      for(i = 0; i < group->num_commands; i++){
	//Fire off a new process for each commmand
	if(i == 0){
	  // First process
	  pipe(fda);
	  
	  if(fork() == 0){
	    //printf("Pre-dup2\n");
	    dup2(fda[1], 1);
	    printf("Initial: Redirecting stdin to %d \n", fda[1]);
	    close(fda[0]);
	    close(fda[1]);
	    close(fdb[0]);
	    close(fdb[1]);
	    run_command(&group->commands[0]);
	  }
	}
	else if(i == group->num_commands-1){
	  // Last Proc
	  if(fork() == 0){
	    printf("last: inOut: %d\n", inOut);
	    if(inOut)
	      dup2(fdb[0], 0);
	    else
	      dup2(fda[0], 0);
	    // printf("Last: Redirecting stdout to %d \n", fdb[0]);
	    close(fdb[1]);
	    close(fdb[0]);
	    close(fda[1]);
	    close(fda[0]);
	    
	    run_command(&group->commands[i]);
	  }
	}
	else{
	  // Intermidiate proc
	  if(!inOut)
	    pipe(fdb);
	  else
	    pipe(fda);

	  if(fork() == 0){
	    printf("inter: inOut: %d\n", inOut);
	    if(!inOut){ // Output end of pipe
	      //printf("pre-dup2");
	      dup2(fda[0], 0);
	      dup2(fdb[1], 1);
	      printf("Inter: Redirecting stdout to %d \n", fda[0]);
	      printf("Inter: Redirecting stdin to %d \n", fdb[1]);
	     
	      close(fda[1]);
	      close(fda[0]);
	      close(fdb[1]);
	      close(fdb[0]);
	      //inOut = !inOut; 
	    }
	    else{ // input end of pipe
	      dup2(fdb[0], 0);
	      dup2(fda[1], 1);
	      printf("Redirecting stdin to %d \n", fda[1]);
	      printf("Redirecting stdout to %d \n", fdb[0]);
	      close(fda[0]);
	      close(fda[1]); 
	      close(fdb[0]);
	      close(fdb[1]);
	      //inOut = !inOut;
	    }
	    
	    run_command(&group->commands[i]);
	  }
	  
	  inOut = !inOut;
	}
      }

      close(fda[0]);
      close(fda[1]);

      close(fdb[0]);
      close(fdb[1]);
  
      int status;
      for(i = 0; i < group->repeats * group->num_commands; i++){
	//printf("waiting on %d\n", i);
	wait(&status);
      }
     }
  }
  else{
    fail("group_and and group_or not supported");
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
