#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "opn_voice_collector.h"

void opn_voice_collector_init(struct opn_voice_collector *collector) {
	memset(collector, 0, sizeof(*collector));
}

void opn_voice_collector_push_voice(struct opn_voice_collector *collector, struct opn_voice_collector_voice *voice, int chan, int midi_note) {
	opn_voice_normalize(&voice->voice);

	int existing_voice = -1;
	for(int i = 0; i < collector->num_voices; i++) {
		struct opn_voice_collector_voice *v = collector->voices + i;
		if(!opn_voice_compare(&voice->voice, &v->voice)) {
			existing_voice = i;
			break;
		}
	}

	if(existing_voice >= 0) {
		struct opn_voice_collector_voice *v = &collector->voices[existing_voice];
		if(chan >= 0) v->chan_used_mask |= 1 << chan;
		if(midi_note >= 0) v->note_usage[midi_note & 0x7f]++;
		return;
	}

	collector->num_voices++;
	collector->voices = realloc(collector->voices, collector->num_voices * sizeof(struct opn_voice_collector_voice));
	if(!collector->voices) {
		fprintf(stderr, "Could not reallocate %d OPN voices\n", collector->num_voices);
		return;
	}
	if(chan >= 0) voice->chan_used_mask |= 1 << chan;
	if(midi_note >= 0) voice->note_usage[midi_note & 0x7f]++;
	memcpy(&collector->voices[collector->num_voices - 1], voice, sizeof(*voice));
}

void opn_voice_collector_dump(struct opn_voice_collector_voice *v) {
	printf(
		"fb=%d connect=%d chan_used_mask=%c%c%c%c%c%c%c%c\n",
		v->voice.fb_con >> 3 & 0x07,
		v->voice.fb_con & 0x07,
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
		struct opn_voice_operator *o = v->voice.operators + j;
		printf(
			" %d"
			" %2d"
			" %2d"
			" %2d"
			" %2d"
			" %2d"
			" %3d"
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

void opn_voice_collector_dump_voices(struct opn_voice_collector *collector) {
	for(int i = 0; i < collector->num_voices; i++) {
		printf("Voice %d\n", i);
		opn_voice_collector_dump(collector->voices + i);
	}
}
