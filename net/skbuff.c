#include <airlys/net.h>
#include <airlys/string.h>

#define SKB_POOL_SIZE 16
static sk_buff_t s_skb_pool[SKB_POOL_SIZE];

sk_buff_t* alloc_skb(uint32_t size) {
    if (size > SKB_MAX_DATA) return NULL;

    for (int i = 0; i < SKB_POOL_SIZE; i++) {
        if (!s_skb_pool[i].allocated) {
            sk_buff_t* skb = &s_skb_pool[i];
            kmemset(skb, 0, sizeof(sk_buff_t));
            skb->allocated = true;
            skb->head = skb->raw_storage;
            skb->data = skb->raw_storage;
            skb->tail = skb->raw_storage;
            skb->end = skb->raw_storage + sizeof(skb->raw_storage);
            skb->len = 0;
            return skb;
        }
    }
    return NULL;
}

void kfree_skb(sk_buff_t* skb) {
    if (!skb) return;
    skb->allocated = false;
    skb->len = 0;
}

uint8_t* skb_put(sk_buff_t* skb, uint32_t len) {
    if (!skb || skb->tail + len > skb->end) return NULL;
    uint8_t* prev_tail = skb->tail;
    skb->tail += len;
    skb->len += len;
    return prev_tail;
}

uint8_t* skb_push(sk_buff_t* skb, uint32_t len) {
    if (!skb || skb->data - len < skb->head) return NULL;
    skb->data -= len;
    skb->len += len;
    return skb->data;
}

uint8_t* skb_pull(sk_buff_t* skb, uint32_t len) {
    if (!skb || len > skb->len) return NULL;
    skb->data += len;
    skb->len -= len;
    return skb->data;
}

void skb_reserve(sk_buff_t* skb, uint32_t len) {
    if (!skb || skb->data + len > skb->end) return;
    skb->data += len;
    skb->tail += len;
}
