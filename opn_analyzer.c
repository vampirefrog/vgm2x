#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "opn_analyzer.h"

static void opn_analyzer_push_voice(struct opn_analyzer *analyzer, uint8_t port, uint8_t chan, uint8_t mask) {
	uint8_t *ofs = analyzer->regs + chan + port * 256;

	const uint8_t slot_masks[8] = { 0x08,0x08,0x08,0x08,0x0c,0x0e,0x0e,0x0f };

	struct opn_voice voice;
	voice.fb_connect = ofs[0xb0];
	// voice.vgm_ofs = vgm_ofs;
	uint8_t slot_mask = slot_masks[voice.fb_connect & 0x07];
	uint8_t min_tl = 127;
	for(int i = 0, m = 1; i < 4; i++, m <<= 1) {
		voice.operators[i].dt_mul   = ofs[0x30 + i * 4] & 0x7f;
		voice.operators[i].tl       = ofs[0x40 + i * 4] & 0x7f;
		voice.operators[i].ks_ar    = ofs[0x50 + i * 4] & 0xdf;
		voice.operators[i].am_dr    = ofs[0x60 + i * 4] & 0x9f;
		voice.operators[i].sr       = ofs[0x70 + i * 4] & 0x1f;
		voice.operators[i].sl_rr    = ofs[0x80 + i * 4];
		voice.operators[i].ssg_eg   = ofs[0x90 + i * 4] & 0x0f;
		if(slot_mask & m) {
			if(voice.operators[i].tl < min_tl)
				min_tl = voice.operators[i].tl;
		}
	}
	for(int i = 0, m = 1; i < 4; i++, m <<= 1) {
		if(slot_mask & m) {
			voice.operators[i].tl -= min_tl;
		}
	}

	int existing_voice = -1;
	for(int i = 0; i < analyzer->num_voices; i++) {
		struct opn_voice *v = analyzer->voices + i;
		if(v->fb_connect != voice.fb_connect) continue;
		int good = 1;
		for(int j = 0, m = 1; j < 4; j++, m <<= 1) {
			struct opn_voice_operator *o1 = &v->operators[j];
			struct opn_voice_operator *o2 = &voice.operators[j];
			if(
				(o1->dt_mul & 0x7f) != (o2->dt_mul & 0x7f) ||
				(o1->ks_ar & 0xdf) != (o2->ks_ar & 0xdf) ||
				(o1->am_dr & 0x9f) != (o2->am_dr & 0x9f) ||
				(o1->sr & 0x1f) != (o2->sr & 0x1f) ||
				(o1->tl & 0x7f) != (o2->tl & 0x7f) ||
				(o1->sl_rr) != (o2->sl_rr)
			) {
				good = 0;
				break;
			}
		}
		if(!good) continue;

		existing_voice = i;
		break;
	}
	if(existing_voice < 0) {
		existing_voice = analyzer->num_voices++;
		analyzer->voices = realloc(analyzer->voices, analyzer->num_voices * sizeof(struct opn_voice));
		if(!analyzer->voices) {
			fprintf(stderr, "Could not reallocate %d OPN voices\n", analyzer->num_voices);
			return;
		}
		memcpy(&analyzer->voices[existing_voice], &voice, sizeof(voice));
	} else {
		struct opn_voice *v = &analyzer->voices[existing_voice];
		v->chan_used_mask |= 1 << (chan + port * 4);
		uint8_t slot_mask = slot_masks[v->fb_connect & 0x07];
		// find lowest attenuation, or highest volume
		for(int i = 0, m = 1; i < 4; i++, m <<= 1) {
			if(slot_mask & m) {
				if(voice.operators[i].tl < v->operators[i].tl)
					v->operators[i].tl = voice.operators[i].tl;
			}
		}
	}
}

static void opn_cmd_port8_reg8_data8(struct chip_analyzer *chip_analyzer, uint8_t port, uint8_t reg, uint8_t data, void *data_ptr) {
	struct opn_analyzer *analyzer = (struct opn_analyzer *)chip_analyzer;

	analyzer->regs[port * 256 + reg] = data;

	if(port == 0 && reg == 0x2b) {
		analyzer->ym_dac = data & 0x80 ? 1 : 0;
		return;
	}

	if(reg == 0x28 && port == 0) {
		uint8_t mask = data >> 4;
		uint8_t chan = data & 0x03;
		uint8_t key_port = data & 0x04 ? 1 : 0;

		if(mask) {
			// ignore DAC writes
			if(chan == 2 && key_port == 1 && analyzer->ym_dac == 1)
				return;
			opn_analyzer_push_voice(analyzer, port, chan, mask);
		}
	}
}

struct opn_analyzer *opn_analyzer_new(int clock, int num_channels) {
	struct opn_analyzer *a = malloc(sizeof(*a));
	if(!a) return 0;
	opn_analyzer_init(a, clock, num_channels);
	return a;
}

void opn_analyzer_init(struct opn_analyzer *analyzer, int clock, int num_channels) {
	chip_analyzer_init(&analyzer->chip_analyzer, clock);
	analyzer->chip_analyzer.cmd_port8_reg8_data8 = opn_cmd_port8_reg8_data8;

	memset(analyzer->regs, 0, sizeof(analyzer->regs) / sizeof(analyzer->regs[0]));
	analyzer->num_voices = 0;
	analyzer->voices = 0;
	analyzer->ym_dac = 0;
	analyzer->num_channels = num_channels;
}

void opn_analyzer_dump_voices(struct opn_analyzer *analyzer) {
	for(int i = 0; i < analyzer->num_voices; i++) {
		printf("Voice %d\n", i);
		struct opn_voice *v = &analyzer->voices[i];
		printf(
			"fb=%d connect=%d chan_used_mask=%c%c%c%c%c%c%c%c\n",
			v->fb_connect >> 3 & 0x07,
			v->fb_connect & 0x07,
			v->chan_used_mask & 1 ? '1' : '0',
			v->chan_used_mask & 2 ? '1' : '0',
			v->chan_used_mask & 4 ? '1' : '0',
			v->chan_used_mask & 8 ? '1' : '0',
			v->chan_used_mask & 16 ? '1' : '0',
			v->chan_used_mask & 32 ? '1' : '0',
			v->chan_used_mask & 64 ? '1' : '0',
			v->chan_used_mask & 128 ? '1' : '0'
		);
		printf("OP AR DR SR RR SL  TL MUL DT KS AME SSG-EG\n");

		for(int j = 0; j < 4; j++) {
			struct opn_voice_operator *o = v->operators + j;
			printf(
				" %d"
				" %2d"
				" %2d"
				" %2d"
				" %2d"
				" %2d"
				"  %2d"
				"  %2d"
				"  %d"
				"  %d"
				"   %d"
				"      %d"
				"\n",
				j,
				o->ks_ar & 0x1f,
				o->am_dr & 0x1f,
				o->sr & 0x1f,
				o->sl_rr & 0x0f,
				o->sl_rr >> 4,
				o->tl & 0x7f,
				o->dt_mul & 0x0f,
				o->dt_mul >> 4 & 0x07,
				o->ks_ar >> 6,
				o->am_dr >> 7,
				o->ssg_eg & 0x0f
			);
		}
	}
}
