#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <libgen.h>
#include <dirent.h>
#include <sys/stat.h>
#ifdef HAVE_LIBARCHIVE
#include <archive.h>
#include <archive_entry.h>
#else
#include <zip.h>
#endif

#include "libfmvoice/fm_voice_bank.h"
#include "libfmvoice/loader.h"
#include "libvgm/utils/FileLoader.h"
#include "libvgm/utils/MemoryLoader.h"
#include "midilib/midi_file.h"
#include "cmdline.h"
#include "tools.h"
#include "vgm_analyzer.h"
#include "opl_voice_collector.h"
#include "opm_voice_collector.h"
#include "opn_voice_collector.h"
#include "opl_analyzer.h"
#include "opm_analyzer.h"
#include "opn_analyzer.h"

char *opt_output = "-";
char *opt_opl_format_str = "BNK";
char *opt_opm_format_str = "OPM";
char *opt_opn_format_str = "INS";
int opt_csv = 0;

int each_file(const char *path, int (*process_file)(const char *, void *), void *data_ptr) {
	struct stat st;
	int r = stat(path, &st);
	if(r < 0) {
		fprintf(stderr, "Could not stat %s: %s (%d)\n", path, strerror(errno), errno);
		return errno;
	}
	if(S_ISDIR(st.st_mode)) {
		DIR *d = opendir(path);
		if(!d) {
			fprintf(stderr, "Could not opendir %s: %s (%d)\n", path, strerror(errno), errno);
			return errno;
		}
		struct dirent *de;
		while((de = readdir(d))) {
			if(de->d_type != DT_REG && de->d_type != DT_DIR) continue;
			if(de->d_name[0] == '.' && de->d_name[1] == 0) continue;
			if(de->d_name[0] == '.' && de->d_name[1] == '.' && de->d_name[2] == 0) continue;
			char rpath[PATH_MAX]; // FIXME: maybe allocate new string instead?
			snprintf(rpath, sizeof(rpath), "%s/%s", path, de->d_name);
			each_file(rpath, process_file, data_ptr);
		}
		if(closedir(d)) {
			fprintf(stderr, "Could not closedir %s: %s (%d)\n", path, strerror(errno), errno);
			return errno;
		}
	} else if(S_ISREG(st.st_mode)) {
		int r = process_file(path, data_ptr);
		if(r) return r;
	}
	return 0;
}

int write_fn(void *buf, size_t bufsize, void *data_ptr) {
	return fwrite(buf, 1, bufsize, (FILE *)data_ptr);
}

static int vgm_file_cb(DATA_LOADER *loader, char *target_dir, char *filename_base, void *data_ptr) {
	struct vgm_analyzer va;
	vgm_analyzer_init(&va);

	DataLoader_ReadAll(loader);
	int r = vgm_analyzer_run(&va, DataLoader_GetData(loader), DataLoader_GetSize(loader));
	if(r) return r;

	struct opl_voice_collector opl_collector;
	opl_voice_collector_init(&opl_collector);
	struct opm_voice_collector opm_collector;
	opm_voice_collector_init(&opm_collector);
	struct opn_voice_collector opn_collector;
	opn_voice_collector_init(&opn_collector);
	enum vgm_chip_id ids[] = {
		YM3526, SECOND_YM3526, // OPL
		YM3812, SECOND_YM3812, // OPL2
		YM2151, SECOND_YM2151, // OPM
		YM2203, SECOND_YM2203, // OPN
		YM2608, SECOND_YM2608, // OPNA
		YM2610, SECOND_YM2610, // OPNB
		YM2612, SECOND_YM2612, // OPN2
	};
	for(int i = 0; i < sizeof(ids) / sizeof(ids[0]); i++) {
		if(!va.analyzers_by_id[ids[i]]) continue;
		if( // OPLx
			ids[i] == YM3526 || ids[i] == SECOND_YM3526 ||
			ids[i] == YM3812 || ids[i] == SECOND_YM3812
		) {
			struct opl_analyzer *a = (struct opl_analyzer *)va.analyzers_by_id[ids[i]];
			for(int i = 0; i < a->collector.num_voices; i++) {
				struct opl_voice_collector_voice oplv;
				memcpy(&oplv, a->collector.voices + i, sizeof(oplv));
				opl_voice_collector_push_voice(&opl_collector, &oplv, -1, -1);
			}
		} else if( // OPM
			ids[i] == YM2151 || ids[i] == SECOND_YM2151
		) {
			struct opm_analyzer *a = (struct opm_analyzer *)va.analyzers_by_id[ids[i]];
			for(int i = 0; i < a->collector.num_voices; i++) {
				struct opm_voice_collector_voice opmv;
				memcpy(&opmv, a->collector.voices + i, sizeof(opmv));
				opm_voice_collector_push_voice(&opm_collector, &opmv, -1, -1);
			}
		} else if( // OPN
			ids[i] == YM2612 || ids[i] == SECOND_YM2612 ||
			ids[i] == YM2203 || ids[i] == SECOND_YM2203 ||
			ids[i] == YM2608 || ids[i] == SECOND_YM2608 ||
			ids[i] == YM2610 || ids[i] == SECOND_YM2610
		) {
			struct opn_analyzer *a = (struct opn_analyzer *)va.analyzers_by_id[ids[i]];
			for(int i = 0; i < a->collector.num_voices; i++) {
				struct opn_voice_collector_voice opnv;
				memcpy(&opnv, a->collector.voices + i, sizeof(opnv));
				opn_voice_collector_push_voice(&opn_collector, &opnv, -1, -1);
			}
		}
	}

	struct fm_voice_bank bank;
	fm_voice_bank_init(&bank);
	for(int i = 0; i < opl_collector.num_voices; i++)
		fm_voice_bank_append_opl_voices(&bank, &opl_collector.voices[i].voice, 1);
	for(int i = 0; i < opm_collector.num_voices; i++)
		fm_voice_bank_append_opm_voices(&bank, &opm_collector.voices[i].voice, 1);
	for(int i = 0; i < opn_collector.num_voices; i++)
		fm_voice_bank_append_opn_voices(&bank, &opn_collector.voices[i].voice, 1);

	char fmfile[PATH_MAX];
	struct loader *loaders[3] = {
		get_loader_by_name(opt_opl_format_str),
		get_loader_by_name(opt_opm_format_str),
		get_loader_by_name(opt_opn_format_str),
	};
	for(int i = 0; i < sizeof(loaders) / sizeof(loaders[0]); i++) {
		struct fm_voice_bank_position pos;
		fm_voice_bank_position_init(&pos);
		for(int j = 0; ;j = pos.opl + pos.opm + pos.opn) {
			if(loaders[i]->max_opl_voices == 1 && loaders[i]->max_opm_voices == 0 && loaders[i]->max_opn_voices == 0) {
				if(bank.opl_voices[pos.opl].name[0])
					snprintf(fmfile, sizeof(fmfile), "%s/%s-%s.%s", target_dir, filename_base, bank.opl_voices[pos.opl].name, loaders[i]->file_ext);
				else
					snprintf(fmfile, sizeof(fmfile), "%s/%s-%d.%s", target_dir, filename_base, pos.opl, loaders[i]->file_ext);
			} else if(loaders[i]->max_opl_voices == 0 && loaders[i]->max_opm_voices == 1 && loaders[i]->max_opn_voices == 0) {
				if(bank.opm_voices[pos.opm].name[0])
					snprintf(fmfile, sizeof(fmfile), "%s/%s-%s.%s", target_dir, filename_base, bank.opm_voices[pos.opm].name, loaders[i]->file_ext);
				else
					snprintf(fmfile, sizeof(fmfile), "%s/%s-%d.%s", target_dir, filename_base, pos.opm, loaders[i]->file_ext);
			} else if(loaders[i]->max_opl_voices == 0 && loaders[i]->max_opm_voices == 0 && loaders[i]->max_opn_voices == 1) {
				if(bank.opn_voices[pos.opn].name[0])
					snprintf(fmfile, sizeof(fmfile), "%s/%s-%s.%s", target_dir, filename_base, bank.opn_voices[pos.opn].name, loaders[i]->file_ext);
				else
					snprintf(fmfile, sizeof(fmfile), "%s/%s-%d.%s", target_dir, filename_base, pos.opn, loaders[i]->file_ext);
			} else if(loaders[i]->max_opl_voices == 0 && loaders[i]->max_opm_voices == 0 && loaders[i]->max_opn_voices == 0) {
				snprintf(fmfile, sizeof(fmfile), "%s/%s.%s", target_dir, filename_base, loaders[i]->file_ext);
			} else {
				int next = j;
				if(loaders[i]->max_opl_voices == 0) next += bank.num_opl_voices - pos.opl; else next += loaders[i]->max_opl_voices;
				if(loaders[i]->max_opm_voices == 0) next += bank.num_opm_voices - pos.opm; else next += loaders[i]->max_opm_voices;
				if(loaders[i]->max_opn_voices == 0) next += bank.num_opn_voices - pos.opn; else next += loaders[i]->max_opn_voices;
				snprintf(fmfile, sizeof(fmfile), "%s/%s-%d-%d.%s", target_dir, filename_base, j + 1, next + 1, loaders[i]->file_ext);
			}
			FILE *o = fopen(fmfile, "w");
			struct fm_voice_bank_position prevpos;
			fm_voice_bank_position_copy(&prevpos, &pos);
			loader_save(loaders[i], &bank, &pos, write_fn, o);
			fclose(o);
			if(
				(loaders[i]->max_opl_voices == 0 || pos.opl < prevpos.opl + loaders[i]->max_opl_voices) &&
				(loaders[i]->max_opm_voices == 0 || pos.opm < prevpos.opm + loaders[i]->max_opm_voices) &&
				(loaders[i]->max_opn_voices == 0 || pos.opn < prevpos.opn + loaders[i]->max_opn_voices)
			) break;
		}
	}

	struct midi_file m;
	midi_file_init(&m, MIDI_FORMAT_MULTI_TRACKS, 0, 44100*120/60);
	for(int i = 0; i < va.num_chip_analyzers; i++)
		for(int j = 0; j < va.analyzers[i]->num_tracks; j++)
			midi_file_append_track(&m, &va.analyzers[i]->tracks[j].midi_track);

	char midifilename[512];
	snprintf(midifilename, sizeof(midifilename), "%s/%s.mid", target_dir, filename_base);
	FILE *f = fopen(midifilename, "wb");
	if(!f) {
		fprintf(stderr, "Could not open %s: %s (%d)\n", midifilename, strerror(errno), errno);
		return -2;
	}
	midi_file_write(&m, write_fn, f);
	fclose(f);

	// if(opt_csv) {
	// 	printf("FB\tCON\tPMS\tAMS\tSLOT");
	// 	for(int i = 0; i < 4; i++) {
	// 		printf("\tAR\tD1R\tD2R\tRR\tD1L\tTL\tKS\tMUL\tDT1\tDT2\tAMS-EN");
	// 	}
	// 	printf("\n");
	// 	for(int i = 0; i < collector.num_voices; i++) {
	// 		struct opm_voice_collector_voice *v = collector.voices + i;
	// 		printf(
	// 			"%d\t%d\t%d\t%d\t%d",
	// 			v->voice.rl_fb_con >> 3 & 0x07,
	// 			v->voice.rl_fb_con & 0x07,
	// 			v->voice.pms_ams >> 4 & 0x07,
	// 			v->voice.pms_ams & 0x03,
	// 			v->voice.slot
	// 		);
	// 		for(int j = 0; j < 4; j++) {
	// 			const uint8_t dtmap[] = { 3, 4, 5, 6,  3, 2, 1, 0 };
	// 			struct opm_voice_operator *op = &v->voice.operators[j];
	// 			printf(
	// 				"\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d",
	// 				op->ks_ar & 0x1f,
	// 				op->ams_d1r & 0x1f,
	// 				op->dt2_d2r & 0x1f,
	// 				op->d1l_rr & 0x0f,
	// 				op->d1l_rr >> 4,
	// 				op->tl & 0x7f,
	// 				op->ks_ar >> 6,
	// 				op->dt1_mul & 0x0f,
	// 				dtmap[op->dt1_mul >> 4 & 0x07],
	// 				op->dt2_d2r >> 6,
	// 				op->ams_d1r >> 7
	// 			);
	// 		}
	// 		printf("\n");
	// 	}

	// 	return 0;
	// }

	return 0;
}

struct vgm_file_cb_pair {
	int (*process_file)(DATA_LOADER *, char *, char *, void *);
	void *data_ptr;
};

static int each_vgm_file_cb(const char *filename, void *data_ptr) {
	struct vgm_file_cb_pair *pair = (struct vgm_file_cb_pair *)data_ptr;
	int l = strlen(filename);
	const char *ext = filename + l - 4;
	if(!strcasecmp(ext, ".zip")) {
#ifdef HAVE_LIBARCHIVE
		struct archive *a;
		a = archive_read_new();
		archive_read_support_filter_all(a);
		archive_read_support_format_all(a);
		int r = archive_read_open_filename(a, filename, 10240);
		if(r != ARCHIVE_OK) return r;
		struct archive_entry *entry;
		while(archive_read_next_header(a, &entry) == ARCHIVE_OK) {
			printf("%s\n",archive_entry_pathname(entry));
			archive_read_data_skip(a);
		}
		r = archive_read_free(a);
		if(r != ARCHIVE_OK) return r;
#else
		int err;
		zip_t *z = zip_open(filename, ZIP_RDONLY, &err);
		if(!z) {
			fprintf(stderr, "Could not open zip file %s: %d\n", filename, err);
			return err;
		}
		int num_entries = zip_get_num_entries(z, 0);
		if(num_entries < 0) {
			fprintf(stderr, "Could not get number of entries\n");
			zip_close(z);
			return -1;
		}
		char writable_path[PATH_MAX];
		strncpy(writable_path, filename, sizeof(writable_path));
		if(writable_path[l-4] == '.')
			writable_path[l-4] = 0;
		mkdir(writable_path, 0777);
		if(errno && errno != EEXIST) {
			fprintf(stderr, "Could not make directory %s: %s (%d)\n", writable_path, strerror(errno), errno);
			return errno;
		}
		for(int j = 0; j < num_entries; j++) {
			zip_stat_t st;
			zip_stat_index(z, j, ZIP_STAT_NAME | ZIP_STAT_SIZE, &st);
			int vl = strlen(st.name);
			if(vl > 4 && (!strcasecmp(st.name + vl - 4, ".vgm") || (!strcasecmp(st.name + vl - 4, ".vgz")))) {
				uint8_t *buf = malloc(st.size);
				if(!buf) {
					fprintf(stderr, "Could not allocate %lu bytes\n", st.size);
					return -1;
				}
				zip_file_t *f = zip_fopen_index(z, j, 0);
				if(!f) {
					fprintf(stderr, "Could not open %s (%d)\n", st.name, j);
					return -1;
				}
				int nr = zip_fread(f, buf, st.size);
				if(nr != st.size) {
					fprintf(stderr, "Could not read %lu bytes from zip\n", st.size);
					return -1;
				}
				zip_fclose(f);

				DATA_LOADER *dload = MemoryLoader_Init(buf, st.size);
				if(!dload) {
					fprintf(stderr, "Could not init loader for %s\n", filename);
					return -1;
				}
				if(DataLoader_Load(dload)) {
					fprintf(stderr, "Could not load %s\n", filename);
					DataLoader_Deinit(dload);
					return -1;
				}
				char base[PATH_MAX];
				strncpy(base, st.name, sizeof(base));
				char *b = basename(base);
				char *dot = strrchr(b, '.');
				if(dot) *dot = 0;
				int r = pair->process_file(dload, writable_path, b, pair->data_ptr);
				DataLoader_Deinit(dload);
				if(r) return r;
			}
		}
		zip_close(z);
#endif
	} else if(!strcasecmp(ext, ".vgm") || !strcasecmp(ext, ".vgz")) {
		DATA_LOADER *dload = FileLoader_Init(filename);
		if(!dload) {
			fprintf(stderr, "Could not init loader for %s\n", filename);
			return -1;
		}
		if(DataLoader_Load(dload)) {
			fprintf(stderr, "Could not load %s\n", filename);
			DataLoader_Deinit(dload);
			return -2;
		}
		char *basepath = strdup(filename);
		char base[PATH_MAX];
		strncpy(base, filename, sizeof(base));
		char *b = basename(base);
		char *dot = strrchr(b, '.');
		if(dot) *dot = 0;
		int r = pair->process_file(dload, dirname(basepath), b, pair->data_ptr);
		free(basepath);
		DataLoader_Deinit(dload);
		if(r) return r;
	}
	return 0;
}

int each_vgm_file(const char *path, int (*process_file)(DATA_LOADER *, char *, char *, void *), void *data_ptr) {
	struct vgm_file_cb_pair pair = {
		.process_file = process_file,
		.data_ptr = data_ptr,
	};
	return each_file(path, each_vgm_file_cb, &pair);
}

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
			'l', "opl-format",
			"Output format for OPL voices: SBI, INS, BNK, IBK, CMF",
			"format",
			TYPE_OPTIONAL,
			TYPE_STRING, &opt_opl_format_str
		},
		{
			'm', "opm-format",
			"Output format for OPM voices: OPM, SYX_FB01, SYX_DX21",
			"format",
			TYPE_OPTIONAL,
			TYPE_STRING, &opt_opm_format_str
		},
		{
			'n', "opn-format",
			"Output format for OPN voices: INS, DMP, TFI, Y12",
			"format",
			TYPE_OPTIONAL,
			TYPE_STRING, &opt_opn_format_str
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

	for(int i = optind; i < argc; i++) {
		int r = each_vgm_file(argv[i], vgm_file_cb, 0);
		if(r) fprintf(stderr, "Could not process %s: %d\n", argv[i], r);
	}

	return 0;
}
