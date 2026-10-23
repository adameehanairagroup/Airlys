#include <airlys/net.h>
#include <airlys/string.h>

static uint16_t swap16(uint16_t v) {
    return (uint16_t)((v << 8) | (v >> 8));
}

int udp_send(net_device_t* dev, uint32_t dest_ip, uint16_t src_port, uint16_t dest_port, const void* data, size_t len) {
    if (!dev || (!data && len > 0) || len > 1024) return -1;

    uint8_t buffer[1024 + sizeof(udphdr_t)];
    udphdr_t* udp = (udphdr_t*)buffer;

    udp->source = swap16(src_port);
    udp->dest = swap16(dest_port);
    udp->len = swap16((uint16_t)(sizeof(udphdr_t) + len));
    udp->check = 0; /* UDP checksum optional in IPv4 */

    if (len > 0) {
        kmemcpy(buffer + sizeof(udphdr_t), data, len);
    }

    return ipv4_send(dev, dest_ip, IPPROTO_UDP, buffer, sizeof(udphdr_t) + len);
}
