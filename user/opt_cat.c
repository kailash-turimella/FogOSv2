#include "kernel/fcntl.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static int
fgets3(char *out, int max, int fd)
{
  enum { CAP = 8192 };
  static char storage[CAP];
  static int  r = 0, w = 0;
  int outi = 0;

  for (;;) {
    if (r >= w) {
      r = w = 0;
      int n = read(fd, storage, CAP);
      if (n <= 0) {
        if (outi == 0) return 0;
        out[outi] = '\0';
        return outi;
      }
      w = n;
    }

    while (r < w && outi + 1 < max) {
      char c = storage[r++];
      out[outi++] = c;
      if (c == '\n' || c == '\r') {
        out[outi] = '\0';
        return outi;
      }
    }

    if (outi + 1 >= max) {
      out[outi] = '\0';
      return outi;
    }
  }
}


int
main(int argc, char *argv[])
{
  if (argc <= 1) {
    fprintf(2, "Usage: %s filename\n", argv[0]);
    return 1;
  }

  int fd = open(argv[1], O_RDONLY);
  char buf[128];
  int line_count = 0;
  while (fgets3(buf, 128, fd) > 0 ) {
    printf("Line %d: %s", line_count++, buf);
  }

  return 0;
}
