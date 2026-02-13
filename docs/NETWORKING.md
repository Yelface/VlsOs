# VlsOs Network Stack Implementation

## Overview

VlsOs includes a complete TCP/IP network stack with support for:

- **Ethernet**: Frame layer with MAC addressing
- **ARP**: Address Resolution Protocol for IP-to-MAC mapping
- **IPv4**: Internet Protocol for routing and delivery
- **ICMP**: Internet Control Message Protocol (ping)
- **UDP**: User Datagram Protocol for connectionless communication
- **TCP**: Transmission Control Protocol for reliable communication
- **Socket API**: Berkeley socket interface for applications

## Architecture

### Layered Design

```
┌─────────────────────────────────────┐
│     Application Layer               │
│  (Shell commands, Utilities)        │
├─────────────────────────────────────┤
│     Socket API Layer                │
│  (socket, bind, listen, connect)    │
├─────────────────────────────────────┤
│     Transport Layer                 │
│  (TCP, UDP)                         │
├─────────────────────────────────────┤
│     Internet Layer                  │
│  (IPv4, ICMP, ARP)                  │
├─────────────────────────────────────┤
│     Link Layer                      │
│  (Ethernet, Network Drivers)        │
├─────────────────────────────────────┤
│     Physical Layer                  │
│  (Network Interface Card)           │
└─────────────────────────────────────┘
```

## Components

### 1. Network Core (`src/net/net.c`)

**Responsibilities:**
- Interface registration and management
- Packet routing to protocol handlers
- Byte order conversions (htons, ntohs, etc.)
- Checksum calculations

**Key Functions:**
- `net_init()` - Initialize network stack
- `net_receive_frame()` - Receive Ethernet frame
- `net_register_interface()` - Register network interface
- `net_poll()` - Poll for incoming packets

### 2. ARP Protocol (`src/net/arp.c`)

**Functionality:**
- Maps IPv4 addresses to MAC addresses
- Caches known mappings
- Responds to ARP requests
- Initiates ARP requests

**Key Functions:**
- `arp_handle_packet()` - Process incoming ARP
- `arp_request()` - Send ARP query
- `arp_lookup()` - Look up MAC from IP
- `arp_cache_learn()` - Learn new mappings

### 3. IPv4 Protocol (`src/net/ip.c`)

**Functionality:**
- IP packet fragmentation/reassembly (stub)
- Routing to transport layer protocols
- IP header validation
- Checksum verification

**Key Functions:**
- `ip_handle_packet()` - Process IP packet
- `ip_send_packet()` - Send IP packet

### 4. ICMP (`src/net/icmp.c`)

**Functionality:**
- Echo requests (ping)
- Echo replies
- Error reporting

**Key Functions:**
- `icmp_handle_packet()` - Process ICMP
- `icmp_send_echo_request()` - Send ping
- `icmp_send_echo_reply()` - Reply to ping

### 5. UDP (`src/net/udp.c`)

**Functionality:**
- Connectionless datagram delivery
- Port-based multiplexing

**Key Functions:**
- `udp_handle_packet()` - Process UDP packet
- `udp_send_packet()` - Send UDP packet

### 6. TCP (`src/net/tcp.c`)

**Functionality:**
- Connection establishment (SYN, SYN-ACK, ACK)
- Reliable data delivery
- Flow control (window size)
- Sequence number tracking

**Key Functions:**
- `tcp_handle_packet()` - Process TCP packet
- `tcp_send_packet()` - Send TCP packet

### 7. Network Driver (`src/net/netdrv.c`)

**Functionality:**
- Hardware interface
- RTL8139 Ethernet driver (for QEMU)
- Packet transmission/reception
- Interrupt handling

**Current Status:** Stub implementation (needs I/O port access)

### 8. Socket API (`src/net/socket.c`)

**Implemented Functions:**
- `socket()` - Create socket
- `bind()` - Bind to port
- `listen()` - Listen for connections
- `accept()` - Accept connection
- `connect()` - Connect to remote
- `send()` / `recv()` - TCP data transfer
- `sendto()` / `recvfrom()` - UDP data transfer
- `close()` - Close socket

**Supported Families:** AF_INET (IPv4)
**Supported Types:** SOCK_STREAM (TCP), SOCK_DGRAM (UDP)

### 9. Network Shell Commands (`src/net/netcmd.c`)

Available commands:

| Command | Purpose |
|---------|---------|
| `ifconfig` | Show network interface configuration |
| `ping <ip>` | Send ICMP echo request |
| `netstat` | Show network connections |
| `arp` | Show ARP cache |
| `route` | Show routing table |

## Configuration

### Network Interface Setup

**Default Configuration (in netdrv.c):**
```
Interface:  eth0
MAC Address: 52:54:00:12:34:56
IP Address:  192.168.1.100
Netmask:     255.255.255.0
Gateway:     192.168.1.1
```

### Protocol Versions

- IPv4 with standard headers (20 bytes minimum)
- No IPv6 support (future enhancement)

## Data Structures

### Ethernet Header

```c
typedef struct {
    mac_addr_t dest_mac;      // 6 bytes
    mac_addr_t src_mac;       // 6 bytes
    uint16_t ethertype;       // 2 bytes (0x0800=IPv4, 0x0806=ARP)
} ethernet_hdr_t;
```

### IPv4 Header

```c
typedef struct {
    uint8_t version_ihl;
    uint8_t dscp_ecn;
    uint16_t total_length;
    uint16_t identification;
    uint16_t flags_frag_offset;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    ipv4_addr_t src_addr;
    ipv4_addr_t dest_addr;
} ipv4_hdr_t;
```

## Usage Examples

### Checking Network Configuration

```bash
vlsos> ifconfig
eth0: MAC=52:54:00:12:34:56
      inet 192.168.1.100 netmask 255.255.255.0
      gateway 192.168.1.1
```

### Pinging a Host

```bash
vlsos> ping 192.168.1.1
PING 192.168.1.1
```

### Creating a UDP Socket

```c
#include "socket.h"

int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
if (sock < 0) return -1;

ipv4_addr_t dest;
net_set_ipaddr_bytes(&dest, 8, 8, 8, 8);  // 8.8.8.8

sendto(sock, "Hello", 5, dest, 53);  // Send to DNS
```

## Building with Network Support

The network stack is automatically built with the kernel:

```bash
cd /workspaces/VlsOs
make clean
make iso
make run
```

## Testing

### In QEMU

To test networking in QEMU with network support:

```bash
qemu-system-i386 -cdrom build/os.iso \
    -net nic,model=rtl8139,macaddr=52:54:00:12:34:56 \
    -net user,hostfwd=tcp:127.0.0.1:8080-:80
```

### Simulator Mode

The current implementation works in "simulator mode" - packets are processed but not actually sent/received. To enable real networking:

1. Implement PCI bus enumeration
2. Add RTL8139 memory-mapped I/O handling
3. Implement DMA for packet buffers
4. Add interrupt handling

## Current Limitations

1. **No PCI Support** - Driver stub only, no actual hardware access
2. **No Fragmentation** - IP packets limited to MTU size
3. **No Routing** - Single network assumed
4. **TCP Incomplete** - State machine not fully implemented
5. **No DNS** - Must use IP addresses directly
6. **No DHCP** - Static IP configuration only
7. **Single Interface** - Only eth0 supported
8. **No IPv6** - IPv4 only

## Future Enhancements

1. **PCI Bus Driver** - Detect and configure NICs
2. **Real RTL8139 Driver** - Full hardware support
3. **TCP Implementation** - Complete state machine
4. **DNS Resolution** - Hostname lookup
5. **DHCP Client** - Dynamic IP configuration
6. **Multicast** - IGMP support
7. **IPv6** - IPv6 addressing and routing
8. **Tunneling** - VPN and encapsulation
9. **QoS** - Traffic prioritization
10. **Monitoring Tools** - tcpdump, wireshark

## Troubleshooting

### No Network Interface Found

**Cause:** Network driver not initialized
**Solution:** Call `net_init()` during kernel startup

### Packets Not Being Sent

**Cause:** Driver in stub mode or no hardware access
**Solution:** Implement PCI enumeration and hardware driver

### ARP Not Working

**Cause:** Network not actually connected
**Solution:** Configure gateway and routing properly

## References

- RFC 791: Internet Protocol
- RFC 792: Internet Control Message Protocol
- RFC 826: Address Resolution Protocol
- RFC 793: Transmission Control Protocol
- RFC 768: User Datagram Protocol
- 4.4BSD Socket API
- Linux socket(7) man page

## Development Notes

The network stack is designed to be:
- **Modular** - Each protocol is independent
- **Extensible** - Easy to add new protocols
- **Testable** - Each layer can be tested separately
- **Efficient** - Minimal memory overhead
- **Portable** - Works with different hardware drivers

## License

See main README.md
