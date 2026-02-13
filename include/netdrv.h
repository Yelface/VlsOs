#ifndef NETDRV_H
#define NETDRV_H

#include "types.h"
#include "net.h"

/* Network driver interface */

/* RTL8139 - Realtek 8139 Fast Ethernet Driver (for QEMU) */
#define RTL8139_VENDOR_ID   0x10EC
#define RTL8139_DEVICE_ID   0x8139

/* PCI Configuration Space Offsets */
#define RTL8139_REG_MAC0    0x00    /* MAC address */
#define RTL8139_REG_MAR0    0x08    /* Multicast address */
#define RTL8139_REG_TXSTATUS 0x10   /* Transmit status (4 registers) */
#define RTL8139_REG_TXADDR  0x20    /* Transmit address (4 registers) */
#define RTL8139_REG_RXBUF   0x30    /* Receive buffer start address */
#define RTL8139_REG_RXBUFTAIL 0x34  /* Receive buffer end of packet */
#define RTL8139_REG_INTRSTATUS 0x3E /* Interrupt status */
#define RTL8139_REG_INTRMASK 0x3C   /* Interrupt mask */
#define RTL8139_REG_TXCONFIG 0x40   /* Transmit config */
#define RTL8139_REG_RXCONFIG 0x44   /* Receive config */
#define RTL8139_REG_CLOCKRUN 0x52   /* Clock run */
#define RTL8139_REG_MEDIASTAT 0x58  /* Media status */
#define RTL8139_REG_CONFIG0  0x51   /* Configuration 0 */
#define RTL8139_REG_CONFIG1  0x52   /* Configuration 1 */

/* Initialization functions */
void rtl8139_init(void);
void rtl8139_send(uint8_t* data, uint16_t len);
void rtl8139_receive(void);
void rtl8139_irq_handler(void);

/* Generic network driver init */
void net_driver_init(void);

#endif
