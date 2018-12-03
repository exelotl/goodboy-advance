#include "common.h"
#include "assets/SprShared1.h"
// #include "assets/BgEndForeground.h"
// #include "assets/BgEndBackground.h"
// #include "assets/BgThanks.h"
// #include "assets/BgEndShared.h"
#include "assets/BgEndAll.h"
#include "fonts/Acknowledge.h"
#include "fonts/Volter.h"
#include "fonts/GelatinMono.h"

static const int sb_size = sizeof(SCREENBLOCK);
static const int sb_size_short = sizeof(SCREENBLOCK) / 2;

static const int shared_palbank = 0;
static const int shared_cb = 0;      // tile gfx

static const int background_sb = 31; // entries
static const int foreground_sb = 29;
static const int thanks_sb = 27;

static int bg_end_background_scrollx;  // the speed lines scroll
static int bg_end_background_scrolly;

static int bg_end_foreground_scrollx;  // the rocket scroll
static int bg_end_foreground_scrolly;

static int bg_thanks_scrollx;
static int bg_thanks_scrolly;

static entity_t label_rik;
static entity_t label_gecko;

static void show(void) {
	
	dma3_cpy(pal_bg_target_bank[shared_palbank], BgEndAllPal, BgEndAllPalLen);
	dma3_cpy(&tile_mem[shared_cb], BgEndAllTiles, BgEndAllTilesLen);

	// dma3_cpy(&se_mem[background_sb], BgEndBackgroundMap, BgEndBackgroundMapLen);
	// dma3_cpy(&se_mem[foreground_sb], BgEndForegroundMap, BgEndForegroundMapLen);
	
	dma3_cpy(&se_mem[background_sb], BgEndAllMap,                   sb_size);
	dma3_cpy(&se_mem[foreground_sb], BgEndAllMap + sb_size_short*1, sb_size);
	dma3_cpy(&se_mem[thanks_sb],     BgEndAllMap + sb_size_short*2, sb_size);
	
	for (int i=0; i<sb_size_short; i++) {
		se_mem[foreground_sb+1][i] = 0; // clear out the lower screenblock so we can scroll the ship
	}
	
	// for (int i=0; i<sb_size_short; i++) {
	// 	int pal = se_mem[foreground_sb][i] & SE_PALBANK_MASK;
	// 	pal >>= SE_PALBANK_SHIFT;
	// 	pal += foreground_palbank;
	// 	pal <<= SE_PALBANK_SHIFT;
	// 	se_mem[foreground_sb][i] |= SE_PALBANK(pal);
	// }
	
	REG_DISPCNT = DCNT_MODE0
				| DCNT_BG0
				| DCNT_BG1
				| DCNT_BG2
				// | DCNT_BG3
				| DCNT_OBJ     // enable sprites
				| DCNT_OBJ_1D; // 1D tile mapping for sprites
	
	REG_BG0CNT = BG_PRIO(3) | BG_8BPP | BG_SBB(background_sb) | BG_CBB(shared_cb) | BG_REG_32x32;
	REG_BG1CNT = BG_PRIO(2) | BG_8BPP | BG_SBB(foreground_sb) | BG_CBB(shared_cb) | BG_REG_32x64;
	REG_BG2CNT = BG_PRIO(1) | BG_8BPP | BG_SBB(thanks_sb) | BG_CBB(shared_cb) | BG_REG_32x32;
	
	// mmStart(MOD_SPACECAT, MM_PLAY_LOOP);
	
	timeout_clear();
	
	
	// text palette
	pal_obj_target_bank[8][0] = CLR_SKYBLUE;
	pal_obj_target_bank[8][1] = CLR_WHITE;
	pal_obj_target_bank[8][2] = CLR_BLACK;
	pal_obj_target_bank[8][3] = RGB15(16,16,16); // for greyed-out text
	
	// reserve sprite tiles in VRAM
	uint tid = 0;
	tid = label_init(&label_rik, &VolterFont, 1, 2, tid, 2);
	tid = label_init(&label_gecko, &VolterFont, 1, 2, tid, 2);
	
	label_begin_write(&label_rik);
	tte_write("@hot_pengu");
	
	label_begin_write(&label_gecko);
	tte_write("@exelotl");
	
	REG_BG0HOFS = 0;
	REG_BG0VOFS = 0;
	bg_end_background_scrollx = 0;
	bg_end_background_scrolly = 0;
	
	bg_end_foreground_scrollx = 0;
	// bg_end_foreground_scrolly = 96;
	bg_end_foreground_scrolly = -120;
	
	bg_thanks_scrollx = -44;
	bg_thanks_scrolly = 98;
	
	fader_init();  // copies all the palettes assuming they won't change from now on.
	fader_fade_in(1, NULL);
	
}

static void hide(void) {
	mmStop();
}
static void update(void) {
	fader_update();
	
	bg_end_background_scrollx += 4;
	bg_end_background_scrolly -= 9;
	REG_BG0HOFS = bg_end_background_scrollx;
	REG_BG0VOFS = bg_end_background_scrolly;
	
	if (global_tick % 8 == 0) {
		// if (bg_end_foreground_scrolly > -160) {
		// 	bg_end_foreground_scrolly -= 1;
		// }
		if (bg_end_foreground_scrolly < 96) {
		// if (bg_end_foreground_scrolly < 96) {
			bg_end_foreground_scrolly += 1;
		}
	}
	REG_BG1HOFS = bg_end_foreground_scrollx + qran_range(-1, 1);
	REG_BG1VOFS = bg_end_foreground_scrolly + qran_range(-1, 1);
	
	REG_BG2HOFS = bg_thanks_scrollx;
	REG_BG2VOFS = bg_thanks_scrolly;
	
	if (global_tick % 8 == 0 && bg_thanks_scrolly > -32) {
		bg_thanks_scrolly -= 1;
	}
		
	label_gecko.x = (-bg_thanks_scrollx + 56) << FIX_SHIFT;
	label_gecko.y = (-bg_thanks_scrolly + 60) << FIX_SHIFT;
	label_rik.x = (-bg_thanks_scrollx + 44) << FIX_SHIFT;
	label_rik.y = (-bg_thanks_scrolly + 80) << FIX_SHIFT;
	
	label_update(&label_gecko);
	label_update(&label_rik);
}

const scene_t end_scene = {
	.show = show,
	.hide = hide,
	.update = update,
};
