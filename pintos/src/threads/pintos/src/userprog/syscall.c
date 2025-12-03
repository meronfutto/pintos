#include "userprog/syscall.h"
#include <debug.h>
#include <stdio.h>
#include <stdint.h>
#include <syscall-nr.h>
#include "devices/shutdown.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"

static void syscall_handler (struct intr_frame *);
static void validate_user_pointer(const void *uaddr);
static void sys_exit(int statis) NO_RETURN;

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  uint32_t *usp = (uint32_t *) f->esp;

  validate_user_pointer(usp);
  int syscall_number = usp[0];

  switch(syscall_number)
  {
  case SYS_HALT:
    shutdown_power_off();
    break;
  case SYS_EXIT:
    validate_user_pointer(usp+1);
    sys_exit((int)usp[1]);
    break;
  case SYS_WRITE:
    validate_user_pointer(usp+1);
    validate_user_pointer(usp+2);
    validate_user_pointer(usp+3);
    {
      int fd = (int) usp[1];
      const void *buffer = (const void *) usp[2];
      unsigned size = (unsigned) usp[3];

      if (size>0)
      {
        validate_user_pointer((const uint8_t *) buffer);
        validate_user_pointer((const uint8_t *) buffer +size -1);
      }

      if (fd==1)
      {
        putbuf(buffer,size);
        f->eax = (int) size;
      }
      else
        f->eax = -1;
    }
    break;
  default:
    sys_exit(-1);
    break;
  }
}

static void validate_user_pointer(const void *uaddr)
{
  if (uaddr == NULL || !is_user_vaddr(uaddr)
      || pagedir_get_page (thread_current()->pagedir, uaddr)==NULL)
  {
    sys_exit(-1);
  }
}

static void sys_exit(int status)
{
  thread_current()->exit_status = status;
  thread_exit();
}
