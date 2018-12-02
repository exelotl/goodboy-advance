#include "common.h"
#include "assets/SprShield.h"

static const anim_t AnimShield = { .speed = 2, .loop = 1, .len = 8, .frames = (int[]){0,1,2,3,4,5,6,7} };

entity_t shield;

uint shield_init(uint tid) {
	shield = (entity_t) {
		.tid = tid,
		.flags = ACTIVE,
		.x = 20 << FIX_SHIFT,
		.y = 20 << FIX_SHIFT,
		.w = 32,
		.h = 32,
		.anim = &AnimShield
	};
	return tid + 16; // single frame
}

void shield_update(void) {
	
	if (!is_active(&shield)) return;
	
	entity_animate(&shield);
	
	set_center(&shield, get_center(&player));
	
	int x = shield.x >> FIX_SHIFT;
	int y = shield.y >> FIX_SHIFT;
	
	if (global_tick & 1) {
		return;
	}
	
	int aff = reserve_aff();
	int scale = Fix(1.025) + lu_sin(global_tick * 1024) / 128;
	obj_aff_scale(obj_aff_mem+aff, scale, scale);
	
	x -= 16;
	y -= 16;
	
	obj_set_attr(&obj_mem[reserve_obj()],
		((y - scrolly - 2) & ATTR0_Y_MASK) | ATTR0_SQUARE | ATTR0_AFF_DBL,
		((x - scrollx) & ATTR1_X_MASK) | ATTR1_SIZE_32 | ATTR1_AFF_ID(aff),
		shield.tid | ATTR2_PRIO(0) | ATTR2_PALBANK(2));
	
	dma3_cpy(
		&tile_mem_obj[0][shield.tid],
		&SprShieldTiles[shield.anim->frames[shield.frame] * sizeof(TILE) * 16],
		sizeof(TILE) * 16);
		
	// REG_BLDCNT = BLD_BUILD(BLD_OBJ, BLD_OBJ, BLD_STD);
	// REG_BLDALPHA = BLDA_BUILD(0x40 / 8, 0x40 / 8);
	// REG_BLDY = BLDY_BUILD(0);
}
