#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "types.h"

/* FAT12 File System Header */

/* Boot sector constants */
#define FS_BYTES_PER_SECTOR     512
#define FS_SECTORS_PER_CLUSTER  1
#define FS_RESERVED_SECTORS     1
#define FS_NUM_FATS             2
#define FS_ROOT_ENTRIES         224
#define FS_SECTORS_PER_TRACK    18
#define FS_HEADS                2
#define FS_TOTAL_SECTORS        2880    /* 1.44 MB floppy */
#define FS_MEDIA_DESCRIPTOR     0xF0
#define FS_SECTORS_PER_FAT      9

/* Derived constants */
#define FS_ROOT_DIR_SECTORS     ((FS_ROOT_ENTRIES * 32) / FS_BYTES_PER_SECTOR)
#define FS_FAT_START_SECTOR     FS_RESERVED_SECTORS
#define FS_ROOT_DIR_SECTOR      (FS_FAT_START_SECTOR + (FS_NUM_FATS * FS_SECTORS_PER_FAT))
#define FS_DATA_START_SECTOR    (FS_ROOT_DIR_SECTOR + FS_ROOT_DIR_SECTORS)

/* File attributes */
#define FS_ATTR_READ_ONLY   0x01
#define FS_ATTR_HIDDEN      0x02
#define FS_ATTR_SYSTEM      0x04
#define FS_ATTR_VOLUME      0x08
#define FS_ATTR_DIRECTORY   0x10
#define FS_ATTR_ARCHIVE     0x20

/* Special cluster values */
#define FS_CLUSTER_FREE     0x000
#define FS_CLUSTER_RESERVED 0xFF0
#define FS_CLUSTER_BAD      0xFF7
#define FS_CLUSTER_MAX      0xFF8
#define FS_CLUSTER_EOF      0xFFF

/* File descriptor table */
#define FS_MAX_FILES        16

/* Directory entry structure (32 bytes) */
typedef struct {
    uint8_t filename[8];
    uint8_t extension[3];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t creation_time_tenths;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t high_cluster;
    uint16_t write_time;
    uint16_t write_date;
    uint16_t low_cluster;
    uint32_t file_size;
} __attribute__((packed)) fs_dir_entry_t;

/* File descriptor */
typedef struct {
    uint32_t    start_cluster;
    uint32_t    current_cluster;
    uint32_t    current_offset;
    uint32_t    file_size;
    uint32_t    position;
    uint8_t     attributes;
    uint8_t     in_use;
    char        filename[16];
} fs_file_t;

/* Directory entry with full path */
typedef struct {
    char filename[13];
    uint8_t attributes;
    uint32_t file_size;
    uint32_t start_cluster;
    uint16_t write_date;
    uint16_t write_time;
} fs_dir_info_t;

/* File system operations */

/* Initialize file system */
int fs_init(void);

/* Open file for reading */
int fs_open(const char* filename, uint8_t mode);

/* Close file */
int fs_close(int fd);

/* Read from file */
int fs_read(int fd, uint8_t* buffer, uint16_t count);

/* Write to file */
int fs_write(int fd, const uint8_t* buffer, uint16_t count);

/* Seek in file */
int fs_seek(int fd, uint32_t offset);

/* Get file size */
uint32_t fs_get_size(int fd);

/* Delete file */
int fs_delete(const char* filename);

/* Create file */
int fs_create(const char* filename, uint8_t attributes);

/* List directory */
int fs_list_dir(const char* dirname, fs_dir_info_t* entries, int max_entries);

/* Check if file exists */
int fs_file_exists(const char* filename);

/* Get file info */
int fs_get_file_info(const char* filename, fs_dir_info_t* info);

/* Read a cluster from disk */
int fs_read_cluster(uint32_t cluster, uint8_t* buffer);

/* Write a cluster to disk */
int fs_write_cluster(uint32_t cluster, const uint8_t* buffer);

/* Get next cluster from FAT */
uint32_t fs_get_next_cluster(uint32_t cluster);

/* Allocate a new cluster */
uint32_t fs_allocate_cluster(void);

/* Free a cluster chain */
int fs_free_cluster_chain(uint32_t start_cluster);

#endif
