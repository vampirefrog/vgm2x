#include <string.h>
#include <stdlib.h>

#include "opm_analyzer.h"

static void opm_analyzer_push_voice(struct opm_analyzer *analyzer, uint8_t chan, uint8_t mask) {
	uint8_t *ofs = analyzer->regs + chan;

	struct opm_voice_collector_voice voice;
	voice.voice.rl_fb_con = ofs[0x20] & 0x3f;
	voice.voice.pms_ams = ofs[0x38] & 0x73;
	voice.voice.slot = mask;
	// voice.vgm_ofs = vgm_ofs;
	for(int i = 0; i < 4; i++) {
		voice.voice.operators[i].dt1_mul = ofs[0x40 + i * 8] & 0x7f;
		voice.voice.operators[i].tl      = ofs[0x60 + i * 8] & 0x7f;
		voice.voice.operators[i].ks_ar   = ofs[0x80 + i * 8] & 0xdf;
		voice.voice.operators[i].ams_d1r = ofs[0xa0 + i * 8] & 0x9f;
		voice.voice.operators[i].dt2_d2r = ofs[0xc0 + i * 8] & 0x9f;
		voice.voice.operators[i].d1l_rr  = ofs[0xe0 + i * 8];
	}

	opm_voice_collector_push_voice(&analyzer->collector, &voice, chan);
}

static void opm_cmd_reg8_data8(struct chip_analyzer *chip_analyzer, uint8_t reg, uint8_t data, void *data_ptr) {
	struct opm_analyzer *analyzer = (struct opm_analyzer *)chip_analyzer;
	if(reg == 0x08) {
		uint8_t mask = (data & 0x78) >> 3;
		uint8_t chan = data & 0x07;
		if(mask) {
			opm_analyzer_push_voice(analyzer, chan, mask);
		}
	} else {
		analyzer->regs[reg] = data;
	}
}

struct opm_analyzer * opm_analyzer_new(int clock) {
	struct opm_analyzer *a = malloc(sizeof(*a));
	if(!a) return 0;
	opm_analyzer_init(a, clock);
	return a;
}

void opm_analyzer_init(struct opm_analyzer *analyzer, int clock) {
	chip_analyzer_init(&analyzer->chip_analyzer, clock);
	analyzer->chip_analyzer.cmd_reg8_data8 = opm_cmd_reg8_data8;

	opm_voice_collector_init(&analyzer->collector);

	memset(analyzer->regs, 0, sizeof(analyzer->regs));
}
