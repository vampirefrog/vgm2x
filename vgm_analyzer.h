#pragma once

#include <stdint.h>
#include <stddef.h>

#include "vgm/interpreter.h"
#include "chip_analyzer.h"

struct vgm_analyzer {
	struct vgm_interpreter interpreter;
	struct chip_analyzer **analyzers;
	int num_chip_analyzers;
	struct chip_analyzer *analyzers_by_id[256];
};

void vgm_analyzer_init(struct vgm_analyzer *);
void vgm_analyzer_clear(struct vgm_analyzer *);
int vgm_analyzer_run(struct vgm_analyzer *, uint8_t *buf, size_t s);
