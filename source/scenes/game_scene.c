#include "common.h"
#include "assets/SprShared1.h"
#include "assets/SprPlayer.h"
#include "fonts/Acknowledge.h"
#include "fonts/Volter.h"
#include "fonts/GelatinMono.h"
#include "assets/levels.h"

static level_t *level = &Level1;

static void show(void) {
	scrollx = 0;
	scrolly = 0;
	// note: sbb = where to store the map entries
	//       cbb = where to store the tile graphics
	
	dma3_cpy(pal_bg_bank[0], level->pal, level->palLen);
	dma3_cpy(&tile_mem[0], level->tiles, level->tilesLen);
	
	int sb_size = sizeof(SCREENBLOCK);
	int sb_size_short = sb_size / 2;
	dma3_cpy(&se_mem[28], level->map, level->mapLen);
	// dma3_cpy(&se_mem[31], Lv1Map,                   sb_size);
	// dma3_cpy(&se_mem[30], Lv1Map + sb_size_short*1, sb_size);
	// dma3_cpy(&se_mem[29], Lv1Map + sb_size_short*2, sb_size);
	// dma3_cpy(&se_mem[28], Lv1Map + sb_size_short*3, sb_size);
	
	pal_bg_mem[0] = RGB15(23,27,30);
	
	REG_BG0CNT = BG_PRIO(2) | BG_8BPP | BG_SBB(28) | BG_CBB(0) | BG_REG_64x64; // main map
	// REG_BG1CNT = BG_PRIO(2) | BG_8BPP | BG_SBB(30) | BG_CBB(0); // static bg
	// REG_BG2CNT = BG_PRIO(2) | BG_8BPP | BG_SBB(29) | BG_CBB(0); // towers
	// REG_BG3CNT = BG_PRIO(2) | BG_8BPP | BG_SBB(28) | BG_CBB(0); // clouds
	
	// set up sprite palettes
	dma3_cpy(&pal_obj_bank[0], SprPlayerPal, SprPlayerPalLen);
	// dma3_cpy(&pal_obj_bank[1], SprShared1Pal, SprShared1PalLen);
	
	// text palette
	pal_obj_bank[8][0] = CLR_SKYBLUE;
	pal_obj_bank[8][1] = CLR_WHITE;
	pal_obj_bank[8][2] = CLR_BLACK;
	pal_obj_bank[8][3] = RGB15(16,16,16); // for greyed-out text
	
	// reserve sprite tiles in VRAM
	uint tid = 0;
	tid = player_init(tid);
	tid = label_init(&label_hello, &VolterFont, 1, 2, tid, 2);
	tid = zombies_init(tid);
	
	zombie_spawn(90<<FIX_SHIFT, 40<<FIX_SHIFT);
	zombie_spawn(40<<FIX_SHIFT, 80<<FIX_SHIFT);
	
	label_hello.x = 30 << FIX_SHIFT;
	label_hello.y = 100 << FIX_SHIFT;
	label_begin_write(&label_hello);
	tte_write("hello world!!!!");
	
	// mmStart(MOD_SPACEDOG_OPTIMISED, MM_PLAY_LOOP);
	// mmStart(MOD_SPACECAT, MM_PLAY_LOOP);
}

static void hide(void) {
	
}


static void update(void) {
	player_update();
	// zombies_update();
	// label_update_all();
	
	REG_BG0HOFS = scrollx;
	REG_BG0VOFS = scrolly;
	REG_BG1HOFS = scrollx;
	REG_BG1VOFS = scrolly;
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