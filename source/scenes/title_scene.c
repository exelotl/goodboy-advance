#include "common.h"
// #include "assets/Lv1.h"
#include "assets/SprShared1.h"
#include "fonts/Acknowledge.h"
#include "fonts/Volter.h"
#include "fonts/GelatinMono.h"

static void show(void) {
	
	pal_bg_mem[0] = RGB15(10, 0, 10);
	
	// set up sprite palettes
	dma3_cpy(&pal_obj_bank[0], SprShared1Pal, SprShared1PalLen);
	
	// text palette
	pal_obj_bank[8][0] = CLR_SKYBLUE;
	pal_obj_bank[8][1] = CLR_WHITE;
	pal_obj_bank[8][2] = CLR_BLACK;
	pal_obj_bank[8][3] = RGB15(16,16,16); // for greyed-out text
	
	// reserve sprite tiles in VRAM
	uint tid = 0;
	tid = label_init(&label_hello, &VolterFont, 1, 2, tid, 3);
	
	label_hello.x = 30 << FIX_SHIFT;
	label_hello.y = 100 << FIX_SHIFT;
	label_begin_write(&label_hello);
	tte_write("press any key to start");
	
	mmStart(MOD_SPACECAT, MM_PLAY_LOOP);
}

static void hide(void) {
	mmStop();
}

static void update(void) {
	label_update(&label_hello);
	
	if (key_hit(KEY_ANY)) {
		scene_set(game_scene);
	}
}

const scene_t title_scene = {
	.show = show,
	.hide = hide,
	.update = update,
};
