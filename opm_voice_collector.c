#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "opm_voice_collector.h"

void opm_voice_collector_init(struct opm_voice_collector *collector) {
	memset(collector, 0, sizeof(*collector));
}

void opm_voice_collector_push_voice(struct opm_voice_collector *collector, struct opm_voice_collector_voice *voice, int chan, int midi_note) {
	opm_voice_normalize(&voice->voice);

	int existing_voice = -1;
	for(int i = 0; i < collector->num_voices; i++) {
		struct opm_voice_collector_voice *v = collector->voices + i;
		if(!opm_voice_compare(&voice->voice, &v->voice)) {
			existing_voice = i;
			break;
		}
	}

	if(existing_voice >= 0) {
		struct opm_voice_collector_voice *v = &collector->voices[existing_voice];
		v->chan_used_mask |= 1 << chan;
		return;
	}

	collector->num_voices++;
	collector->voices = realloc(collector->voices, collector->num_voices * sizeof(struct opm_voice_collector_voice));
	if(!collector->voices) {
		fprintf(stderr, "Could not reallocate %d OPM voices\n", collector->num_voices);
		return;
	}
	voice->chan_used_mask |= 1 << chan;
	memcpy(&collector->voices[collector->num_voices - 1], voice, sizeof(*voice));
}

void opm_voice_collector_voice_dump(struct opm_voice_collector_voice *v) {
	printf(
		"fb=%d connect=%d chan_used_mask=%c%c%c%c%c%c%c%c\n",
		v->voice.rl_fb_con >> 3 & 0x07,
		v->voice.rl_fb_con & 0x07,
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
		struct opm_voice_operator *o = v->voice.operators + j;
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
			o->ams_d1r & 0x1f,
			o->dt2_d2r & 0x1f,
			o->d1l_rr & 0x0f,
			o->d1l_rr >> 4,
			o->tl,
			o->dt1_mul & 0x0f,
			o->dt1_mul >> 4 & 0x07,
			o->dt2_d2r >> 6,
			o->ks_ar >> 6,
			o->ams_d1r >> 7
		);
	}
}

void opm_voice_collector_dump_voices(struct opm_voice_collector *collector) {
	for(int i = 0; i < collector->num_voices; i++) {
		printf("Voice %d\n", i);
		opm_voice_collector_voice_dump(collector->voices + i);
	}
}
