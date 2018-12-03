#include "common.h"
// #include "assets/Lv1.h"
#include "assets/SprShared1.h"
#include "assets/BgIntro1.h"
#include "assets/BgIntro2.h"
#include "fonts/Acknowledge.h"
#include "fonts/Volter.h"
#include "fonts/GelatinMono.h"

static entity_t label_thanks;

static void show(void) {
	
	REG_DISPCNT = DCNT_MODE1 // 2 regular, 1 affine
				// | DCNT_BG0
				// | DCNT_BG1
				// | DCNT_BG2
				// | DCNT_BG3
				| DCNT_OBJ     // enable sprites
				| DCNT_OBJ_1D; // 1D tile mapping for sprites
	
	// text palette
	pal_obj_target_bank[8][0] = CLR_SKYBLUE;
	pal_obj_target_bank[8][1] = CLR_WHITE;
	pal_obj_target_bank[8][2] = CLR_BLACK;
	pal_obj_target_bank[8][3] = RGB15(16,16,16); // for greyed-out text
	
	// reserve sprite tiles in VRAM
	uint tid = 0;
	tid = label_init(&label_thanks, &VolterFont, 1, 2, tid, 3);
	
	label_thanks.x = 30 << FIX_SHIFT;
	label_thanks.y = 100 << FIX_SHIFT;
	label_begin_write(&label_thanks);
	tte_write("Thanks for p");
	
	// mmStart(MOD_SPACECAT, MM_PLAY_LOOP);
	
	timeout_clear();
	
	REG_BG0HOFS = 0;
	REG_BG0VOFS = 0;
}

static void hide(void) {
	mmStop();
}

static void play_level_2(void) {
	scene_set(game_scene);
}

static void update(void) {
	label_update(&label_thanks);
}

const scene_t fake_end_scene = {
	.show = show,
	.hide = hide,
	.update = update,
};
