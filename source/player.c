#include "common.h"
#include "assets/SprPlayer1.h"

static const int AnimIdle[] = {0,1,2,3,4,5};
static const int AnimJump[] = {6,7,8,9};
static const int AnimWalk[] = {10,11,12,13,14,15};

#define KEY_JUMP (KEY_A)
#define KEY_SHOOT (KEY_B)

static const int SPEED = (int)(2.0f * (FIX_SCALE));
static const int GRAVITY = (int)(0.2f * (FIX_SCALE));

entity_t player;

uint player_init(uint tid) {
	player = (entity_t) {
		.tid = tid,
		.flags = ACTIVE,
		.x = 20 << FIX_SHIFT,
		.y = 20 << FIX_SHIFT,
		.w = 8,
		.h = 9,
	};
	SetAnim(&player, AnimIdle);
	return tid+4;
}

void player_update(void) {
	
	if (!is_active(&player)) return;
	
	if (global_tick % 6 == 0) {
		entity_animate(&player);
	}
	
	player.velx = 0;
	
	if (map_collide_at(&player, 0, FIX_ONE + player.vely)) {
		if (player.anim == AnimJump) {
			SetAnim(&player, AnimIdle);
		}
	} else {
		SetAnim(&player, AnimJump);
		player.vely += GRAVITY;
	}
	
	if (key_is_down(KEY_LEFT) && !map_collide_at(&player, -SPEED, 0)) {
		player.velx = -SPEED;
		set_hflip(&player, true);
		SetAnim(&player, AnimWalk);
	}
	if (key_is_down(KEY_RIGHT) && !map_collide_at(&player, SPEED, 0)) {
		player.velx = SPEED;
		set_hflip(&player, false);
		SetAnim(&player, AnimWalk);
	}
	if (key_is_up(KEY_LEFT) && key_is_up(KEY_RIGHT)) {
		player.velx = 0;
		SetAnim(&player, AnimIdle);
	}
	
	if (key_is_down(KEY_DOWN) && !map_collide_at(&player, 0, SPEED)) {
		player.vely = 3<<FIX_SHIFT;
	}
	if (key_is_down(KEY_UP) && !map_collide_at(&player, 0, -SPEED)) {
		player.vely = -3<<FIX_SHIFT;
	}
	
	bool hit_x = entity_move_x(&player, player.velx);
	bool hit_y = entity_move_y(&player, player.vely);
	
	if (hit_y) {
		player.vely = 0;
		player.y = fxceil(player.y);
		if (map_collide(&player)) {
			player.y -= FIX_ONE;
		}
	}
	
	int px = player.x >> FIX_SHIFT;
	int py = player.y >> FIX_SHIFT;
	
	obj_set_attr(&obj_mem[reserve_obj()],
		((py - scrolly - 5) & ATTR0_Y_MASK) | ATTR0_SQUARE,
		((px - scrollx - 4) & ATTR1_X_MASK) | (player.flags & HVFLIP) | ATTR1_SIZE_16,
		player.tid | ATTR2_PRIO(1) | ATTR2_PALBANK(0));
	
	dma3_cpy(
		&tile_mem_obj[0][player.tid],
		&SprPlayer1Tiles[player.anim[player.frame]*sizeof(TILE)*4],
		sizeof(TILE)*4);
		
}
