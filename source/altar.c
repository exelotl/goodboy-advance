#include "common.h"

#include "assets/levels.h"
#include "assets/SprBarrier.h"
#include "assets/SprSacrificedItems.h"

// copy of player states
#define STATE_ALL 0
#define STATE_NOJET 1
#define STATE_NOGUN 2
#define STATE_NOSHIELD 3 
#define NUM_STATES 4

entity_t altar_gun, altar_shield, altar_jetpack;
entity_t barrier_gun, barrier_shield, barrier_jetpack;
entity_t icon;

static const anim_t AnimBarrierOpen = { .speed = 3, .loop = 1, .len = 1, .frames = (int[]){0} };
static const anim_t AnimBarrier = { .speed = 3, .loop = 0, .len = 10, .frames = (int[]){0,1,2,3,4,5,6,7,8,9} };

static const anim_t AnimSacrificeShield = { .speed = 3, .loop = 1, .len = 8, .frames = (int[]){0,1,2,3,4,5,6,7} };
static const anim_t AnimSacrificeJetpack = { .speed = 3, .loop = 1, .len = 8, .frames = (int[]){8,9,10,11,12,13,14,15} };
static const anim_t AnimSacrificeGun = { .speed = 3, .loop = 1, .len = 8, .frames = (int[]){16,17,18,19,20,21,22,23} };

static bool gun_taken;
static bool shield_taken;
static bool jetpack_taken;

// static int barrier_tid;

static int rocket_dialog_timer;

#define ROCKET_DIALOG_COOLDOWN (10 * 60)

uint altars_init(uint tid) {
	
	rocket = (entity_t) {
		.flags = ACTIVE,
		.w = 100,
		.h = 100,
	};
	
	rocket_dialog_timer = ROCKET_DIALOG_COOLDOWN;
	
	/// instawin cheat
	// rocket_dialog_timer = 0;
	// gems_deposited = 3;
	gems_deposited = 0;
	
	// collision triggers
	altar_gun = (entity_t) {
		.flags = ACTIVE,
		.w = 32,
		.h = 32,
	};
	altar_jetpack = altar_gun;
	altar_shield = altar_gun;
	
	// closing barriers
	barrier_gun = (entity_t) {
		.flags = ACTIVE,
		.w = 16,
		.h = 64,
		.anim = &AnimBarrierOpen,
		.door_closed = false,
	};
	barrier_jetpack = barrier_gun;
	barrier_shield = barrier_gun;
	
	icon = (entity_t) {
		.flags = 0,
	};
	
	barrier_gun.tid = tid;      tid += (4*8);
	barrier_jetpack.tid = tid;  tid += (4*8);
	barrier_shield.tid = tid;   tid += (4*8);
	// barrier_tid = tid;          tid += 10 * (4*8);
	icon.tid = tid;             tid += (4*4);
	
	// dma3_cpy(&tile_mem_obj[0][barrier_tid], SprBarrierTiles, SprBarrierTilesLen);
		
	return tid;
}


void rocket_spawn(int x, int y) {
	rocket.x = x;
	rocket.y = y;
}

static void level_complete_fadeout(void);
static void level_complete_wait(void);
static void level_complete_set_scene(void);

static void level_complete_fadeout(void) {
	fader_fade_out(1, level_complete_wait);
}
static void level_complete_wait(void) {
	timeout_set(10, level_complete_set_scene);
}
static void level_complete_set_scene(void) {
	slide_number = 2;
	if (level == &Level1) {
		scene_set(fake_end_scene);
	} else {
		scene_set(end_scene);
	}
}

extern void barrier_open(entity_t *barrier);

static void rocket_update() {
	
	if (rocket_dialog_timer > 0) {
		rocket_dialog_timer--;
	}
	
	if (entity_collide(&player, &rocket)) {
		
		// check for any undeposited gems and deposit them
		
		bool found_gem = false;
		
		for (int i = 0; i < GEM_COUNT; i++) {
			entity_t *gem = &gems[i];
			if (!gem->flags & ACTIVE) continue;
			if (gem->gem_state == GEM_TAKEN) {
				gem->x = player.x;
				gem->y = player.y;
				gem->gem_state = GEM_DEPOSITING;
				gems_deposited++;
				found_gem = true;
			}
		}
		
		
		if (player.player_state != STATE_ALL) {
			
			// reopen door if player missed gem
			if (!found_gem) {
				if (player.player_state == STATE_NOGUN) {
					barrier_open(&barrier_gun);
					altar_gun.flags |= ACTIVE;
				}
				if (player.player_state == STATE_NOJET) {
					barrier_open(&barrier_jetpack);
					altar_jetpack.flags |= ACTIVE;
				}
				if (player.player_state == STATE_NOSHIELD) {
					barrier_open(&barrier_shield);
					altar_shield.flags |= ACTIVE;
				}
			}
			
			player.player_state = STATE_ALL;
			entity_deactivate(&icon);
		}
		
		// say appropriate dialog line, if enough time has passed
		
		if (rocket_dialog_timer <= 0) {
			rocket_dialog_timer = ROCKET_DIALOG_COOLDOWN;
			switch (gems_deposited) {
				case 0:
					dialog_say("I still need 3 power gems.", 160, Fix(40));
					break;
				case 1:
					dialog_say("Another 2 power gems to go.", 160, Fix(40));
					break;
				case 2:
					dialog_say("One more power gem left.", 160, Fix(40));
					break;
				case 3:
					dialog_say("Oh woof. I've done it.", 160, Fix(40));
					dialog_say_next("I am a good boy.", 160, Fix(70));
					timeout_set(160*2 + 30, level_complete_fadeout);
					break;
				default:
					dialog_say("???", 160, Fix(80));
					break;
			}
		}
	}
	
}


void altargun_spawn(int x, int y) {
	altar_gun.x = x + Fix(20);
	altar_gun.y = y - Fix(16);
	barrier_gun.x = x - Fix(16);
	barrier_gun.y = y - Fix(16);
}
void altarshield_spawn(int x, int y) {
	altar_shield.x = x + Fix(20);
	altar_shield.y = y - Fix(16);
	barrier_shield.x = x - Fix(16);
	barrier_shield.y = y - Fix(16);
}
void altarjetpack_spawn(int x, int y) {
	altar_jetpack.x = x + Fix(20);
	altar_jetpack.y = y - Fix(16);
	barrier_jetpack.x = x - Fix(16);
	barrier_jetpack.y = y - Fix(16);
}

void update_barrier(entity_t *barrier) {
	if (!barrier->door_closed) {
		entity_animate(barrier);
		
		if (barrier->frame == 7) {
			shake_timer = 30;
		}
		
		int px = (barrier->x >> FIX_SHIFT) - scrollx;
		int py = (barrier->y >> FIX_SHIFT) - scrolly;
		
		if (px+32 < 0 || py+64 < 0 || px > 240 || py > 160) {
			return;
		}
		
		obj_set_attr(&obj_mem[reserve_obj()],
			(py & ATTR0_Y_MASK) | ATTR0_TALL,
			(px & ATTR1_X_MASK) | ATTR1_SIZE_64,
			barrier->tid | ATTR2_PRIO(2) | ATTR2_PALBANK(5));
		
		if (!anim_finished(barrier)) {
			dma3_cpy(
				&tile_mem_obj[0][barrier->tid],
				&SprBarrierTiles[barrier->anim->frames[barrier->frame] * sizeof(TILE) * (4*8)],
				sizeof(TILE) * (4*8));
		}
	}
}

static void barrier_set_cells(int x, int y, int val) {
	int cell_x = (x>>FIX_SHIFT) / CELL_SIZE;
	int cell_y = (y>>FIX_SHIFT) / CELL_SIZE;
	for (int i = 0; i < 1; i++) {
		for (int j = 0; j < 4; j++) {
			map_set_cell(cell_x+i, cell_y+j, val);
		}
	}
}

void barrier_open(entity_t *barrier) {
	barrier_set_cells(barrier->x, barrier->y, CELL_EMPTY);
	set_anim(barrier, &AnimBarrierOpen);
	barrier->door_closed = false;
}

void altars_update() {
	
	rocket_update();
	
	update_barrier(&barrier_gun);
	update_barrier(&barrier_jetpack);
	update_barrier(&barrier_shield);
	
	if ((altar_gun.flags & ACTIVE) && player.player_state == STATE_ALL && entity_collide(&altar_gun, &player)) {
	// if ((altar_gun.flags & ACTIVE) && entity_collide(&altar_gun, &player)) {
		altar_gun.flags &= ~ACTIVE;
		icon.flags |= ACTIVE;
		icon.x = altar_gun.x - Fix(20);
		icon.y = altar_gun.y + Fix(8);
		set_anim(&icon, &AnimSacrificeGun);
		player.player_state = STATE_NOGUN;
		set_anim(&barrier_gun, &AnimBarrier);
		barrier_set_cells(barrier_gun.x, barrier_gun.y, CELL_SOLID);
		spawn_x = icon.x + Fix(4);
		spawn_y = icon.y - Fix(4);
		dialog_say("I sacrificed my blaster.", 160, Fix(50));
		if (level == &Level1) {
			mmStart(MOD_SPACEHAMSTER, MM_PLAY_LOOP);
		} else {
			mmStart(MOD_SPACEHAMSTER4, MM_PLAY_LOOP);
		}
	}
	
	// if ((altar_jetpack.flags & ACTIVE) && entity_collide(&altar_jetpack, &player)) {
	if ((altar_jetpack.flags & ACTIVE) && player.player_state == STATE_ALL && entity_collide(&altar_jetpack, &player)) {
		altar_jetpack.flags &= ~ACTIVE;
		icon.flags |= ACTIVE;
		icon.x = altar_jetpack.x - Fix(20);
		icon.y = altar_jetpack.y + Fix(8);
		set_anim(&icon, &AnimSacrificeJetpack);
		player.player_state = STATE_NOJET;
		set_anim(&barrier_jetpack, &AnimBarrier);
		barrier_set_cells(barrier_jetpack.x, barrier_jetpack.y, CELL_SOLID);
		spawn_x = icon.x + Fix(4);
		spawn_y = icon.y - Fix(4);
		dialog_say("I sacrificed my jetpack.", 160, Fix(50));
		dialog_say("Now I can't jump!", 160, Fix(60));
		if (level == &Level1) {
			mmStart(MOD_SPACEHAMSTER3, MM_PLAY_LOOP);
		} else {
			mmStart(MOD_SPACEHAMSTER6, MM_PLAY_LOOP);
		}
	}
	
	if ((altar_shield.flags & ACTIVE) && player.player_state == STATE_ALL && entity_collide(&altar_shield, &player)) {
	// if ((altar_shield.flags & ACTIVE) && entity_collide(&altar_shield, &player)) {
		altar_shield.flags &= ~ACTIVE;
		icon.flags |= ACTIVE;
		icon.x = altar_shield.x - Fix(20);
		icon.y = altar_shield.y + Fix(8);
		set_anim(&icon, &AnimSacrificeShield);
		player.player_state = STATE_NOSHIELD;
		set_anim(&barrier_shield, &AnimBarrier);
		barrier_set_cells(barrier_shield.x, barrier_shield.y, CELL_SOLID);
		spawn_x = icon.x + Fix(4);
		spawn_y = icon.y - Fix(4);
		dialog_say("So dangerous without a shield.", 160, Fix(30));
		if (level == &Level1) {
			mmStart(MOD_SPACEHAMSTER2, MM_PLAY_LOOP);
		} else {
			mmStart(MOD_SPACEHAMSTER5, MM_PLAY_LOOP);
		}
	}
	
	int px = (icon.x >> FIX_SHIFT) - scrollx;
	int py = (icon.y >> FIX_SHIFT) - scrolly;
	
	if (px+32 < 0 || py+32 < 0 || px > 240 || py > 160) {
		return;
	}
	
	if (is_active(&icon)) {
		entity_animate(&icon);
		obj_set_attr(&obj_mem[reserve_obj()],
			(py & ATTR0_Y_MASK) | ATTR0_SQUARE,
			(px & ATTR1_X_MASK) | ATTR1_SIZE_32,
			icon.tid | ATTR2_PRIO(1) | ATTR2_PALBANK(6));
		dma3_cpy(
			&tile_mem_obj[0][icon.tid],
			&SprSacrificedItemsTiles[icon.anim->frames[icon.frame] * sizeof(TILE) * (4*4)],
			sizeof(TILE) * (4*4));
	}
}
