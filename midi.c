#include <math.h>

#include "midi.h"

float midi_note_freq(int note, float cents) {
	return 440.0 * pow(2, (note + cents / 100 - 69) / 12.0);
}

int midi_pitch_to_note(float pitch_hz, float *cents) {
	float f = 12 * log(pitch_hz / 220.0) / log(2.0);
	int note = round(f);
	if(cents) *cents = (f - note) * 100;
	return (int)note + 57;
}
