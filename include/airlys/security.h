#ifndef AIRLYS_SECURITY_H
#define AIRLYS_SECURITY_H

#include <airlys/types.h>

/* Airlys Sovereign Sentinel Security Subsystem */
void security_init(void);
bool security_audit_kernel_integrity(void);
bool security_verify_ring_boundary(void);

#endif /* AIRLYS_SECURITY_H */
