#include "filesys/inode.h"
#include <list.h>
#include <debug.h>
#include <round.h>
#include <string.h>
#include "filesys/filesys.h"
#include "filesys/free-map.h"
#include "filesys/buffer-cache.h"
#include "threads/malloc.h"

/* Identifies an inode. */
#define INODE_MAGIC 0x494e4f44
#define DIRECT_BLK_CNT 123
#define IND_BLK_CNT 128
#define DBL_IND_BLK_CNT 128


struct indirect_block 
  {
    block_sector_t blocks[IND_BLK_CNT];
  } typedef indirect_block;

struct double_indirect_block 
  {
    block_sector_t indirect_blocks[DBL_IND_BLK_CNT];
  } typedef double_indirect_block;

/* On-disk inode.
   Must be exactly BLOCK_SECTOR_SIZE bytes long. */
struct inode_disk
  {
    block_sector_t db[DIRECT_BLK_CNT];  /* direct blocks */
    block_sector_t ib;                  /* indirect block */
    block_sector_t dib;          /* double indirect block */
    bool is_dir;                        /* is directory */
    off_t length;                       /* File size in bytes. */
    unsigned magic;                     /* Magic number. */
  };


bool allocate_direct_inode (struct inode_disk* in_disk_inode, int block_count);
bool allocate_double_indirect_inode (block_sector_t *sector, size_t length);
bool allocate_inode (struct inode_disk*, off_t);

bool allocate_inode_sector (block_sector_t *);
bool allocate_indirect_inode (block_sector_t *, size_t);
struct inode_disk *get_inode_disk (const struct inode *inode);


static void deallocate_inode (struct inode*);
void deallocate_inode_indirect (block_sector_t sector, size_t count);
void deallocate_inode_double_indirect (block_sector_t sector, size_t count);
/* Returns the number of sectors to allocate for an inode SIZE
   bytes long. */
static inline size_t
bytes_to_sectors (off_t size)
{
  return DIV_ROUND_UP (size, BLOCK_SECTOR_SIZE);
}

/* In-memory inode. */
struct inode
  {
    struct list_elem elem;              /* Element in inode list. */
    block_sector_t sector;              /* Sector number of disk location. */
    int open_cnt;                       /* Number of openers. */
    bool removed;                       /* True if deleted, false otherwise. */
    int deny_write_cnt;                 /* 0: writes ok, >0: deny writes. */
    struct inode_disk data;             /* Inode content. */
    struct lock inode_lock;             /* Inode lock. */
  };

/* Returns the block device sector that contains byte offset POS
   within INODE.
   Returns -1 if INODE does not contain data for a byte at offset
   POS. */
static block_sector_t
byte_to_sector (const struct inode *inode, off_t pos)
{
  ASSERT (inode != NULL);
  
  struct inode_disk *disk_inode = get_inode_disk (inode);

  if (pos >= disk_inode->length)
    {
    
      free(disk_inode);
      return -1;
    }

  off_t target_block = pos / BLOCK_SECTOR_SIZE;

  if (target_block < DIRECT_BLK_CNT)
    {
      free(disk_inode);
      return disk_inode->db[target_block];
    }

  if (target_block < DIRECT_BLK_CNT + IND_BLK_CNT)
    {
      target_block -= DIRECT_BLK_CNT;

      indirect_block ib;
      // TODO problem here
      buffer_cache_read (disk_inode->ib,
                  &ib, BLOCK_SECTOR_SIZE, 0);
      free(disk_inode);
      return ib.blocks[target_block];
    }

  target_block -= (DIRECT_BLK_CNT + IND_BLK_CNT);
  indirect_block ib;

  int dib_target_block = target_block / IND_BLK_CNT;
  int ib_target_block  = target_block % IND_BLK_CNT;

  // TODO problem here
  buffer_cache_read (disk_inode->dib, &ib, BLOCK_SECTOR_SIZE, 0);
  buffer_cache_read (ib.blocks[dib_target_block], &ib, BLOCK_SECTOR_SIZE, 0);
  return ib.blocks[ib_target_block];
}

/* List of open inodes, so that opening a single inode twice
   returns the same `struct inode'. */
static struct list open_inodes;

/* Initializes the inode module. */
void
inode_init (void)
{
  list_init (&open_inodes);
  buffer_cache_init ();
}

/* Initializes an inode with LENGTH bytes of data and
   writes the new inode to sector SECTOR on the file system
   device.
   Returns true if successful.
   Returns false if memory or disk allocation fails. */
bool
inode_create (block_sector_t sector, off_t length)
{
  struct inode_disk *disk_inode = NULL;
  bool success = false;

  ASSERT (length >= 0);

  /* If this assertion fails, the inode structure is not exactly
     one sector in size, and you should fix that. */
  ASSERT (sizeof *disk_inode == BLOCK_SECTOR_SIZE);

  disk_inode = calloc (1, sizeof *disk_inode);
  if (disk_inode != NULL)
    {
      disk_inode->is_dir = false;
      size_t sectors = bytes_to_sectors (length);
      disk_inode->length = length;
      disk_inode->magic = INODE_MAGIC;
      if (allocate_inode (disk_inode, length))
        {
          buffer_cache_write (sector, disk_inode, BLOCK_SECTOR_SIZE, 0);
          success = true;
        }
      free (disk_inode);
    }
  return success;
}

/* allocate inode */
bool 
allocate_inode (struct inode_disk* in_disk_inode, off_t length)
{
  ASSERT (in_disk_inode != NULL);
  if (length < 0) return false;

  size_t direct_block_count, indirect_block_count, double_indirect_block_count;
  size_t sectors_count = bytes_to_sectors (length);

  direct_block_count = (sectors_count < DIRECT_BLK_CNT) ? sectors_count: DIRECT_BLK_CNT;
  indirect_block_count = (sectors_count < IND_BLK_CNT) ? sectors_count: IND_BLK_CNT;
  double_indirect_block_count = (sectors_count < DBL_IND_BLK_CNT) ? sectors_count: DBL_IND_BLK_CNT;

  if (!allocate_direct_inode(in_disk_inode, direct_block_count))
    return false;
  sectors_count -= direct_block_count;
  if (!sectors_count)
    return true;
  if (!allocate_indirect_inode (&in_disk_inode->ib, indirect_block_count))
    return false;
  sectors_count -= indirect_block_count;
  if (!sectors_count)
    return true;
  if (!allocate_double_indirect_inode (&in_disk_inode->dib, double_indirect_block_count))
    return false;
  sectors_count -= double_indirect_block_count;
  if (!sectors_count)
    return true;

  return false;
}

/* allocate direct pointers for inode */
bool
allocate_direct_inode (struct inode_disk* in_disk_inode, int block_count)
{
 for (size_t i = 0; i < block_count; i++)
 if (!allocate_inode_sector (&in_disk_inode->db[i]))
      return false;
  return true;
}

/* allocate indirect inode */
bool
allocate_indirect_inode (block_sector_t *sector, size_t length)
{
  if (!allocate_inode_sector (sector))
    return false;

  indirect_block ib;
  buffer_cache_read (*sector, &ib, BLOCK_SECTOR_SIZE, 0);

  for (size_t i = 0; i < length; i++)
    if (!allocate_inode_sector (&ib.blocks[i]))
      return false;

  buffer_cache_write (*sector, &ib, BLOCK_SECTOR_SIZE, 0);

  return true;
}

bool
allocate_double_indirect_inode (block_sector_t *sector, size_t length)
{
  if (!allocate_inode_sector (sector))
    return false;

  indirect_block ib;
  buffer_cache_read (*sector, &ib, BLOCK_SECTOR_SIZE, 0);

  size_t sectors_count;
  size_t block_count = DIV_ROUND_UP (length, IND_BLK_CNT);
  for (size_t i = 0; i < block_count; i++)
    {
      sectors_count = (length < IND_BLK_CNT) ? length: IND_BLK_CNT;
      if (!allocate_indirect_inode (&ib.blocks[i], sectors_count))
        return false;
      length -= sectors_count;
    }

  buffer_cache_write (*sector, &ib, BLOCK_SECTOR_SIZE, 0);

  return true;
}

/* allocate sector for inode */
bool
allocate_inode_sector (block_sector_t *sector)
{
  static char buffer[BLOCK_SECTOR_SIZE];
  if (!(*sector))
    {
      if (!free_map_allocate (1, sector))
        return false;
      buffer_cache_write (*sector, buffer, BLOCK_SECTOR_SIZE, 0);
    }
  return true;
}



void deallocate_inode_indirect (block_sector_t sector, size_t count)
{
  indirect_block ib;
  buffer_cache_read (sector, &ib, BLOCK_SECTOR_SIZE, 0);

  int i;
  for (i = 0; i < count; i++)
    free_map_release (ib.blocks[i], 1);

  free_map_release (sector, 1);
}

void deallocate_inode_double_indirect (block_sector_t sector, size_t count)
{
  double_indirect_block dib;
  buffer_cache_read (sector, &dib, BLOCK_SECTOR_SIZE, 0);

  int i, j = DIV_ROUND_UP (count, IND_BLK_CNT);
  for (i = 0; i < j; i++)
    {
      int sector_cnt = count < IND_BLK_CNT ? count : IND_BLK_CNT;
      deallocate_inode_indirect (dib.indirect_blocks[i], sector_cnt);
      count -= sector_cnt;
    }

  free_map_release(sector, 1);
}

void deallocate_inode (struct inode* inode)
{
  ASSERT (inode != NULL)

  struct inode_disk* disk = get_inode_disk (inode);
  off_t length = disk->length;
  if(length < 0)
    {
      return;
    }

  size_t sectors = bytes_to_sectors(length);
  int i, j;
  // free direct blocks
  j = sectors < DIRECT_BLK_CNT ? sectors : DIRECT_BLK_CNT;
  for(i = 0; i < j; i++)
    {
      free_map_release(disk->db[i], 1);
    }

  sectors -= j;

  if(sectors == 0)
    {
      free(disk);
      return;
    }

  // free indirect block
  j = sectors < IND_BLK_CNT ? sectors : IND_BLK_CNT;
  deallocate_inode_indirect(disk->ib ,j);
  sectors -= j;

  if(sectors == 0)
    {
      free(disk);
      return;
    }

  // free double indirect blocks
  j = sectors < (IND_BLK_CNT*IND_BLK_CNT) ? sectors : IND_BLK_CNT*IND_BLK_CNT;
  deallocate_inode_double_indirect (disk->dib, j);
  sectors -= j;

  free(disk);
  ASSERT (sectors == 0)

}


/* Reads an inode from SECTOR
   and returns a `struct inode' that contains it.
   Returns a null pointer if memory allocation fails. */
struct inode *
inode_open (block_sector_t sector)
{
  struct list_elem *e;
  struct inode *inode;

  /* Check whether this inode is already open. */
  for (e = list_begin (&open_inodes); e != list_end (&open_inodes);
       e = list_next (e))
    {
      inode = list_entry (e, struct inode, elem);
      if (inode->sector == sector)
        {
          inode_reopen (inode);
          return inode;
        }
    }

  /* Allocate memory. */
  inode = malloc (sizeof *inode);
  if (inode == NULL)
    return NULL;

  /* Initialize. */
  list_push_front (&open_inodes, &inode->elem);
  inode->sector = sector;
  inode->open_cnt = 1;
  inode->deny_write_cnt = 0;
  inode->removed = false;
  lock_init (&inode->inode_lock);
  return inode;
}

/* Reopens and returns INODE. */
struct inode *
inode_reopen (struct inode *inode)
{
  if (inode != NULL)
    inode->open_cnt++;
  return inode;
}

/* get disk inode given by in-memory inode */
struct inode_disk *
get_inode_disk (const struct inode *inode)
{
  ASSERT (inode != NULL);
  struct inode_disk *inode_disk = malloc (sizeof *inode_disk);
  buffer_cache_read (inode_get_inumber (inode), (void *)inode_disk, BLOCK_SECTOR_SIZE, 0);
  return inode_disk;
}

/* Returns INODE's inode number. */
block_sector_t
inode_get_inumber (const struct inode *inode)
{
  return inode->sector;
}

/* Closes INODE and writes it to disk.
   If this was the last reference to INODE, frees its memory.
   If INODE was also a removed inode, frees its blocks. */
void
inode_close (struct inode *inode)
{
  /* Ignore null pointer. */
  if (inode == NULL)
    return;

  /* Release resources if this was the last opener. */
  if (--inode->open_cnt == 0)
    {
      /* Remove from inode list and release lock. */
      list_remove (&inode->elem);

      /* Deallocate blocks if removed. */
      if (inode->removed)
        {
          free_map_release (inode->sector, 1);
          deallocate_inode (inode);
        }

      free (inode);
    }
}

/* Marks INODE to be deleted when it is closed by the last caller who
   has it open. */
void
inode_remove (struct inode *inode)
{
  ASSERT (inode != NULL);
  inode->removed = true;
}

/* Reads SIZE bytes from INODE into BUFFER, starting at position OFFSET.
   Returns the number of bytes actually read, which may be less
   than SIZE if an error occurs or end of file is reached. */
off_t
inode_read_at (struct inode *inode, void *buffer_, off_t size, off_t offset)
{
  uint8_t *buffer = buffer_;
  off_t bytes_read = 0;
  lock_acquire (&inode->inode_lock);

  while (size > 0)
    {
      /* Disk sector to read, starting byte offset within sector. */
      block_sector_t sector_idx = byte_to_sector (inode, offset);
      int sector_ofs = offset % BLOCK_SECTOR_SIZE;

      /* Bytes left in inode, bytes left in sector, lesser of the two. */
      off_t inode_left = inode_length (inode) - offset;
      int sector_left = BLOCK_SECTOR_SIZE - sector_ofs;
      int min_left = inode_left < sector_left ? inode_left : sector_left;

      /* Number of bytes to actually copy out of this sector. */
      int chunk_size = size < min_left ? size : min_left;
      if (chunk_size <= 0)
        break;

      /* Read data from sector */
      buffer_cache_read (sector_idx, buffer + bytes_read, chunk_size, sector_ofs);

      /* Advance. */
      size -= chunk_size;
      offset += chunk_size;
      bytes_read += chunk_size;
    }

  lock_release (&inode->inode_lock);

  return bytes_read;
}

/* Writes SIZE bytes from BUFFER into INODE, starting at OFFSET.
   Returns the number of bytes actually written, which may be
   less than SIZE if end of file is reached or an error occurs.
   (Normally a write at end of file would extend the inode, but
   growth is not yet implemented.) */
off_t
inode_write_at (struct inode *inode, const void *buffer_, off_t size,
                off_t offset)
{
  const uint8_t *buffer = buffer_;
  off_t bytes_written = 0;

  if (inode->deny_write_cnt)
    return 0;


  lock_acquire (&inode->inode_lock);
  /* extend file if needed */
  if (byte_to_sector (inode, offset + size - 1) == (size_t) - 1)
    {
      struct inode_disk* idisk = get_inode_disk (inode);

      if (!allocate_inode (idisk, offset + size))
        {
          free (idisk);
          return 0;
        }

        idisk->length = offset + size;
        buffer_cache_write (inode_get_inumber (inode), (void *) idisk, BLOCK_SECTOR_SIZE, 0);
        free (idisk);
    }



  while (size > 0)
    {
      /* Sector to write, starting byte offset within sector. */
      block_sector_t sector_idx = byte_to_sector (inode, offset);
      int sector_ofs = offset % BLOCK_SECTOR_SIZE;

      /* Bytes left in inode, bytes left in sector, lesser of the two. */
      off_t inode_left = inode_length (inode) - offset;
      int sector_left = BLOCK_SECTOR_SIZE - sector_ofs;
      int min_left = inode_left < sector_left ? inode_left : sector_left;

      /* Number of bytes to actually write into this sector. */
      int chunk_size = size < min_left ? size : min_left;
      if (chunk_size <= 0)
        break;
      
      /* Write data to sector */
      buffer_cache_write (sector_idx, buffer + bytes_written, chunk_size, sector_ofs);

      /* Advance. */
      size -= chunk_size;
      offset += chunk_size;
      bytes_written += chunk_size;
    }

  lock_release (&inode->inode_lock);
  return bytes_written;
}

/* Disables writes to INODE.
   May be called at most once per inode opener. */
void
inode_deny_write (struct inode *inode)
{
  inode->deny_write_cnt++;
  ASSERT (inode->deny_write_cnt <= inode->open_cnt);
}

/* Re-enables writes to INODE.
   Must be called once by each inode opener who has called
   inode_deny_write() on the inode, before closing the inode. */
void
inode_allow_write (struct inode *inode)
{
  ASSERT (inode->deny_write_cnt > 0);
  ASSERT (inode->deny_write_cnt <= inode->open_cnt);
  inode->deny_write_cnt--;
}

/* Returns the length, in bytes, of INODE's data. */
off_t
inode_length (const struct inode *inode)
{
  ASSERT (inode != NULL);
  struct inode_disk *disk_inode = get_inode_disk (inode);
  off_t length = disk_inode->length;
  free(disk_inode);
  return length;
}
