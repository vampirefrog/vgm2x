#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "opn_voice_collector.h"

void opn_voice_collector_init(struct opn_voice_collector *collector) {
	memset(collector, 0, sizeof(*collector));
}

void opn_voice_collector_push_voice(struct opn_voice_collector *collector, struct opn_voice *voice, int chan) {
	const uint8_t slot_masks[8] = { 0x08,0x08,0x08,0x08,0x0c,0x0e,0x0e,0x0f };

	// maximize volume
	uint8_t slot_mask = slot_masks[voice->fb_connect & 0x07];

	uint8_t min_tl = 127;
	for(int i = 0, m = 1; i < 4; i++, m <<= 1) {
		if(slot_mask & m) {
			if(voice->operators[i].tl < min_tl)
				min_tl = voice->operators[i].tl;
		}
	}

	for(int i = 0, m = 1; i < 4; i++, m <<= 1) {
		if(slot_mask & m) {
			voice->operators[i].tl -= min_tl;
		}
	}

	int existing_voice = -1;
	for(int i = 0; i < collector->num_voices; i++) {
		struct opn_voice *v = collector->voices + i;
		if(v->fb_connect != voice->fb_connect) continue;
		int good = 1;
		for(int j = 0, m = 1; j < 4; j++, m <<= 1) {
			struct opn_voice_operator *o1 = &v->operators[j];
			struct opn_voice_operator *o2 = &voice->operators[j];
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
		existing_voice = collector->num_voices++;
		collector->voices = realloc(collector->voices, collector->num_voices * sizeof(struct opn_voice));
		if(!collector->voices) {
			fprintf(stderr, "Could not reallocate %d OPN voices\n", collector->num_voices);
			return;
		}
		memcpy(&collector->voices[existing_voice], voice, sizeof(*voice));
	} else {
		struct opn_voice *v = &collector->voices[existing_voice];
		v->chan_used_mask |= 1 << chan;
	}
}

void opn_voice_collector_dump_voices(struct opn_voice_collector *collector) {
	for(int i = 0; i < collector->num_voices; i++) {
		printf("Voice %d\n", i);
		struct opn_voice *v = &collector->voices[i];
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
