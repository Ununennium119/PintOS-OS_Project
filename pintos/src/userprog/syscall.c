#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

int write_syscall (int fd, void *buffer, unsigned size);

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}



static void
syscall_handler (struct intr_frame *f UNUSED)
{
  uint32_t* args = ((uint32_t*) f->esp);

  /*
   * The following print statement, if uncommented, will print out the syscall
   * number whenever a process enters a system call. You might find it useful
   * when debugging. It will cause tests to fail, however, so you should not
   * include it in your final submission.
   */

  /* printf("System call number: %d\n", args[0]); */

  if (args[0] == SYS_EXIT)
    {
      f->eax = args[1];
      printf ("%s: exit(%d)\n", &thread_current ()->name, args[1]);
      thread_exit ();
    }
  else if (args[0] == SYS_WRITE)
    {
      int fd = (int) args[1];
      void *buffer = (void *) args[2];
      unsigned size = (unsigned) args[3];

      int result = write_syscall (fd, buffer, size);
      f->eax = result;
    }
  else if (args[0] == SYS_PRACTICE)
    {
      f->eax = args[1] + 1;
    }
  else if (args[0] == SYS_HALT)
    {
      shutdown_power_off(); 
    }
  else if (args[0] == SYS_WAIT)
    {

    }
  else if (args[0] == SYS_EXEC)
    {

    }
}

int
write_syscall (int fd, void *buffer, unsigned size)
{
  if (fd == STDOUT_FILENO)
    {
      putbuf(buffer, size);
      return size;
    }
  
  return -1;
}
