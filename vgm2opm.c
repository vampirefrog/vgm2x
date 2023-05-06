#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "cmdline.h"
#include "tools.h"
#include "vgm/interpreter.h"
#include "opn_analyzer.h"
#include "opm_analyzer.h"

void opn_voice_to_opm_voice(struct opn_voice *opnv, struct opm_voice *opmv) {
	opmv->vgm_ofs = opnv->vgm_ofs;
	opmv->chan_used_mask = opnv->chan_used_mask;

	/* per chip registers */
	// opnv->lfo = opmv->lfrq >> 4;

	/* per channel registers */
	opmv->fb_connect = opnv->fb_connect & 0x3f;
	// opnv->lr_ams_pms = (opmv->rl_fb_con & 0xc0) | (opmv->pms_ams & 0x03) << 4 | (opmv->pms_ams & 0x70) >> 3;

	// /* slot mask */
	// opnv->sm = opmv->sm << 1;

	/* operators */
	for(int j = 0; j < 4; j++) {
		struct opn_voice_operator *nop = &opnv->operators[j];
		struct opm_voice_operator *mop = &opmv->operators[j];

		mop->dt1_mul = nop->dt_mul & 0x7f;
		mop->tl = nop->tl & 0x7f;
		mop->ks_ar = nop->ks_ar & 0xdf;
		mop->ame_d1r = nop->am_dr & 0x9f;
		mop->dt2_d2r = nop->sr & 0x1f;
		mop->d1l_rr = nop->sl_rr;
	}
}

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
		case SECOND_YM2612:
		case SECOND_YM2203:
		case SECOND_YM2608:
		case SECOND_YM2610:
		case SECOND_YM3812:
		case SECOND_YM3526:
			add_analyzer((struct chip_analyzer *)opn_analyzer_new(clock, chip_id == YM2203 || chip_id == SECOND_YM2203 ? 3 : 6), chip_id);
			break;
		case YM2151:
		case SECOND_YM2151:
			add_analyzer((struct chip_analyzer *)opm_analyzer_new(clock), chip_id);
			break;
		default:
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

char *opt_output = "-";
int main(int argc, char **argv) {
	int optind = cmdline_parse_args(argc, argv, (struct cmdline_option[]){
		{
			'o', "output",
			"Output filename",
			"name",
			TYPE_OPTIONAL,
			TYPE_STRING, &opt_output
		},

		CMDLINE_ARG_TERMINATOR
	}, 1, 0, "files.vg[mz]");

	if(optind < 0) exit(-optind);

	for(int i = optind; i < argc; i++) {
		printf("%s\n", argv[i]);
		size_t s = 0;
		uint8_t *buf = load_gzfile(argv[i], &s);
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
	}

	struct opm_voice_collector collector;
	opm_voice_collector_init(&collector);
	enum vgm_chip_id ids[] = {
		YM2151, SECOND_YM2151,
		YM2612, SECOND_YM2612,
		YM2203, SECOND_YM2203,
		YM2608, SECOND_YM2608,
		YM2610, SECOND_YM2610,
		YM3812, SECOND_YM3812,
		YM3526, SECOND_YM3526,
	};
	for(int i = 0; i < sizeof(ids) / sizeof(ids[0]); i++) {
		if(!analyzers_by_id[ids[i]]) continue;
		if(ids[i] == YM2151 || ids[i] == SECOND_YM2151) {
			struct opm_analyzer *a = (struct opm_analyzer *)analyzers_by_id[ids[i]];
			for(int i = 0; i < a->collector.num_voices; i++)
				opm_voice_collector_push_voice(&collector, a->collector.voices + i, 0);
		} else {
			struct opn_analyzer *a = (struct opn_analyzer *)analyzers_by_id[ids[i]];
			for(int i = 0; i < a->collector.num_voices; i++) {
				struct opn_voice *opnv = a->collector.voices + i;
				struct opm_voice opmv;
				opn_voice_to_opm_voice(opnv, &opmv);
				opm_voice_collector_push_voice(&collector, &opmv, 0);
			}
		}
	}
	opm_voice_collector_dump_voices(&collector);

	return 0;
}
