#ifndef AIRLYS_NET_H
#define AIRLYS_NET_H

/*
 * Airlys Operating System Kernel - Networking Architecture (Phase 6 TCP/IP)
 * Lead Architect: Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
 * Co-Pilot: Sovereign AI Aira 3 Ultra (Engine v3.6)
 */

#include <airlys/types.h>

#define ETH_ALEN       6
#define ETH_HLEN       14
#define ETH_P_IP       0x0800
#define ETH_P_ARP      0x0806

#define IPPROTO_ICMP   1
#define IPPROTO_TCP    6
#define IPPROTO_UDP    17

#define MAX_NET_DEVS   4
#define SKB_MAX_DATA   2048

/* Ethernet Header */
typedef struct {
    uint8_t  h_dest[ETH_ALEN];
    uint8_t  h_source[ETH_ALEN];
    uint16_t h_proto;
} __attribute__((packed)) ethhdr_t;

/* ARP Header */
typedef struct {
    uint16_t ar_hrd;     /* Hardware type */
    uint16_t ar_pro;     /* Protocol type */
    uint8_t  ar_hln;     /* Hardware address length */
    uint8_t  ar_pln;     /* Protocol address length */
    uint16_t ar_op;      /* ARP opcode */
    uint8_t  ar_sha[6];  /* Sender hardware address */
    uint32_t ar_sip;     /* Sender IP address */
    uint8_t  ar_tha[6];  /* Target hardware address */
    uint32_t ar_tip;     /* Target IP address */
} __attribute__((packed)) arphdr_t;

/* IPv4 Header */
typedef struct {
    uint8_t  version_ihl;
    uint8_t  tos;
    uint16_t tot_len;
    uint16_t id;
    uint16_t frag_off;
    uint8_t  ttl;
    uint8_t  protocol;
    uint16_t check;
    uint32_t saddr;
    uint32_t daddr;
} __attribute__((packed)) iphdr_t;

/* ICMP Header */
typedef struct {
    uint8_t  type;       /* 8 = Echo Request, 0 = Echo Reply */
    uint8_t  code;
    uint16_t checksum;
    uint16_t id;
    uint16_t sequence;
} __attribute__((packed)) icmphdr_t;

/* UDP Header */
typedef struct {
    uint16_t source;
    uint16_t dest;
    uint16_t len;
    uint16_t check;
} __attribute__((packed)) udphdr_t;

/* TCP Header */
typedef struct {
    uint16_t source;
    uint16_t dest;
    uint32_t seq;
    uint32_t ack_seq;
    uint16_t flags;
    uint16_t window;
    uint16_t check;
    uint16_t urg_ptr;
} __attribute__((packed)) tcphdr_t;

#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10
#define TCP_URG 0x20

/* Linux sk_buff (Socket Buffer) Representation */
typedef struct sk_buff {
    uint8_t* head;
    uint8_t* data;
    uint8_t* tail;
    uint8_t* end;
    uint32_t len;
    uint16_t protocol;
    struct net_device* dev;
    bool     allocated;
    uint8_t  raw_storage[SKB_MAX_DATA];
} sk_buff_t;

struct net_device;

typedef int (*netdev_xmit_fn)(sk_buff_t* skb, struct net_device* dev);

typedef struct net_device {
    char           name[16];        /* e.g., "lo", "eth0" */
    uint8_t        dev_addr[ETH_ALEN];
    uint32_t       ip_addr;
    uint32_t       ip_mask;
    uint32_t       gateway;
    uint32_t       flags;
    uint32_t       mtu;
    uint64_t       rx_packets;
    uint64_t       tx_packets;
    uint64_t       rx_bytes;
    uint64_t       tx_bytes;
    netdev_xmit_fn xmit;
} net_device_t;

/* sk_buff Operations */
sk_buff_t* alloc_skb(uint32_t size);
void kfree_skb(sk_buff_t* skb);
uint8_t* skb_put(sk_buff_t* skb, uint32_t len);
uint8_t* skb_push(sk_buff_t* skb, uint32_t len);
uint8_t* skb_pull(sk_buff_t* skb, uint32_t len);
void skb_reserve(sk_buff_t* skb, uint32_t len);

/* Network Device API */
void net_init(void);
int register_netdev(net_device_t* dev);
net_device_t* get_netdev(const char* name);
uint32_t get_netdev_count(void);
net_device_t* get_netdev_by_index(uint32_t idx);
int dev_queue_xmit(sk_buff_t* skb);
int netif_rx(sk_buff_t* skb);

/* Packet Processing & Protocols */
uint16_t net_checksum(const void* data, size_t len);
void net_rx_packet(net_device_t* dev, const uint8_t* frame, size_t len);
int ipv4_send(net_device_t* dev, uint32_t dest_ip, uint8_t protocol, const void* payload, size_t len);

/* ICMP & Ping */
int icmp_send_ping(net_device_t* dev, uint32_t dest_ip, uint16_t seq);
int icmp_process(net_device_t* dev, const iphdr_t* ip, const uint8_t* payload, size_t len);

/* UDP & TCP */
int udp_send(net_device_t* dev, uint32_t dest_ip, uint16_t src_port, uint16_t dest_port, const void* data, size_t len);
int tcp_send_syn(net_device_t* dev, uint32_t dest_ip, uint16_t src_port, uint16_t dest_port);

/* DHCP Client */
int dhcp_send_discover(net_device_t* dev);

/* ARP API */
void arp_init(void);
int arp_insert(uint32_t ip, const uint8_t mac[ETH_ALEN]);
uint8_t* arp_lookup(uint32_t ip);

#endif /* AIRLYS_NET_H */
