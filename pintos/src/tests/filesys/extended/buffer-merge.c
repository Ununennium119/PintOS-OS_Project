/* Writes in a file and then reads it. Most of the writes should
   be done on buffer. */

#include "tests/lib.h"
#include "tests/main.h"
#include <random.h>
#include <syscall.h>

static char buffer[1];

void
test_main (void)
{
  int file;

  CHECK (create ("test_file", 0), "create \"test_file\"");
  CHECK ((file = open ("test_file")) > 1, "open \"test_file\"");

  msg ("invalidating buffer cache");
  buffer_cache_invalidate ();

  msg ("writing 64kB to \"test_file\"");
  int init_write = get_buffer_disk_write ();
  int num_write_bytes = 1 << 16;
  for (int i = 0; i < num_write_bytes; i++)
    {
      random_init (0);
      random_bytes (buffer, sizeof buffer);
      if (write (file, buffer, 1) != 1)
        fail ("invalid number of written bytes");
    }
  msg ("close \"test_file\"");
  close (file);

  CHECK ((file = open ("test_file")) > 1, "open \"test_file\"");
  msg ("read \"test_file\"");
  for (int i = 0; i < num_write_bytes; i++)
    if (read (file, buffer, 1) != 1)
      fail ("invalid number of read bytes");
  msg ("close \"test_file\"");
  close (file);

  int num_writes = get_buffer_disk_write () - init_write;
  if (num_writes > 120 && num_writes < 136)
    msg ("total number of writes is about 128");

  remove ("test_file");
}
