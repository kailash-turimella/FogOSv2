#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  char *p = (char*)mmap();
  if (!p) {
    printf("mmap failed\n");
    exit(1);
  }

  strcpy(p, "hello from mmap");
  printf("%s\n", p);
  exit(0);
}
