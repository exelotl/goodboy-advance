#include "common.h"
#include "assets/SprShared1.h"
#include "assets/SprPlayer.h"
#include "assets/SprMuzzle.h"
#include "assets/SprShield.h"
#include "assets/SprBullet.h"
#include "assets/SprBreakable.h"
#include "assets/SprBarrier.h"
#include "assets/SprSacrificedItems.h"
#include "assets/SprGem.h"
#include "assets/BgTalkie.h"
#include "assets/BgPlanet.h"
#include "fonts/Acknowledge.h"
#include "fonts/Volter.h"
#include "fonts/GelatinMono.h"
#include "assets/levels.h"

const level_t *level = &Level1;

static int cells[LEVEL_CELL_COUNT];

// cb = where to store the tile gfx
// sb = where to store the map entries

static const int sb_size = sizeof(SCREENBLOCK);
static const int sb_size_short = sizeof(SCREENBLOCK) / 2;

static const int level_palbank = 0;
static const int level_cb = 0;      // tile gfx
static const int level_sb = 24;     // entries

// TODO figure out why talkie aint talkin
static const int talkie_palbank = 2;
static const int talkie_cb = 2;      // tile gfx
static const int talkie_sb = 20;     // entries

static const int planet_palbank = 3;
static const int planet_cb = 2;      // tile gfx
static const int planet_sb = 22;     // entries


static void say_initial_dialog(void);

static void show(void) {
				
	scrollx = 0;
	scrolly = 0;
	parallax_x = 0;
	parallax_y = 0;
	
	for (int i = 0; i < LEVEL_CELL_COUNT; i++) {
		cells[i] = level->cells[i];
	}
	
	// LEVEL
	dma3_cpy(pal_bg_target_bank[level_palbank], level->pal, level->palLen);
	dma3_cpy(&tile_mem[level_cb], level->tiles, level->tilesLen);
	dma3_cpy(&se_mem[level_sb], level->map, level->mapLen);
	
	// TALKIE
	dma3_cpy(pal_bg_target_bank[talkie_palbank], BgTalkiePal, BgTalkiePalLen);
	dma3_cpy(&tile_mem[talkie_cb], BgTalkieTiles, BgTalkieTilesLen);
	dma3_cpy(&se_mem[talkie_sb], BgTalkieMap, BgTalkieMapLen);
	for (int i=0; i<sb_size_short; i++) {
		se_mem[talkie_sb][i] |= SE_PALBANK(talkie_palbank);
	}
	
	// PLANET
	dma3_cpy(pal_bg_target_bank[planet_palbank], BgPlanetPal, BgPlanetPalLen);
	dma3_cpy(&tile_mem[planet_cb], BgPlanetTiles, BgPlanetTilesLen);
	dma3_cpy(&se_mem[planet_sb], BgPlanetMap, BgPlanetMapLen);
	for (int i=0; i<sb_size_short; i++) {
		se_mem[planet_sb][i] |= SE_PALBANK(planet_palbank);
	}
	
	pal_bg_target_mem[0] = RGB15(23,27,30);
	
	REG_DISPCNT = DCNT_MODE0
				| DCNT_BG0
				| DCNT_BG1
				// | DCNT_BG2
				| DCNT_BG3
				| DCNT_OBJ     // enable sprites
				| DCNT_OBJ_1D; // 1D tile mapping for sprites
	
	REG_BG0CNT = BG_PRIO(2) | BG_8BPP | BG_SBB(level_sb) | BG_CBB(level_cb) | BG_REG_64x64; // main map
	REG_BG1CNT = BG_PRIO(3) | BG_4BPP | BG_SBB(planet_sb) | BG_CBB(planet_cb) | BG_REG_32x32;
	// REG_BG2CNT = BG_PRIO(3) | BG_4BPP | BG_SBB(planet_sb+1) | BG_CBB(planet_cb) | BG_REG_32x32;
	// REG_BG2CNT = BG_PRIO(2) | BG_8BPP | BG_SBB(29) | BG_CBB(0);
	// REG_BG2CNT = BG_PRIO(2) | BG_SBB(level_sb) | BG_8BPP | BG_CBB(level_cb) | BG_AFF_128x128;
	REG_BG3CNT = BG_PRIO(0) | BG_4BPP | BG_SBB(talkie_sb) | BG_CBB(talkie_cb); // talkie
	
	// REG_BG_AFFINE[2] = bg_aff_default;
	
	// set up sprite palettes
	dma3_cpy(&pal_obj_target_bank[0], SprPlayerPal, SprPlayerPalLen);
	dma3_cpy(&pal_obj_target_bank[1], SprMuzzlePal, SprMuzzlePalLen);
	dma3_cpy(&pal_obj_target_bank[2], SprShieldPal, SprShieldPalLen);
	dma3_cpy(&pal_obj_target_bank[3], SprBulletPal, SprBulletPalLen);
	dma3_cpy(&pal_obj_target_bank[4], SprBreakablePal, SprBreakablePalLen);
	dma3_cpy(&pal_obj_target_bank[5], SprBarrierPal, SprBarrierPalLen);
	dma3_cpy(&pal_obj_target_bank[6], SprSacrificedItemsPal, SprSacrificedItemsPalLen);
	dma3_cpy(&pal_obj_target_bank[7], SprGemPal, SprGemPalLen);
	// dma3_cpy(&pal_obj_target_bank[1], SprShared1Pal, SprShared1PalLen);
	
	// text palette
	pal_obj_target_bank[8][0] = CLR_SKYBLUE;
	pal_obj_target_bank[8][1] = CLR_WHITE;
	pal_obj_target_bank[8][2] = CLR_BLACK;
	pal_obj_target_bank[8][3] = RGB15(16,16,16); // for greyed-out text
	
	// reserve sprite tiles in VRAM
	uint tid = 0;
	tid = player_init(tid);
	tid = label_init(&label_dialog, &VolterFont, 1, 2, tid, 4);
	tid = muzzles_init(tid);
	tid = shield_init(tid);
	tid = bullets_init(tid);
	tid = breakables_init(tid);
	tid = altars_init(tid);
	tid = gems_init(tid);
	
	dialog_init();
	
	timeout_clear();
	timeout_set(30, say_initial_dialog);
	
	// label_begin_write(&label_dialog);
	// tte_write("Darn... looks like my ship has crashed.");
	
	// mmStart(MOD_SPACEDOG_OPTIMISED, MM_PLAY_LOOP);
	// mmStart(MOD_SPACECAT, MM_PLAY_LOOP);
	
	spawn_all(level);
	
	fader_init();  // copies all the palettes assuming they won't change from now on.
	fader_fade_in(1, NULL);
	// fader_fade_out(1, NULL);
	
	
	// initial screenshake and sfx
	shake_timer = 50;
}


static void say_initial_dialog(void) {
	if (level == &Level1) {
		dialog_say("My poor rocket.", 160, Fix(70));
		dialog_say_next("It needs some power gems.", 160, Fix(40));
	}
	if (level == &Level2) {
		dialog_say("Oh no! I crashed again!", 180, Fix(50));
		dialog_say_next("Need more gems.", 160, Fix(70));
	}
}

static void hide(void) {
	
}


static void update(void) {
	player_update();
	bullets_update();
	muzzles_update();
	breakables_update();
	altars_update();
	gems_update();
	
	//// shield_update();  // player is responsible for updating shield
	
	dialog_update();
	
	fader_update();
	
	uint ofs_x = (uint) scrollx;
	uint ofs_y = (uint) scrolly;
	
	int chunk_x0 = (ofs_x/CHUNK_WIDTH_PIXELS) % LEVEL_WIDTH_CHUNKS;
	int chunk_y0 = (ofs_y/CHUNK_HEIGHT_PIXELS) % LEVEL_HEIGHT_CHUNKS;
	int chunk_x1 = (ofs_x/CHUNK_WIDTH_PIXELS + 1) % LEVEL_WIDTH_CHUNKS;
	int chunk_y1 = (ofs_y/CHUNK_HEIGHT_PIXELS + 1) % LEVEL_HEIGHT_CHUNKS;
	
	short *src_sb_0 = level->map + sb_size_short * (chunk_x0 + chunk_y0*LEVEL_WIDTH_CHUNKS);
	short *src_sb_1 = level->map + sb_size_short * (chunk_x1 + chunk_y0*LEVEL_WIDTH_CHUNKS);
	short *src_sb_2 = level->map + sb_size_short * (chunk_x0 + chunk_y1*LEVEL_WIDTH_CHUNKS);
	short *src_sb_3 = level->map + sb_size_short * (chunk_x1 + chunk_y1*LEVEL_WIDTH_CHUNKS);
	
	dma3_cpy(&se_mem[level_sb], src_sb_0, sb_size);
	dma3_cpy(&se_mem[level_sb+1], src_sb_1, sb_size);
	dma3_cpy(&se_mem[level_sb+2], src_sb_2, sb_size);
	dma3_cpy(&se_mem[level_sb+3], src_sb_3, sb_size);
	
	REG_BG0HOFS = ofs_x % CHUNK_WIDTH_PIXELS;
	REG_BG0VOFS = ofs_y % CHUNK_HEIGHT_PIXELS;
	
	// REG_BG1HOFS = parallax_x >> FIX_SHIFT;
	// REG_BG1VOFS = parallax_y >> FIX_SHIFT;
	
	// REG_BG0HOFS = scrollx;
	// REG_BG0VOFS = scrolly;
}

const scene_t game_scene = {
	.show = show,
	.hide = hide,
	.update = update,
};


// returns a bitmask of all tile types collided with

int map_collide_rect(int x, int y, int w, int h) {
	int startx = x / CELL_SIZE;
	int starty = y / CELL_SIZE;
	int endx = (x+w-1) / CELL_SIZE;
	int endy = (y+h-1) / CELL_SIZE;
	
	// safety check
	// if (startx < 0 || starty < 0 || endx >= LEVEL_WIDTH_CELLS || endy >= LEVEL_HEIGHT_CELLS) return 1;
	
	int res = 0;
	
	for (int i=startx; i<=endx; i++) {
		uint i_wrap = (uint)(i<0 ? i-1 : i) % LEVEL_WIDTH_CELLS;
		for (int j=starty; j<=endy; j++) {
			uint j_wrap = (uint)(j<0 ? j-1 : j) % LEVEL_HEIGHT_CELLS;
			res |= cells[i_wrap + j_wrap*LEVEL_WIDTH_CELLS];
		}
	}
	return res;
}

void map_set_cell(int x, int y, int val) {
	uint x_wrap = (uint)(x<0 ? x-1 : x) % LEVEL_WIDTH_CELLS;
	uint y_wrap = (uint)(y<0 ? y-1 : y) % LEVEL_HEIGHT_CELLS;
	cells[x + y*LEVEL_WIDTH_CELLS] = val;
}