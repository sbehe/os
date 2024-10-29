#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "fcntl.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

addr_t
sys_sbrk(void)
{
  addr_t addr;
  addr_t n;

  argaddr(0, &n);
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// Fetch the nth word-sized system call argument as a file descriptor
// and return both the descriptor and the corresponding struct file.
int
argfd(int n, int *pfd, struct file **pf);

#define MMAP_EAGER_START (char *)0x0000400000000000
#define MMAP_LAZY_START (char *)0x0000400000000000

#define MMAP_FAILED ((~0lu))
addr_t eager_mmap ()
{
  int fd, flags;
  int bytes_read = 0;
  struct file * fl;
  char *mem;
  if(argfd(0, &fd, &fl) < 0)
    return MMAP_FAILED;

  if (proc->mmapcount == 0) {
    proc->mmaptop = MMAP_EAGER_START;
  }
  proc->mmaps[proc->mmapcount].fd = fd;
  proc->mmaps[proc->mmapcount].start = (addr_t)proc->mmaptop;
  flags = PTE_W | PTE_U;
  do
  {
    mem = kalloc();
    mappages (proc->pgdir, proc->mmaptop, PGSIZE, V2P(mem), flags);
    bytes_read = fileread (fl, proc->mmaptop, PGSIZE);
    if (bytes_read > 0) {
      proc->mmaptop += PGSIZE;
    }
  } while (bytes_read == PGSIZE);

  fl->off = 0;
  proc->mmapcount++;
  return (addr_t)proc->mmaps[proc->mmapcount - 1].start;
}
addr_t lazy_mmap ()
{
  int fd, flags;
  int bytes_read = 0;
  struct file * fl;
  char *mem;
  if(argfd(0, &fd, &fl) < 0)
    return MMAP_FAILED;

  if (proc->mmapcount == 0) {
    proc->mmaptop = MMAP_LAZY_START;
  }
  proc->mmaps[proc->mmapcount].fd = fd;
  proc->mmaps[proc->mmapcount].start = (addr_t)proc->mmaptop;
  proc->mmapcount++;
  return (addr_t)proc->mmaps[proc->mmapcount - 1].start;
}
  addr_t
sys_mmap(void)
{
  int param;
  if(argint(1,&param) < 0)
    return MMAP_FAILED;
  if (param == 0) {
    return eager_mmap ();
  } else {
    return lazy_mmap ();
  }
}

  int
handle_pagefault(addr_t va)
{
  // TODO: your code here
  return 0;
}
