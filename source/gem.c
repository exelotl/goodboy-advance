#include "common.h"
#include "assets/SprGem.h"

static const anim_t AnimGem = { .speed = 5, .loop = 1, .len = 8, .frames = (int[]){0,1,2,3,4,5,6,7} };

static uint gem_tid;

uint gems_init(uint tid) {
	gem_tid = tid;
	dma3_cpy(&tile_mem_obj[0][gem_tid], SprGemTiles, SprGemTilesLen);
	
	for (int i=0; i<GEM_COUNT; i++) {
		gems[i] = (entity_t){ .w = 32, .h = 32 };
	}
		
	return tid + 8 * 16;  // 8 frames
}


entity_t *gem_spawn(int x, int y) {
	entity_t *e = NULL;
	for (int i = 0; i < GEM_COUNT; i++) {
		if (!is_active(&gems[i])) {
			e = &gems[i];
			break;
		}
	}
	if (e) {
		e->flags = ACTIVE;
		e->x = x;
		e->y = y;
		set_anim(e, &AnimGem);
	}
	return e;
}

void gems_update(void) {
	
	for (int i = 0; i < GEM_COUNT; i++) {
		entity_t *e = &gems[i];
		if (!e->flags & ACTIVE) continue;
		
		int px = (e->x >> FIX_SHIFT) - scrollx;
		int py = (e->y >> FIX_SHIFT) - scrolly;
		
		if (px+32 < 0 || py+32 < 0 || px > 240 || py > 160) {
			continue;
		}
		
		entity_animate(e);
		
		obj_set_attr(&obj_mem[reserve_obj()],
			(py & ATTR0_Y_MASK) | ATTR0_SQUARE,
			(px & ATTR1_X_MASK) | ATTR1_SIZE_32,
			(gem_tid + e->anim->frames[e->frame]*16) | ATTR2_PRIO(1) | ATTR2_PALBANK(7));
	}
		
}