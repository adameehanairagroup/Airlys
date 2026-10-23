#include <airlys/net.h>
#include <airlys/string.h>
#include <airlys/printk.h>

typedef struct {
    uint8_t  op;           /* 1 = BOOTREQUEST */
    uint8_t  htype;        /* 1 = 10Mb Ethernet */
    uint8_t  hlen;         /* 6 */
    uint8_t  hops;
    uint32_t xid;          /* Transaction ID */
    uint16_t secs;
    uint16_t flags;        /* 0x8000 = Broadcast */
    uint32_t ciaddr;       /* Client IP */
    uint32_t yiaddr;       /* Your IP */
    uint32_t siaddr;       /* Server IP */
    uint32_t giaddr;       /* Gateway IP */
    uint8_t  chaddr[16];   /* Client hardware address */
    char     sname[64];
    char     file[128];
    uint32_t magic_cookie; /* 0x63825363 */
    uint8_t  options[64];
} __attribute__((packed)) dhcp_packet_t;

int dhcp_send_discover(net_device_t* dev) {
    if (!dev) return -1;

    dhcp_packet_t packet;
    kmemset(&packet, 0, sizeof(packet));

    packet.op = 1; /* Request */
    packet.htype = 1;
    packet.hlen = 6;
    packet.xid = 0x3903F301;
    packet.flags = 0x0080; /* Broadcast flag in network byte order */
    kmemcpy(packet.chaddr, dev->dev_addr, 6);
    packet.magic_cookie = 0x63538263; /* Magic cookie (DHCP) */

    /* Options: Option 53 (Message Type = 1: Discover), Option 255 (End) */
    packet.options[0] = 53; /* DHCP Message Type */
    packet.options[1] = 1;  /* Length 1 */
    packet.options[2] = 1;  /* DHCP Discover */
    packet.options[3] = 255;/* End */

    printk(KERN_INFO "dhcp: Broadcasting DHCPDISCOVER on %s (Port 68 -> 67)\n", dev->name);
    return udp_send(dev, 0xFFFFFFFF, 68, 67, &packet, sizeof(packet));
}
