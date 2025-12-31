#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
	uint64 ns = clock();                   // call the clock function
	uint64 sec64 = ns / 1000000000ULL;     // convert to seconds
	printf("%d\n", (int)sec64);
	exit(0);
 
}
