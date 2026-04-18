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

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
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
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

// Allocate a 2MB contiguous and aligned superpage
void *
kalloc_super(void)
{
  struct run *r, *prev;
  struct run *block_start_prev = 0; // Khởi tạo = 0 để fix warning
  struct run *target = 0;
  int count = 0;
  uint64 expected_next = 0;

  acquire(&kmem.lock);

  prev = 0;
  r = kmem.freelist;
  while(r){
    if(count == 0){
      uint64 base = (uint64)r - 511 * PGSIZE;
      // Khối bộ nhớ phải được align đúng 2MB
      if (base % (1<<21) == 0) {
        block_start_prev = prev;
        count = 1;
        expected_next = (uint64)r - PGSIZE;
      }
    } else {
      if ((uint64)r == expected_next) {
        count++;
        expected_next -= PGSIZE;
        if (count == 512) {
          target = r; // Tìm thấy khối 512 trang liên tiếp
          break;
        }
      } else {
        count = 0;
        continue; // Bắt đầu tìm lại từ r hiện tại
      }
    }
    prev = r;
    r = r->next;
  }

  if(count == 512 && target){
    if(block_start_prev)
      block_start_prev->next = target->next;
    else
      kmem.freelist = target->next;
      
    release(&kmem.lock);
    memset((char*)target, 5, 512 * PGSIZE);
    return (void*)target;
  }
  
  release(&kmem.lock);
  return 0;
}