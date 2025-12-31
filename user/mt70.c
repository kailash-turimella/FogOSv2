#include "kernel/types.h"
#include "user/user.h"

#pragma GCC diagnostic ignored "-Wunused-variable"

int main(void)
{
//  malloc_setfsm(FSM_FIRST_FIT);

  char *a = malloc(132);
  char *b = malloc(42);  /* Will be deleted */
  char *c = malloc(132);
  char *d = malloc(132); /* Will be deleted */
  char *e = malloc(132);
  char *f = malloc(3132);

  /* These allocations total 4096 bytes after alignment is included. */

  malloc_name(a, "A");
  malloc_name(b, "B");
  malloc_name(c, "C");
  malloc_name(d, "D");
  malloc_name(e, "E");
  malloc_name(f, "F");

  free(b);
  free(d);

  /**
   * The following allocation will choose:
   * - First fit: free space at the end of the page (unnamed, no variable)
   * - Best fit:  B
   * - Worst fit: D
   */
  char *g = malloc(42);

  malloc_name(g, "G");

  malloc_print();

  /**
   * Things to check:
   * - correct alignment (evenly divisible by 16)
   * - free block sizes are correct (make sure not off by one)
   * - end of last block's memory address - first address = 4096
   */

  return 0;
}
