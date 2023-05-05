#include <string.h>

#include "header.h"

enum vgm_error_code vgm_header_read(struct vgm_header *header, uint8_t *data, int data_len, struct vgm_error *error) {
	if(data_len < 64) return TOO_SHORT;
	if(data[0] != 'V' || data[1] != 'g' || data[2] != 'm' || data[3] != ' ') return BAD_IDENT;
	memset(header, 0, sizeof(*header));
#define READ_OFFSET(field, ofs) { header->field = data[ofs] | data[ofs+1] << 8 | data[ofs+2] << 16 | data[ofs+3] << 24; if(header->field) header->field += ofs; }
#define READ_UINT32(field, ofs) { header->field = data[ofs] | data[ofs+1] << 8 | data[ofs+2] << 16 | data[ofs+3] << 24; }
#define READ_UINT16(field, ofs) { header->field = data[ofs] | data[ofs+1] << 8; }
#define READ_UINT8(field, ofs) { header->field = data[ofs]; }
	READ_OFFSET(eof_offset, 0x04);
	READ_UINT32(version, 0x08);
	READ_UINT32(sn76489_clock, 0x0c);
	READ_UINT32(ym2413_clock, 0x10);
	READ_UINT32(gd3_offset, 0x14);
	READ_UINT32(total_samples, 0x18);
	READ_UINT32(loop_offset, 0x1C);
	READ_UINT32(loop_samples, 0x20);
#define READ_VER_UINT32(field, ofs, minver) if(header->version >= minver) READ_UINT32(field, ofs);
#define READ_VER_UINT16(field, ofs, minver) if(header->version >= minver) READ_UINT16(field, ofs);
#define READ_VER_UINT8(field, ofs, minver) if(header->version >= minver) READ_UINT8(field, ofs);
	READ_VER_UINT32(rate, 0x24, 0x101);
	READ_VER_UINT16(sn76489_feedback, 0x28, 0x110);
	READ_VER_UINT8(sn76489_shift_register_width, 0x2A, 0x110);
	READ_VER_UINT8(sn76489_flags, 0x2B, 0x151);
	READ_VER_UINT32(ym2612_clock, 0x2C, 0x110);
	READ_VER_UINT32(ym2151_clock, 0x30, 0x110);
	if(header->version >= 0x150) READ_OFFSET(vgm_data_offset, 0x34);
	if(header->vgm_data_offset == 0) header->vgm_data_offset = 0x40;
	READ_VER_UINT32(sega_pcm_clock, 0x38, 0x151);
	READ_VER_UINT32(sega_pcm_interface_register, 0x3C, 0x151);
#define MAYBE_READ_VER_UINT32(field, ofs, minver) if(data_len >= ofs + 4 && header->version >= minver) READ_UINT32(field, ofs);
#define MAYBE_READ_VER_UINT16(field, ofs, minver) if(data_len >= ofs + 2 && header->version >= minver) READ_UINT16(field, ofs);
#define MAYBE_READ_VER_UINT8(field, ofs, minver) if(data_len >= ofs + 1 && header->version >= minver) READ_UINT8(field, ofs);
	MAYBE_READ_VER_UINT32(rf5c68_clock, 0x40, 0x151);
	MAYBE_READ_VER_UINT32(ym2203_clock, 0x44, 0x151);
	MAYBE_READ_VER_UINT32(ym2608_clock, 0x48, 0x151);
	MAYBE_READ_VER_UINT32(ym2610_clock, 0x4C, 0x151);
	MAYBE_READ_VER_UINT32(ym3812_clock, 0x50, 0x151);
	MAYBE_READ_VER_UINT32(ym3526_clock, 0x54, 0x151);
	MAYBE_READ_VER_UINT32(y8950_clock, 0x58, 0x151);
	MAYBE_READ_VER_UINT32(ymf262_clock, 0x5C, 0x151);
	MAYBE_READ_VER_UINT32(ymf278b_clock, 0x60, 0x151);
	MAYBE_READ_VER_UINT32(ymf271_clock, 0x64, 0x151);
	MAYBE_READ_VER_UINT32(ymz280b_clock, 0x68, 0x151);
	MAYBE_READ_VER_UINT32(rf5c164_clock, 0x6C, 0x151);
	MAYBE_READ_VER_UINT32(pwm_clock, 0x70, 0x151);
	MAYBE_READ_VER_UINT32(ay8910_clock, 0x74, 0x151);
	MAYBE_READ_VER_UINT8(ay8910_chip_type, 0x78, 0x151);
	MAYBE_READ_VER_UINT8(ay8910_flags, 0x79, 0x151);
	MAYBE_READ_VER_UINT8(ym2203_flags, 0x7A, 0x151);
	MAYBE_READ_VER_UINT8(ym2608_flags, 0x7B, 0x151);
	MAYBE_READ_VER_UINT8(volume_modifier, 0x7C, 0x160);
	MAYBE_READ_VER_UINT8(loop_base, 0x7E, 0x160);
	MAYBE_READ_VER_UINT8(loop_modifier, 0x7F, 0x151);
	MAYBE_READ_VER_UINT32(gameboy_dmg_clock, 0x80, 0x161);
	MAYBE_READ_VER_UINT32(nes_apu_clock, 0x84, 0x161);
	MAYBE_READ_VER_UINT32(multipcm_clock, 0x88, 0x161);
	MAYBE_READ_VER_UINT32(upd7759_clock, 0x8C, 0x161);
	MAYBE_READ_VER_UINT32(okim6258_clock, 0x90, 0x161);
	MAYBE_READ_VER_UINT8(okim6258_flags, 0x94, 0x161);
	MAYBE_READ_VER_UINT8(k054539_flags, 0x95, 0x161);
	MAYBE_READ_VER_UINT8(c140_chip_type, 0x96, 0x161);
	MAYBE_READ_VER_UINT32(okim6295_clock, 0x98, 0x161);
	MAYBE_READ_VER_UINT32(k051649_clock, 0x9C, 0x161);
	MAYBE_READ_VER_UINT32(k054539_clock, 0xA0, 0x161);
	MAYBE_READ_VER_UINT32(huc6280_clock, 0xA4, 0x161);
	MAYBE_READ_VER_UINT32(c140_clock, 0xA8, 0x161);
	MAYBE_READ_VER_UINT32(k053260_clock, 0xAC, 0x161);
	MAYBE_READ_VER_UINT32(pokey_clock, 0xB0, 0x161);
	MAYBE_READ_VER_UINT32(qsound_clock, 0xB4, 0x161);
	MAYBE_READ_VER_UINT32(scsp_clock, 0xB8, 0x171);
	if(data_len >= 0xBC + 4 && header->version >= 0x170) READ_OFFSET(extra_header_offset, 0xBC);
	MAYBE_READ_VER_UINT32(wonderswan_clock, 0xC0, 0x171);
	MAYBE_READ_VER_UINT32(vsu_clock, 0xC4, 0x171);
	MAYBE_READ_VER_UINT32(saa1099_clock, 0xC8, 0x171);
	MAYBE_READ_VER_UINT32(es5503_clock, 0xCC, 0x171);
	MAYBE_READ_VER_UINT32(es5505_clock, 0xD0, 0x171);
	MAYBE_READ_VER_UINT8(es5503_amount_of_output_channels, 0xD4, 0x171);
	MAYBE_READ_VER_UINT8(es5505_es5506_amount_of_output_channels, 0xD5, 0x171);
	MAYBE_READ_VER_UINT8(c352_clock_divider, 0xD6, 0x171);
	MAYBE_READ_VER_UINT32(x1_010_clock, 0xD8, 0x171);
	MAYBE_READ_VER_UINT32(c352_clock, 0xDC, 0x171);
	MAYBE_READ_VER_UINT32(ga20_clock, 0xE0, 0x171);
	return SUCCESS;
}
