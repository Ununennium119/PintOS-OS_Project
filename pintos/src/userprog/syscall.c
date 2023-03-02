#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "pagedir.h"

static void syscall_handler (struct intr_frame *);

bool is_address_valid (const void *address);

void halt_syscall (void);
void exit_syscall (struct intr_frame *f, int status);
void exec_syscall (struct intr_frame *f, const char *file);
void wait_syscall (struct intr_frame *f, tid_t tid);
void create_syscall (struct intr_frame *f, const char *file, unsigned initial_size);
void remove_syscall (struct intr_frame *f, const char *file);
void open_syscall (struct intr_frame *f, const char *file);
void filesize_syscall (struct intr_frame *f, int fd);
void read_syscall (struct intr_frame *f, int fd, void *buffer, unsigned size);
void write_syscall (struct intr_frame *f, int fd, void *buffer, unsigned size);
void seek_syscall (struct intr_frame *f, int fd, unsigned position);
void tell_syscall (struct intr_frame *f, int fd);
void close_syscall (struct intr_frame *f, int fd);
void practice_syscall (struct intr_frame *f, int i);


void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


static void
syscall_handler (struct intr_frame *f)
{
  uint32_t *args = ((uint32_t*) f->esp);
  if (!is_address_valid(args))
    exit_syscall(f, -1);

  uint32_t syscall_number = args[0];
  switch (syscall_number)
    {
      case SYS_HALT:
        halt_syscall ();
        break;
      case SYS_EXIT:
        exit_syscall (f, (int) args[1]);
        break;
      case SYS_EXEC:
        exec_syscall (f, (const char *) args[1]);
        break;
      case SYS_WAIT:
        wait_syscall (f, args[1]);
        break;
      case SYS_CREATE:
        create_syscall (f, (const char *) args[1], args[2]);
        break;
      case SYS_REMOVE:
        remove_syscall (f, (const char *) args[1]);
        break;
      case SYS_OPEN:
        open_syscall (f, (const char *) args[1]);
        break;
      case SYS_FILESIZE:
        exit_syscall (f, args[1]);
        break;
      case SYS_READ:
        read_syscall (f, args[1], (void *) args[2], args[3]);
        break;
      case SYS_WRITE:
        write_syscall (f, args[1], (void *) args[2], args[3]);
        break;
      case SYS_SEEK:
        seek_syscall (f, args[1], args[2]);
        break;
      case SYS_TELL:
        tell_syscall (f, args[1]);
        break;
      case SYS_CLOSE:
        close_syscall (f, args[1]);
        break;
      case SYS_PRACTICE:
        practice_syscall (f, args[1]);
        break;
    }
}


bool
is_address_valid (const void *address)
{
  if (address == NULL || !is_user_vaddr (address))
    return false;
  
#ifdef USERPROG
  uint32_t *pagedir = thread_current()->pagedir;
  if (pagedir_get_page(pagedir, address) == NULL)
    return false;
#endif
  
  return true;
}


/* Process Control Syscalls */
void
halt_syscall (void)
{
  shutdown_power_off ();
}

void
exit_syscall (struct intr_frame *f, int status)
{
  f->eax = status;
  printf ("%s: exit(%d)\n", thread_current ()->name, status);
  thread_exit ();
}

void
exec_syscall (struct intr_frame *f UNUSED, const char *file UNUSED)
{
  // ToDo: Implement
}

void
wait_syscall (struct intr_frame *f UNUSED, tid_t tid UNUSED)
{
    // ToDo: Implement
}

/* File Operation Syscalls */
void
create_syscall (struct intr_frame *f UNUSED, const char *file UNUSED, unsigned initial_size UNUSED)
{
    // ToDo: Implement
}

void
remove_syscall (struct intr_frame *f UNUSED, const char *file UNUSED)
{
    // ToDo: Implement
}

void
open_syscall (struct intr_frame *f UNUSED, const char *file UNUSED)
{
    // ToDo: Implement
}

void
filesize_syscall (struct intr_frame *f UNUSED, int fd UNUSED)
{
    // ToDo: Implement
}

void
read_syscall (struct intr_frame *f UNUSED, int fd UNUSED, void *buffer UNUSED, unsigned size UNUSED)
{
    // ToDo: Implement
}

void
write_syscall (struct intr_frame *f, int fd, void *buffer, unsigned size)
{
  /* ToDo: Complete */

  int result = -1;
  if (fd == STDOUT_FILENO)
    {
      putbuf (buffer, size);
      result = size;
    }
  
  f->eax = result;
}

void
seek_syscall (struct intr_frame *f UNUSED, int fd UNUSED, unsigned position UNUSED)
{
    // ToDo: Implement
}

void
tell_syscall (struct intr_frame *f UNUSED, int fd UNUSED)
{
    // ToDo: Implement
}

void
close_syscall (struct intr_frame *f UNUSED, int fd UNUSED)
{
    // ToDo: Implement
}

/* Other Syscalls */
void
practice_syscall (struct intr_frame *f, int i)
{
  f->eax = i + 1;
}
