#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  char buf[128];
  getcwd(buf, 128);
  printf("%s\n", buf);
  return 0;
}
