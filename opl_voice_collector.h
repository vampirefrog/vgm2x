#pragma once

#include <stdint.h>
#include "libfmvoice/fm_voice.h"

struct opl_voice_collector_voice {
	struct opl_voice voice;
	int vgm_ofs;
	uint8_t chan_used_mask;
	int note_usage[128];
};

struct opl_voice_collector {
	struct opl_voice_collector_voice *voices;
	int num_voices;
};

void opl_voice_collector_init(struct opl_voice_collector *c);
void opl_voice_collector_push_voice(struct opl_voice_collector *c, struct opl_voice_collector_voice *v, int chan, int midi_note);
void opl_voice_collector_dump_voices(struct opl_voice_collector *c);
