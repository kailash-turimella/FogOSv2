#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char **argv)
{
	printf("Shutting down...\n");
	shutdown();

	// error message just in case
	fprintf(2, "shutdown failed!\n");
	exit(1);
}
