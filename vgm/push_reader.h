#pragma once

#include <stdint.h>

#include "error.h"

/*
 * VGM file push mode reader,
 * suitable for reading VGM files from stdin,
 * or for performing a hex dump along with a
 * per-command analysis (like vgm2txt)
 */

struct vgm_push_reader {
	struct {
		void *data_ptr;
		void (*header_data_cb)(struct vgm_push_reader *, uint8_t *bytes, int num_bytes, void *data_ptr);
		void (*cmd_data_cb)(struct vgm_push_reader *, uint8_t *bytes, int num_bytes, void *data_ptr);
		void (*extra_header_data_cb)(struct vgm_push_reader *, uint8_t *bytes, int num_bytes, void *data_ptr);
		void (*gd3_data_cb)(struct vgm_push_reader *, uint8_t *bytes, int num_bytes, void *data_pr);
	} callbacks;
	enum {
		IN_HEADER,
		IN_COMMANDS,
		IN_EXTRA_HEADER,
		IN_GD3,
	} state;
	int filepos;
	uint32_t version, gd3_offset, vgm_data_offset, extra_header_offset;
};

void vgm_push_reader_init(struct vgm_push_reader *d);
enum vgm_error_code vgm_push_reader_push(struct vgm_push_reader *d, uint8_t *data, int len, struct vgm_error *error);
enum vgm_error_code vgm_push_reader_push_eof(struct vgm_push_reader *d, struct vgm_error *error);
