#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vgm_analyzer.h"
#include "opn_analyzer.h"
#include "opm_analyzer.h"

static int add_analyzer(struct chip_analyzer *a, int id, struct vgm_analyzer *analyzer) {
	analyzer->num_chip_analyzers++;
	analyzer->analyzers = realloc(analyzer->analyzers, analyzer->num_chip_analyzers * sizeof(analyzer->analyzers[0]));
	if(!analyzer->analyzers) {
		fprintf(stderr, "Could not reallocate %d analyzers\n", analyzer->num_chip_analyzers);
		return -1;
	}
	analyzer->analyzers[analyzer->num_chip_analyzers - 1] = a;
	analyzer->analyzers_by_id[id] = a;
	return 0;
}

static int init_chip_fn(enum vgm_chip_id chip_id, int clock, void *data_ptr) {
	struct vgm_analyzer *analyzer = (struct vgm_analyzer *)data_ptr;
	if(analyzer->analyzers_by_id[chip_id]) return -2;

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
			return add_analyzer((struct chip_analyzer *)opn_analyzer_new(clock, chip_id == YM2203 || chip_id == SECOND_YM2203 ? 3 : 6), chip_id, analyzer);
			break;
		case YM2151:
		case SECOND_YM2151:
			return add_analyzer((struct chip_analyzer *)opm_analyzer_new(clock), chip_id, analyzer);
			break;
		default:
			return -1;
	}
}

static void write_reg8_data8_fn(enum vgm_chip_id chip_id, uint8_t reg, uint8_t data, void *data_ptr) {
	struct vgm_analyzer *analyzer = (struct vgm_analyzer *)data_ptr;
	if(!analyzer->analyzers_by_id[chip_id]) return;
	chip_analyzer_cmd_reg8_data8(analyzer->analyzers_by_id[chip_id], reg, data);
}

static void write_port8_reg8_data8_fn(enum vgm_chip_id chip_id, uint8_t port, uint8_t reg, uint8_t data, void *data_ptr) {
	struct vgm_analyzer *analyzer = (struct vgm_analyzer *)data_ptr;
	if(!analyzer->analyzers_by_id[chip_id]) return;
	chip_analyzer_cmd_port8_reg8_data8(analyzer->analyzers_by_id[chip_id], port, reg, data);
}

static void wait_fn(int samples, void *data_ptr) {
	// printf("wait %d\n", samples);
}

static void end_fn(void *data_ptr) {
	// printf("end\n");
}

void vgm_analyzer_init(struct vgm_analyzer *a) {
	vgm_interpreter_init(&a->interpreter);
	a->analyzers = 0;
	a->num_chip_analyzers = 0;
	memset(a->analyzers_by_id, 0, sizeof(a->analyzers_by_id));
}

void vgm_analyzer_clear(struct vgm_analyzer *a) {
	free(a->analyzers);
}

int vgm_analyzer_run(struct vgm_analyzer *analyzer, uint8_t *buf, size_t s) {
	vgm_interpreter_init(&analyzer->interpreter);
	analyzer->interpreter.init_chip = init_chip_fn;
	analyzer->interpreter.write_reg8_data8 = write_reg8_data8_fn;
	analyzer->interpreter.write_port8_reg8_data8 = write_port8_reg8_data8_fn;
	analyzer->interpreter.wait = wait_fn;
	analyzer->interpreter.end = end_fn;
	analyzer->interpreter.data_ptr = analyzer;
	struct vgm_error error;
	enum vgm_error_code e = vgm_interpreter_run(&analyzer->interpreter, buf, s, &error);
	return e;
}

