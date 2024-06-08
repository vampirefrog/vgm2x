#include <string.h>
#include <stdlib.h>

#include "chip_analyzer.h"

int chip_analyzer_init(struct chip_analyzer *analyzer, int clock, int num_tracks) {
	memset(analyzer, 0, sizeof(*analyzer));
	analyzer->clock = clock;
	analyzer->num_tracks = num_tracks;
	analyzer->tracks = calloc(num_tracks, sizeof(analyzer->tracks[0]));
	if(!analyzer->tracks) return -1;
	for(int i = 0; i < num_tracks; i++) {
		midi_track_init(&analyzer->tracks[i].midi_track);
		analyzer->tracks[i].wait_samples = 0;
		analyzer->tracks[i].on_note = -1;
	}
	return 0;
}

void chip_analyzer_set_data_ptr(struct chip_analyzer *analyzer, void *data_ptr) {
	analyzer->data_ptr = data_ptr;
}

void chip_analyzer_cmd_reg8_data8(struct chip_analyzer *analyzer, uint8_t reg, uint8_t data) {
	if(analyzer->cmd_reg8_data8) analyzer->cmd_reg8_data8(analyzer, reg, data, analyzer->data_ptr);
}

void chip_analyzer_cmd_ofs8_data8(struct chip_analyzer *analyzer, uint8_t ofs, uint8_t data) {
	if(analyzer->cmd_ofs8_data8) analyzer->cmd_ofs8_data8(analyzer, ofs, data, analyzer->data_ptr);
}

void chip_analyzer_cmd_ofs16_data8(struct chip_analyzer *analyzer, uint16_t ofs, uint8_t data) {
	if(analyzer->cmd_ofs16_data8) analyzer->cmd_ofs16_data8(analyzer, ofs, data, analyzer->data_ptr);
}

void chip_analyzer_cmd_ofs16_data16(struct chip_analyzer *analyzer, uint16_t ofs, uint16_t data) {
	if(analyzer->cmd_ofs16_data16) analyzer->cmd_ofs16_data16(analyzer, ofs, data, analyzer->data_ptr);
}

void chip_analyzer_cmd_port8_reg8_data8(struct chip_analyzer *analyzer, uint8_t port, uint8_t reg, uint8_t data) {
	if(analyzer->cmd_port8_reg8_data8) analyzer->cmd_port8_reg8_data8(analyzer, port, reg, data, analyzer->data_ptr);
}

void chip_analyzer_cmd_write8(struct chip_analyzer *analyzer, uint8_t data) {
	if(analyzer->cmd_write8) analyzer->cmd_write8(analyzer, data, analyzer->data_ptr);
}

void chip_analyzer_wait(struct chip_analyzer *analyzer, int wait_samples) {
	for(int i = 0; i < analyzer->num_tracks; i++)
		analyzer->tracks[i].wait_samples += wait_samples;
}

void chip_analyzer_note_on(struct chip_analyzer *analyzer, int track, int note, int velocity) {
	midi_track_write_note_on(&analyzer->tracks[track].midi_track, analyzer->tracks[track].wait_samples, track, note, velocity);
	analyzer->tracks[track].on_note = note;
	analyzer->tracks[track].wait_samples = 0;
}

void chip_analyzer_note_off(struct chip_analyzer *analyzer, int track) {
	if(analyzer->tracks[track].on_note >= 0) {
		midi_track_write_note_off(&analyzer->tracks[track].midi_track, analyzer->tracks[track].wait_samples, track, analyzer->tracks[track].on_note, 0);
		analyzer->tracks[track].on_note = -1;
	}
	analyzer->tracks[track].wait_samples = 0;
}
