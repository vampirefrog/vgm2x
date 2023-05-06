#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "vgm/interpreter.h"
#include "opn_analyzer.h"
#include "ym2151_analyzer.h"

// void opn_voice_to_opm_voice(struct opn_voice *opnv, struct opm_voice *opmv) {
// 	opmv->vgm_ofs = opnv->vgm_ofs;
// 	opmv->chan_used_mask = opnv->chan_used_mask;

// 	/* per chip registers */
// 	opnv->lfo = opmv->lfrq >> 4;

// 	/* per channel registers */
// 	opnv->fb_connect = (opmv->rl_fb_con & 0x38) | (opmv->rl_fb_con & 0x07);
// 	opnv->lr_ams_pms = (opmv->rl_fb_con & 0xc0) | (opmv->pms_ams & 0x03) << 4 | (opmv->pms_ams & 0x70) >> 3;

// 	/* slot mask */
// 	opnv->sm = opmv->sm << 1;

// 	/* operators */
// 	for(int j = 0; j < 4; j++) {
// 		struct opn_voice_operator *nop = &opnv->operators[j];
// 		struct opm_voice_operator *mop = &opmv->operators[j];

// 		nop->dt_mul = mop->dt1_mul & 0x7f;
// 		nop->tl = mop->tl & 0x7f;
// 		nop->ks_ar = mop->ks_ar;
// 		nop->am_dr = mop->ame_d1r;
// 		nop->sr = mop->dt2_d2r & 0x1f;
// 		nop->sl_rr = mop->d1l_rr;
// 		nop->ssg_eg = 0;
// 	}
// }

// void opn_voice_dump_opm(struct opn_voice *v, int n) {
// 	struct opm_voice opm;
// 	opn_voice_to_opm_voice(v, &opm);
// 	opm_voice_dump_opm(&opm);
// }

// void opm_voice_dump_opm(struct opm_voice *v, int n) {
// 	printf(
// 		"// vgm offset = %08x, channels used = %c%c%c%c%c%c%c%c\n",
// 		v->vgm_ofs,
// 		v->chan_used_mask & 0x01 ? '1' : '-',
// 		v->chan_used_mask & 0x02 ? '2' : '-',
// 		v->chan_used_mask & 0x04 ? '3' : '-',
// 		v->chan_used_mask & 0x08 ? '4' : '-',
// 		v->chan_used_mask & 0x10 ? '5' : '-',
// 		v->chan_used_mask & 0x20 ? '6' : '-',
// 		v->chan_used_mask & 0x40 ? '7' : '-',
// 		v->chan_used_mask & 0x80 ? '8' : '-'
// 	);
// 	printf("@:%d Instrument %d\n", n, n);
// 	printf("LFO: 0 0 0 0 0\n");
// 	printf("CH: 64 %i %i 0 0 120 0\n", v->fb_connect >> 3 & 0x07, v->fb_connect & 0x07);
// 	int op_order[4] = { 0, 2, 1, 3 };
// 	char *op_names[4] = { "M1", "C1", "M2", "C2" };
// 	for(int i = 0; i < 4; i++) {
// 		struct opm_voice_operator *op = &v->operators[op_order[i]];
// 		printf(
// 			"%s: %2d %2d %2d %2d %2d %3d %2d %2d %2d %2d %2d\n",
// 			op_names[i],
// 			op->ks_ar & 0x1f,   // AR
// 			op->ame_d1r & 0x1f, // D1R
// 			op->dt2_d2r & 0x1f, // D2R
// 			op->d1l_rr & 0x0f,  // RR
// 			op->d1l_rr >> 4,    // D1L
// 			op->tl & 0x7f,      // TL
// 			op->ks_ar >> 6,     // KS
// 			op->dt1_mul & 0x0f, // MUL
// 			op->dt1_mul >> 4,   // DT1
// 			op->dt2_d2r >> 6,   // DT2
// 			op->ame_d1r >> 7    // AME
// 		);
// 	}
// 	printf("\n");
// }

struct chip_analyzer **analyzers = 0;
int num_chip_analyzers = 0;
struct chip_analyzer *analyzers_by_id[256];

static void add_analyzer(struct chip_analyzer *a, int id) {
	num_chip_analyzers++;
	analyzers = realloc(analyzers, num_chip_analyzers * sizeof(*analyzers));
	if(!analyzers) {
		fprintf(stderr, "Could not reallocate %d analyzers\n", num_chip_analyzers);
		return;
	}
	analyzers[num_chip_analyzers - 1] = a;
	analyzers_by_id[id] = a;
}

static void init_chip(enum vgm_chip_id chip_id, int clock, void *data_ptr) {
	if(analyzers_by_id[chip_id]) return;

	switch(chip_id) {
		case YM2612:
		case YM2203:
		case YM2608:
		case YM2610:
		case YM3812:
		case YM3526:
			add_analyzer((struct chip_analyzer *)opn_analyzer_new(clock, chip_id = YM2203 ? 3 : 6), chip_id);
			break;
		case YM2151:
			add_analyzer((struct chip_analyzer *)ym2151_analyzer_new(clock), chip_id);
			break;
	}
}

static void write_reg8_data8(enum vgm_chip_id chip_id, uint8_t reg, uint8_t data, void *data_ptr) {
	if(!analyzers_by_id[chip_id]) return;
	chip_analyzer_cmd_reg8_data8(analyzers_by_id[chip_id], reg, data);
}

static void write_port8_reg8_data8(enum vgm_chip_id chip_id, uint8_t port, uint8_t reg, uint8_t data, void *data_ptr) {
	if(!analyzers_by_id[chip_id]) return;
	chip_analyzer_cmd_port8_reg8_data8(analyzers_by_id[chip_id], port, reg, data);
}

static void wait(int samples, void *data_ptr) {
	// printf("wait %d\n", samples);
}

static void end(void *data_ptr) {
	// printf("end\n");
}

int main(int argc, char **argv) {
	for(int i = 1; i < argc; i++) {
		FILE *f = fopen(argv[i], "rb");
		if(!f) {
			fprintf(stderr, "Could not open %s: %s (%d)\n", argv[i], strerror(errno), errno);
			return 1;
		}

		int r = fseek(f, 0, SEEK_END);
		if(r) {
			fprintf(stderr, "Could not seek to end of file for %s: %s (%d)\n", argv[i], strerror(errno), errno);
			return 1;
		}

		long s = ftell(f);
		if(s < 0) {
			fprintf(stderr, "Could not get file end position for %s: %s (%d)\n", argv[i], strerror(errno), errno);
			return 1;
		}

		rewind(f);

		uint8_t *buf = malloc(s);
		if(!buf) {
			fprintf(stderr, "Could not allocate %lu bytes for %s: %s (%d)\n", s, argv[i], strerror(errno), errno);
			fclose(f);
			return 1;
		}

		size_t rd = fread(buf, 1, s, f);
		if(rd != s) {
			fprintf(stderr, "Short read %zu of %lu bytes from %s: %s (%d)\n", rd, s, argv[i], strerror(errno), errno);
			if(fclose(f))
				fprintf(stderr, "Could not close %s: %s (%d)\n", argv[i], strerror(errno), errno);
			free(buf);
			return 1;
		}

		if(fclose(f)) {
			fprintf(stderr, "Warning: Could not close %s: %s (%d)\n", argv[i], strerror(errno), errno);
		}

		struct vgm_interpreter interpreter;
		vgm_interpreter_init(&interpreter);
		interpreter.init_chip = init_chip;
		interpreter.write_reg8_data8 = write_reg8_data8;
		interpreter.write_port8_reg8_data8 = write_port8_reg8_data8;
		interpreter.wait = wait;
		interpreter.end = end;
		struct vgm_error error;
		enum vgm_error_code e = vgm_interpreter_run(&interpreter, buf, s, &error);
		if(e != SUCCESS) {
		}
		free(buf);

		int voice_num = 0;

		// for(int j = 0; j < num_opn_voices; j++) {
		// 	//opn_voice_dump(&opn_voices[j]);
		// 	opn_voice_dump_opm(&opn_voices[j], voice_num);
		// 	voice_num++;
		// }
		// free(opn_voices);
		// opn_voices = 0;
		// num_opn_voices = 0;

		// for(int j = 0; j < num_opm_voices; j++) {
		// 	opm_voice_dump_opm(&opm_voices[j], voice_num);
		// 	voice_num++;
		// }
		// free(opm_voices);
		// opm_voices = 0;
		// num_opm_voices = 0;
	}

	if(analyzers[YM2151])
		ym2151_analyzer_dump_voices((struct ym2151_analyzer *)analyzers_by_id[YM2151]);

	return 0;
}
