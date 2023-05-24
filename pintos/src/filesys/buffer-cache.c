#include <stdbool.h>
#include <string.h>
#include <list.h>
#include "filesys/buffer-cache.h"
#include "threads/synch.h"
#include "filesys/filesys.h"
#include "devices/block.h"
#include "threads/malloc.h"

uint32_t hit_count;
uint32_t miss_count;
uint32_t disk_write_count;

buffer_cache_block_t *check_buffer_cache (block_sector_t sector);
buffer_cache_block_t *get_new_block (void);

/* Searches for the sector in buffer cache */
buffer_cache_block_t *
check_buffer_cache (block_sector_t sector)
{
  /* Get buffer cache lock */
  lock_acquire (&buffer_cache_lock);

  /* Search for the sector in buffer cache */
  for (struct list_elem *e = list_begin (&buffer_cache_list);
       e != list_end (&buffer_cache_list);
       e = list_next (e))
    {
      buffer_cache_block_t *block = list_entry (e, buffer_cache_block_t, elem);
      lock_acquire (&block->block_lock);
      if (block->valid && block->sector == sector)
        {
          /* Set block as recently used */
          list_remove (&block->elem);
          list_push_front (&buffer_cache_list, &block->elem);
          lock_release (&buffer_cache_lock);

          return block;
        }

      /* Release buffer cache lock */
      lock_release (&block->block_lock);
    }
  
  /* Release buffer cache lock */
  lock_release (&buffer_cache_lock);

  return NULL;
}

/* Returns a new buffer cache block to use */
buffer_cache_block_t *
get_new_block (void)
{
  /* Get cache buffer lock */
  lock_acquire (&buffer_cache_lock);

  /* Get least recently used cache block */
  buffer_cache_block_t *block = list_entry (list_back (&buffer_cache_list), buffer_cache_block_t, elem);

  /* Set block as recently used */
  list_remove (&block->elem);
  list_push_front (&buffer_cache_list, &block->elem);

  /* Release cache buffer lock */
  lock_release (&buffer_cache_lock);


  /* Get block lock */
  lock_acquire (&block->block_lock);

  /* Write the block's data to disk if dirty */
  if (block->valid && block->dirty)
    {
      block_write (fs_device, block->sector, block->data);
      disk_write_count++;
      block->dirty = false;
    }

  /* Set block as invalid */
  block->valid = false;
  
  return block;
}

/* Initializes buffer cache */
void
buffer_cache_init (void)
{
  lock_init (&buffer_cache_lock);

  lock_acquire (&buffer_cache_lock);
  list_init (&buffer_cache_list);
  for (int i = 0; i < BUFFER_CACHE_SIZE; i++)
    {
      buffer_cache_block_t *block = malloc (sizeof (buffer_cache_block_t));
      lock_init (&block->block_lock);
      block->valid = false;

      list_push_front (&buffer_cache_list, &block->elem);
    }
  lock_release (&buffer_cache_lock);

  hit_count = 0;
  miss_count = 0;
  disk_write_count = 0;
}

/* Reads from the sector */
void buffer_cache_read (block_sector_t sector, void *buffer, off_t size, off_t block_ofs)
{
  buffer_cache_block_t *block = check_buffer_cache (sector);
  if (!block)
    {
      /* Increment miss count */
      miss_count++;

      /* Get a new buffer cache block */
      block = get_new_block ();
      block->sector = sector;
      block_read (fs_device, sector, block->data);
      block->valid = true;
      block->dirty = false;
    }
  else
    {
      /* Increment hit count */
      hit_count++;
    }

  /* Copy data to buffer */
  memcpy (buffer, block->data + block_ofs, size);

  /* Release block lock */
  lock_release (&block->block_lock);
}

/* Writes at the sector */
void
buffer_cache_write (block_sector_t sector, const void *buffer, off_t size, off_t block_ofs)
{
  buffer_cache_block_t *block = check_buffer_cache (sector);
  if (!block)
    {
      /* Increment miss count */
      miss_count++;

      /* Get a new buffer cache block */
      block = get_new_block ();
      block->sector = sector;
      if (!(size == BLOCK_SECTOR_SIZE && block_ofs == 0))
        block_read (fs_device, sector, block->data);
      block->valid = true;
      block->dirty = false;
    }
  else
    {
      /* Increment hit count */
      hit_count++;
    }

  /* Copy data to cache buffer */
  memcpy(block->data + block_ofs, buffer, size);
  block->dirty = true;

  /* Release block lock */
  lock_release (&block->block_lock);
}

/* Writes all dirty blocks to disk */
void
buffer_cache_flush (void)
{
  /* Get buffer cache lock */
  lock_acquire (&buffer_cache_lock);

  /* Write all dirty blocks' data to disk */
  for (struct list_elem *e = list_begin (&buffer_cache_list);
       e != list_end (&buffer_cache_list);
       e = list_next (e))
    {
      buffer_cache_block_t *block = list_entry (e, buffer_cache_block_t, elem);
      lock_acquire (&block->block_lock);
      if (block->valid && block->dirty)
        {
          block_write (fs_device, block->sector, block->data);
          disk_write_count++;
          block->dirty = false;
        }
      lock_release (&block->block_lock);
    }

  /* Release buffer cache lock */
  lock_release (&buffer_cache_lock);
}

/* Clears buffer cache */
void
buffer_cache_invalidate (void)
{
  /* Get buffer cache lock */
  lock_acquire (&buffer_cache_lock);

  /* Write all dirty blocks' data to disk and mark all block as invalid */
  for (struct list_elem *e = list_begin (&buffer_cache_list);
       e != list_end (&buffer_cache_list);
       e = list_next (e))
    {
      buffer_cache_block_t *block = list_entry (e, buffer_cache_block_t, elem);
      lock_acquire (&block->block_lock);
      if (block->valid && block->dirty)
        {
          block_write (fs_device, block->sector, block->data);
          disk_write_count++;
          block->dirty = false;
        }
      block->valid = false;
      lock_release (&block->block_lock);
    }

  /* Release buffer cache lock */
  lock_release (&buffer_cache_lock);
}

uint32_t
get_hit_count (void)
{
  return hit_count;
}

uint32_t
get_miss_count (void)
{
  return miss_count;
}

uint32_t
get_disk_write_count (void)
{
  return disk_write_count;
}
