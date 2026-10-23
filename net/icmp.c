#include <airlys/net.h>
#include <airlys/string.h>
#include <airlys/printk.h>

int icmp_send_ping(net_device_t* dev, uint32_t dest_ip, uint16_t seq) {
    if (!dev) return -1;

    struct {
        icmphdr_t hdr;
        char payload[32];
    } packet;

    packet.hdr.type = 8; /* Echo Request */
    packet.hdr.code = 0;
    packet.hdr.checksum = 0;
    packet.hdr.id = 0x1337;
    packet.hdr.sequence = seq;
    kmemcpy(packet.payload, "AirlysSovereignKernelPingPing!", 31);
    packet.payload[31] = '\0';

    packet.hdr.checksum = net_checksum(&packet, sizeof(packet));

    return ipv4_send(dev, dest_ip, IPPROTO_ICMP, &packet, sizeof(packet));
}

int icmp_process(net_device_t* dev, const iphdr_t* ip, const uint8_t* payload, size_t len) {
    if (!dev || !ip || !payload || len < sizeof(icmphdr_t)) return -1;

    const icmphdr_t* req = (const icmphdr_t*)payload;

    /* Handle Echo Request (Ping from outside) */
    if (req->type == 8) {
        struct {
            icmphdr_t hdr;
            uint8_t   data[64];
        } reply;

        size_t data_len = len - sizeof(icmphdr_t);
        if (data_len > sizeof(reply.data)) data_len = sizeof(reply.data);

        reply.hdr.type = 0; /* Echo Reply */
        reply.hdr.code = 0;
        reply.hdr.checksum = 0;
        reply.hdr.id = req->id;
        reply.hdr.sequence = req->sequence;

        if (data_len > 0) {
            kmemcpy(reply.data, payload + sizeof(icmphdr_t), data_len);
        }

        size_t total_reply_len = sizeof(icmphdr_t) + data_len;
        reply.hdr.checksum = net_checksum(&reply, total_reply_len);

        return ipv4_send(dev, ip->saddr, IPPROTO_ICMP, &reply, total_reply_len);
    }

    return 0;
}
