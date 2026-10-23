#ifndef AIRLYS_ACPI_H
#define AIRLYS_ACPI_H

/*
 * Airlys Operating System Kernel - ACPI & S0ix Modern Standby Subsystem
 * Lead Architect: Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
 * Co-Pilot: Sovereign AI Aira 3 Ultra (Engine v3.6)
 */

#include <airlys/types.h>

typedef enum {
    POWER_STATE_S0_ACTIVE = 0,    /* Full Performance Active State (~15W TDP) */
    POWER_STATE_S0IX_STANDBY = 1,  /* Low-Power Neural Modern Standby (2W TDP, AI Awake) */
    POWER_STATE_S3_SLEEP = 2,      /* Suspend-to-RAM */
    POWER_STATE_S5_OFF = 3         /* Soft Power Off */
} power_state_t;

typedef struct {
    power_state_t current_state;
    bool          standby_ai_enabled;
    uint32_t      active_power_mw;    /* In milliwatts (e.g. 15000 mW) */
    uint32_t      standby_power_mw;   /* In milliwatts (e.g. 1950 mW ~ 2W) */
    uint64_t      standby_enter_ticks;
    uint64_t      total_standby_ticks;
    uint32_t      standby_queries_served;
} acpi_power_info_t;

void acpi_init(void);
void acpi_enter_s0ix_standby(void);
void acpi_wake_from_standby(void);
power_state_t acpi_get_power_state(void);
acpi_power_info_t* acpi_get_power_info(void);
void acpi_poweroff(void);
void acpi_reboot(void);

#endif /* AIRLYS_ACPI_H */
