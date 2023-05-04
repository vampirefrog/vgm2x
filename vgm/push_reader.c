#include <stdio.h>
#include <string.h>

#include "push_reader.h"

void vgm_push_reader_init(struct vgm_push_reader *d) {
	memset(d, 0, sizeof(*d));
	d->version = 0x100; // default version, for parsing the fields before the version field
	d->vgm_data_offset = 0x40;
}

enum vgm_error_code vgm_push_reader_push(struct vgm_push_reader *d, uint8_t *data, int len, struct vgm_error *error) {
	int prevpos = 0;
	for(int i = 0; i < len; i++) {
		if(d->state == IN_HEADER) {
			switch(d->filepos) {
				case 0x00: if(data[i] != 'V') return BAD_IDENT; break;
				case 0x01: if(data[i] != 'g') return BAD_IDENT; break;
				case 0x02: if(data[i] != 'm') return BAD_IDENT; break;
				case 0x03: if(data[i] != ' ') return BAD_IDENT; break;
				case 0x08: d->version = data[i]; break;
				case 0x09: d->version |= data[i] << 8; break;
				case 0x0a: d->version |= data[i] << 16; break;
				case 0x0b: d->version |= data[i] << 24; break;
#define PARSE_OFFSET(hofs, field, minver) \
				case hofs: if(d->version >= minver) d->field = data[i]; break; \
				case hofs+1: if(d->version >= minver) d->field |= data[i] << 8; break; \
				case hofs+2: if(d->version >= minver) d->field |= data[i] << 16; break; \
				case hofs+3: if(d->version >= minver) { d->field |= data[i] << 24; if(d->field) d->field += hofs; } break;
				PARSE_OFFSET(0x14, gd3_offset, 0x100)
				PARSE_OFFSET(0x34, vgm_data_offset, 0x150)
				PARSE_OFFSET(0xbc, extra_header_offset, 0x170)
#undef PARSE_OFFSET
			}
		}

		d->filepos++;

		int push_data = 0;
		if(d->filepos == d->vgm_data_offset) {
			push_data = 1;
			d->state = IN_COMMANDS;
		} else if(d->gd3_offset && d->filepos == d->gd3_offset) {
			push_data = 1;
			d->state = IN_GD3;
		} else if(d->extra_header_offset && d->filepos == d->extra_header_offset) {
			push_data = 1;
			d->state = IN_EXTRA_HEADER;
		}
		if(push_data && i != prevpos) {
			switch(d->state) {
				case IN_HEADER:
					if(d->callbacks.header_data_cb)
						d->callbacks.header_data_cb(d, data + prevpos, i - prevpos, d->callbacks.data_ptr);
					break;
				case IN_COMMANDS:
					if(d->callbacks.cmd_data_cb)
						d->callbacks.cmd_data_cb(d, data + prevpos, i - prevpos, d->callbacks.data_ptr);
					break;
				case IN_GD3:
					if(d->callbacks.gd3_data_cb)
						d->callbacks.gd3_data_cb(d, data + prevpos, i - prevpos, d->callbacks.data_ptr);
					break;
				case IN_EXTRA_HEADER:
					if(d->callbacks.extra_header_data_cb)
						d->callbacks.extra_header_data_cb(d, data + prevpos, i - prevpos, d->callbacks.data_ptr);
					break;
			}
			prevpos = i;
		}
	}
	if(prevpos < len - 1) {
		switch(d->state) {
			case IN_HEADER:
				if(d->callbacks.header_data_cb)
					d->callbacks.header_data_cb(d, data + prevpos, len - prevpos, d->callbacks.data_ptr);
				break;
			case IN_COMMANDS:
				if(d->callbacks.cmd_data_cb)
					d->callbacks.cmd_data_cb(d, data + prevpos, len - prevpos, d->callbacks.data_ptr);
				break;
			case IN_GD3:
				if(d->callbacks.gd3_data_cb)
					d->callbacks.gd3_data_cb(d, data + prevpos, len - prevpos, d->callbacks.data_ptr);
				break;
			case IN_EXTRA_HEADER:
				if(d->callbacks.extra_header_data_cb)
					d->callbacks.extra_header_data_cb(d, data + prevpos, len - prevpos, d->callbacks.data_ptr);
				break;
		}
	}

	return SUCCESS;
}

enum vgm_error_code vgm_push_reader_push_eof(struct vgm_push_reader *d, struct vgm_error *error) {
	return SUCCESS;
}
