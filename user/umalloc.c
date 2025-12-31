#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define NULL 0
#define PAGE_SIZE 4096
#define MIN_SPLIT_BLOCK_SIZE (sizeof(struct mem_block) + 16)


#define NAME_LEN 16

#define BLOCK_HEADER_SIZE sizeof(struct mem_block)


/* If we haven't passed -DDEBUG=1 to gcc, then this will be set to 0: */
#ifndef DEBUG
#define DEBUG 1
#endif


// Real allocator block metadata
struct mem_block {
  char name[8];              
  uint64 size;                                  
  struct mem_block *next;    
  struct mem_block *prev;    
};


struct mem_block *head;
struct mem_block *tail;

// Free space management algorithms
enum {
  FSM_FIRST_FIT = 0,
  FSM_BEST_FIT  = 1,
  FSM_WORST_FIT = 2,
};

static int current_fsm = FSM_FIRST_FIT; 

// Our memory:
// +----------+   <--- base
// |          |
// |          |
// |          |   <--- bound
// |          |
// |          |
// |          |
// |          |
// |          |
// |          |
// |          |
// +----------+
//
// malloc -> ask kernel for more memory
// free   -> tell kernel to shrink our memory
//
// "Hello Kernel, can I have 32 bytes of space please?"    (sbrk)
// -> allocate 1 page of memory
// -> hand us 32 bytes of it
// "Hello Kernel, can I have 32 bytes of space please?"    (sbrk)
// -> it would be bad to allocate another page here
//      maybe the previous thing is not used anymore
//      maybe we want to make use of the 4096 - 32 bytes left from the first
//      page
//
//
// So, our allocator is going to allocate *pages* of memory at a time.
// One page can hold multiple blocks of memory:
// +-------------------------------------------+
// |     |             |       | |       |     | 1 page (4096 bytes)
// +-------------------------------------------+
//       ^             ^       ^ ^       ^

// The 'size' of a block:
// 1101 .... 0010 0000 0001  <- free
// 1101 .... 1110 0000 0000  <- used
//
// Why we can do this:
//  - We are going to round everything up so that sizes are multiples of 16
//  - That means the smallest size we can have is 16 (0001 0000)
//  - Note how the last 4 bits are never used... so we store the free bit there!

// Get the block header from a data pointer
struct mem_block* get_header(void* ptr) {
    return (struct mem_block*)((char*)ptr - BLOCK_HEADER_SIZE);
}

static uint
align16(uint size)
{
  return (size + 15) & ~0x0F;
}

//
void set_used(struct mem_block *block)
{
  // turn off the least significant bit
  block->size = block->size & (~0x01);
}

void set_free(struct mem_block *block)
{
  // turn on the least significant bit
  block->size = block->size | 0x01;
}

int is_free(struct mem_block *block)
{
  // check whether the last bit is set or not
  return block->size & 0x01;
}

uint get_size(struct mem_block *block)
{
  // ignore the free bit when retrieving the size
  return block->size & (~0x01);
}

/* Given an existing block, this function should split it in two */
// How this works:
// Maybe we allocate 1 byte
// That gets turned into 16 (that's our minimum size)
// But then that gets turned into 4096 (our page size)
struct mem_block *
split(struct mem_block *block, uint total_sz)
{
  if (block == NULL) {
    return NULL;
  }

  if (!is_free(block)) {
    return NULL;
  }

  uint old_size = get_size(block);

  // Don't split if remainder would be too small
  if (old_size <= total_sz + MIN_SPLIT_BLOCK_SIZE) {
    return NULL;
  }

  char *base = (char *) block;
  struct mem_block *new_block = (struct mem_block *) (base + total_sz);

  uint remainder_size = old_size - total_sz;

  new_block->size = remainder_size;
  set_free(new_block);

  new_block->next = block->next;
  new_block->prev = block;

  if (block->next != NULL) {
    block->next->prev = new_block;
  } else {
    // if block was tail, new_block becomes new tail
    tail = new_block;
  }

  block->size = total_sz;
  set_used(block);
  block->next = new_block;

  return new_block;
}

// We have some space like this:
// +-----------------------------+
// | mem_block | data            |
// +-----------------------------+
// ^           ^
// |           |
// |           +--- actual data pointer (for user space programs)
// |
// +----- header
//

// find first free whose size is big enough
static struct mem_block*
find_free_first_fit(uint total_sz)
{
	struct mem_block *curr = head;

	while (curr != NULL) {
		if (is_free(curr) && get_size(curr) >= total_sz) {
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}

// find free block with smallest size  >= total_sz
static struct mem_block*
find_free_best_fit(uint total_sz)
{
	struct mem_block *curr = head;
	struct mem_block *best = NULL;

	while (curr != NULL) {
		if (is_free(curr) && get_size(curr) >= total_sz) {
			if (best == NULL || get_size(curr) < get_size(best)) {
				best = curr;
			}
		}
		curr = curr->next;
	}

	return best;
}

static struct mem_block*
find_free_worst_fit(uint total_sz)
{
	struct mem_block *curr = head;
	struct mem_block *worst = NULL;

	while (curr != NULL) {
		if (is_free(curr) && get_size(curr) >= total_sz) {
			if (worst == NULL || get_size(curr) > get_size(worst)) {
				worst = curr;
			}
		}
		curr = curr->next;
	}
	return worst;
}

static struct mem_block *
find_free_block(uint total_sz)
{
	switch (current_fsm) {
		case FSM_BEST_FIT:
			return find_free_best_fit(total_sz);
		case FSM_WORST_FIT:
			return find_free_worst_fit(total_sz);
		case FSM_FIRST_FIT:
		default:
			return find_free_first_fit(total_sz);
	}
}


void *
malloc(uint size)
{
  if (size == 0) {
    return NULL;
  }

  uint aligned = align16(size);
  uint total_sz = BLOCK_HEADER_SIZE + aligned;

  //  try to find a suitable free block first
  struct mem_block *block = find_free_block(total_sz);
  if (block != NULL) {
    // take space from this free block
    split(block, total_sz);
    set_used(block);
    return (void *)(block + 1);
  }

  // no suitable free block, request a new chunk from the OS
  uint request = total_sz;
  if (request < PAGE_SIZE) {
    request = PAGE_SIZE;
  }

  block = (struct mem_block *) sbrk(request);
  if ((long)block == -1) {
    return NULL;
  }

  // treat the whole new region as a single free block
  block->size = request;
  set_free(block);
  block->next = NULL;
  block->prev = tail;

  if (head == NULL) {
    head = block;
    tail = block;
  } else {
    tail->next = block;
    tail = block;
  }

  // now carve out the requested block from this new free block
  struct mem_block *alloc_block = block;
  split(alloc_block, total_sz);
  set_used(alloc_block);

  return (void *)(alloc_block + 1);
}
// linked list helpers:
// * add a new block
// * remove a block
// * AND make sure you can handle both the (1) block list, and (2) free list

void
free(void *ptr)
{

  if (ptr == NULL) {
  	return;
  }

  // ptr points to the user data; header is right before that
  struct mem_block *block = ((struct mem_block *)ptr) -1;

  if (is_free(block)) {
  	return;
  }

  set_free(block);
 
}

// This is great because we can allocate arrays of things easily
// And they're zeroed out
// This function is perfect and never needs to be changed
void *
calloc(uint nmemb, uint size)
{
  char *mem = malloc(nmemb * size);
  if (mem == NULL) {
    return NULL;
  }
  memset(mem, 0, nmemb * size);
  return mem;
}

// takes a pointer to a previous allocation and resizes it (shrink / grow)
void *
realloc(void *ptr, uint size)
{
	if (ptr == NULL) {
		return malloc(size);
	}

	if (size == 0) {
		free(ptr);
		return NULL;
	}

	struct mem_block *old_block = ((struct mem_block *) ptr) - 1;
	uint old_total_size = old_block->size;
	uint old_payload_size = old_total_size - sizeof(struct mem_block);

	void *new_ptr = malloc(size);
	if (new_ptr == NULL) {
		return NULL;
	}

	uint copy_size = old_payload_size < size ? old_payload_size : size;
	memmove(new_ptr, ptr, copy_size);

	free(ptr);

	return new_ptr;
}

void
malloc_print(void)
{
	struct mem_block *curr;

	printf("--Current Memory State --\n");

	curr = head;
	while (curr != NULL) {
		char *start = (char *)curr;
		char *end = start + get_size(curr);
		const char *status = is_free(curr) ? "FREE" : "USED";

		printf("[BLOCK %p-%p] %d\t[%s]\n",
				start,
				end,
				get_size(curr),
				status);
				
		curr = curr->next;
	}

	printf("\n-- Free List --\n");

	curr = head;
	int first = 1;
	while (curr != NULL) {
		if (is_free(curr)) {
			if (!first) {
				printf(" -> ");
			}
			printf("[%p]", curr);
			first = 0;
		}
		curr = curr->next;
	}
	if (!first) {
		printf(" -> ");
	}
	printf("NULL\n");
}

void
malloc_setfsm(int algo)
{
  if (algo == FSM_FIRST_FIT ||
      algo == FSM_BEST_FIT  ||
      algo == FSM_WORST_FIT) {
    current_fsm = algo;
  }
}

void malloc_name(void *ptr, const char *name) {
    struct mem_block *block = get_header(ptr);
    int i;
    for (i = 0; i < 7 && name[i] != 0; i++) {
        block->name[i] = name[i];
    }
    block->name[i] = 0;
}
