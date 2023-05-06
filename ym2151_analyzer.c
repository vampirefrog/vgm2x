#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ym2151_analyzer.h"

static void ym2151_analyzer_push_voice(struct ym2151_analyzer *analyzer, uint8_t chan, uint8_t mask) {
	uint8_t *ofs = analyzer->regs;

	const uint8_t slot_masks[8] = { 0x08,0x08,0x08,0x08,0x0c,0x0e,0x0e,0x0f };

	struct opm_voice voice;
	voice.fb_connect = ofs[0x20];
	// voice.vgm_ofs = vgm_ofs;
	uint8_t slot_mask = slot_masks[voice.fb_connect & 0x07];
	uint8_t min_tl = 127;
	for(int i = 0, m = 1; i < 4; i++, m <<= 1) {
		voice.operators[i].dt1_mul = ofs[0x40 + i * 8] & 0x7f;
		voice.operators[i].tl      = ofs[0x60 + i * 8] & 0x7f;
		voice.operators[i].ks_ar   = ofs[0x80 + i * 8] & 0xdf;
		voice.operators[i].ame_d1r = ofs[0xa0 + i * 8] & 0x9f;
		voice.operators[i].dt2_d2r = ofs[0xc0 + i * 8] & 0x9f;
		voice.operators[i].d1l_rr  = ofs[0xe0 + i * 8];
		if(slot_mask & m) {
			if(voice.operators[i].tl < min_tl)
				min_tl = voice.operators[i].tl;
		}
	}
	for(int i = 0, m = 1; i < 4; i++, m <<= 1) {
		// printf("tl %d (%d): %d - %d f\n", i, slot_mask & m ? 1 : 0, voice.operators[i].tl, min_tl);
		if(slot_mask & m) {
			voice.operators[i].tl -= min_tl;
		}
	}

	int existing_voice = -1;
	for(int i = 0; i < analyzer->num_voices; i++) {
		struct opm_voice *v = analyzer->voices + i;
		if((v->fb_connect & 0x3f) != (voice.fb_connect & 0x3f)) continue;
		int good = 1;
		for(int j = 0, m = 1; j < 4; j++, m <<= 1) {
			struct opm_voice_operator *o1 = &v->operators[j];
			struct opm_voice_operator *o2 = &voice.operators[j];
			if(
				(o1->dt1_mul & 0x7f) != (o2->dt1_mul & 0x7f) ||
				(o1->ks_ar & 0xdf) != (o2->ks_ar & 0xdf) ||
				(o1->ame_d1r & 0x9f) != (o2->ame_d1r & 0x9f) ||
				(o1->dt2_d2r & 0xdf) != (o2->dt2_d2r & 0xdf) ||
				(o1->tl & 0x7f) != (o2->tl & 0x7f) ||
				(o1->d1l_rr) != (o2->d1l_rr)
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
		analyzer->voices = realloc(analyzer->voices, analyzer->num_voices * sizeof(struct opm_voice));
		if(!analyzer->voices) {
			fprintf(stderr, "Could not reallocate %d OPM voices\n", analyzer->num_voices);
			return;
		}
		memcpy(&analyzer->voices[existing_voice], &voice, sizeof(voice));
	} else {
		struct opm_voice *v = &analyzer->voices[existing_voice];
		v->chan_used_mask |= 1 << chan;
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

static void ym2151_cmd_reg8_data8(struct chip_analyzer *chip_analyzer, uint8_t reg, uint8_t data, void *data_ptr) {
	struct ym2151_analyzer *analyzer = (struct ym2151_analyzer *)chip_analyzer;
	if(reg == 0x08) {
		uint8_t mask = (data & 0x78) >> 3;
		uint8_t chan = data & 0x07;
		if(mask) {
			ym2151_analyzer_push_voice(analyzer, chan, mask);
		}
	} else {
		analyzer->regs[reg] = data;
	}
}

struct ym2151_analyzer * ym2151_analyzer_new(int clock) {
	struct ym2151_analyzer *a = malloc(sizeof(*a));
	if(!a) return 0;
	ym2151_analyzer_init(a, clock);
	return a;
}

void ym2151_analyzer_init(struct ym2151_analyzer *analyzer, int clock) {
	chip_analyzer_init(&analyzer->chip_analyzer, clock);
	analyzer->chip_analyzer.cmd_reg8_data8 = ym2151_cmd_reg8_data8;

	memset(analyzer->regs, 0, sizeof(analyzer->regs) / sizeof(analyzer->regs[0]));
	analyzer->num_voices = 0;
	analyzer->voices = 0;
}

void ym2151_analyzer_dump_voices(struct ym2151_analyzer *analyzer) {
	for(int i = 0; i < analyzer->num_voices; i++) {
		printf("Voice %d\n", i);
		struct opm_voice *v = &analyzer->voices[i];
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
		printf("OP AR D1R D2R RR D1L  TL MUL DT1 DT2 KS AME\n");
		for(int j = 0; j < 4; j++) {
			struct opm_voice_operator *o = v->operators + j;
			printf(
				" %d"
				" %2d"
				"  %2d"
				"  %2d"
				" %2d"
				"  %2d"
				" %3d"
				"  %2d"
				"  %2d"
				"   %d"
				"  %d"
				"   %d"
				"\n",
				j,
				o->ks_ar & 0x1f,
				o->ame_d1r & 0x1f,
				o->dt2_d2r & 0x1f,
				o->d1l_rr & 0x0f,
				o->d1l_rr >> 4,
				o->tl,
				o->dt1_mul & 0x0f,
				o->dt1_mul >> 4 & 0x07,
				o->dt2_d2r >> 6,
				o->ks_ar >> 6,
				o->ame_d1r >> 7
			);
		}
	}
}
