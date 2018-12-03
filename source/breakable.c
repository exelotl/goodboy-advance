#include "common.h"
#include "assets/SprBreakable.h"

static const anim_t AnimAppear = { .speed = 5, .loop = 0, .len = 7, .frames = (int[]){0,1,2,3,4,5,6} };
static const anim_t AnimIdle = { .speed = 3, .loop = 1, .len = 7, .frames = (int[]){7,8,9,10,11,12,13} };
static const anim_t AnimDie = { .speed = 4, .loop = 0, .len = 6, .frames = (int[]){14,15,16,17,18,19} };

static uint breakable_tid;

uint breakables_init(uint tid) {
	breakable_tid = tid;
	dma3_cpy(&tile_mem_obj[0][breakable_tid], SprBreakableTiles, SprBreakableTilesLen);
	
	for (int i=0; i<BREAKABLE_COUNT; i++) {
		breakables[i] = (entity_t){ .w = 32, .h = 32 };
	}
		
	return tid + 20 * 16;  // 20 frames
}


static void brk_set_cells(int x, int y, int val) {
	int cell_x = (x>>FIX_SHIFT) / CELL_SIZE;
	int cell_y = (y>>FIX_SHIFT) / CELL_SIZE;
	map_set_cell(cell_x, cell_y, val);
	map_set_cell(cell_x+1, cell_y, val);
	map_set_cell(cell_x, cell_y+1, val);
	map_set_cell(cell_x+1, cell_y+1, val);
}


entity_t *breakable_spawn(int x, int y) {
	entity_t *e = NULL;
	for (int i = 0; i < BREAKABLE_COUNT; i++) {
		if (!is_active(&breakables[i])) {
			e = &breakables[i];
			break;
		}
	}
	if (e) {
		e->flags = ACTIVE;
		e->x = x;
		e->y = y;
		set_anim(e, &AnimIdle);
		brk_set_cells(x, y, CELL_SOLID);
	}
	return e;
}

void breakables_check_bullet(entity_t *bullet) {
	for (int i = 0; i < BREAKABLE_COUNT; i++) {
		entity_t *e = &breakables[i];
		if ((e->flags & ACTIVE) && (e->anim == &AnimIdle) && entity_collide(bullet, e)) {
			set_anim(e, &AnimDie);
			brk_set_cells(e->x, e->y, CELL_EMPTY);
			e->brk_timer = 0;
			e->frame = 0;
			shake_timer = 20;
		}
	}
}

#define RESPAWN_TIME (5*60)

void breakables_update(void) {
	
	for (int i = 0; i < BREAKABLE_COUNT; i++) {
		entity_t *e = &breakables[i];
		if (!e->flags & ACTIVE) continue;
		
		int px = (e->x >> FIX_SHIFT) - scrollx;
		int py = (e->y >> FIX_SHIFT) - scrolly;
		
		if (e->anim == &AnimIdle) {
			
		} else if (e->anim == &AnimAppear) {
			if (anim_finished(e)) {
				set_anim(e, &AnimIdle);
				e->frame = 0;
			}
		} else if (e->anim == &AnimDie) {
			e->brk_timer = MIN(e->brk_timer+1, RESPAWN_TIME);
			if (e->brk_timer == RESPAWN_TIME && !entity_collide(e, &player)) {
				set_anim(e, &AnimAppear);
				e->frame = 0;
				brk_set_cells(e->x, e->y, CELL_SOLID);
			}
			else if (anim_finished(e) && e->brk_timer > 2) {
				continue; // don't draw at all while respawning
			}
		}
		
		if (px+32 < 0 || py+32 < 0 || px > 240 || py > 160) {
			continue;
		}
		
		entity_animate(e);
		
		obj_set_attr(&obj_mem[reserve_obj()],
			(py & ATTR0_Y_MASK) | ATTR0_SQUARE,
			(px & ATTR1_X_MASK) | ATTR1_SIZE_32,
			(breakable_tid + e->anim->frames[e->frame]*16) | ATTR2_PRIO(1) | ATTR2_PALBANK(4));
	}
		
}