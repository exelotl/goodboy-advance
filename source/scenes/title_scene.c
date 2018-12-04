#include "common.h"
// #include "assets/Lv1.h"
#include "assets/SprShared1.h"
#include "assets/BgIntro1.h"
#include "assets/BgIntro2.h"
#include "assets/BgTitle.h"
#include "fonts/Acknowledge.h"
#include "fonts/Volter.h"
#include "assets/SprPlayer.h"
#include "fonts/GelatinMono.h"

static bool has_pressed;

// player anim for first screen only
static entity_t title_player;
static const anim_t AnimWalkAll = { .speed = 5, .loop = 1, .len = 8, .frames = (int[]){0,1,2,3,4,5,6,7} };
static uint title_player_init(uint tid);
static void title_player_update(void);

static entity_t label_anykey;


int slide_number = 0;

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
			SetImage(BgTitle);
			mmStart(MOD_SPACEDOG, MM_PLAY_LOOP);
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
	tid = title_player_init(tid);
	tid = label_init(&label_anykey, &VolterFont, 1, 2, tid, 3);
	
	label_anykey.x = Fix(88);
	label_anykey.y = Fix(130);
	label_begin_write(&label_anykey);
	tte_write("PRESS START");
	
	// set up sprite palettes
	dma3_cpy(&pal_obj_target_bank[0], SprPlayerPal, SprPlayerPalLen);
	
	
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
	// mmStop();
}

static void play_level_1(void) {
	scene_set(game_scene);
}

static void goto_next_scene(void) {
	
	switch (slide_number) {
		case 0:
			slide_number = 1;
			scene_set(title_scene);
			break;
		case 1:
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
	fader_update();
	
	if (slide_number == 0) {
		title_player_update();
		if ((global_tick % 64) < 32) {
			label_update(&label_anykey);
		}
	}
	
	if (!has_pressed && key_hit(KEY_A | KEY_B | KEY_START)) {
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


static uint title_player_init(uint tid) {
	title_player = (entity_t) {
		.tid = tid,
		.flags = ACTIVE,
		.x = Fix(30),
		.y = Fix(117),
	};
	set_anim(&title_player, &AnimWalkAll);
	return tid + 4*4; // single frame
}

static void title_player_update(void) {
	int px = title_player.x >> FIX_SHIFT;
	int py = title_player.y >> FIX_SHIFT;
	
	entity_animate(&title_player);
	
	obj_set_attr(&obj_mem[reserve_obj()],
		(py & ATTR0_Y_MASK) | ATTR0_SQUARE,
		(px & ATTR1_X_MASK) | ATTR1_SIZE_32,
		title_player.tid | ATTR2_PRIO(1) | ATTR2_PALBANK(0));
	
	dma3_cpy(
		&tile_mem_obj[0][title_player.tid],
		&SprPlayerTiles[title_player.anim->frames[title_player.frame] * sizeof(TILE) * 16],
		sizeof(TILE) * 16);
}