#pragma once

#include <stdint.h>
#include "libfmvoice/fm_voice.h"

struct opn_voice_collector_voice {
	struct opn_voice voice;
	int vgm_ofs;
	uint8_t chan_used_mask;
	int note_usage[128];
};

struct opn_voice_collector {
	struct opn_voice_collector_voice *voices;
	int num_voices;
};

void opn_voice_collector_init(struct opn_voice_collector *c);
void opn_voice_collector_push_voice(struct opn_voice_collector *c, struct opn_voice_collector_voice *v, int chan);
void opn_voice_collector_dump_voices(struct opn_voice_collector *c);
