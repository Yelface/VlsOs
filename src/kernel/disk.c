#include "disk.h"
#include "types.h"
#include "memory.h"
#include "string.h"

/* Global disk information structure for up to 4 drives */
static ata_disk_t g_disks[MAX_DRIVES];
static uint8_t g_drive_count = 0;

/* Port I/O functions (defined in interrupts.asm) */
extern void outb(uint16_t port, uint8_t value);
extern uint8_t inb(uint16_t port);
extern void outw(uint16_t port, uint16_t value);
extern uint16_t inw(uint16_t port);

/* Helper: Wait for drive to be ready */
static uint8_t ata_wait_status(uint16_t base, uint8_t mask, uint8_t value) {
    uint32_t timeout = 100000;  /* Timeout counter */
    uint8_t status;

    while (timeout--) {
        status = inb(base + ATA_REG_STATUS);
        if ((status & mask) == value) {
            return 1;
        }
    }
    return 0;
}

/* Helper: Wait for drive ready without DRQ */
static uint8_t ata_wait_ready(uint16_t base) {
    uint32_t timeout = 30000;
    uint8_t status;

    while (timeout--) {
        status = inb(base + ATA_REG_STATUS);
        if ((status & (ATA_SR_BSY | ATA_SR_DF)) == 0) {
            if (status & ATA_SR_DRDY) {
                return 1;
            }
        }
    }
    return 0;
}

/* Helper: Process identify data */
static void ata_process_identify(uint8_t drive, uint16_t* data) {
    ata_disk_t* disk = &g_disks[drive];

    disk->cylinders = data[1];
    disk->heads = data[3];
    disk->sectors = data[6];

    /* Total sectors from LBA28 (word 60-61) */
    uint32_t lba_sectors = ((uint32_t)data[61] << 16) | data[60];
    disk->total_sectors = lba_sectors > 0 ? lba_sectors : 
                          (disk->cylinders * disk->heads * disk->sectors);

    /* Extract model from words 27-46 (serial is 10-19) */
    char* model = disk->model;
    for (int i = 0; i < 20; i++) {
        uint16_t word = data[27 + i];
        model[i * 2] = (word >> 8) & 0xFF;
        model[i * 2 + 1] = word & 0xFF;
    }
    model[40] = '\0';

    disk->present = 1;
    disk->status = 1;
}

/* Identify a drive (primary master, slave, secondary master, slave) */
void disk_identify(uint8_t drive) {
    uint16_t base;
    uint8_t drive_sel;

    if (drive == 0) {
        base = ATA_PRIMARY_IO_BASE;
        drive_sel = ATA_MASTER;
    } else if (drive == 1) {
        base = ATA_PRIMARY_IO_BASE;
        drive_sel = ATA_SLAVE;
    } else if (drive == 2) {
        base = ATA_SECONDARY_IO_BASE;
        drive_sel = ATA_MASTER;
    } else if (drive == 3) {
        base = ATA_SECONDARY_IO_BASE;
        drive_sel = ATA_SLAVE;
    } else {
        return;
    }

    /* Select drive */
    outb(base + ATA_REG_HDDEVSEL, drive_sel);

    /* Small delay */
    for (volatile int i = 0; i < 100; i++);

    /* Check if drive exists (status register should not be 0xFF) */
    uint8_t status = inb(base + ATA_REG_STATUS);
    if (status == 0xFF) {
        g_disks[drive].present = 0;
        return;
    }

    /* Issue IDENTIFY command */
    outb(base + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

    /* Wait for DRQ */
    if (!ata_wait_status(base, ATA_SR_DRQ, ATA_SR_DRQ)) {
        g_disks[drive].present = 0;
        return;
    }

    /* Read identify data (256 words = 512 bytes) */
    uint16_t identify_data[256];
    for (int i = 0; i < 256; i++) {
        identify_data[i] = inw(base + ATA_REG_DATA);
    }

    /* Process and store identify data */
    ata_process_identify(drive, identify_data);

    if (g_disks[drive].present) {
        g_drive_count++;
    }
}

/* Read a single sector from disk */
int disk_read_sector(uint8_t drive, uint32_t lba, uint8_t* buffer) {
    return disk_read_sectors(drive, lba, 1, buffer);
}

/* Read multiple sectors from disk using CHS addressing */
int disk_read_sectors(uint8_t drive, uint32_t lba, uint8_t count, uint8_t* buffer) {
    uint16_t base;
    uint8_t drive_sel;
    uint16_t* buffer_word = (uint16_t*)buffer;

    if (drive >= MAX_DRIVES || !g_disks[drive].present) {
        return -1;
    }

    if (drive < 2) {
        base = ATA_PRIMARY_IO_BASE;
        drive_sel = (drive == 0) ? ATA_MASTER : ATA_SLAVE;
    } else {
        base = ATA_SECONDARY_IO_BASE;
        drive_sel = (drive == 2) ? ATA_MASTER : ATA_SLAVE;
    }

    /* Wait for drive ready */
    if (!ata_wait_ready(base)) {
        return -1;
    }

    /* Set sector count */
    outb(base + ATA_REG_SECCOUNT, count);

    /* Set LBA address (CHS mode for compatibility) */
    outb(base + ATA_REG_LBA0, lba & 0xFF);
    outb(base + ATA_REG_LBA1, (lba >> 8) & 0xFF);
    outb(base + ATA_REG_LBA2, (lba >> 16) & 0xFF);

    /* Drive/head select with LBA bit (0xE0 bit 6 = LBA mode) */
    outb(base + ATA_REG_HDDEVSEL, drive_sel | ((lba >> 24) & 0x0F));

    /* Issue READ command */
    outb(base + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

    /* Read each sector */
    for (uint8_t s = 0; s < count; s++) {
        /* Wait for data ready */
        if (!ata_wait_status(base, ATA_SR_DRQ, ATA_SR_DRQ)) {
            return -1;
        }

        /* Read sector data (256 words) */
        for (int i = 0; i < 256; i++) {
            *buffer_word++ = inw(base + ATA_REG_DATA);
        }
    }

    return count;
}

/* Write a single sector to disk */
int disk_write_sector(uint8_t drive, uint32_t lba, uint8_t* buffer) {
    return disk_write_sectors(drive, lba, 1, buffer);
}

/* Write multiple sectors to disk */
int disk_write_sectors(uint8_t drive, uint32_t lba, uint8_t count, uint8_t* buffer) {
    uint16_t base;
    uint8_t drive_sel;
    uint16_t* buffer_word = (uint16_t*)buffer;

    if (drive >= MAX_DRIVES || !g_disks[drive].present) {
        return -1;
    }

    if (drive < 2) {
        base = ATA_PRIMARY_IO_BASE;
        drive_sel = (drive == 0) ? ATA_MASTER : ATA_SLAVE;
    } else {
        base = ATA_SECONDARY_IO_BASE;
        drive_sel = (drive == 2) ? ATA_MASTER : ATA_SLAVE;
    }

    /* Wait for drive ready */
    if (!ata_wait_ready(base)) {
        return -1;
    }

    /* Set sector count */
    outb(base + ATA_REG_SECCOUNT, count);

    /* Set LBA address */
    outb(base + ATA_REG_LBA0, lba & 0xFF);
    outb(base + ATA_REG_LBA1, (lba >> 8) & 0xFF);
    outb(base + ATA_REG_LBA2, (lba >> 16) & 0xFF);

    /* Drive/head select with LBA mode */
    outb(base + ATA_REG_HDDEVSEL, drive_sel | ((lba >> 24) & 0x0F));

    /* Issue WRITE command */
    outb(base + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);

    /* Write each sector */
    for (uint8_t s = 0; s < count; s++) {
        /* Wait for ready to accept data */
        if (!ata_wait_status(base, ATA_SR_DRQ, ATA_SR_DRQ)) {
            return -1;
        }

        /* Write sector data (256 words) */
        for (int i = 0; i < 256; i++) {
            outw(base + ATA_REG_DATA, *buffer_word++);
        }
    }

    return count;
}

/* Get disk information */
ata_disk_t* disk_get_info(uint8_t drive) {
    if (drive >= MAX_DRIVES) {
        return NULL;
    }
    return &g_disks[drive];
}

/* Get number of detected drives */
uint8_t disk_get_drive_count(void) {
    return g_drive_count;
}

/* Check if drive exists and is ready */
uint8_t disk_drive_exists(uint8_t drive) {
    if (drive >= MAX_DRIVES) {
        return 0;
    }
    return g_disks[drive].present;
}

/* Initialize disk subsystem */
void disk_init(void) {
    /* Initialize all disks as not present */
    for (int i = 0; i < MAX_DRIVES; i++) {
        g_disks[i].present = 0;
        g_disks[i].status = 0;
        g_disks[i].cylinders = 0;
        g_disks[i].heads = 0;
        g_disks[i].sectors = 0;
        g_disks[i].total_sectors = 0;
    }

    g_drive_count = 0;

    /* Identify all possible drives */
    disk_identify(0);  /* Primary master */
    disk_identify(1);  /* Primary slave */
    disk_identify(2);  /* Secondary master */
    disk_identify(3);  /* Secondary slave */
}
