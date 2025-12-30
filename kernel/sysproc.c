#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait2(p, 0);
}

uint64
sys_wait2(void)
{
	uint64 p, p2;
	argaddr(0, &p);
	argaddr(1, &p2);
	return kwait2(p, p2);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if(t == SBRK_EAGER || n < 0) {
    if(growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if(addr + n < addr)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// shutdown
uint64
sys_shutdown(void)
{
	volatile uint32 *test_dev = (uint32 *) VIRT_TEST;
	*test_dev = 0x5555;	

	return 0;
}

// reboot
uint64
sys_reboot(void)
{
	volatile uint32 *test_dev = (uint32 *) VIRT_TEST;
	*test_dev = 0x7777;

	return 0;
}

uint64
sys_clock(void)
{
	volatile uint32 *base = (uint32*)GOLDFISH;

	uint32 first = base[0];   // low 32 bits
	uint32 second = base[1];  // high 32 

	return ((uint64)second << 32) | first;
}

uint64
sys_strace_on(void)
{
	struct proc *p = myproc();
	p->tracing = 1;
	return 0;
}

uint64
sys_nice(void)
{
	int n;
	argint(0, &n);
	
	if (n < 0)  n = 0;
	if (n > 3) n = 3;

	struct proc *p = myproc();
	acquire(&p->lock);
	
	p->nice = n;
	p->priority = 3 - n;
	release(&p->lock);
	
	return p->priority;
}

uint64
sys_freemem(void)
{
  uint64 pages = kfreepages();
  return pages * (PGSIZE / 1024);
}

uint64
sys_mmap(void)
{
  struct proc *p = myproc();

  void *pa = kalloc();
  if(pa == 0)
    return (uint64)-1;

  memset(pa, 0, PGSIZE);

  uint64 va = p->mmap - PGSIZE;

  if(mappages(p->pagetable, va, PGSIZE, (uint64)pa,
              PTE_R | PTE_W | PTE_U) < 0){
    kfree(pa);
    return (uint64)-1;
  }

  if(walkaddr(p->pagetable, va) != (uint64)pa)
    panic("sys_mmap: mapping error");

  p->mmap = va;
  p->mmap_pages++;

  return va;
}
