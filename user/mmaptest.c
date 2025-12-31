#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  char *a = mmap();   
  char *b = mmap();    
  char *c = mmap();   

  strcpy(a, "a: hello");
  strcpy(b, "b: world");
  strcpy(c, "c: original");

  printf("parent original:\n%s\n%s\n%s\n\n", a, b, c);


  int pid = fork();

  if(pid == 0){
 
    printf("child sees after fork:\n%s\n%s\n%s\n\n", a, b, c);

    // child modifies first two pages
    strcpy(a, "a: spurs");
    strcpy(b, "b: child changed world");

	// forks and makes another child (grandchild)
    int pid2 = fork();
    if(pid2 == 0){
      printf("grandchild sees:\n%s\n%s\n%s\n\n", a, b, c);

      // grandchild modifies the 3rd page
      strcpy(c, "C: grandchild updated this");

      printf("grandchild done.\n");
      exit(0);
    }

    // child waits for grandchild to finish updating page C
    wait(0);

    printf("child after grandchild:\n%s\n%s\n%s\n\n", a, b, c);
    exit(0);
  }
  
  wait(0);

  printf("parent final:\n%s\n%s\n%s\n\n", a, b, c);

  exit(0);
}
