#include "kernel/types.h"
#include "user/user.h"

#pragma GCC diagnostic ignored "-Wunused-variable"

void
merge_test(void)
{
  char *start = sbrk(0);
  printf("Starting address: %p\n\n", start);

  void *a = malloc(532);   malloc_name(a, "A");
  void *b = malloc(326);   malloc_name(b, "B");
  void *c = malloc(282);   malloc_name(c, "C");
  void *d = malloc(1032);  malloc_name(d, "D");
  void *e = malloc(432);   malloc_name(e, "E");

  free(b);
  free(c);
  free(a);
  free(d);
  free(e);

  malloc_print();

  printf("\n\n");

  void *f = malloc(632);   malloc_name(f, "F");
  void *g = malloc(182);   malloc_name(f, "G");
  void *h = malloc(9928);  malloc_name(f, "H");

  free(f);
  free(g);
  free(h);

  malloc_print();

  char *end = sbrk(0);
  printf("\n\nEnding address: %p\n", end);

  if (start == end) {
    printf("Success! start == end!\n");
  }
}

void
realloc_test(void)
{
  char *a = malloc(532);    malloc_name(a, "A");
  void *b = malloc(326);    malloc_name(b, "B");
  void *c = malloc(282);    malloc_name(c, "C");
  void *d = malloc(1032);   malloc_name(d, "D");
  void *e = malloc(432);    malloc_name(e, "E");

  /* Does not actually need to be resized */
  a = realloc(a, 542);      malloc_name(a, "R1");

  /* No room, must free + allocate new */
  c = realloc(c, 9632);      malloc_name(c, "R2");

  /* Now that 'c' is free, we can expand 'b' into it */
  b = realloc(b, 632);      malloc_name(b, "R3");

  /* Expand into free space at the end of the region. This will produce a new
     * free block out of the remaining space */
  e = realloc(e, 1312);      malloc_name(e, "R4");

  /* Shrink 'd' down to make a new free block */
  d = realloc(d, 832);      malloc_name(d, "R5");

  /* Shrink 'd' one more time to make sure that the two neighboring free
     * blocks get merged back together */
  d = realloc(d, 632);      malloc_name(d, "R6");

  malloc_print();
}

int
main(void)
{
  printf("---- MERGE TEST ----\n");
  merge_test();
  printf("\n\n");

  printf("---- REALLOC TEST ----\n");
  realloc_test();
  printf("\n\n");

  return 0;
}
