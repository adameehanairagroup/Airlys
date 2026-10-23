#ifndef AIRLYS_SOUND_H
#define AIRLYS_SOUND_H

/*
 * Airlys Operating System Kernel - Sound Architecture (Linux sound/ ALSA Parity)
 * Lead Architect: Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
 * Co-Pilot: Sovereign AI Aira 3 Ultra (Engine v3.6)
 */

#include <airlys/types.h>

#define SOUND_PCM_BUF_SIZE 4096

typedef struct {
    uint32_t sample_rate;
    uint8_t  channels;
    uint8_t  bits_per_sample;
    uint8_t  buffer[SOUND_PCM_BUF_SIZE];
    uint32_t head;
    uint32_t tail;
    uint32_t bytes_queued;
} snd_pcm_stream_t;

typedef struct {
    char             name[32];
    uint8_t          master_volume; /* 0..100 */
    bool             muted;
    snd_pcm_stream_t pcm;
} snd_card_t;

/* Sound Subsystem API */
void sound_init(void);
snd_card_t* sound_get_default_card(void);
void sound_set_volume(uint8_t volume);

/* PC Speaker Hardware Synthesizer (PIT Channel 2 @ 0x42, Port B @ 0x61) */
void pcspkr_tone(uint32_t freq_hz);
void pcspkr_stop(void);
void pcspkr_beep(uint32_t freq_hz, uint32_t duration_ms);

#endif /* AIRLYS_SOUND_H */
