#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "opl_voice_collector.h"

void opl_voice_collector_init(struct opl_voice_collector *collector) {
	memset(collector, 0, sizeof(*collector));
}

void opl_voice_collector_push_voice(struct opl_voice_collector *collector, struct opl_voice_collector_voice *voice, int chan) {
	opl_voice_normalize(&voice->voice);

	int existing_voice = -1;
	for(int i = 0; i < collector->num_voices; i++) {
		struct opl_voice_collector_voice *v = collector->voices + i;
		if(!opl_voice_compare(&voice->voice, &v->voice)) {
			existing_voice = i;
			break;
		}
	}

	if(existing_voice >= 0) {
		struct opl_voice_collector_voice *v = &collector->voices[existing_voice];
		v->chan_used_mask |= 1 << chan;
		return;
	}

	collector->num_voices++;
	collector->voices = realloc(collector->voices, collector->num_voices * sizeof(struct opl_voice_collector_voice));
	if(!collector->voices) {
		fprintf(stderr, "Could not reallocate %d OPN voices\n", collector->num_voices);
		return;
	}
	voice->chan_used_mask |= 1 << chan;
	memcpy(&collector->voices[collector->num_voices - 1], voice, sizeof(*voice));
}

void opl_voice_collector_dump(struct opl_voice_collector_voice *v) {
	// printf(
	// 	"fb=%d connect=%d chan_used_mask=%c%c%c%c%c%c%c%c\n",
	// 	v->voice.fb_con >> 3 & 0x07,
	// 	v->voice.fb_con & 0x07,
	// 	v->chan_used_mask & 1 ? '1' : '0',
	// 	v->chan_used_mask & 2 ? '1' : '0',
	// 	v->chan_used_mask & 4 ? '1' : '0',
	// 	v->chan_used_mask & 8 ? '1' : '0',
	// 	v->chan_used_mask & 16 ? '1' : '0',
	// 	v->chan_used_mask & 32 ? '1' : '0',
	// 	v->chan_used_mask & 64 ? '1' : '0',
	// 	v->chan_used_mask & 128 ? '1' : '0'
	// );
	// printf("OP AR DR SR RR SL  TL MUL DT KS AME SSG-EG\n");

	// for(int j = 0; j < 4; j++) {
	// 	struct opl_voice_operator *o = v->voice.operators + j;
	// 	printf(
	// 		" %d"
	// 		" %2d"
	// 		" %2d"
	// 		" %2d"
	// 		" %2d"
	// 		" %2d"
	// 		" %3d"
	// 		"  %2d"
	// 		"  %d"
	// 		"  %d"
	// 		"   %d"
	// 		"      %d"
	// 		"\n",
	// 		j,
	// 		o->ks_ar & 0x1f,
	// 		o->am_dr & 0x1f,
	// 		o->sr & 0x1f,
	// 		o->sl_rr & 0x0f,
	// 		o->sl_rr >> 4,
	// 		o->tl & 0x7f,
	// 		o->dt_mul & 0x0f,
	// 		o->dt_mul >> 4 & 0x07,
	// 		o->ks_ar >> 6,
	// 		o->am_dr >> 7,
	// 		o->ssg_eg & 0x0f
	// 	);
	// }
}

void opl_voice_collector_dump_voices(struct opl_voice_collector *collector) {
	for(int i = 0; i < collector->num_voices; i++) {
		printf("Voice %d\n", i);
		opl_voice_collector_dump(collector->voices + i);
	}
}
