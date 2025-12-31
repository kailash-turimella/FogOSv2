#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[]) {
	if(argc < 2) {
		fprintf(2, "Usage: tracer command...\n");
		exit(1);
	}

	if (fork() == 0) {
		strace_on();
		exec(argv[1], argv + 1);
		fprintf(2, "tracer: exec %s failed\n", argv[1]);
		exit(1);
	}
	wait(0);

	exit(0);
}
