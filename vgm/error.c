#include <stdio.h>

#include "error.h"

#define VGM_ERROR_CODE(c) #c,
static const char *vgm_error_names[] = {
	VGM_ERROR_CODES
};
#undef VGM_ERROR_CODE

int vgm_get_error_string(char *buf, int buf_size, enum vgm_error_code error_code, struct vgm_error *error) {
	return snprintf(buf, buf_size, "%s", error_code < NUM_ERRORS ? vgm_error_names[error_code] : "Unknown");
}
