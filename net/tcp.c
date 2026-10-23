#include <airlys/net.h>
#include <airlys/string.h>

static uint16_t swap16(uint16_t v) {
    return (uint16_t)((v << 8) | (v >> 8));
}

static uint32_t swap32(uint32_t v) {
    return ((v >> 24) & 0xFF) | ((v >> 8) & 0xFF00) |
           ((v << 8) & 0xFF0000) | ((v << 24) & 0xFF000000);
}

int tcp_send_syn(net_device_t* dev, uint32_t dest_ip, uint16_t src_port, uint16_t dest_port) {
    if (!dev) return -1;

    tcphdr_t tcp;
    kmemset(&tcp, 0, sizeof(tcp));

    tcp.source = swap16(src_port);
    tcp.dest = swap16(dest_port);
    tcp.seq = swap32(100000);
    tcp.ack_seq = 0;
    tcp.flags = swap16((5 << 12) | TCP_SYN); /* 5 32-bit words header length + SYN */
    tcp.window = swap16(65535);
    tcp.check = 0;
    tcp.urg_ptr = 0;

    return ipv4_send(dev, dest_ip, IPPROTO_TCP, &tcp, sizeof(tcp));
}
