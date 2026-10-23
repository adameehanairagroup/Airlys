#include <airlys/sound.h>
#include <airlys/string.h>
#include <airlys/printk.h>

static snd_card_t s_default_card;

void sound_init(void) {
    kmemset(&s_default_card, 0, sizeof(snd_card_t));
    kstrcpy(s_default_card.name, "Airlys Sovereign Audio Core");
    s_default_card.master_volume = 80;
    s_default_card.muted = false;

    s_default_card.pcm.sample_rate = 44100;
    s_default_card.pcm.channels = 2;
    s_default_card.pcm.bits_per_sample = 16;
    s_default_card.pcm.head = 0;
    s_default_card.pcm.tail = 0;
    s_default_card.pcm.bytes_queued = 0;

    printk(KERN_INFO "sound: ALSA Core sound subsystem initialized (%s)\n", s_default_card.name);
}

snd_card_t* sound_get_default_card(void) {
    return &s_default_card;
}

void sound_set_volume(uint8_t volume) {
    if (volume > 100) volume = 100;
    s_default_card.master_volume = volume;
}
