#include <string.h>

#include "command_parser.h"
#include "commands.h"

void vgm_command_parser_init(struct vgm_command_parser *p) {
	memset(p, 0, sizeof(*p));
}

enum vgm_error_code vgm_command_parser_push(struct vgm_command_parser *p, uint8_t *bytes, int num_bytes) {
	for(int i = 0; i < num_bytes; i++) {
		switch(p->state) {
			case IN_COMMAND:
				if(p->cmd_buf_pos == 0) {
					p->cmd_len = vgm_command_sizes[bytes[i]];
					if(bytes[i] == 0x67) {
						p->cmd_len = 7;
					}
					if(p->cmd_len == 0x00) return INVALID_CMD;
				}
				if(p->cmd_buf_pos < sizeof(p->cmd_buf))
					p->cmd_buf[p->cmd_buf_pos++] = bytes[i];
				if(p->cmd_buf_pos >= p->cmd_len) {
					if(p->cmd_cb)
						p->cmd_cb(p, p->cmd_buf, p->cmd_len, p->data_ptr);
					if(p->cmd_buf[0] == 0x67) {
						p->state = IN_DATA_BLOCK_DATA;
						p->cmd_len = p->cmd_buf[3] | p->cmd_buf[4] << 8 | p->cmd_buf[5] << 16 | p->cmd_buf[6] << 24;
					}
					p->cmd_buf_pos = 0;
				}

				break;
			case IN_DATA_BLOCK_DATA:
				int remaining_data_len = num_bytes - i;
				if(p->cmd_len <= remaining_data_len) {
					remaining_data_len = p->cmd_len;
					p->state = IN_COMMAND;
				}
				if(p->data_block_chunk_cb)
					p->data_block_chunk_cb(p, bytes + i, remaining_data_len, p->data_ptr);
				i += remaining_data_len - 1;
				p->cmd_len -= remaining_data_len;
				break;
		}
	}
	return SUCCESS;
}
