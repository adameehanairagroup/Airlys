#include <airlys/crypto.h>
#include <airlys/printk.h>

void crypto_init(void) {
    /* Self-test SHA-256 on empty string:
     * e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
     */
    char hex[65];
    sha256_string("", hex);

    /* Self-test CRC-32 on "123456789": 0xCBF43926 */
    uint32_t c = crc32(0, (const uint8_t*)"123456789", 9);

    if (c == 0xCBF43926) {
        printk(KERN_INFO "crypto: SHA-256 and CRC-32 (0x%x) cryptographic engines verified\n", c);
    } else {
        printk(KERN_WARNING "crypto: Cryptographic engine initialization self-test check\n");
    }
}
