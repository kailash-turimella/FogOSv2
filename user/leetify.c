/**
 * @file leetify.c
 *
 * Scaffolding to create an amazing l337speak generator using only external
 * commands combined with pipelines.
 */

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

/**
 * Represents a command in a pipeline.
 * Note that EITHER stdout_pipe can be true, or a stdout_file can be set.
 *
 * - tokens Array of strings that describe the command to run
 * - stdout_pipe set to true if this command's output should be written to pipe
 * - stdout_file set to a file name if this command's output should be written
 *     to a file
 */
struct command {
  char **tokens;
  int stdout_pipe;
  char *stdout_file;
};

void
execute_pipeline(struct command *cmds, int n)
{

   // go through all the commands
   for (int i = 0; i < n - 1; i++) {

   	int p[2]; // p[0] stdin, p[1] stdout
   	if (pipe(p) < 0) {
   		fprintf(2, "pipe failed\n");
   		exit(1);
   	}
   	
	int pid = fork();
	if (pid < 0) {
		fprintf(2, "fork failed\n");
		exit(1);
	}
	
   	if (pid == 0) {
   		// child

   		// redirect stdout to write end of pipe
   		close(p[0]); // close read
   		close(1);    // redirect stdout -> pipe write end
   		dup(p[1]);
   		close(p[1]);

   		// exec
   		exec(cmds[i].tokens[0], cmds[i].tokens);

   		fprintf(2, "exec %s failed\n", cmds[i].tokens[0]);
   		exit(1);
   	} else {
   		// parent

   		// close write end of pipe
   		close(p[1]);

   		// redirect stdin
   		close(0);
   		dup(p[0]);

   		// close copy of the read end
   		close(p[0]);
   		
   	}
   }
    struct command *final_command = &cmds[n-1];
    if (final_command->stdout_file) {
    	int fd = open(final_command->stdout_file, O_CREATE | O_WRONLY | O_TRUNC);
    	if (fd < 0) {
    		fprintf(2, "open failed for %s\n", final_command->tokens[0]);
    		exit(1);
    	}
    	close(1);
    	dup(fd);
    	close(fd);
    }

    exec(final_command->tokens[0], final_command->tokens);
    fprintf(2, "exec failed: %s\n", final_command->tokens[0]);
    exit(1);
   
}

int
main(int argc, char *argv[])
{
  char *input_file = (char *)0;
  char *output_file = (char *) 0;

  if (argc < 2 || argc > 3) {
    printf("Usage: %s file-to-leetify [output-file]\n", argv[0]);
    return 1;
  }

  input_file = argv[1];

  if (argc == 3) {
    output_file = argv[2];
  }

  printf("Input file: %s\n", input_file);
  if (output_file) {
    printf("Writing to output file: %s\n", output_file);
  }

  // cat input file
  char *command1[] = { "cat", input_file, (char *)0 };
  // tolower
  char *command2[] = { "tolower", (char *)0 };
  // fnr 
  char *command3[] = { "fnr", "the", "teh", "a", "4", "e", "3", "i", "!", (char *)0 };
  // fnr 2
  char *command4[] = { "fnr", "l", "1", "o", "0", "s", "5", (char *)0 };

  struct command cmds[4] = { 0 };
  cmds[0].tokens = command1;  /* What this command runs */
  cmds[0].stdout_pipe = 1;
  cmds[0].stdout_file = (char *)0; /* This command is not writing to a file. */

  cmds[1].tokens = command2;
  cmds[1].stdout_pipe = 1;
  cmds[1].stdout_file = (char *)0; /* This command's output goes to a pipe. */

  cmds[2].tokens = command3;
  cmds[2].stdout_pipe = 1;
  cmds[2].stdout_file = (char *)0;

  cmds[3].tokens = command4;
  cmds[3].stdout_pipe = 0; /* Last command so set stdout_pipe = false */
  cmds[3].stdout_file = output_file;

  int pid = fork();
  if (pid == -1) {
    fprintf(2, "Fork failed\n");
    return 1;
  } else if (pid == 0) {
    execute_pipeline(cmds, 4);
  } else {
    int status;
    wait(&status);
  }

  return 0;
}
