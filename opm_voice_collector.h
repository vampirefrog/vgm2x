#pragma once

#include <stdint.h>

struct opm_voice_operator {
	uint8_t dt1_mul;
	uint8_t tl;
	uint8_t ks_ar;
	uint8_t ame_d1r;
	uint8_t dt2_d2r;
	uint8_t d1l_rr;
};

struct opm_voice {
	uint8_t fb_connect;
	int vgm_ofs;
	uint8_t chan_used_mask;
	int note_usage[128];
	uint8_t sm, pms_ams;
	struct opm_voice_operator operators[4];
};

void opm_voice_dump(struct opm_voice *v);

struct opm_voice_collector {
	struct opm_voice *voices;
	int num_voices;
};

void opm_voice_collector_init(struct opm_voice_collector *c);
void opm_voice_collector_push_voice(struct opm_voice_collector *c, struct opm_voice *v, int chan);
void opm_voice_collector_dump_voices(struct opm_voice_collector *c);
