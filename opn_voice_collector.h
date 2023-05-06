#pragma once

#include <stdint.h>

struct opn_voice_operator {
	uint8_t dt_mul;
	uint8_t tl;
	uint8_t ks_ar;
	uint8_t am_dr;
	uint8_t sr;
	uint8_t sl_rr;
	uint8_t ssg_eg;
};

struct opn_voice {
	uint8_t fb_connect;
	int vgm_ofs;
	uint8_t chan_used_mask;
	struct opn_voice_operator operators[4];
};

struct opn_voice_collector {
	struct opn_voice *voices;
	int num_voices;
};

void opn_voice_collector_init(struct opn_voice_collector *c);
void opn_voice_collector_push_voice(struct opn_voice_collector *c, struct opn_voice *v, int chan);
void opn_voice_collector_dump_voices(struct opn_voice_collector *c);
