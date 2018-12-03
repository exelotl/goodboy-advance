#include "common.h"
#include "assets/SprPlayer.h"

static const anim_t AnimWalkAll =          { .speed = 5, .loop = 1, .len = 8, .frames = (int[]){0,1,2,3,4,5,6,7} };
static const anim_t AnimWalkNoJet =        { .speed = 5, .loop = 1, .len = 8, .frames = (int[]){16,17,18,19,20,21,22,23} };
static const anim_t AnimWalkNoGun =        { .speed = 5, .loop = 1, .len = 8, .frames = (int[]){8,9,10,11,12,13,14,15} };
static const anim_t AnimIdleAll =          { .speed = 7, .loop = 1, .len = 4, .frames = (int[]){24,25,26,27} };
static const anim_t AnimIdleNoJet =        { .speed = 7, .loop = 1, .len = 4, .frames = (int[]){28,29,30,31} };
static const anim_t AnimIdleNoGun =        { .speed = 7, .loop = 1, .len = 4, .frames = (int[]){32,33,34,35} };
static const anim_t AnimJumpUpAllStart =   { .speed = 8, .loop = 0, .len = 2, .frames = (int[]){36,37} };
static const anim_t AnimJumpUpAllLoop =    { .speed = 8, .loop = 1, .len = 2, .frames = (int[]){38,39} };
static const anim_t AnimJumpUpNoJetStart = { .speed = 8, .loop = 0, .len = 2, .frames = (int[]){40,41} };
static const anim_t AnimJumpUpNoJetLoop =  { .speed = 8, .loop = 1, .len = 2, .frames = (int[]){42,43} };
static const anim_t AnimJumpUpNoGunStart = { .speed = 8, .loop = 0, .len = 2, .frames = (int[]){44,45} };
static const anim_t AnimJumpUpNoGunLoop =  { .speed = 8, .loop = 1, .len = 2, .frames = (int[]){46,47} };
static const anim_t AnimFallAll =          { .speed = 8, .loop = 1, .len = 2, .frames = (int[]){48,49} };
static const anim_t AnimFallNoJet =        { .speed = 8, .loop = 1, .len = 2, .frames = (int[]){50,51} };
static const anim_t AnimFallNoGun =        { .speed = 8, .loop = 1, .len = 2, .frames = (int[]){52,53} };
static const anim_t AnimDead =             { .speed = 8, .loop = 1, .len = 1, .frames = (int[]){54} };

#define STATE_ALL 0
#define STATE_NOJET 1
#define STATE_NOGUN 2
#define STATE_NOSHIELD 3 
#define NUM_STATES 4

#define IDLE 0
#define WALK 1
#define JUMP_UP_START 2
#define JUMP_UP_LOOP 3
#define FALL 4
#define DEAD 5
#define NUM_ANIMS 6

static const anim_t *Anims[NUM_STATES][NUM_ANIMS] = {
	{
		&AnimIdleAll,
		&AnimWalkAll,
		&AnimJumpUpAllStart,
		&AnimJumpUpAllLoop,
		&AnimFallAll,
		&AnimDead,
	}, {
		&AnimIdleNoJet,
		&AnimWalkNoJet,
		&AnimJumpUpNoJetStart,
		&AnimJumpUpNoJetLoop,
		&AnimFallNoJet,
		&AnimDead,
	}, {
		&AnimIdleNoGun,
		&AnimWalkNoGun,
		&AnimJumpUpNoGunStart,
		&AnimJumpUpNoGunLoop,
		&AnimFallNoGun,
		&AnimDead,
	}, {
		&AnimIdleAll,
		&AnimWalkAll,
		&AnimJumpUpAllStart,
		&AnimJumpUpAllLoop,
		&AnimFallAll,
		&AnimDead,
	},
};

static void player_set_anim(int anim_index) {
	player.player_anim = anim_index;
	set_anim(&player, Anims[player.player_state][anim_index]);
}

#define KEY_JUMP (KEY_A)
#define KEY_SHOOT (KEY_B)

// static const int SPEED = (int)(2.0f * (FIX_SCALE));
static const int SPEED = (int)(4.0f * (FIX_SCALE));
static const int GRAVITY = (int)(0.2f * (FIX_SCALE));
static const int JUMP_SPEED = (int)(4.4f * (FIX_SCALE));
static const int HOP_SPEED = (int)(1.75f * (FIX_SCALE));

int prev_state;

uint player_init(uint tid) {
	player = (entity_t) {
		.tid = tid,
		.flags = ACTIVE,
		.x = 20 << FIX_SHIFT,
		.y = 20 << FIX_SHIFT,
		.w = 20,
		.h = 27, 
		.player_state = STATE_ALL,
		.player_anim = IDLE,
	};
	player_set_anim(IDLE);
	prev_state = player.player_state;
	return tid + 4*4; // single frame
}

static int respawn_timer = 0;

static vec2 death_pos;  // for camera to still shake when dying

void player_update(void) {
	int start_x = player.x;
	int start_y = player.y;
	vec2 cam_pos;	
	
	if (!is_active(&player)) return;
	
	// account for random state changes by altars.
	if (prev_state != player.player_state) {
		player_set_anim(player.player_anim);
		prev_state = player.player_state;
	}
	
	entity_animate(&player);
	
	if (player.player_anim == DEAD) {
		
		// death animation
		
		if ((player.y >> FIX_SHIFT)-scrolly > 170) {
			if (respawn_timer-- < 0) {
				// go to last altar
				player.x = spawn_x;
				player.y = spawn_y;
				player_set_anim(IDLE);
				set_hflip(&player, false);
				// player.player_state = spawn_state;
			}
		} else {
			player.vely += Fix(0.2);
			player.y += player.vely;
		}
		
		cam_pos = death_pos;
		
	} else {
		
		player.velx = 0;
		
		int collide_down = map_collide_at(&player, 0, FIX_ONE + player.vely);
		int collide_left = map_collide_at(&player, -SPEED, 0);
		int collide_right = map_collide_at(&player, SPEED, 0);
		
		// bool on_oneway = player.vely > 0 && (collision_down & CELL_ONEWAY);
		// bool on_floor = (collide_down & CELL_SOLID);
		bool on_floor = did_hit_y(&player, FIX_ONE + player.vely) && player.velx >= 0;
		
		if (on_floor) {
			player.flags |= CAN_JET;
		} else {
			player.vely += GRAVITY;
		}
		
		if (key_is_down(KEY_LEFT) && !(collide_left & (CELL_SPIKE | CELL_SOLID))) {
			player.velx = -SPEED;
			set_hflip(&player, true);
		}
		if (key_is_down(KEY_RIGHT) && !(collide_right & (CELL_SPIKE | CELL_SOLID))) {
			player.velx = SPEED;
			set_hflip(&player, false);
		}
		if (key_is_up(KEY_LEFT) && key_is_up(KEY_RIGHT)) {
			player.velx = 0;
		}
		
		vec2 center = get_center(&player);
		
		if (player.player_state != STATE_NOGUN) {
			if (key_hit(KEY_SHOOT)) {
				if (player.flags & HFLIP) {
					muzzle_spawn(center.x + -30*FIX_SCALE, center.y + 4*FIX_SCALE, 0, HFLIP);
					bullet_spawn(center.x + -12*FIX_SCALE, center.y + 2*FIX_SCALE, -1);
				} else {
					muzzle_spawn(center.x + 30*FIX_SCALE, center.y + 4*FIX_SCALE, 0, 0);
					bullet_spawn(center.x + 12*FIX_SCALE, center.y + 2*FIX_SCALE, 1);
				}
			}
		}
		
		if (player.player_state != STATE_NOJET) {
			if (key_hit(KEY_JUMP) && (player.flags & CAN_JET)) {
				player.vely = -JUMP_SPEED;
				player.flags &= ~CAN_JET;
				if (player.flags & HFLIP) {
					muzzle_spawn(center.x + 8*FIX_SCALE, center.y + 12*FIX_SCALE, ATTR0_AFF, ATTR1_AFF_ID(aff_rotate_270));
				} else {
					muzzle_spawn(center.x + -8*FIX_SCALE, center.y + 12*FIX_SCALE, ATTR0_AFF, ATTR1_AFF_ID(aff_rotate_270));
				}
			}
			if (key_released(KEY_JUMP) && player.vely < 0) {
				player.vely /= 2;
			}
		} else {
			if (key_hit(KEY_JUMP) && map_collide_at(&player, 0, FIX_ONE)) {
				player.vely = -HOP_SPEED;
			}
		}
		if (player.player_state != STATE_NOSHIELD) {
			shield_update();
		}
		
		int hit_side = collide_right | collide_left;
		int hit_any = hit_side | collide_down;
		
		int hit_spike = player.player_state == STATE_NOSHIELD
		                && (
							(hit_side & CELL_SPIKE)
							|| ((collide_down & CELL_SPIKE) && !(collide_down & CELL_SOLID))
						);
		
		if (hit_spike) {
			player_set_anim(DEAD);
			player.vely = -Fix(4);
			shake_timer = 20;
			death_pos = get_center(&player);
			cam_pos = death_pos;
			respawn_timer = 50;
		} else {
			if (on_floor) {
				if (player.velx == 0) {
					player_set_anim(IDLE);
				} else {
					player_set_anim(WALK);
				}
			} else {
				if (player.vely > 0) {
					player_set_anim(FALL);
				} else {
					switch (player.player_anim) {
						case IDLE:
						case WALK:
							player_set_anim(JUMP_UP_START);
							break;
						case JUMP_UP_START:
							if (anim_finished(&player)) {
								player_set_anim(JUMP_UP_LOOP);
							}
							break;
					}	
				}
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
			
			if (player.x > Fix(LEVEL_WIDTH_PX)) {
				player.x -= Fix(LEVEL_WIDTH_PX);
			} else if (player.x < 0) {
				player.x += Fix(LEVEL_WIDTH_PX);
			}
			if (player.y > Fix(LEVEL_HEIGHT_PX)) {
				player.y -= Fix(LEVEL_HEIGHT_PX);
			} else if (player.y < 0) {
				player.y += Fix(LEVEL_HEIGHT_PX);
			}
			
			cam_pos = get_center(&player);
		}
	}
	
	if (shake_timer > 0) {
		shake_timer--;
		cam_pos.x += qran_range(-shake_timer*44, shake_timer*44);
		cam_pos.y += qran_range(-shake_timer*44, shake_timer*44);
	}
	
	scrollx = (cam_pos.x>>FIX_SHIFT) - (240/2);
	scrolly = (cam_pos.y>>FIX_SHIFT) - (160/2 + 2);
	
	parallax_x += CLAMP(player.x - start_x, -FIX_ONE, FIX_ONE) / 8;
	parallax_y += CLAMP(player.y - start_y, -FIX_ONE, FIX_ONE) / 8;
	
	int px = player.x >> FIX_SHIFT;
	int py = player.y >> FIX_SHIFT;
	
	obj_set_attr(&obj_mem[reserve_obj()],
		((py - scrolly - 5) & ATTR0_Y_MASK) | ATTR0_SQUARE,
		((px - scrollx - 6) & ATTR1_X_MASK) | (player.flags & HVFLIP) | ATTR1_SIZE_32,
		player.tid | ATTR2_PRIO(1) | ATTR2_PALBANK(0));
	
	dma3_cpy(
		&tile_mem_obj[0][player.tid],
		&SprPlayerTiles[player.anim->frames[player.frame] * sizeof(TILE) * 16],
		sizeof(TILE) * 16);
		
}
