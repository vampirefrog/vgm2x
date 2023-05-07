#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "tools.h"
#include "../push_reader.h"
#include "../command_parser.h"

static void hexdump(uint8_t *bytes, int num_bytes) {
	for(int i = 0; i < num_bytes; i+=16) {
		for(int j = 0; j < 16; j++) {
			if(i + j < num_bytes) printf("%02x ", bytes[i + j]);
			else printf("   ");
		}

		for(int j = 0; j < 16; j++) {
			if(i + j < num_bytes) printf("%c", safechar(bytes[i + j]));
		}

		printf("\n");
	}
}

void header_data_cb(struct vgm_push_reader *, uint8_t *bytes, int num_bytes, void *data_ptr) {
}

void cmd_data_cb(struct vgm_push_reader *, uint8_t *bytes, int num_bytes, void *data_ptr) {
	struct vgm_command_parser *p = (struct vgm_command_parser *)data_ptr;
	enum vgm_error_code er = vgm_command_parser_push(p, bytes, num_bytes);
}

void extra_header_data_cb(struct vgm_push_reader *, uint8_t *bytes, int num_bytes, void *data_ptr) {
}

void gd3_data_cb(struct vgm_push_reader *, uint8_t *bytes, int num_bytes, void *data_ptr) {
}

void cmd_cb(struct vgm_command_parser *p, uint8_t *cmd_data, int cmd_data_len, void *data_ptr) {
	hexdump(cmd_data, cmd_data_len);
	// printf("cmd_cb %d\n", cmd_data_len);
}

void data_block_chunk_cb(struct vgm_command_parser *p, uint8_t *data, int data_len, void *data_ptr) {
	hexdump(data, data_len);
	printf("data_block_chunk_cb %d\n", data_len);
}

int main(int argc, char **argv) {
	for(int i = 1; i < argc; i++) {
		FILE *f = fopen(argv[i], "rb");
		if(!f) {
			fprintf(stderr, "Could not open %s: %s (%d)\n", argv[i], strerror(errno), errno);
			return 1;
		}

		struct vgm_push_reader d;
		vgm_push_reader_init(&d);

		struct vgm_command_parser p;
		vgm_command_parser_init(&p);
		p.cmd_cb = cmd_cb;
		p.data_block_chunk_cb = data_block_chunk_cb;
		d.callbacks.data_ptr = &p;
		d.callbacks.header_data_cb = header_data_cb;
		d.callbacks.cmd_data_cb = cmd_data_cb;
		d.callbacks.extra_header_data_cb = extra_header_data_cb;
		d.callbacks.gd3_data_cb = gd3_data_cb;
		uint8_t buf[1024];
		size_t s;
		while((s = fread(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), f))) {
			struct vgm_error e;
			enum vgm_error_code ec = vgm_push_reader_push(&d, buf, s, &e);
			if(ec != SUCCESS) {
				char erbuf[256];
				vgm_get_error_string(erbuf, sizeof(erbuf), ec, &e);
				fprintf(stderr, "Could not deserialize %d bytes: %s\n", (int)s, erbuf);
				break;
			}
		}

		if(fclose(f)) {
			fprintf(stderr, "Warning: Could not close %s: %s (%d)\n", argv[i], strerror(errno), errno);
		}

		vgm_push_reader_push_eof(&d, 0);
	}

	return 0;
}
