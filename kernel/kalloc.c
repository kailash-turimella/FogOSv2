// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

// Store a reference count for each physical page
static int ref_count[(PHYSTOP - KERNBASE) / PGSIZE];

// Convert physical address to index in reference count array
static inline int
pa2idx(uint64 pa)
{
	return (pa - KERNBASE) / PGSIZE;
}

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
incref(uint64 pa)
{
	int idx = pa2idx(pa);
	acquire(&kmem.lock);
	ref_count[idx]++;
	release(&kmem.lock);
}

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);	
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;
  uint64 addr = (uint64)pa;

  if((addr % PGSIZE) != 0 || addr < (uint64)end || addr >= PHYSTOP)
    panic("kfree");

  int idx = pa2idx(addr);

  acquire(&kmem.lock);

  if(ref_count[idx] < 0)
    panic("kfree: ref_count underflow");

  if(ref_count[idx] > 0){
    // normal case: page was allocated by kalloc
    ref_count[idx]--;
    if(ref_count[idx] > 0){
      // still in use somewhere else – don't free yet
      release(&kmem.lock);
      return;
    }
  }

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;
  r->next = kmem.freelist;
  kmem.freelist = r;

  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    ref_count[pa2idx((uint64)r)] = 1;   // << MUST be inside lock
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE);

  return (void*)r;
}	

uint64
kfreepages(void)
{
	uint64 num_pages = 0;
	struct run *r;

	acquire(&kmem.lock);
	r = kmem.freelist;
	while (r) {
		num_pages++;
		r = r->next;
	}
	release(&kmem.lock);

	return num_pages;
}
