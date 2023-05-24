/* Reads a file twice. Buffer cache hit rate should be better
   after second read */

#include "tests/lib.h"
#include "tests/main.h"
#include <random.h>
#include <syscall.h>

#define BLOCK_SIZE 512
#define NUM_BLOCKS 50
static char buffer[BLOCK_SIZE];

void read_file (int fd);
void write_file (int fd);


void
test_main (void)
{
  int fd;
  random_init (0);
  random_bytes (buffer, sizeof buffer);
  CHECK (create ("test_file", 0), "create \"test_file\"");
  CHECK ((fd = open ("test_file")) > 1, "open \"test_file\"");

  msg ("writing random bytes in \"test_file\"");
  write_file (fd);
  msg ("close \"test_file\"");
  close (fd);

  msg ("invalidating buffer cache");
  buffer_cache_invalidate ();

  CHECK ((fd = open ("test_file")) > 1, "open \"test_file\"");
  msg ("read \"test_file\"");
  read_file (fd);
  close (fd);
  msg ("close \"test_file\"");

  int old_hit = get_buffer_hit ();
  int old_miss = get_buffer_miss ();
  int old_hit_rate = old_hit * 1000 / (old_hit + old_miss);

  CHECK ((fd = open ("test_file")) > 1, "open \"test_file\"");
  msg ("read \"test_file\"");
  read_file (fd);
  close (fd);
  msg ("close \"test_file\"");

  int new_hit = get_buffer_hit ();
  int new_miss = get_buffer_miss ();
  int new_hit_rate = new_hit * 1000 / (new_hit + new_miss);

  if (new_hit_rate > old_hit_rate)
    msg ("new hit rate is better than old hit rate");

  remove ("test_file");
}

void
read_file (int fd)
{
  for (int i = 0; i < NUM_BLOCKS; i++)
    {
      int bytes_read = read (fd, buffer, BLOCK_SIZE);
      if (bytes_read != BLOCK_SIZE)
        fail ("read %zu bytes in \"a\" returned %zu",
              BLOCK_SIZE, bytes_read);
    }
}

void
write_file (int fd)
{
  for (int i = 0; i < NUM_BLOCKS; i++)
    {
      int bytes_written = write (fd, buffer, BLOCK_SIZE);
      if (bytes_written != BLOCK_SIZE)
        fail ("write %zu bytes in \"test_file\" returned %zu",
              BLOCK_SIZE, bytes_written);
    }
}
