#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <libgen.h>
#include <dirent.h>
#ifdef HAVE_LIBARCHIVE
#include <archive.h>
#include <archive_entry.h>
#else
#include <zip.h>
#endif

#include "cmdline.h"
#include "tools.h"
#include "vgm_analyzer.h"
#include "opm_voice_collector.h"
#include "opm_analyzer.h"
#include "opn_analyzer.h"
#include "libvgm/utils/FileLoader.h"
#include "libvgm/utils/MemoryLoader.h"

#include <sys/stat.h>
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

static int vgm_file_cb(DATA_LOADER *loader, char *filename_base, char *target_dir, void *data_ptr) {
	printf("vgm_file_cb %s/%s.opm\n", filename_base, target_dir);
	DataLoader_ReadAll(loader);
	return vgm_analyzer_run(data_ptr, DataLoader_GetData(loader), DataLoader_GetSize(loader));
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
				char writable_path[PATH_MAX];
				char base[PATH_MAX];
				strncpy(writable_path, filename, sizeof(writable_path));
				if(writable_path[l-4] == '.')
					writable_path[l-4] = 0;
				strncpy(base, st.name, sizeof(base));
				char *b = basename(base);
				char *dot = strrchr(b, '.');
				if(dot) *dot = 0;
				int r = pair->process_file(dload, basename(writable_path), b, pair->data_ptr);
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
		int r = pair->process_file(dload, basename(basepath), b, pair->data_ptr);
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
		int r = each_vgm_file(argv[i], vgm_file_cb, &va);
		if(r) fprintf(stderr, "Could not proces %s: %d\n", argv[i], r);
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
