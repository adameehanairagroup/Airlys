#ifndef AIRLYS_CERTS_H
#define AIRLYS_CERTS_H

/*
 * Airlys Operating System Kernel - Keyring & Signature Verification (Linux certs/ Parity)
 * Lead Architect: Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
 * Co-Pilot: Sovereign AI Aira 3 Ultra (Engine v3.6)
 */

#include <airlys/types.h>

#define MAX_KEYRING_KEYS 8

typedef struct {
    char     owner[64];
    uint8_t  fingerprint[32];
    uint32_t key_bits;
    bool     trusted;
} sovereign_key_t;

/* Keyring API */
void certs_init(void);
int certs_register_key(const char* owner, const uint8_t fingerprint[32], uint32_t key_bits);
bool certs_verify_signature(const uint8_t* hash, const uint8_t* sig, size_t sig_len);
uint32_t certs_get_key_count(void);
sovereign_key_t* certs_get_key(uint32_t index);

#endif /* AIRLYS_CERTS_H */
