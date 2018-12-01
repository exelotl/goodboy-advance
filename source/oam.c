#include "common.h"

static const int obj_max = 128;
static int aff_max = 32;
static int obj_n = 0, aff_n = 0;

int reserve_obj(void) {
	return obj_n < obj_max ? obj_n++ : -1;
}

int reserve_obj_multi(int n) {
	int id = obj_n;
	if (obj_n + n <= obj_max) {
		obj_n += n;
		return id;
	}
	return -1;
}

int reserve_aff(void) {
	return aff_n < aff_max ? aff_n++ : -1;
}

// reduce the number of available affine matrices
// allowing you to reserve a matrix permanently
// e.g. scaled text never changes size so it would be pointless to
//      recompute the matrix every frame
int reserve_aff_perm(void) {
	return --aff_max;
}

void oam_update(void) {
	// hide remaining objects in case they were used last frame
	obj_hide_multi(&obj_mem[obj_n], 128 - obj_n);
	obj_n = 0;
	aff_n = 0;
}

