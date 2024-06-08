#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "midi_file.h"

int midi_file_init(struct midi_file *f, int file_format, int num_tracks, int ticks_per_quarter_note) {
	f->file_format = file_format;
	f->ticks_per_quarter_note = ticks_per_quarter_note;
	f->num_tracks = num_tracks;

	if(num_tracks > 0) {
		f->tracks = malloc(sizeof(struct midi_track) * num_tracks);
		if(!f->tracks) return MIDI_ERR_OUT_OF_MEMORY;
	}

	return MIDI_SUCCESS;
}

int midi_file_clear(struct midi_file *f) {
	if(f->tracks) free(f->tracks);
	if(errno) return errno;
	f->tracks = 0;
	f->num_tracks = 0;

	return 0;
}

int midi_file_write(struct midi_file *f, int (*write_fn)(void *buf, int len, void *data_ptr), void *data_ptr) {
	uint8_t buf[4];
#define WRITE4(a, b, c, d) { buf[0] = a; buf[1] = b; buf[2] = c; buf[3] = d; if(write_fn(buf, 4, data_ptr) != 4) return -1; }
#define WRITE2(a, b) { buf[0] = a; buf[1] = b; if(write_fn(buf, 2, data_ptr) != 2) return -1; }
#define WRITEU32(u) WRITE4(u >> 24, u >> 16, u >> 8, u)
#define WRITEU16(u) WRITE2(u >> 8, u)
	WRITE4('M', 'T', 'h', 'd');
	WRITEU32(6);
	WRITEU16(f->file_format);
	WRITEU16(f->num_tracks);
	WRITEU16(f->ticks_per_quarter_note);

	for(int i = 0; i < f->num_tracks; i++) {
		struct midi_track *t = &f->tracks[i];
		WRITE4('M', 'T', 'r', 'k');
		WRITEU32(t->buffer.data_len);
		if(write_fn(t->buffer.data, t->buffer.data_len, data_ptr) != t->buffer.data_len) return -1;
	}

	return 0;
}

struct midi_track *midi_file_append_track(struct midi_file *f) {
	f->num_tracks++;
	f->tracks = realloc(f->tracks, f->num_tracks * sizeof(struct midi_track));
	if(!f->tracks)
		return 0;
	struct midi_track *track = &f->tracks[f->num_tracks - 1];
	midi_track_init(track);
	return track;
}

struct midi_track *midi_file_prepend_track(struct midi_file *f) {
	f->num_tracks++;
	f->tracks = realloc(f->tracks, f->num_tracks * sizeof(struct midi_track));
	if(!f->tracks)
		return 0;
	memmove(f->tracks + 1, f->tracks, (f->num_tracks - 1) * sizeof(struct midi_track));
	struct midi_track *track = &f->tracks[0];
	midi_track_init(track);
	return track;
}
