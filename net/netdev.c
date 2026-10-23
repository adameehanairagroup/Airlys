#include <airlys/net.h>
#include <airlys/string.h>
#include <airlys/printk.h>

static net_device_t g_net_devs[MAX_NET_DEVS];
static uint32_t     g_num_net_devs = 0;

static int loopback_xmit(sk_buff_t* skb, net_device_t* dev) {
    if (!skb || !dev) return -1;
    dev->tx_packets++;
    dev->tx_bytes += skb->len;

    /* Loopback directly loops packet into rx */
    net_rx_packet(dev, skb->data, skb->len);
    kfree_skb(skb);
    return 0;
}

static int eth_xmit(sk_buff_t* skb, net_device_t* dev) {
    if (!skb || !dev) return -1;
    dev->tx_packets++;
    dev->tx_bytes += skb->len;

    /* Simulate wire reflection or hardware transmit */
    kfree_skb(skb);
    return 0;
}

static net_device_t s_loopback_dev = {
    .name = "lo",
    .dev_addr = {0, 0, 0, 0, 0, 0},
    .ip_addr = 0x7F000001, /* 127.0.0.1 */
    .ip_mask = 0xFF000000, /* 255.0.0.0 */
    .gateway = 0,
    .flags = 0x01,         /* UP */
    .mtu = 65536,
    .rx_packets = 0,
    .tx_packets = 0,
    .rx_bytes = 0,
    .tx_bytes = 0,
    .xmit = loopback_xmit
};

static net_device_t s_eth0_dev = {
    .name = "eth0",
    .dev_addr = {0x52, 0x54, 0x00, 0x12, 0x34, 0x56},
    .ip_addr = 0xC0A80189, /* 192.168.1.137 */
    .ip_mask = 0xFFFFFF00, /* 255.255.255.0 */
    .gateway = 0xC0A80101, /* 192.168.1.1 */
    .flags = 0x03,         /* UP, BROADCAST */
    .mtu = 1500,
    .rx_packets = 0,
    .tx_packets = 0,
    .rx_bytes = 0,
    .tx_bytes = 0,
    .xmit = eth_xmit
};

void net_init(void) {
    g_num_net_devs = 0;
    kmemset(g_net_devs, 0, sizeof(g_net_devs));
    arp_init();

    register_netdev(&s_loopback_dev);
    register_netdev(&s_eth0_dev);

    /* Prime ARP cache with default gateway */
    uint8_t gw_mac[6] = {0x52, 0x54, 0x00, 0x12, 0x34, 0x01};
    arp_insert(s_eth0_dev.gateway, gw_mac);

    printk(KERN_INFO "net: TCP/IP Stack online (eth0 @ 192.168.1.137, lo @ 127.0.0.1)\n");
}

int register_netdev(net_device_t* dev) {
    if (!dev || g_num_net_devs >= MAX_NET_DEVS) return -1;
    kmemcpy(&g_net_devs[g_num_net_devs], dev, sizeof(net_device_t));
    g_num_net_devs++;
    return 0;
}

net_device_t* get_netdev(const char* name) {
    if (!name) return NULL;
    for (uint32_t i = 0; i < g_num_net_devs; i++) {
        if (kstrcmp(g_net_devs[i].name, name) == 0) {
            return &g_net_devs[i];
        }
    }
    return NULL;
}

uint32_t get_netdev_count(void) {
    return g_num_net_devs;
}

net_device_t* get_netdev_by_index(uint32_t idx) {
    if (idx >= g_num_net_devs) return NULL;
    return &g_net_devs[idx];
}

int dev_queue_xmit(sk_buff_t* skb) {
    if (!skb || !skb->dev || !skb->dev->xmit) return -1;
    return skb->dev->xmit(skb, skb->dev);
}

int netif_rx(sk_buff_t* skb) {
    if (!skb) return -1;
    if (skb->dev) {
        skb->dev->rx_packets++;
        skb->dev->rx_bytes += skb->len;
    }
    kfree_skb(skb);
    return 0;
}
