#pragma once

#define VGM_ERROR_CODES \
	VGM_ERROR_CODE(SUCCESS) \
	VGM_ERROR_CODE(BAD_IDENT) \
	VGM_ERROR_CODE(INVALID_CMD) \
	VGM_ERROR_CODE(TOO_SHORT)

#define VGM_ERROR_CODE(c) c,
enum vgm_error_code {
	VGM_ERROR_CODES
	NUM_ERRORS
};
#undef VGM_ERROR_CODE

struct vgm_error {
	enum vgm_error_code code;
	int file_pos, sample_pos;
};

int vgm_get_error_string(char *buf, int buf_size, enum vgm_error_code error_code, struct vgm_error *error);
