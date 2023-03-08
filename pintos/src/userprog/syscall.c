#include "userprog/syscall.h"
#include <stdio.h>
#include <string.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "process.h"
#include "pagedir.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/synch.h"

static void syscall_handler (struct intr_frame *);

bool is_string_valid (const char *string);
bool is_address_valid (const void *address);
struct file * get_file_by_fd (int fd);
int get_filesize (int fd);

unsigned get_argc (int syscall_number);

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
struct lock filesys_lock;

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init (&filesys_lock);
}


static void
syscall_handler (struct intr_frame *f)
{
  if (f == NULL)
    exit_syscall (f, -1);
  
  uint32_t *args = ((uint32_t*) f->esp);

  /* validate syscall number address */
  if (!is_address_valid (args) || !is_address_valid (args + 1))
    exit_syscall (f, -1);
  int syscall_number = (int) args[0];

  /* validate arguments addresses */
  unsigned argc = get_argc (syscall_number);
  for (unsigned i = 1; i <= argc + 1; i++)
    {
      if (!is_address_valid (args + i))
        exit_syscall (f, -1);
    }

  /* run syscall function */
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
        filesize_syscall (f, args[1]);
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
      default:
        exit_syscall (f, -1);
    }
}


/* Checks validity of the given address. An address is valid if it is
   a user virtual address and is in a current thread's pages */
bool
is_address_valid (const void *address)
{
  if (address == NULL || !is_user_vaddr (address))
    return false;
  
#ifdef USERPROG
  uint32_t *pagedir = thread_current ()->pagedir;
  if (pagedir == NULL || pagedir_get_page (pagedir, address) == NULL)
    return false;
#endif
  
  return true;
}

bool
is_string_valid (const char *string)
{
  if (!is_address_valid (string))
    return false;
  
#ifdef USERPROG
  uint32_t *pagedir = thread_current ()->pagedir;
  char *kernel_virtual_address = pagedir_get_page (pagedir, string);
  if (kernel_virtual_address == NULL)
    return false;
  return is_address_valid (string + strlen (kernel_virtual_address) + 1);
#endif

  return true;
}


struct file * 
get_file_by_fd (int fd) 
{
  struct thread *t = thread_current ();
  if (fd < 0 || fd >= MAX_FILE_DESCRIPTOR || t->fd[fd]->file_id != fd)
    return NULL;
  return t->fd[fd]->file;
}


int
get_filesize (int fd)
{
  struct file *file = get_file_by_fd (fd);
  if (!file)
    return -1;
  
  lock_acquire (&filesys_lock);
  int file_len = file_length (file);
  lock_release (&filesys_lock);

  return file_len;
}


unsigned
get_argc (int syscall_number)
{
  switch (syscall_number)
    {
      case SYS_EXIT:
      case SYS_EXEC:
      case SYS_WAIT:
      case SYS_REMOVE:
      case SYS_OPEN:
      case SYS_FILESIZE:
      case SYS_TELL:
      case SYS_CLOSE:
      case SYS_PRACTICE:
        return 1;
      case SYS_CREATE:
      case SYS_SEEK:
        return 2;
      case SYS_READ:
      case SYS_WRITE:
        return 3;
      default:
        return 0;
    }
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
  thread_current ()->thread_details->exit_code = status;
  printf ("%s: exit(%d)\n", thread_current ()->name, status);
  thread_exit ();
}

void
exec_syscall (struct intr_frame *f, const char *file)
{
  if (!is_string_valid (file))
    exit_syscall (f, -1);

  f->eax = process_execute (file);
}

void
wait_syscall (struct intr_frame *f, tid_t tid)
{
    f->eax = process_wait (tid);
}

/* File Operation Syscalls */
void
create_syscall (struct intr_frame *f, const char *file, unsigned initial_size)
{
  if (!is_address_valid (file))
    exit_syscall (f, -1);
  
  if (strlen (file) > 14)
    f->eax = false;
  else
    {
      lock_acquire (&filesys_lock);
      f->eax = filesys_create (file, initial_size);
      lock_release (&filesys_lock);
    }
}

void
remove_syscall (struct intr_frame *f, const char *file)
{
  if (!is_string_valid (file))
    exit_syscall (f, -1);
  
  lock_acquire (&filesys_lock);
  f->eax = filesys_remove (file);
  lock_release (&filesys_lock);
}

void
open_syscall (struct intr_frame *f, const char *file)
{
    if (!is_string_valid (file))
      exit_syscall (f, -1);

    lock_acquire (&filesys_lock);
    struct file* file_ = filesys_open (file);
    lock_release (&filesys_lock);
    if (file_)
      {
        int fd = create_fd (file_);
        f->eax = fd;
      }
    else
      f->eax = -1;
}

void
filesize_syscall (struct intr_frame *f, int fd)
{
  f->eax = get_filesize (fd);
}

void
read_syscall (struct intr_frame *f, int fd, void *buffer, unsigned size)
{
  if (!is_string_valid (buffer))
    exit_syscall (f, -1);
  int read_bytes_conut = 0;
  if (fd == STDIN_FILENO) 
    {
      for (unsigned i = 0; i < size; i++) 
        {
          ((char *) buffer)[i] = input_getc ();
          read_bytes_conut++;
        }
    } 
  else if (fd == STDOUT_FILENO) 
    exit_syscall (f, -1);
  else 
    {
      struct file *file = get_file_by_fd (fd);
      if (file)
        {
          lock_acquire (&filesys_lock);
          read_bytes_conut = file_read (file, buffer, size);
          lock_release (&filesys_lock);
        }
      else
        f->eax = -1;
    }
  f->eax = read_bytes_conut;
}

void
write_syscall (struct intr_frame *f, int fd, void *buffer, unsigned size)
{
  if (!is_string_valid (buffer))
    exit_syscall (f, -1);
  
  int write_bytes_count = -1;
  if (fd == STDOUT_FILENO)
    {
      putbuf (buffer, size);
      write_bytes_count = size;
    } 
  else if (fd == STDIN_FILENO)
    {
      exit_syscall (f, -1);
    }
  else
    {
      struct file *file = get_file_by_fd (fd);
        if (file)
          {
            lock_acquire (&filesys_lock);
            write_bytes_count = file_write (file, buffer, size);
            lock_release (&filesys_lock);
          }
        else 
          f->eax = -1;
    }
  f->eax = write_bytes_count;
}

void
seek_syscall (struct intr_frame *f, int fd, unsigned position)
{    
  struct file* file = get_file_by_fd (fd);
  if (file)
    {
      lock_acquire (&filesys_lock);
      file_seek (file, position);
      lock_release (&filesys_lock);
    }
  else
    f->eax = -1;
}

void
tell_syscall (struct intr_frame *f, int fd)
{
  struct file* file = get_file_by_fd (fd);
  if (file)
    {
      lock_acquire (&filesys_lock);
      f->eax = file_tell (file);
      lock_release (&filesys_lock);
    }
  else
    f->eax = -1;
}

void
close_syscall (struct intr_frame *f, int fd)
{
    struct file *file = get_file_by_fd (fd);
    if (file)
      {
        int result = free_fd (fd);
        if (result == -1)
          f->eax = -1;
        else  
          {
            lock_acquire (&filesys_lock);
            file_close (file);
            lock_release (&filesys_lock);

            f->eax = 0;
          }
      } 
    else 
      f->eax = -1;
}

/* Other Syscalls */
void
practice_syscall (struct intr_frame *f, int i)
{
  f->eax = i + 1;
}
