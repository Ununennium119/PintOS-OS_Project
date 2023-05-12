#ifndef FILESYS_BUFFER_CACHE_H
#define FILESYS_BUFFER_CACHE_H

#include <stdbool.h>
#include <list.h>
#include "devices/block.h"
#include "threads/synch.h"
#include "filesys/off_t.h"

#define BUFFER_CACHE_SIZE 64     /* Number of buffer cache blocks */

struct list buffer_cache_list;    /* List of buffer cache blocks */
struct lock buffer_cache_lock;    /* Lock for modifying buffer_cache_list */

typedef struct buffer_cache_block
  {
    block_sector_t sector;            /* Index of sector */         
    char data[BLOCK_SECTOR_SIZE];     /* Sector's data */
    struct lock block_lock;           /* Lock for accessing block */
    bool valid;                       /* Valid bit */
    bool dirty;                       /* Dirty bit */

    struct list_elem elem;            /* List element */
  } buffer_cache_block_t;

void buffer_cache_init (void);
void buffer_cache_read (block_sector_t sector, void *buffer, off_t size, off_t block_ofs);
void buffer_cache_write (block_sector_t sector, const void *buffer, off_t size, off_t block_ofs);
void buffer_cache_flush (void);

#endif  /* filesys/buffer-cache.h */
