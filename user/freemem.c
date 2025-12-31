#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  int k = freemem();
  printf("%d KiB\n", k);
  exit(0);
}
