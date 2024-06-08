#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "cmdline.h"
#include "tools.h"
#include "vgm_analyzer.h"
#include "opm_voice_collector.h"
#include "opm_analyzer.h"
#include "opn_analyzer.h"
#include "libvgm/utils/FileLoader.h"

size_t write_fn(void *buf, size_t bufsize, void *data_ptr) {
	return fwrite(buf, 1, bufsize, (FILE *)data_ptr);
}

char *opt_output = "-";
int opt_csv = 0;
int main(int argc, char **argv) {
	int optind = cmdline_parse_args(argc, argv, (struct cmdline_option[]){
		{
			'o', "output",
			"Output filename",
			"name",
			TYPE_OPTIONAL,
			TYPE_STRING, &opt_output
		},
		{
			'c', "csv",
			"Output CSV instead",
			NULL,
			TYPE_OPTIONAL,
			TYPE_NONE, &opt_csv
		},

		CMDLINE_ARG_TERMINATOR
	}, 1, 0, "files.vg[mz]");

	if(optind < 0) exit(-optind);

	struct vgm_analyzer va;
	vgm_analyzer_init(&va);

	for(int i = optind; i < argc; i++) {
		DATA_LOADER *dload = FileLoader_Init(argv[i]);
		if(!dload) {
			fprintf(stderr, "Could not init loader for %s\n", argv[i]);
			continue;
		}
		if(DataLoader_Load(dload)) {
			fprintf(stderr, "Could not load %s\n", argv[i]);
			DataLoader_Deinit(dload);
			continue;
		}
		DataLoader_ReadAll(dload);
		DataLoader_GetData(dload);
		int r = vgm_analyzer_run(&va, DataLoader_GetData(dload), DataLoader_GetSize(dload));
		if(r) fprintf(stderr, "Could not analyze %s: error %d\n", argv[i], r);
		DataLoader_Deinit(dload);
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
		if(!va.analyzers_by_id[ids[i]]) continue;
		if(ids[i] == YM2151 || ids[i] == SECOND_YM2151) {
			struct opm_analyzer *a = (struct opm_analyzer *)va.analyzers_by_id[ids[i]];
			for(int i = 0; i < a->collector.num_voices; i++) {
				struct opm_voice_collector_voice opmv;
				memcpy(&opmv, a->collector.voices + i, sizeof(opmv));
				opm_voice_collector_push_voice(&collector, &opmv, -1, -1);
			}
		} else {
			struct opn_analyzer *a = (struct opn_analyzer *)va.analyzers_by_id[ids[i]];
			for(int i = 0; i < a->collector.num_voices; i++) {
				struct opn_voice_collector_voice *opnv = a->collector.voices + i;
				struct opm_voice_collector_voice opmv;
				opmv.vgm_ofs = opnv->vgm_ofs;
				opmv.chan_used_mask = opnv->chan_used_mask;
				memcpy(opmv.note_usage, opnv->note_usage, sizeof(opnv->note_usage));
				opm_voice_load_opn_voice(&opmv.voice, &opnv->voice);
				opm_voice_collector_push_voice(&collector, &opmv, -1, -1);
			}
		}
	}

	if(opt_csv) {
		printf("FB\tCON\tPMS\tAMS\tSLOT");
		for(int i = 0; i < 4; i++) {
			printf("\tAR\tD1R\tD2R\tRR\tD1L\tTL\tKS\tMUL\tDT1\tDT2\tAMS-EN");
		}
		printf("\n");
		for(int i = 0; i < collector.num_voices; i++) {
			struct opm_voice_collector_voice *v = collector.voices + i;
			printf(
				"%d\t%d\t%d\t%d\t%d",
				v->voice.rl_fb_con >> 3 & 0x07,
				v->voice.rl_fb_con & 0x07,
				v->voice.pms_ams >> 4 & 0x07,
				v->voice.pms_ams & 0x03,
				v->voice.slot
			);
			for(int j = 0; j < 4; j++) {
				const uint8_t dtmap[] = { 3, 4, 5, 6,  3, 2, 1, 0 };
				struct opm_voice_operator *op = &v->voice.operators[j];
				printf(
					"\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d",
					op->ks_ar & 0x1f,
					op->ams_d1r & 0x1f,
					op->dt2_d2r & 0x1f,
					op->d1l_rr & 0x0f,
					op->d1l_rr >> 4,
					op->tl & 0x7f,
					op->ks_ar >> 6,
					op->dt1_mul & 0x0f,
					dtmap[op->dt1_mul >> 4 & 0x07],
					op->dt2_d2r >> 6,
					op->ams_d1r >> 7
				);
			}
			printf("\n");
		}

		return 0;
	}

	struct fm_voice_bank bank;
	fm_voice_bank_init(&bank);
	for(int i = 0; i < collector.num_voices; i++) {
		struct opm_voice_collector_voice *v = collector.voices + i;
		fm_voice_bank_append_opm_voice(&bank, &v->voice);
	}

	int is_stdout = opt_output && opt_output[0] == '-' && opt_output[1] == 0;
	FILE *o = is_stdout ? stdout : fopen(opt_output, "w");
	fm_voice_bank_save(&bank, FORMAT_OPM, write_fn, o);
	if(!is_stdout) fclose(o);

	return 0;
}
