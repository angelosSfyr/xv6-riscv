// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

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
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

int reference_counter[PHYSTOP / PGSIZE];

// called by kinit to acquire memory for freelist with kfree
void freerange(void *pa_start, void *pa_end)
{
  // PGROUNDUP ensures that we free only aligned physical adresses
  char *p = (char *)PGROUNDUP((uint64)pa_start);
  for (; p + PGSIZE <= (char *)pa_end; p += PGSIZE)
  {
    // initialize reference counter for every page
    int page_number = ((uint64)p)/ PGSIZE;
    reference_counter[page_number] = 1; // kfree decreases reference counter
    kfree(p);
  }
}

// increases reference counter
void add_reference(int page_number)
{
  acquire(&kmem.lock);
  reference_counter[page_number]++;
  release(&kmem.lock);
}

// decreases reference counter and returns it
int remove_reference(int page_number)
{
  acquire(&kmem.lock);
  reference_counter[page_number]--;
  release(&kmem.lock);
  return reference_counter[page_number];
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r  = (struct run *)pa;
  if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");
  int page_number = (uint64)r / PGSIZE;
  
  if (remove_reference(page_number) == 0)
  {
    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);
    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  }
  return;
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  // each free page's list element is a struct run
  struct run *r;
  acquire(&kmem.lock);
  r = kmem.freelist;
  if (r)
  {
    int page_number = (uint64)r / PGSIZE;
    reference_counter[page_number] = 1; // pages gets attached to a process --> increase counter
    kmem.freelist = r->next;
  }
  release(&kmem.lock);

  if (r)
    memset((char *)r, 5, PGSIZE); // fill with junk
  return (void *)r;
}
