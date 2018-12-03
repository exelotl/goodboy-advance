#include "common.h"
#include "tte_multiobj.h"

#include "fonts/Acknowledge.h"
#include "fonts/Volter.h"

#define LABEL_OBJ_W 8
#define LABEL_OBJ_H 4

static int text_aff_id = -1;

uint label_init(entity_t *label, const TFont *font, uint ink, uint shadow, uint tid, uint obj_count) {
	
	// if (text_aff_id == -1) {
	// 	text_aff_id = reserve_aff_perm();
	// 	obj_aff_scale(&obj_aff_mem[text_aff_id], TEXT_INV_SCALE, TEXT_INV_SCALE);
	// }
	
	entity_activate(label);
	label->x = 0;
	label->y = 0;
	label->velx = 0;
	label->vely = 0;
	label->font = font;
	label->ink = ink;
	label->shadow = shadow;
	label->tid = tid;
	label->obj_count = obj_count;
	
	return tid + LABEL_OBJ_W * LABEL_OBJ_H * label->obj_count;
}


// call this to clear a label and prepare TTE, then use the TTE functions to draw to it
void label_begin_write(entity_t *label) {
	tte_set_multiobj_dest(label->tid, LABEL_OBJ_W, LABEL_OBJ_H, label->obj_count);
	tte_erase_multiobj();
	tte_set_font(label->font);
	tte_set_ink(label->ink);
	tte_set_shadow(label->shadow);
}

void label_update(entity_t *label) {
	
	if (!is_active(label)) return;
	
	label->x += label->velx;
	label->y += label->vely;
	
	int obj_id = reserve_obj_multi(label->obj_count);
	if (obj_id == -1)
		return;
	
	int x = label->x >> FIX_SHIFT;
	int y = label->y >> FIX_SHIFT;
	int w = LABEL_OBJ_W * 8;
	int h = LABEL_OBJ_H * 8;
	
	if (label->font == &AcknowledgeFont) {
		// x -= fxmul(w, FIX_ONE - TEXT_SCALE/2); // positional correction
		// y -= fxmul(h, FIX_ONE - TEXT_SCALE/2);
		multiobj_set_attr(&obj_mem[obj_id],
			label->obj_count,
			(y & ATTR0_Y_MASK) | ATTR0_WIDE | ATTR0_AFF_DBL,
			(x & ATTR1_X_MASK) | ATTR1_SIZE_64x32 | ATTR1_AFF_ID(text_aff_id),
			label->tid | ATTR2_PALBANK(8));
	} else {
		multiobj_set_attr(&obj_mem[obj_id],
			label->obj_count,
			(y & ATTR0_Y_MASK) | ATTR0_WIDE,
			(x & ATTR1_X_MASK) | ATTR1_SIZE_64x32,
			label->tid | ATTR2_PALBANK(8));
	}
}

