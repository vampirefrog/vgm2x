#include <string.h>

#include "chip_analyzer.h"

void chip_analyzer_init(struct chip_analyzer *analyzer, int clock) {
	memset(analyzer, 0, sizeof(*analyzer));
	analyzer->clock = clock;
}

void chip_analyzer_set_data_ptr(struct chip_analyzer *analyzer, void *data_ptr) {
	analyzer->data_ptr = data_ptr;
}

void chip_analyzer_cmd_reg8_data8(struct chip_analyzer *analyzer, uint8_t reg, uint8_t data) {
	if(analyzer->cmd_reg8_data8) analyzer->cmd_reg8_data8(analyzer, reg, data, analyzer->data_ptr);
}

void chip_analyzer_cmd_ofs8_data8(struct chip_analyzer *analyzer, uint8_t ofs, uint8_t data) {
	if(analyzer->cmd_ofs8_data8) analyzer->cmd_ofs8_data8(analyzer, ofs, data, analyzer->data_ptr);
}

void chip_analyzer_cmd_ofs16_data8(struct chip_analyzer *analyzer, uint16_t ofs, uint8_t data) {
	if(analyzer->cmd_ofs16_data8) analyzer->cmd_ofs16_data8(analyzer, ofs, data, analyzer->data_ptr);
}

void chip_analyzer_cmd_ofs16_data16(struct chip_analyzer *analyzer, uint16_t ofs, uint16_t data) {
	if(analyzer->cmd_ofs16_data16) analyzer->cmd_ofs16_data16(analyzer, ofs, data, analyzer->data_ptr);
}

void chip_analyzer_cmd_port8_reg8_data8(struct chip_analyzer *analyzer, uint8_t port, uint8_t reg, uint8_t data) {
	if(analyzer->cmd_port8_reg8_data8) analyzer->cmd_port8_reg8_data8(analyzer, port, reg, data, analyzer->data_ptr);
}

void chip_analyzer_cmd_write8(struct chip_analyzer *analyzer, uint8_t data) {
	if(analyzer->cmd_write8) analyzer->cmd_write8(analyzer, data, analyzer->data_ptr);
}
