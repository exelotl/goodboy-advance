#include "common.h"
#include "assets/SprMuzzle.h"

static const anim_t AnimMuzzle = { .speed = 2, .loop = 0, .len = 8, .frames = (int[]){0,1,2,3,4,5,6,7} };

entity_t muzzles[MUZZLE_COUNT];
static uint muzzle_tid;

uint muzzles_init(uint tid) {
	dma3_cpy(&tile_mem_obj[0][muzzle_tid], SprMuzzleTiles, SprMuzzleTilesLen);
	
	for (int i=0; i<MUZZLE_COUNT; i++)
		muzzles[i] = (entity_t){};
		
	return tid + 8 * 16;  // 8 frames
}

entity_t *muzzle_spawn(int x, int y, int aff, int attr1) {
	entity_t *e = NULL;
	for (int i = 0; i < MUZZLE_COUNT; i++) {
		if (!is_active(&muzzles[i])) {
			e = &muzzles[i];
			break;
		}
	}
	if (e) {
		*e = (entity_t) {
			.flags = ACTIVE,
			.x = x,
			.y = y,
			.muzzle_aff = aff,
			.muzzle_attr1 = attr1,
		};
		set_anim(e, &AnimMuzzle);
	}
	return e;
}

void muzzles_update(void) {
	
	for (int i = 0; i < MUZZLE_COUNT; i++) {
		entity_t *e = &muzzles[i];
		if (!e->flags & ACTIVE) continue;
		
		entity_animate(e);
		
		if (anim_finished(e)) {
			entity_deactivate(e);
		}
		
		int x = e->x >> FIX_SHIFT;
		int y = e->y >> FIX_SHIFT;
		
		// int attr1_aff = 0;
		// if (e->muzzle_aff != -1) {
		// 	attr1_aff = ATTR1_AFF_ID(e->muzzle_aff);
		// } else {
		// 	attr1_aff = (e->flags & (HVFLIP));
		// }
		
		obj_set_attr(&obj_mem[reserve_obj()],
			((y - scrolly - 16) & ATTR0_Y_MASK) | e->muzzle_aff | ATTR0_SQUARE,
			((x - scrollx - 16) & ATTR1_X_MASK) | e->muzzle_attr1 | ATTR1_SIZE_32,
			(muzzle_tid + e->anim->frames[e->frame]*16) | ATTR2_PRIO(1) | ATTR2_PALBANK(1));
	}
		
}