#include <airlys/net.h>
#include <airlys/string.h>
#include <airlys/printk.h>

static uint16_t s_ip_ident = 1000;

uint16_t net_checksum(const void* data, size_t len) {
    const uint16_t* ptr = (const uint16_t*)data;
    uint32_t sum = 0;

    while (len > 1) {
        sum += *ptr++;
        len -= 2;
    }
    if (len == 1) {
        sum += *(const uint8_t*)ptr;
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return (uint16_t)(~sum);
}

static uint16_t swap16(uint16_t v) {
    return (uint16_t)((v << 8) | (v >> 8));
}

int ipv4_send(net_device_t* dev, uint32_t dest_ip, uint8_t protocol, const void* payload, size_t len) {
    if (!dev || !payload || len > 1400) return -1;

    sk_buff_t* skb = alloc_skb(ETH_HLEN + sizeof(iphdr_t) + (uint32_t)len);
    if (!skb) return -2;

    skb->dev = dev;

    /* Build Ethernet Header */
    ethhdr_t* eth = (ethhdr_t*)skb_put(skb, ETH_HLEN);
    kmemcpy(eth->h_source, dev->dev_addr, ETH_ALEN);

    uint8_t* dst_mac = arp_lookup(dest_ip);
    if (!dst_mac && dev->gateway) {
        dst_mac = arp_lookup(dev->gateway);
    }
    if (dst_mac) {
        kmemcpy(eth->h_dest, dst_mac, ETH_ALEN);
    } else {
        kmemset(eth->h_dest, 0xFF, ETH_ALEN); /* Broadcast fallback */
    }
    eth->h_proto = swap16(ETH_P_IP);

    /* Build IPv4 Header */
    iphdr_t* ip = (iphdr_t*)skb_put(skb, sizeof(iphdr_t));
    ip->version_ihl = 0x45; /* IPv4, 5 DWORDs (20 bytes) */
    ip->tos = 0;
    ip->tot_len = swap16((uint16_t)(sizeof(iphdr_t) + len));
    ip->id = swap16(s_ip_ident++);
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = protocol;
    ip->check = 0;
    ip->saddr = dev->ip_addr;
    ip->daddr = dest_ip;
    ip->check = net_checksum(ip, sizeof(iphdr_t));

    /* Append Payload */
    uint8_t* body = skb_put(skb, (uint32_t)len);
    kmemcpy(body, payload, len);

    return dev_queue_xmit(skb);
}

void net_rx_packet(net_device_t* dev, const uint8_t* frame, size_t len) {
    if (!dev || !frame || len < ETH_HLEN) return;

    const ethhdr_t* eth = (const ethhdr_t*)frame;
    uint16_t proto = swap16(eth->h_proto);

    if (proto == ETH_P_ARP) {
        if (len < ETH_HLEN + sizeof(arphdr_t)) return;
        const arphdr_t* arp = (const arphdr_t*)(frame + ETH_HLEN);
        if (swap16(arp->ar_op) == 1) { /* ARP Request */
            arp_insert(arp->ar_sip, arp->ar_sha);
        }
    } else if (proto == ETH_P_IP) {
        if (len < ETH_HLEN + sizeof(iphdr_t)) return;
        const iphdr_t* ip = (const iphdr_t*)(frame + ETH_HLEN);
        size_t ip_hdr_len = (ip->version_ihl & 0x0F) * 4;
        if (len < ETH_HLEN + ip_hdr_len) return;

        const uint8_t* payload = frame + ETH_HLEN + ip_hdr_len;
        size_t payload_len = len - (ETH_HLEN + ip_hdr_len);

        if (ip->protocol == IPPROTO_ICMP) {
            icmp_process(dev, ip, payload, payload_len);
        }
    }
}
