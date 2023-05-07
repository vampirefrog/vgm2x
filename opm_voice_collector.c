#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "opm_voice_collector.h"

void opm_voice_collector_init(struct opm_voice_collector *collector) {
	memset(collector, 0, sizeof(*collector));
}

void opm_voice_collector_push_voice(struct opm_voice_collector *collector, struct opm_voice *voice, int chan) {
	const uint8_t slot_masks[8] = { 0x08,0x08,0x08,0x08,0x0c,0x0e,0x0e,0x0f };

	/* Fix volume */
	uint8_t slot_mask = slot_masks[voice->fb_connect & 0x07];
	uint8_t min_tl = 127;

	for(int i = 0, m = 1; i < 4; i++, m <<= 1) {
		if(!(slot_mask & m)) continue;
		if(voice->operators[i].tl >= min_tl) continue;
		min_tl = voice->operators[i].tl;
	}

	for(int i = 0, m = 1; i < 4; i++, m <<= 1) {
		if(!(slot_mask & m)) continue;
		voice->operators[i].tl -= min_tl;
	}

	int existing_voice = -1;
	for(int i = 0; i < collector->num_voices; i++) {
		struct opm_voice *v = collector->voices + i;
		if((v->fb_connect & 0x3f) != (voice->fb_connect & 0x3f)) continue;
		int good = 1;
		for(int j = 0, m = 1; j < 4; j++, m <<= 1) {
			struct opm_voice_operator *o1 = &v->operators[j];
			struct opm_voice_operator *o2 = &voice->operators[j];
			if(
				(o1->dt1_mul & 0x7f) != (o2->dt1_mul & 0x7f) ||
				(o1->ks_ar & 0xdf) != (o2->ks_ar & 0xdf) ||
				(o1->ame_d1r & 0x9f) != (o2->ame_d1r & 0x9f) ||
				(o1->dt2_d2r & 0xdf) != (o2->dt2_d2r & 0xdf) ||
				(o1->tl & 0x7f) != (o2->tl & 0x7f) ||
				o1->d1l_rr != o2->d1l_rr
			) {
				good = 0;
				break;
			}
		}
		if(!good) continue;

		existing_voice = i;
		break;
	}

	if(existing_voice >= 0) {
		struct opm_voice *v = &collector->voices[existing_voice];
		v->chan_used_mask |= 1 << chan;
		return;
	}

	collector->num_voices++;
	collector->voices = realloc(collector->voices, collector->num_voices * sizeof(struct opm_voice));
	if(!collector->voices) {
		fprintf(stderr, "Could not reallocate %d OPM voices\n", collector->num_voices);
		return;
	}
	memcpy(&collector->voices[collector->num_voices - 1], voice, sizeof(*voice));
}

void opm_voice_dump(struct opm_voice *v) {
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

void opm_voice_collector_dump_voices(struct opm_voice_collector *collector) {
	for(int i = 0; i < collector->num_voices; i++) {
		printf("Voice %d\n", i);
		opm_voice_dump(collector->voices + i);
	}
}
