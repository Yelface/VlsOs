#ifndef DISK_H
#define DISK_H

#include "types.h"

/* Disk driver for ATA/IDE controllers */

/* ATA Command Register Offsets */
#define ATA_PRIMARY_IO_BASE     0x1F0
#define ATA_PRIMARY_CTRL_BASE   0x3F6
#define ATA_SECONDARY_IO_BASE   0x170
#define ATA_SECONDARY_CTRL_BASE 0x376

/* ATA Register Offsets */
#define ATA_REG_DATA        0x00
#define ATA_REG_ERROR       0x01
#define ATA_REG_FEATURES    0x01
#define ATA_REG_SECCOUNT    0x02
#define ATA_REG_LBA0        0x03
#define ATA_REG_LBA1        0x04
#define ATA_REG_LBA2        0x05
#define ATA_REG_HDDEVSEL    0x06
#define ATA_REG_COMMAND     0x07
#define ATA_REG_STATUS      0x07
#define ATA_REG_CONTROL     0x06

/* ATA Commands */
#define ATA_CMD_READ_PIO    0x20
#define ATA_CMD_WRITE_PIO   0x30
#define ATA_CMD_IDENTIFY    0xEC

/* ATA Status Register Bits */
#define ATA_SR_BSY  0x80
#define ATA_SR_DRDY 0x40
#define ATA_SR_DF   0x20
#define ATA_SR_DSC  0x10
#define ATA_SR_DRQ  0x08
#define ATA_SR_ERR  0x01

/* Drive Select */
#define ATA_MASTER  0xA0
#define ATA_SLAVE   0xB0

#define SECTOR_SIZE 512
#define MAX_DRIVES  4

typedef struct {
    uint8_t     status;         /* Current drive status */
    uint16_t    cylinders;      /* Number of cylinders */
    uint16_t    heads;          /* Number of heads */
    uint16_t    sectors;        /* Sectors per track */
    uint32_t    total_sectors;  /* Total sector count */
    char        model[41];      /* Drive model string */
    uint8_t     present;        /* Drive present flag */
} ata_disk_t;

typedef struct {
    uint32_t boot_sector;
    uint8_t  drive_number;
    uint8_t  reserved;
    uint8_t  signature;
    uint8_t  reserved2[3];
    uint32_t partition_lba;
    uint32_t partition_sectors;
} mbr_partition_t;

typedef struct {
    uint32_t boot_code[88];
    mbr_partition_t partitions[4];
    uint16_t signature;
} mbr_t;

/* Initialize disk subsystem */
void disk_init(void);

/* Identify ATA drives */
void disk_identify(uint8_t drive);

/* Read sectors from disk */
int disk_read_sector(uint8_t drive, uint32_t lba, uint8_t* buffer);
int disk_read_sectors(uint8_t drive, uint32_t lba, uint8_t count, uint8_t* buffer);

/* Write sectors to disk */
int disk_write_sector(uint8_t drive, uint32_t lba, uint8_t* buffer);
int disk_write_sectors(uint8_t drive, uint32_t lba, uint8_t count, uint8_t* buffer);

/* Get disk information */
ata_disk_t* disk_get_info(uint8_t drive);

/* Get drive count */
uint8_t disk_get_drive_count(void);

/* Check if drive exists */
uint8_t disk_drive_exists(uint8_t drive);

#endif
