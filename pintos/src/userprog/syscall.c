#include "userprog/syscall.h"
#include <debug.h>
#include <stdio.h>
#include <stdint.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "devices/shutdown.h"

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
  case SYS_EXEC:
	validate_user_pointer(usp+1);
	validate_user_pointer((void*) usp[1]);
	const char *cmd_line = (const char *) usp[1];
	
	tid_t child_tid=process_execute(cmd_line);
	
	if(child_tid == TID_ERROR)
	{
		f->eax=-1;
		break;
	}
	
	struct child_status *cs = find_child_status(thread_current(),child_tid);
	if (cs==NULL)
	{
		f->eax=-1;
		break;
	}
	
	sema_down(&cs->sema_load);
	
	if(!cs->load_success)
	{
		f->eax=-1;
	}
	else
	{
		f->eax=child_tid;
	}
	break;
	
	  
	  
  case SYS_HALT:
	shutdown_power_off();
	break;
	
	  
	

	 
  case SYS_EXIT:
    validate_user_pointer(usp+1);
    sys_exit((int)usp[1]);
    break;
  case SYS_WRITE: //write(fd(stdin-0,stdout-1), buffer, size)
    validate_user_pointer(usp+1);
    validate_user_pointer(usp+2);
    validate_user_pointer(usp+3);
    {
      int fd = (int) usp[1];
      const void *buffer = (const void *) usp[2];
      unsigned size = (unsigned) usp[3];

      if (size>0)
      {
        validate_user_pointer((const uint8_t *) buffer); // проверяем начало буфера
        validate_user_pointer((const uint8_t *) buffer +size -1); // проверяем конец буфера
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
