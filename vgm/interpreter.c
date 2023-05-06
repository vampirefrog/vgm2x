#include <string.h>
#include <stdio.h>

#include "interpreter.h"
#include "header.h"
#include "commands.h"
#include "error.h"

void vgm_interpreter_init(struct vgm_interpreter *i) {
	memset(i, 0, sizeof(*i));
}

enum vgm_error_code vgm_interpreter_run(struct vgm_interpreter *it, uint8_t *buf, int data_len, struct vgm_error *error) {
	enum vgm_error_code er;

	struct vgm_header header;
	er = vgm_header_read(&header, buf, data_len, error);
	if(er != SUCCESS) return er;

#define INIT_DUAL_CHIP(header_field, chip_type) \
	if(header.header_field) { \
		if(it->init_chip) it->init_chip(chip_type, header.header_field & 0x3fffffff, it->data_ptr); \
		if(header.header_field & 0x40000000) it->init_chip(SECOND_##chip_type, header.header_field & 0x3fffffff, it->data_ptr); \
	}

	INIT_DUAL_CHIP(ym2612_clock, YM2612)
	INIT_DUAL_CHIP(ym2413_clock, YM2413)
	INIT_DUAL_CHIP(ym2151_clock, YM2151)
	INIT_DUAL_CHIP(ym2203_clock, YM2203)
	INIT_DUAL_CHIP(ym2608_clock, YM2608)
	INIT_DUAL_CHIP(ym2610_clock, YM2610)
	INIT_DUAL_CHIP(ym3812_clock, YM3812)
	INIT_DUAL_CHIP(ym3526_clock, YM3526)
	INIT_DUAL_CHIP(y8950_clock, Y8950)
	INIT_DUAL_CHIP(ymz280b_clock, YMZ280B)
	INIT_DUAL_CHIP(ymf262_clock, YMF262)
	INIT_DUAL_CHIP(ymf278b_clock, YMF278B)
	INIT_DUAL_CHIP(ymf271_clock, YMF271)

	uint32_t end_offset = data_len;
	if(header.eof_offset && header.eof_offset < end_offset) end_offset = header.eof_offset;
	if(header.gd3_offset && header.gd3_offset < end_offset) end_offset = header.gd3_offset;
	for(uint32_t i = header.vgm_data_offset; i < end_offset;) {
		int sz = vgm_command_sizes[buf[i]];
		if(buf[i] == 0x67) {
			if(end_offset - i < 7) {
				fprintf(stderr, "0x67 command cuts off\n");
				break;
			}
			uint32_t block_size = buf[i+3] | buf[i+4] << 8 | buf[i+5] << 16 | buf[i+6] << 24;
			block_size &= 0x7fffffff;
			if(end_offset - i < 7 + block_size) {
				fprintf(stderr, "0x67 data of size %d cuts off with %d left\n", block_size, end_offset - i);
				break;
			}
			i += 7 + block_size;
		} else {
			if(sz == 0) {
				fprintf(stderr, "zero size command 0x%02x at 0x%08x / 0x%08x\n", buf[i], i, end_offset);
				break;
			}
			if(sz > end_offset - i) {
				fprintf(stderr, "Command 0x%02x (%d bytes) cuts off at 0x%08x / 0x%08x\n", buf[i], sz, i, end_offset);
				break;
			}
			if(buf[i] >= 0x70 && buf[i] <= 0x7f) {
				if(it->wait) it->wait(buf[i] - 0x6f, it->data_ptr);
			} else if(buf[i] >= 0x80 && buf[i] <= 0x8f) {
				if(it->wait) it->wait(buf[i] - 0x80, it->data_ptr);
				// if(it->write_port8_reg8_data8) it->write_port8_reg8_data8(YM2612, 0, 0x2a, data_bank_byte);
			} else switch(buf[i]) {
				case 0x61: if(it->wait) it->wait(buf[i+1] | buf[i+2] << 8, it->data_ptr); break;
				case 0x62: if(it->wait) it->wait(735, it->data_ptr); break;
				case 0x63: if(it->wait) it->wait(882, it->data_ptr); break;
				case 0x66: if(it->end) it->end(it->data_ptr); i = end_offset; break;
				case 0x31: if(it->write_stereo_mask) it->write_stereo_mask(i & 0x80 ? SECOND_AY8910 : AY8910, buf[i+1], it->data_ptr); break;
				case 0x51: if(it->write_reg8_data8) it->write_reg8_data8(YM2413, buf[i+1], buf[i+2], it->data_ptr); break;
				case 0x52: if(it->write_port8_reg8_data8) it->write_port8_reg8_data8(YM2612, 0, buf[i+1], buf[i+2], it->data_ptr); break;
				case 0x53: if(it->write_port8_reg8_data8) it->write_port8_reg8_data8(YM2612, 1, buf[i+1], buf[i+2], it->data_ptr); break;
				case 0x54: if(it->write_reg8_data8) it->write_reg8_data8(YM2151, buf[i+1], buf[i+2], it->data_ptr); break;
				case 0x55: if(it->write_reg8_data8) it->write_reg8_data8(YM2203, buf[i+1], buf[i+2], it->data_ptr); break;
				case 0x56: if(it->write_port8_reg8_data8) it->write_port8_reg8_data8(YM2608, 0, buf[i+1], buf[i+2], it->data_ptr); break;
				case 0x57: if(it->write_port8_reg8_data8) it->write_port8_reg8_data8(YM2608, 1, buf[i+1], buf[i+2], it->data_ptr); break;
				case 0x58: if(it->write_port8_reg8_data8) it->write_port8_reg8_data8(YM2610, 0, buf[i+1], buf[i+2], it->data_ptr); break;
				case 0x59: if(it->write_port8_reg8_data8) it->write_port8_reg8_data8(YM2610, 1, buf[i+1], buf[i+2], it->data_ptr); break;
				case 0x5A: if(it->write_reg8_data8) it->write_reg8_data8(YM3812, buf[i+1], buf[i+2], it->data_ptr); break;
				case 0x5B: if(it->write_reg8_data8) it->write_reg8_data8(YM3526, buf[i+1], buf[i+2], it->data_ptr); break;
				case 0x5C: if(it->write_reg8_data8) it->write_reg8_data8(Y8950, buf[i+1], buf[i+2], it->data_ptr); break;
				case 0x5D: if(it->write_reg8_data8) it->write_reg8_data8(YMZ280B, buf[i+1], buf[i+2], it->data_ptr); break;
				case 0x5E: if(it->write_port8_reg8_data8) it->write_port8_reg8_data8(YMF262, 0, buf[i+1], buf[i+2], it->data_ptr); break;
				case 0x5F: if(it->write_port8_reg8_data8) it->write_port8_reg8_data8(YMF262, 1, buf[i+1], buf[i+2], it->data_ptr); break;
				case 0xd0: if(it->write_port8_reg8_data8) it->write_port8_reg8_data8(YMF278B, buf[i+1], buf[i+2], buf[i+3], it->data_ptr); break;
				case 0xd1: if(it->write_port8_reg8_data8) it->write_port8_reg8_data8(YMF271, buf[i+1], buf[i+2], buf[i+3], it->data_ptr); break;
				case 0xd2: if(it->write_port8_reg8_data8) it->write_port8_reg8_data8(K051649, buf[i+1], buf[i+2], buf[i+3], it->data_ptr); break;
				case 0xd3: if(it->write_reg16_data8) it->write_reg16_data8(K054539, buf[i+1] | buf[i+2] << 8, buf[i+3], it->data_ptr); break;
				case 0xd4: if(it->write_reg16_data8) it->write_reg16_data8(C140, buf[i+1] | buf[i+2] << 8, buf[i+3], it->data_ptr); break;
				case 0xd5: if(it->write_reg16_data8) it->write_reg16_data8(ES5503, buf[i+1] | buf[i+2] << 8, buf[i+3], it->data_ptr); break;
				case 0xd6: if(it->write_reg8_data16) it->write_reg8_data16(ES5506, buf[i+1], buf[i+2] | buf[i+3] << 8, it->data_ptr); break;
				case 0xe1: if(it->write_reg16_data16) it->write_reg16_data16(C352, buf[i+1] | buf[i+2] << 8, buf[i+3] | buf[i+4] << 8, it->data_ptr); break;
			}
		}
		i += sz;
	}

	return SUCCESS;
}
