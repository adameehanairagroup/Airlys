#include <airlys/net.h>
#include <airlys/string.h>

typedef struct {
    uint32_t ip;
    uint8_t  mac[ETH_ALEN];
    bool     valid;
} arp_entry_t;

#define ARP_TABLE_SIZE 16
static arp_entry_t s_arp_table[ARP_TABLE_SIZE];

void arp_init(void) {
    kmemset(s_arp_table, 0, sizeof(s_arp_table));
}

int arp_insert(uint32_t ip, const uint8_t mac[ETH_ALEN]) {
    for (int i = 0; i < ARP_TABLE_SIZE; i++) {
        if (!s_arp_table[i].valid || s_arp_table[i].ip == ip) {
            s_arp_table[i].ip = ip;
            kmemcpy(s_arp_table[i].mac, mac, ETH_ALEN);
            s_arp_table[i].valid = true;
            return 0;
        }
    }
    return -1;
}

uint8_t* arp_lookup(uint32_t ip) {
    for (int i = 0; i < ARP_TABLE_SIZE; i++) {
        if (s_arp_table[i].valid && s_arp_table[i].ip == ip) {
            return s_arp_table[i].mac;
        }
    }
    return NULL;
}
