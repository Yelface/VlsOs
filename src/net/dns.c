#include "dns.h"
#include "socket.h"
#include "string.h"
#include "drivers.h"
#include "net.h"

static dns_server_t dns_server;

/* Helper: decode DNS qname into dot string; returns length consumed */
static int dns_decode_qname(uint8_t* buf, int buflen, char* out, int outlen) {
    int pos = 0, o = 0;
    while (pos < buflen) {
        uint8_t len = buf[pos++];
        if (len == 0) {
            if (o == 0) {
                out[o++] = '.'; /* root? keep as dot */
            }
            out[o] = 0;
            return pos;
        }
        if (o + len + 1 >= outlen) return -1;
        if (o != 0) out[o++] = '.';
        for (int i = 0; i < len && pos < buflen; i++) {
            out[o++] = buf[pos++];
        }
    }
    return -1;
}

/* Helper: encode qname from dot string into buffer; returns bytes written */
static int dns_encode_qname(const char* name, uint8_t* out, int outlen) {
    int o = 0;
    const char* start = name;
    while (*start) {
        const char* dot = start;
        while (*dot && *dot != '.') dot++;
        int label_len = dot - start;
        if (label_len == 0) break;
        if (o + 1 + label_len >= outlen) return -1;
        out[o++] = (uint8_t)label_len;
        for (int i = 0; i < label_len; i++) out[o++] = start[i];
        if (*dot == '.') start = dot + 1; else start = dot;
    }
    if (o >= outlen) return -1;
    out[o++] = 0; /* terminator */
    return o;
}

void dns_init(void) {
    memset(&dns_server, 0, sizeof(dns_server));
    dns_server.running = 0;
    dns_server.sockfd = -1;
}

void dns_start(void) {
    if (dns_server.running) {
        vga_write_string("DNS server already running\n");
        return;
    }

    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s < 0) {
        vga_write_string("Failed to create DNS socket\n");
        return;
    }

    ipv4_addr_t any;
    memset(&any, 0, sizeof(any));
    if (bind(s, any, DNS_PORT) < 0) {
        vga_write_string("Failed to bind DNS socket to port 53\n");
        close(s);
        return;
    }

    dns_server.sockfd = s;
    dns_server.running = 1;
    vga_write_string("DNS server started on port 53\n");
}

void dns_stop(void) {
    if (!dns_server.running) {
        vga_write_string("DNS server not running\n");
        return;
    }
    close(dns_server.sockfd);
    dns_server.sockfd = -1;
    dns_server.running = 0;
    vga_write_string("DNS server stopped\n");
}

/* Very small DNS responder: answers A queries for "vlsos.local" returning
   the interface IP; otherwise returns RCODE=3 (NXDOMAIN) */
void dns_poll(void) {
    if (!dns_server.running) return;
    uint8_t buf[DNS_BUFFER_SIZE];
    ipv4_addr_t from;
    uint16_t port = 0;
    int n = recvfrom(dns_server.sockfd, buf, sizeof(buf), &from, &port);
    if (n <= 0) return;

    if (n < (int)sizeof(dns_hdr_t)) return;
    dns_hdr_t* hdr = (dns_hdr_t*)buf;

    uint16_t qd = (hdr->qdcount);
    /* Only handle first question */
    int pos = sizeof(dns_hdr_t);
    char qname[DNS_MAX_QNAME];
    int consumed = dns_decode_qname(buf + pos, n - pos, qname, sizeof(qname));
    if (consumed < 0) return;
    pos += consumed;
    if (pos + 4 > n) return; /* qtype(2) + qclass(2) */
    uint16_t qtype = (buf[pos] << 8) | buf[pos+1];
    uint16_t qclass = (buf[pos+2] << 8) | buf[pos+3];

    /* Prepare response in same buffer */
    uint8_t resp[DNS_BUFFER_SIZE];
    memset(resp, 0, sizeof(resp));
    dns_hdr_t* rh = (dns_hdr_t*)resp;
    rh->id = hdr->id;
    rh->flags = 0x8000; /* QR=1 (response) */
    rh->qdcount = hdr->qdcount;
    rh->nscount = 0;
    rh->arcount = 0;
    uint16_t ancount = 0;

    int roff = sizeof(dns_hdr_t);
    /* copy question */
    int qlen = dns_encode_qname(qname, resp + roff, sizeof(resp) - roff);
    if (qlen < 0) return;
    roff += qlen;
    if (roff + 4 > (int)sizeof(resp)) return;
    resp[roff++] = (buf[pos] & 0xFF);
    resp[roff++] = (buf[pos+1] & 0xFF);
    resp[roff++] = (buf[pos+2] & 0xFF);
    resp[roff++] = (buf[pos+3] & 0xFF);

    /* If query is A IN for "vlsos.local" or "vlsos", answer with interface IP */
    if (qtype == 1 && qclass == 1) {
        /* Normalize name (lowercase) */
        char name_l[DNS_MAX_QNAME];
        int i=0; while (qname[i]) { char c=qname[i]; if (c>='A'&&c<='Z') c+=32; name_l[i++]=c;} name_l[i]=0;
        if (strcmp(name_l, "vlsos.local") == 0 || strcmp(name_l, "vlsos") == 0) {
            /* answer */
            /* Encode name again */
            int an_start = roff;
            int nql = dns_encode_qname(qname, resp + roff, sizeof(resp) - roff);
            if (nql < 0) return;
            roff += nql;
            /* type A */
            resp[roff++] = 0x00; resp[roff++] = 0x01;
            /* class IN */
            resp[roff++] = 0x00; resp[roff++] = 0x01;
            /* TTL 300 */
            resp[roff++] = 0x00; resp[roff++] = 0x00; resp[roff++] = 0x01; resp[roff++] = 0x2C;
            /* rdlength 4 */
            resp[roff++] = 0x00; resp[roff++] = 0x04;
            /* rdata: use first interface IP */
            net_interface_t* iface = net_get_interface("eth0");
            uint8_t a=0,b=0,c2=0,d=0;
            if (iface) {
                a = iface->ip_addr.octets[0];
                b = iface->ip_addr.octets[1];
                c2 = iface->ip_addr.octets[2];
                d = iface->ip_addr.octets[3];
            }
            resp[roff++] = a; resp[roff++] = b; resp[roff++] = c2; resp[roff++] = d;
            ancount = 1;
        } else {
            /* NXDOMAIN */
            rh->flags |= 0x0003; /* RCODE=3 */
            ancount = 0;
        }
    } else {
        /* Not implemented: respond with RCODE=4 (Not Implemented) */
        rh->flags |= 0x0004;
        ancount = 0;
    }

    rh->ancount = ancount;

    /* send response */
    sendto(dns_server.sockfd, resp, roff, from, port);
}
