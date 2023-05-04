#include "commands.h"

#define VGM_COMMAND(sz) sz,
const int vgm_command_sizes[] = {
	VGM_COMMANDS
};
#undef VGM_COMMAND

int vgm_cmd_size(uint8_t *bytes, int remaining_bytes) {
	if(remaining_bytes < 1) return -1;
	if(*bytes == 0x67) {
		if(remaining_bytes < 7) return -1;
		int cmd_data_len = bytes[3] | bytes[4] << 8 | bytes[5] << 16 | bytes[6] << 24;
		if(remaining_bytes < 7 + cmd_data_len) return -1;
		return 7 + cmd_data_len;
	}
	if(vgm_command_sizes[bytes[0]]) return vgm_command_sizes[bytes[0]];
	return -1;
}
