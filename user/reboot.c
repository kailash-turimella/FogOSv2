#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char **argv)
{
	printf("Rebooting...\n");
	reboot();

	// error message just in case
	fprintf(2, "reboot failed!\n");
	exit(1);
}
