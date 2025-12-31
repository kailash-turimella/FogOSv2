#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(2, "Usage: %s <program> [args...]\n", argv[0]);
		exit(1);
	}

	uint64 start = clock();

	int pid = fork();
	if (pid < 0) {
		fprintf(2, "benchmark: fork failed\n");
		exit(1);
	}
	if (pid == 0) {
		exec(argv[1], &argv[1]);
		fprintf(2, "benchmark: exec %s failed\n", argv[1]);
		exit(1);
	}

	int status;
	int counter;
	
	if (wait2(&status, &counter) < 0) {
		fprintf(2, "benchmark: wait2 failed\n");
		exit(1);
	}

	uint64 end = clock();
	uint64 time_taken = end - start;
	uint64 time_taken_ms = time_taken / 1000000ULL; 

	printf("Benchmark Complete\n");
	printf("Time Elapsed: %d ms\n", (int)time_taken_ms);
	printf("System Calls: %d\n", (int)counter);
	exit(0);
}
