#pragma once

#include <stdint.h>

#include "error.h"

/* Push mode command parser,
 * usable for both types of readers, push and pull mode.
 */

struct vgm_command_parser {
	void *data_ptr;
	void (*cmd_cb)(struct vgm_command_parser *p, uint8_t *cmd_data, int cmd_data_len, void *data_ptr);
	void (*data_block_chunk_cb)(struct vgm_command_parser *p, uint8_t *data, int data_len, void *data_ptr);
	enum {
		IN_COMMAND,
		IN_DATA_BLOCK_DATA,
	} state;
	int cmd_len;
	uint8_t cmd_buf[16];
	int cmd_buf_pos;
};

void vgm_command_parser_init(struct vgm_command_parser *p);
enum vgm_error_code vgm_command_parser_push(struct vgm_command_parser *p, uint8_t *bytes, int num_bytes);
