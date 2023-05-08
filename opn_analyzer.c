#include <string.h>
#include <stdlib.h>

#include "opn_analyzer.h"

static void opn_analyzer_push_voice(struct opn_analyzer *analyzer, uint8_t port, uint8_t chan, uint8_t mask) {
	uint8_t *ofs = &analyzer->regs[(chan & 0x03) | (chan & 0x04) << 6];

	struct opn_voice voice;
	voice.fb_connect = ofs[0xb0];
	voice.lr_ams_pms = ofs[0xb4] & 0xf7;
	voice.sm = mask;
	// voice.vgm_ofs = vgm_ofs;
	for(int i = 0; i < 4; i++) {
		voice.operators[i].dt_mul   = ofs[0x30 + i * 4] & 0x7f;
		voice.operators[i].tl       = ofs[0x40 + i * 4] & 0x7f;
		voice.operators[i].ks_ar    = ofs[0x50 + i * 4] & 0xdf;
		voice.operators[i].am_dr    = ofs[0x60 + i * 4] & 0x9f;
		voice.operators[i].sr       = ofs[0x70 + i * 4] & 0x1f;
		voice.operators[i].sl_rr    = ofs[0x80 + i * 4];
		voice.operators[i].ssg_eg   = ofs[0x90 + i * 4] & 0x0f;
	}

	opn_voice_collector_push_voice(&analyzer->collector, &voice, chan + port * 3);
}

static void opn_cmd_port8_reg8_data8(struct chip_analyzer *chip_analyzer, uint8_t port, uint8_t reg, uint8_t data, void *data_ptr) {
	struct opn_analyzer *analyzer = (struct opn_analyzer *)chip_analyzer;

	analyzer->regs[port * 256 + reg] = data;

	if(port == 0 && reg == 0x2b) {
		analyzer->ym_dac = data & 0x80 ? 1 : 0;
		return;
	}

	if(reg == 0x28 && port == 0) {
		uint8_t mask = data >> 4;
		uint8_t chan = data & 0x07;

		if(mask) {
			// ignore DAC writes
			if(chan == 6 && analyzer->ym_dac == 1)
				return;
			opn_analyzer_push_voice(analyzer, port, chan, mask);
		}
	}
}

struct opn_analyzer *opn_analyzer_new(int clock, int num_channels) {
	struct opn_analyzer *a = malloc(sizeof(*a));
	if(!a) return 0;
	opn_analyzer_init(a, clock, num_channels);
	return a;
}

void opn_analyzer_init(struct opn_analyzer *analyzer, int clock, int num_channels) {
	chip_analyzer_init(&analyzer->chip_analyzer, clock);
	analyzer->chip_analyzer.cmd_port8_reg8_data8 = opn_cmd_port8_reg8_data8;

	opn_voice_collector_init(&analyzer->collector);

	memset(analyzer->regs, 0, sizeof(analyzer->regs) / sizeof(analyzer->regs[0]));
	analyzer->ym_dac = 0;
	analyzer->num_channels = num_channels;
}
