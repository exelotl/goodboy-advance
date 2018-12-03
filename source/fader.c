#include "common.h"

#define FADE_NONE 0
#define FADE_IN 1
#define FADE_OUT 2

static int fade_mode;
static int fade_amount, old_fade_amount;
static bool fade_color;
static bool fade_speed;
static callback_t fade_cb;

void fader_init(void) {
	fade_mode = FADE_NONE;
	
	// instead of doing this, we assume that the target banks were written to directly...
	
	// for (int i=0; i<PALBANK_NUM_COLORS; i++) {
	// 	pal_bg_target[i] = pal_bg_mem[i];
	// 	pal_obj_target[i] = pal_obj_mem[i];
	// }
	fade_amount = 0;
	old_fade_amount = 0;
	fade_cb = NULL;
}

void fader_update(void) {
	old_fade_amount = fade_amount;
	
	switch (fade_mode) {
		case FADE_IN:
			fade_amount = MAX(fade_amount-fade_speed, 0);
			clr_fade_fast(pal_bg_target, RGB15(0,0,0), pal_bg_mem, PALBANK_NUM_COLORS, fade_amount);
			clr_fade_fast(pal_obj_target, RGB15(0,0,0), pal_obj_mem, PALBANK_NUM_COLORS, fade_amount);
			if (fade_cb && old_fade_amount != fade_amount && fade_amount == 0) {
				fade_cb();
			}
			break;
		case FADE_OUT:
			fade_amount = MIN(fade_amount+fade_speed, 31);
			clr_fade_fast(pal_bg_target, RGB15(0,0,0), pal_bg_mem, PALBANK_NUM_COLORS, fade_amount);
			clr_fade_fast(pal_obj_target, RGB15(0,0,0), pal_obj_mem, PALBANK_NUM_COLORS, fade_amount);
			// clr_adj_brightness(pal_bg_mem, pal_bg_mem, NUM_COLORS, -fade_speed);
			// clr_adj_brightness(pal_obj_mem, pal_obj_mem, NUM_COLORS, -fade_speed);
			if (fade_cb && old_fade_amount != fade_amount && fade_amount == 31) {
				fade_cb();
			}
			break;
		default:
			return;
	}
}

void fader_fade_out(int spd, callback_t cb) {
	fade_mode = FADE_OUT;
	fade_speed = spd;
	fade_amount = 0;
	old_fade_amount = fade_amount;
	fade_cb = cb;
}

void fader_fade_in(int spd, callback_t cb) {
	fade_mode = FADE_IN;
	fade_speed = spd;
	fade_amount = 31;
	old_fade_amount = fade_amount;
	fade_cb = cb;
}
