#include "defs.h"
#include "syscall.h"
#pragma GCC diagnostic ignored "-Wunused-function"

static uint64
read_int(int n)
{
  int i;
  argint(n, &i);
  return i;
}

static void *
read_ptr(int n)
{
  // TODO: read an argument as a pointer. wait() in sysproc.c
  // is a good example of doing this.
  uint64 addr;
  argaddr(n, &addr);
  return (void *)addr;
}

static char *
read_str(int n)
{
  // TODO: read an argument as a string. open() or mkdir() in sysfile.c 
  // are good examples of doing this.
  static char buf[128];
  if (argstr(n, buf, sizeof(buf)) < 0)
  	return "(null)";
  return buf;
}

void
strace(struct proc *p, int syscall_num, uint64 ret_val)
{
  // printf("[strace] syscall: %d\n", syscall_num);
  // This will drop the raw syscall output here, so it's commented out for
  // now... we need to update this function so that it prints system call
  // tracing information (based on the syscall_num passed in).
  switch(syscall_num){
	case SYS_fork:
  printf("[%d|%s] fork() = %ld\n", p->pid, p->name,   ret_val);
 break;
case SYS_exit:
  printf("[%d|%s] exit(int = %ld) = %ld\n", p->pid, p->name, read_int(0),   ret_val);
 break;
case SYS_wait:
  printf("[%d|%s] wait(status = %p) = %ld\n", p->pid, p->name, read_ptr(0),   ret_val);
 break;
case SYS_pipe:
  printf("[%d|%s] pipe(pipefd = %p) = %ld\n", p->pid, p->name, read_ptr(0),   ret_val);
 break;
case SYS_read:
  printf("[%d|%s] read(fd = %ld, buf = %p, count = %ld) = %ld\n", p->pid, p->name, read_int(0), read_ptr(1), read_int(2),   ret_val);
 break;
case SYS_write:
  printf("[%d|%s] write(fd = %ld, buf = %p, count = %ld) = %ld\n", p->pid, p->name, read_int(0), read_ptr(1), read_int(2),   ret_val);
 break;
case SYS_close:
  printf("[%d|%s] close(fd = %ld) = %ld\n", p->pid, p->name, read_int(0),   ret_val);
 break;
case SYS_kill:
  printf("[%d|%s] kill(pid = %ld) = %ld\n", p->pid, p->name, read_int(0),   ret_val);
 break;
case SYS_exec:
  printf("[%d|%s] exec(path = %p, argv = %p) = %ld\n", p->pid, p->name, read_ptr(0), read_ptr(1),   ret_val);
 break;
case SYS_open:
  printf("[%d|%s] open(path = %s, mode = %ld) = %ld\n", p->pid, p->name, read_str(0), read_int(1),   ret_val);
 break;
case SYS_mknod:
  printf("[%d|%s] mknod(path = %s, major = %ld, minor = %ld) = %ld\n", p->pid, p->name, read_str(0), read_int(1), read_int(2),   ret_val);
 break;
case SYS_unlink:
  printf("[%d|%s] unlink(path = %s) = %ld\n", p->pid, p->name, read_str(0),   ret_val);
 break;
case SYS_fstat:
  printf("[%d|%s] fstat(fd = %ld, st = %p) = %ld\n", p->pid, p->name, read_int(0), read_ptr(1),   ret_val);
 break;
case SYS_link:
  printf("[%d|%s] link(old_path = %s, new_path = %s) = %ld\n", p->pid, p->name, read_str(0), read_str(1),   ret_val);
 break;
case SYS_mkdir:
  printf("[%d|%s] mkdir(path = %s) = %ld\n", p->pid, p->name, read_str(0),   ret_val);
 break;
case SYS_chdir:
  printf("[%d|%s] chdir(path = %s) = %ld\n", p->pid, p->name, read_str(0),   ret_val);
 break;
case SYS_dup:
  printf("[%d|%s] dup(fd = %ld) = %ld\n", p->pid, p->name, read_int(0),   ret_val);
 break;
case SYS_getpid:
  printf("[%d|%s] getpid() = %ld\n", p->pid, p->name,   ret_val);
 break;
case SYS_sbrk:
  printf("[%d|%s] sbrk(int = %ld) = %p\n", p->pid, p->name, read_int(0),  (void *) ret_val);
 break;
case SYS_pause:
  printf("[%d|%s] pause(int = %ld) = %ld\n", p->pid, p->name, read_int(0),   ret_val);
 break;
case SYS_uptime:
  printf("[%d|%s] uptime() = %ld\n", p->pid, p->name,   ret_val);
 break;
case SYS_shutdown:
  printf("[%d|%s] shutdown() = %ld\n", p->pid, p->name,   ret_val);
 break;
case SYS_reboot:
  printf("[%d|%s] reboot() = %ld\n", p->pid, p->name,   ret_val);
 break;
case SYS_clock:
  printf("[%d|%s] clock() = %ld\n", p->pid, p->name,   ret_val);
 break;
case SYS_strace_on:
  printf("[%d|%s] strace_on() = %ld\n", p->pid, p->name,   ret_val);
 break;
case SYS_wait2:
  printf("[%d|%s] wait2(status = %p, counter = %p) = %ld\n", p->pid, p->name, read_ptr(0), read_ptr(1),   ret_val);
 break;
  }
}
