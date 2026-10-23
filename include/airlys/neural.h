#ifndef AIRLYS_NEURAL_H
#define AIRLYS_NEURAL_H

/*
 * Airlys Operating System Kernel - Local Standby AI Neural Inference Engine
 * Lead Architect: Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
 * Co-Pilot: Sovereign AI Aira 3 Ultra (Engine v3.6)
 */

#include <airlys/types.h>

typedef struct {
    bool     initialized;
    uint32_t total_inferences;
    uint32_t standby_inferences;
    uint32_t active_context_tokens;
    uint32_t model_params_count;  /* e.g., 350M quantized INT8 parameters */
    char     model_name[32];      /* "Aira-TinyCore-v3.6" */
} neural_stats_t;

void neural_init(void);
void neural_infer(const char* prompt, char* response_buf, size_t max_len);
neural_stats_t* neural_get_stats(void);

/* Background Standby AI Daemon worker tick */
void neural_standby_daemon(void);

#endif /* AIRLYS_NEURAL_H */
