#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(2, "usage: nice <0..3> cmd [args...]\n");
    exit(1);
  }
  int n = atoi(argv[1]);
  if (n < 0)
  	n = 0;
  if (n > 3)
  	n = 3;

  int pid = fork();

  if (pid < 0) {
  	fprintf(2, "nice: fork failed\n");
  	exit(1);
  }

  if (pid == 0) {
  	nice(n);
  	exec(argv[2], &argv[2]);
  	fprintf(2, "nice: exec %s failed\n", argv[2]);
  	exit(1);
  }

  wait(0);
  exit(0);
}
