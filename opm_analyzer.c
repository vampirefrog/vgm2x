#include <string.h>
#include <stdlib.h>

#include "opm_analyzer.h"
#include "midi.h"

static void opm_analyzer_push_voice(struct opm_analyzer *analyzer, uint8_t chan, uint8_t mask, uint8_t midi_note) {
	uint8_t *ofs = analyzer->regs + chan;

	struct opm_voice_collector_voice voice;
	memset(&voice, 0, sizeof(voice));
	voice.voice.lfrq = analyzer->regs[0x18];
	voice.voice.amd = analyzer->regs[0x19] & 0x7f;
	voice.voice.pmd = analyzer->pmd & 0x7f;
	voice.voice.w = analyzer->regs[0x1b] & 0x03;
	voice.voice.ne_nfrq = analyzer->regs[0x0f] & 0x9f;
	voice.voice.rl_fb_con = ofs[0x20];
	voice.voice.pms_ams = ofs[0x38] & 0x73;
	voice.voice.slot = mask;
	// voice.vgm_ofs = vgm_ofs;
	for(int i = 0; i < 4; i++) {
		uint8_t *opofs = ofs + i * 8;
		voice.voice.operators[i].dt1_mul = opofs[0x40] & 0x7f;
		voice.voice.operators[i].tl      = opofs[0x60] & 0x7f;
		voice.voice.operators[i].ks_ar   = opofs[0x80] & 0xdf;
		voice.voice.operators[i].ams_d1r = opofs[0xa0] & 0x9f;
		voice.voice.operators[i].dt2_d2r = opofs[0xc0] & 0x9f;
		voice.voice.operators[i].d1l_rr  = opofs[0xe0];
		voice.voice.operators[i].ws      = 0;
	}

	opm_voice_collector_push_voice(&analyzer->collector, &voice, chan, midi_note);
}

static void opm_cmd_reg8_data8(struct chip_analyzer *chip_analyzer, uint8_t reg, uint8_t data, void *data_ptr) {
	struct opm_analyzer *analyzer = (struct opm_analyzer *)chip_analyzer;
	if(reg == 0x08) {
		uint8_t mask = (data & 0x78) >> 3;
		uint8_t chan = data & 0x07;
		if(mask) {
			float pitch = opm_kc_kf_to_pitch(analyzer->regs[0x28 + chan], analyzer->regs[0x30 + chan], chip_analyzer->clock);
			int midi_note = midi_pitch_to_note(pitch, 0);
			opm_analyzer_push_voice(analyzer, chan, mask, midi_note);
			chip_analyzer_note_on(chip_analyzer, chan, midi_note, 127);
		} else {
			chip_analyzer_note_off(chip_analyzer, chan);
		}
	} else {
		if(reg == 0x19 && data >= 128) analyzer->pmd = data & 0x7f;
		else analyzer->regs[reg] = data;
	}
}

struct opm_analyzer * opm_analyzer_new(int clock) {
	struct opm_analyzer *a = malloc(sizeof(*a));
	if(!a) return 0;
	opm_analyzer_init(a, clock);
	return a;
}

void opm_analyzer_init(struct opm_analyzer *analyzer, int clock) {
	chip_analyzer_init(&analyzer->chip_analyzer, clock, 8);
	analyzer->chip_analyzer.cmd_reg8_data8 = opm_cmd_reg8_data8;

	opm_voice_collector_init(&analyzer->collector);

	memset(analyzer->regs, 0, sizeof(analyzer->regs));
}
