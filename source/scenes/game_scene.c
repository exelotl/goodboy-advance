#include "common.h"
#include "assets/SprShared1.h"
#include "assets/SprPlayer.h"
#include "assets/SprMuzzle.h"
#include "assets/SprShield.h"
#include "assets/SprBullet.h"
#include "assets/BgTalkie.h"
#include "fonts/Acknowledge.h"
#include "fonts/Volter.h"
#include "fonts/GelatinMono.h"
#include "assets/levels.h"

static const level_t *level = &Level1;

static void show(void) {
				
	scrollx = 0;
	scrolly = 0;
	
	// cb = where to store the tile gfx
	// sb = where to store the map entries
	
	int sb_size = sizeof(SCREENBLOCK);
	int sb_size_short = sb_size / 2;
	
	const int level_palbank = 0;
	const int level_cb = 0;      // tile gfx
	const int level_sb = 24;     // entries
	
	const int talkie_palbank = 2;
	const int talkie_cb = 1;      // tile gfx
	const int talkie_sb = 23;     // entries
	
	// LEVEL
	dma3_cpy(pal_bg_bank[level_palbank], level->pal, level->palLen);
	dma3_cpy(&tile_mem[level_cb], level->tiles, level->tilesLen);
	dma3_cpy(&se_mem[level_sb], level->map, level->mapLen);
	
	// TALKIE
	// dma3_cpy(pal_bg_bank[talkie_palbank], BgTalkiePal, BgTalkiePalLen);
	// dma3_cpy(&tile_mem[talkie_cb], BgTalkieTiles, BgTalkieTilesLen);
	// dma3_cpy(&se_mem[talkie_sb], BgTalkieMap, BgTalkieMapLen);
	// for (int i=0; i<sb_size_short; i++) {
	// 	se_mem[talkie_sb][i] |= SE_PALBANK(talkie_palbank);
	// }
	
	pal_bg_mem[0] = RGB15(23,27,30);
	
	REG_DISPCNT = DCNT_MODE1 // 2 regular, 1 affine
				// | DCNT_BG0
				// | DCNT_BG1
				| DCNT_BG2
				// | DCNT_BG3
				| DCNT_OBJ     // enable sprites
				| DCNT_OBJ_1D; // 1D tile mapping for sprites
	
	// REG_BG0CNT = BG_PRIO(2) | BG_8BPP | BG_SBB(28) | BG_CBB(0) | BG_REG_64x64; // main map
	// REG_BG1CNT = BG_PRIO(2) | BG_8BPP | BG_SBB(30) | BG_CBB(0);
	// REG_BG2CNT = BG_PRIO(2) | BG_8BPP | BG_SBB(29) | BG_CBB(0);
	REG_BG2CNT = BG_PRIO(2) | BG_SBB(level_sb) | BG_8BPP | BG_CBB(level_cb) | BG_AFF_128x128;
	// REG_BG3CNT = BG_PRIO(0) | BG_4BPP | BG_SBB(27) | BG_CBB(1); // talkie
	
	REG_BG_AFFINE[2] = bg_aff_default;
	
	// set up sprite palettes
	dma3_cpy(&pal_obj_bank[0], SprPlayerPal, SprPlayerPalLen);
	dma3_cpy(&pal_obj_bank[1], SprMuzzlePal, SprMuzzlePalLen);
	dma3_cpy(&pal_obj_bank[2], SprShieldPal, SprShieldPalLen);
	dma3_cpy(&pal_obj_bank[3], SprBulletPal, SprBulletPalLen);
	// dma3_cpy(&pal_obj_bank[1], SprShared1Pal, SprShared1PalLen);
	
	// text palette
	pal_obj_bank[8][0] = CLR_SKYBLUE;
	pal_obj_bank[8][1] = CLR_WHITE;
	pal_obj_bank[8][2] = CLR_BLACK;
	pal_obj_bank[8][3] = RGB15(16,16,16); // for greyed-out text
	
	// reserve sprite tiles in VRAM
	uint tid = 0;
	tid = player_init(tid);
	// tid = label_init(&label_dialog, &VolterFont, 1, 2, tid, 3);
	tid = zombies_init(tid);
	tid = muzzles_init(tid);
	tid = shield_init(tid);
	tid = bullets_init(tid);
	
	// zombie_spawn(90<<FIX_SHIFT, 40<<FIX_SHIFT);
	// zombie_spawn(40<<FIX_SHIFT, 80<<FIX_SHIFT);
	
	dialog_init();
	
	// label_dialog.x = 30 << FIX_SHIFT;
	// label_dialog.y = 100 << FIX_SHIFT;
	// label_begin_write(&label_dialog);
	// tte_write("Darn... looks like my ship\nhas crashed.");
	
	// mmStart(MOD_SPACEDOG_OPTIMISED, MM_PLAY_LOOP);
	// mmStart(MOD_SPACECAT, MM_PLAY_LOOP);
}

static void hide(void) {
	
}


static void update(void) {
	player_update();
	// zombies_update();
	bullets_update();
	muzzles_update();
	// shield_update();  // player is responsible for updating shield
	// label_update_all();
	
	dialog_update();
	
	REG_BG2X = scrollx << 8;
	REG_BG2Y = scrolly << 8;
	// REG_BG0HOFS = scrollx;
	// REG_BG0VOFS = scrolly;
	// REG_BG1HOFS = scrollx;
	// REG_BG1VOFS = scrolly;
}

const scene_t game_scene = {
	.show = show,
	.hide = hide,
	.update = update,
};


bool map_collide_rect(int x, int y, int w, int h) {
	int startx = x / CELL_SIZE;
	int starty = y / CELL_SIZE;
	int endx = (x+w-1) / CELL_SIZE;
	int endy = (y+h-1) / CELL_SIZE;
	
	// safety check
	if (startx < 0 || starty < 0 || endx >= LEVEL_WIDTH_CELLS || endy >= LEVEL_HEIGHT_CELLS) return 1;
	
	for (int i=startx; i<=endx; i++) {
		for (int j=starty; j<=endy; j++) {
			int t = level->cells[i + j*LEVEL_WIDTH_CELLS];
			if (t == CELL_SOLID) return 1;
		}
	}
	return 0;
}