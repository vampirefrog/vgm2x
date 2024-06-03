#pragma once

#include <stdint.h>

#include "chip_ids.h"
#include "error.h"

struct vgm_interpreter {
	int (*init_chip)(enum vgm_chip_id, int clock, void *data_ptr);
	void (*init_sn_chip)(enum vgm_chip_id, int clock, uint16_t feedback, uint8_t sr_width, uint8_t flags);
	void (*init_ay_chip)(enum vgm_chip_id, int clock, uint8_t flags, uint8_t ym2203_flags, uint8_t ym2608_flags);
	void (*init_chip_flags8)(enum vgm_chip_id, int clock, uint8_t flags);

	void *data_ptr;
	void (*write_stereo_mask)(enum vgm_chip_id chip_id, uint8_t dd, void *data_ptr);
	void (*write_gg_psg_stereo)(enum vgm_chip_id chip_id, uint8_t dd, void *data_ptr);
	void (*write_data8)(enum vgm_chip_id chip_id, uint8_t data, void *data_ptr);
	void (*write_reg8_data8)(enum vgm_chip_id chip_id, uint8_t reg, uint8_t data, void *data_ptr);
	void (*write_port8_reg8_data8)(enum vgm_chip_id chip_id, uint8_t port, uint8_t reg, uint8_t data, void *data_ptr);
	void (*write_pcm_ram)(enum vgm_chip_id chip_id, uint32_t write_offset, uint32_t data_size, void *data, void *data_ptr);
	void (*write_reg8_data16)(enum vgm_chip_id chip_id, uint8_t reg, uint16_t data, void *data_ptr);
	void (*write_addr16_data8)(enum vgm_chip_id chip_id, uint16_t addr, uint8_t data, void *data_ptr);
	void (*write_bank_ofs8_chan8)(enum vgm_chip_id chip_id, uint16_t ofs, uint8_t chan, void *data_ptr);
	void (*write_reg16_data8)(enum vgm_chip_id chip_id, uint16_t reg, uint8_t data, void *data_ptr);
	void (*write_reg16_data16)(enum vgm_chip_id chip_id, uint16_t reg, uint16_t value, void *data_ptr);
	void (*wait)(int samples, void *data_ptr);
	void (*end)(void *data_ptr);
	void (*loop)(void *data_ptr); //?
};

void vgm_interpreter_init(struct vgm_interpreter *i);
enum vgm_error_code vgm_interpreter_run(struct vgm_interpreter *i, uint8_t *data, int data_len, struct vgm_error *error);
