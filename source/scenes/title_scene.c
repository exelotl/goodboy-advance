#include "common.h"
// #include "assets/Lv1.h"
#include "assets/SprShared1.h"
#include "assets/BgIntro1.h"
#include "assets/BgIntro2.h"
#include "fonts/Acknowledge.h"
#include "fonts/Volter.h"
#include "fonts/GelatinMono.h"

static entity_t label_hello;

static bool has_pressed;

int slide_number = 1;

	
// sources
static unsigned short *ImagePal;
static unsigned char *ImageTiles;
static unsigned short *ImageMap;
static int ImagePalLen;
static int ImageTilesLen;
static int ImageMapLen;

// destinations
static const int image_cb = 0;
static const int image_sb = 24;
static const int image_palbank = 0;

#define SetImage(name) {            \
	ImagePal = name##Pal;           \
	ImageTiles = name##Tiles;       \
	ImageMap = name##Map;           \
	ImagePalLen = name##PalLen;     \
	ImageTilesLen = name##TilesLen; \
	ImageMapLen = name##MapLen;     \
}

static void show(void) {
	
	has_pressed = false;
	
	pal_bg_target_mem[0] = RGB15(10, 0, 10);
	
	switch (slide_number) {
		case 0:
			break;
		case 1:
			SetImage(BgIntro1);
			break;	
		case 2:
			SetImage(BgIntro2);
			break;
		case 3:
			break;
		default:
			break;
	}
	
	
	dma3_cpy(pal_bg_target_bank[image_palbank], ImagePal, ImagePalLen);
	dma3_cpy(&tile_mem[image_cb], ImageTiles, ImageTilesLen);
	dma3_cpy(&se_mem[image_sb], ImageMap, ImageMapLen);
	
	REG_DISPCNT = DCNT_MODE1 // 2 regular, 1 affine
				| DCNT_BG0
				// | DCNT_BG1
				// | DCNT_BG2
				// | DCNT_BG3
				| DCNT_OBJ     // enable sprites
				| DCNT_OBJ_1D; // 1D tile mapping for sprites
	
	REG_BG0CNT = BG_PRIO(3) | BG_8BPP | BG_SBB(image_sb) | BG_CBB(image_cb) | BG_REG_32x32;
	
	// text palette
	pal_obj_target_bank[8][0] = CLR_SKYBLUE;
	pal_obj_target_bank[8][1] = CLR_WHITE;
	pal_obj_target_bank[8][2] = CLR_BLACK;
	pal_obj_target_bank[8][3] = RGB15(16,16,16); // for greyed-out text
	
	// reserve sprite tiles in VRAM
	uint tid = 0;
	// tid = label_init(&label_hello, &VolterFont, 1, 2, tid, 3);
	
	// label_hello.x = 30 << FIX_SHIFT;
	// label_hello.y = 100 << FIX_SHIFT;
	// label_begin_write(&label_hello);
	// tte_write("press any key to start");
	
	// mmStart(MOD_SPACECAT, MM_PLAY_LOOP);
	
	fader_init();  // copies all the palettes assuming they won't change from now on.
	
	int fade_in_speed = 4;
	
	// slow fade for transitioning from beginning for main game
	if (slide_number == 1 || slide_number == 3) {
		fade_in_speed = 1;
	}
	fader_fade_in(fade_in_speed, NULL);
	
	timeout_clear();
	
	REG_BG0HOFS = 0;
	REG_BG0VOFS = 0;
}

static void hide(void) {
	mmStop();
}

static void play_level_1(void) {
	scene_set(game_scene);
}

static void goto_next_scene(void) {
	
	switch (slide_number) {
		case 0:
			break;
		case 1:
			SetImage(BgIntro1);
			slide_number = 2;
			scene_set(title_scene);
			break;
		case 2:
			timeout_set(30, play_level_1);
			break;
		case 3:
			break;
		default:
			break;
	}
}

static void update(void) {
	// label_update(&label_hello);
	fader_update();
	
	if (!has_pressed && key_hit(KEY_ANY)) {
		int fade_out_speed = 4;
		
		// slow fade for transitioning to main game
		if (slide_number == 2) {
			fade_out_speed = 1;
		}
		
		fader_fade_out(fade_out_speed, goto_next_scene);
	}
}

const scene_t title_scene = {
	.show = show,
	.hide = hide,
	.update = update,
};
