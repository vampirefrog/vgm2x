#pragma once

#include <stdint.h>
#include "libfmvoice/fm_voice.h"

struct opm_voice_collector_voice {
	struct opm_voice voice;
	int vgm_ofs;
	uint8_t chan_used_mask;
	int note_usage[128];
};

struct opm_voice_collector {
	struct opm_voice_collector_voice *voices;
	int num_voices;
};

void opm_voice_collector_init(struct opm_voice_collector *c);
void opm_voice_collector_push_voice(struct opm_voice_collector *c, struct opm_voice_collector_voice *v, int chan);
void opm_voice_collector_dump_voices(struct opm_voice_collector *c);
