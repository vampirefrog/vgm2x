#pragma once

#include <stdint.h>
#include "midilib/midi_track.h"

struct chip_analyzer_track {
	struct midi_track midi_track;
	int wait_samples;
	int on_note;
};

struct chip_analyzer {
	int clock;
	void *data_ptr;
	void (*cmd_reg8_data8)(struct chip_analyzer *, uint8_t, uint8_t, void *data_ptr);
	void (*cmd_ofs8_data8)(struct chip_analyzer *, uint8_t, uint8_t, void *data_ptr);
	void (*cmd_ofs16_data8)(struct chip_analyzer *, uint16_t, uint8_t, void *data_ptr);
	void (*cmd_ofs16_data16)(struct chip_analyzer *, uint16_t, uint16_t, void *data_ptr);
	void (*cmd_port8_reg8_data8)(struct chip_analyzer *, uint8_t, uint8_t, uint8_t, void *data_ptr);
	void (*cmd_write8)(struct chip_analyzer *, uint8_t, void *data_ptr);
	struct chip_analyzer_track *tracks;
	int num_tracks;
};

int chip_analyzer_init(struct chip_analyzer *analyzer, int clock, int num_tracks);
void chip_analyzer_set_data_ptr(struct chip_analyzer *analyzer, void *data_ptr);
void chip_analyzer_cmd_reg8_data8(struct chip_analyzer *analyzer, uint8_t reg, uint8_t data);
void chip_analyzer_cmd_ofs8_data8(struct chip_analyzer *analyzer, uint8_t ofs, uint8_t data);
void chip_analyzer_cmd_ofs16_data8(struct chip_analyzer *analyzer, uint16_t ofs, uint8_t data);
void chip_analyzer_cmd_ofs16_data16(struct chip_analyzer *analyzer, uint16_t ofs, uint16_t data);
void chip_analyzer_cmd_port8_reg8_data8(struct chip_analyzer *analyzer, uint8_t port, uint8_t reg, uint8_t data);
void chip_analyzer_cmd_write8(struct chip_analyzer *analyzer, uint8_t data);
void chip_analyzer_wait(struct chip_analyzer *analyzer, int wait_samples);
void chip_analyzer_note_on(struct chip_analyzer *analyzer, int track, int note, int velocity);
void chip_analyzer_note_off(struct chip_analyzer *analyzer, int track);
