#ifndef AIRLYS_CRYPTO_H
#define AIRLYS_CRYPTO_H

/*
 * Airlys Operating System Kernel - Cryptographic Architecture (Linux crypto/ Parity)
 * Lead Architect: Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
 * Co-Pilot: Sovereign AI Aira 3 Ultra (Engine v3.6)
 */

#include <airlys/types.h>

/* SHA-256 (FIPS 180-4) */
typedef struct {
    uint32_t state[8];
    uint64_t count;
    uint8_t buffer[64];
} sha256_ctx_t;

void sha256_init(sha256_ctx_t* ctx);
void sha256_update(sha256_ctx_t* ctx, const uint8_t* data, size_t len);
void sha256_final(sha256_ctx_t* ctx, uint8_t hash[32]);
void sha256_string(const char* str, char hex_out[65]);

/* Standard IEEE 802.3 CRC-32 */
uint32_t crc32(uint32_t crc, const uint8_t* data, size_t len);

/* Crypto Subsystem Init */
void crypto_init(void);

#endif /* AIRLYS_CRYPTO_H */
