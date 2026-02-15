#include "filesystem.h"
#include "disk.h"
#include "memory.h"
#include "string.h"

/* FAT12 File System Implementation */

/* Global state */
static fs_file_t g_files[FS_MAX_FILES];
static uint8_t* g_fat_cache = NULL;        /* Cached FAT (2 sectors = 1KB) */
static uint8_t* g_root_dir_cache = NULL;   /* Cached root directory */
static uint8_t g_fs_initialized = 0;

/* Cluster buffer for I/O */
static uint8_t g_cluster_buffer[FS_BYTES_PER_SECTOR];

/* Helper: Convert LBA to cluster offset on disk */
static uint32_t cluster_to_lba(uint32_t cluster) {
    if (cluster < 2) {
        return FS_ROOT_DIR_SECTOR;
    }
    return FS_DATA_START_SECTOR + ((cluster - 2) * FS_SECTORS_PER_CLUSTER);
}

/* Helper: Find directory entry by filename */
static int dir_find_entry(const char* filename, fs_dir_entry_t* entry) {
    if (!g_root_dir_cache) {
        return -1;
    }

    fs_dir_entry_t* entries = (fs_dir_entry_t*)g_root_dir_cache;
    
    for (int i = 0; i < FS_ROOT_ENTRIES; i++) {
        if (entries[i].filename[0] == 0x00) {
            return -1;  /* End of directory */
        }
        
        if (entries[i].filename[0] == 0xE5) {
            continue;  /* Deleted entry */
        }

        /* Convert 8.3 format to full filename */
        char full_name[13];
        int j = 0;
        
        /* Copy filename */
        for (int k = 0; k < 8 && entries[i].filename[k] != ' '; k++) {
            full_name[j++] = entries[i].filename[k];
        }

        /* Add dot if extension exists */
        if (entries[i].extension[0] != ' ') {
            full_name[j++] = '.';
            for (int k = 0; k < 3 && entries[i].extension[k] != ' '; k++) {
                full_name[j++] = entries[i].extension[k];
            }
        }
        full_name[j] = 0;

        if (strcmp(full_name, filename) == 0) {
            *entry = entries[i];
            return i;
        }
    }

    return -1;
}

/* Helper: Get next cluster from FAT12 */
static uint32_t fat12_get_next(uint32_t cluster) {
    if (!g_fat_cache) {
        return FS_CLUSTER_EOF;
    }

    /* FAT12: Each entry is 1.5 bytes */
    uint32_t fat_offset = cluster + (cluster / 2);
    
    if (fat_offset >= (FS_SECTORS_PER_FAT * FS_BYTES_PER_SECTOR)) {
        return FS_CLUSTER_EOF;
    }

    uint16_t value = *(uint16_t*)(g_fat_cache + fat_offset);
    
    if (cluster & 1) {
        return (value >> 4) & 0xFFF;
    } else {
        return value & 0xFFF;
    }
}

/* Helper: Set next cluster in FAT12 */
static void fat12_set_next(uint32_t cluster, uint32_t next) {
    if (!g_fat_cache) {
        return;
    }

    uint32_t fat_offset = cluster + (cluster / 2);
    
    if (cluster & 1) {
        uint16_t* ptr = (uint16_t*)(g_fat_cache + fat_offset);
        *ptr = (*ptr & 0x000F) | ((next & 0xFFF) << 4);
    } else {
        uint16_t* ptr = (uint16_t*)(g_fat_cache + fat_offset);
        *ptr = (*ptr & 0xF000) | (next & 0xFFF);
    }
}

/* Find free cluster in FAT */
static uint32_t fat12_find_free_cluster(void) {
    for (uint32_t i = 2; i < 0xFF0; i++) {
        if (fat12_get_next(i) == FS_CLUSTER_FREE) {
            return i;
        }
    }
    return FS_CLUSTER_EOF;
}

/* Initialize file system */
int fs_init(void) {
    if (g_fs_initialized) {
        return 0;
    }

    /* Initialize file table */
    for (int i = 0; i < FS_MAX_FILES; i++) {
        g_files[i].in_use = 0;
    }

    /* Allocate FAT cache */
    g_fat_cache = (uint8_t*)malloc(FS_SECTORS_PER_FAT * FS_BYTES_PER_SECTOR);
    if (!g_fat_cache) {
        return -1;
    }

    /* Allocate root directory cache */
    g_root_dir_cache = (uint8_t*)malloc(FS_ROOT_DIR_SECTORS * FS_BYTES_PER_SECTOR);
    if (!g_root_dir_cache) {
        free(g_fat_cache);
        return -1;
    }

    /* Read FAT from disk */
    if (disk_read_sectors(0, FS_FAT_START_SECTOR, FS_SECTORS_PER_FAT, g_fat_cache) < 0) {
        return -1;
    }

    /* Read root directory from disk */
    if (disk_read_sectors(0, FS_ROOT_DIR_SECTOR, FS_ROOT_DIR_SECTORS, g_root_dir_cache) < 0) {
        return -1;
    }

    g_fs_initialized = 1;
    return 0;
}

/* Open file */
int fs_open(const char* filename, uint8_t mode) {
    (void)mode;  /* Mode not fully supported yet */

    if (!g_fs_initialized || !filename) {
        return -1;
    }

    /* Find free file descriptor */
    int fd = -1;
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (!g_files[i].in_use) {
            fd = i;
            break;
        }
    }

    if (fd < 0) {
        return -1;  /* No free file descriptors */
    }

    /* Find file in directory */
    fs_dir_entry_t entry;
    if (dir_find_entry(filename, &entry) < 0) {
        return -1;  /* File not found */
    }

    /* Initialize file descriptor */
    g_files[fd].in_use = 1;
    g_files[fd].start_cluster = entry.low_cluster;
    g_files[fd].current_cluster = entry.low_cluster;
    g_files[fd].current_offset = 0;
    g_files[fd].file_size = entry.file_size;
    g_files[fd].position = 0;
    g_files[fd].attributes = entry.attributes;
    strncpy(g_files[fd].filename, filename, sizeof(g_files[fd].filename) - 1);

    return fd;
}

/* Close file */
int fs_close(int fd) {
    if (fd < 0 || fd >= FS_MAX_FILES || !g_files[fd].in_use) {
        return -1;
    }

    g_files[fd].in_use = 0;
    return 0;
}

/* Read from file */
int fs_read(int fd, uint8_t* buffer, uint16_t count) {
    if (fd < 0 || fd >= FS_MAX_FILES || !g_files[fd].in_use || !buffer) {
        return -1;
    }

    fs_file_t* file = &g_files[fd];
    uint32_t bytes_read = 0;
    uint32_t remaining = count;

    while (remaining > 0 && file->position < file->file_size) {
        /* Read current cluster */
        if (disk_read_sector(0, cluster_to_lba(file->current_cluster), g_cluster_buffer) < 0) {
            break;
        }

        /* Copy data from cluster */
        uint32_t offset_in_cluster = file->position % FS_BYTES_PER_SECTOR;
        uint32_t to_copy = FS_BYTES_PER_SECTOR - offset_in_cluster;
        
        if (to_copy > remaining) {
            to_copy = remaining;
        }
        if (file->position + to_copy > file->file_size) {
            to_copy = file->file_size - file->position;
        }

        memcpy(buffer, g_cluster_buffer + offset_in_cluster, to_copy);

        buffer += to_copy;
        file->position += to_copy;
        bytes_read += to_copy;
        remaining -= to_copy;

        /* Move to next cluster if needed */
        if (file->position % FS_BYTES_PER_SECTOR == 0 && file->position < file->file_size) {
            uint32_t next = fat12_get_next(file->current_cluster);
            if (next >= FS_CLUSTER_MAX) {
                break;  /* EOF */
            }
            file->current_cluster = next;
        }
    }

    return bytes_read;
}

/* Write to file (stub - not fully implemented) */
int fs_write(int fd, const uint8_t* buffer, uint16_t count) {
    (void)fd;
    (void)buffer;
    (void)count;
    return -1;  /* Not implemented yet */
}

/* Seek in file */
int fs_seek(int fd, uint32_t offset) {
    if (fd < 0 || fd >= FS_MAX_FILES || !g_files[fd].in_use) {
        return -1;
    }

    fs_file_t* file = &g_files[fd];
    
    if (offset > file->file_size) {
        return -1;
    }

    file->position = offset;
    
    /* Reset to start and navigate to offset */
    file->current_cluster = file->start_cluster;
    uint32_t pos = 0;

    while (pos < offset && pos < file->file_size) {
        uint32_t next = fat12_get_next(file->current_cluster);
        if (next >= FS_CLUSTER_MAX) {
            return -1;
        }
        file->current_cluster = next;
        pos += FS_BYTES_PER_SECTOR;
    }

    return 0;
}

/* Get file size */
uint32_t fs_get_size(int fd) {
    if (fd < 0 || fd >= FS_MAX_FILES || !g_files[fd].in_use) {
        return 0;
    }
    return g_files[fd].file_size;
}

/* Delete file (stub) */
int fs_delete(const char* filename) {
    (void)filename;
    return -1;  /* Not implemented yet */
}

/* Create file (stub) */
int fs_create(const char* filename, uint8_t attributes) {
    (void)filename;
    (void)attributes;
    return -1;  /* Not implemented yet */
}

/* List directory */
int fs_list_dir(const char* dirname, fs_dir_info_t* entries, int max_entries) {
    (void)dirname;
    
    if (!g_fs_initialized || !entries) {
        return -1;
    }

    fs_dir_entry_t* dir_entries = (fs_dir_entry_t*)g_root_dir_cache;
    int count = 0;

    for (int i = 0; i < FS_ROOT_ENTRIES && count < max_entries; i++) {
        if (dir_entries[i].filename[0] == 0x00) {
            break;
        }

        if (dir_entries[i].filename[0] == 0xE5 || dir_entries[i].attributes & FS_ATTR_VOLUME) {
            continue;
        }

        /* Build filename */
        int j = 0;
        for (int k = 0; k < 8 && dir_entries[i].filename[k] != ' '; k++) {
            entries[count].filename[j++] = dir_entries[i].filename[k];
        }

        if (dir_entries[i].extension[0] != ' ') {
            entries[count].filename[j++] = '.';
            for (int k = 0; k < 3 && dir_entries[i].extension[k] != ' '; k++) {
                entries[count].filename[j++] = dir_entries[i].extension[k];
            }
        }
        entries[count].filename[j] = 0;

        entries[count].attributes = dir_entries[i].attributes;
        entries[count].file_size = dir_entries[i].file_size;
        entries[count].start_cluster = dir_entries[i].low_cluster;
        entries[count].write_date = dir_entries[i].write_date;
        entries[count].write_time = dir_entries[i].write_time;

        count++;
    }

    return count;
}

/* Check if file exists */
int fs_file_exists(const char* filename) {
    fs_dir_entry_t entry;
    return (dir_find_entry(filename, &entry) >= 0) ? 1 : 0;
}

/* Get file info */
int fs_get_file_info(const char* filename, fs_dir_info_t* info) {
    if (!info) {
        return -1;
    }

    fs_dir_entry_t entry;
    if (dir_find_entry(filename, &entry) < 0) {
        return -1;
    }

    /* Build filename */
    int j = 0;
    for (int k = 0; k < 8 && entry.filename[k] != ' '; k++) {
        info->filename[j++] = entry.filename[k];
    }

    if (entry.extension[0] != ' ') {
        info->filename[j++] = '.';
        for (int k = 0; k < 3 && entry.extension[k] != ' '; k++) {
            info->filename[j++] = entry.extension[k];
        }
    }
    info->filename[j] = 0;

    info->attributes = entry.attributes;
    info->file_size = entry.file_size;
    info->start_cluster = entry.low_cluster;
    info->write_date = entry.write_date;
    info->write_time = entry.write_time;

    return 0;
}

/* Read cluster from disk */
int fs_read_cluster(uint32_t cluster, uint8_t* buffer) {
    if (!buffer) {
        return -1;
    }
    return disk_read_sector(0, cluster_to_lba(cluster), buffer);
}

/* Write cluster to disk */
int fs_write_cluster(uint32_t cluster, const uint8_t* buffer) {
    if (!buffer) {
        return -1;
    }
    return disk_write_sector(0, cluster_to_lba(cluster), (uint8_t*)buffer);
}

/* Get next cluster from FAT */
uint32_t fs_get_next_cluster(uint32_t cluster) {
    return fat12_get_next(cluster);
}

/* Allocate new cluster */
uint32_t fs_allocate_cluster(void) {
    return fat12_find_free_cluster();
}

/* Free cluster chain */
int fs_free_cluster_chain(uint32_t start_cluster) {
    (void)start_cluster;
    return -1;  /* Not implemented yet */
}
