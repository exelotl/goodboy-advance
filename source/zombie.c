#include "common.h"
#include "assets/SprZombie.h"

static const int AnimZombieRun[] = {0,1,2,3,4,5,6,7,8,9,10};
static const int AnimZombieFall[] = {11};
static const int AnimZombieAngryRun[] = {12,13,14,15,16,17};
static const int AnimZombieAngryFall[] = {18};

entity_t zombies[ZOMBIE_COUNT];

static uint zombie_tid;


static const int SPEED = (int)(1.0f * (FIX_SCALE));
static const int SPEED_ANGRY = (int)(2.0f * (FIX_SCALE));
static const int GRAVITY = (int)(0.2f * (FIX_SCALE));


uint zombies_init(uint tid) {
	zombie_tid = tid;
	
	dma3_cpy(&tile_mem_obj[0][zombie_tid], SprZombieTiles, SprZombieTilesLen);
	
	for (int i=0; i<ZOMBIE_COUNT; i++)
		zombies[i] = (entity_t){ .w=8, .h=9 };
	
	return tid + 4*19;
}

entity_t *zombie_spawn(int x, int y) {
	entity_t *e = NULL;
	for (int i = 0; i < ZOMBIE_COUNT; i++) {
		if (!is_active(&zombies[i])) {
			e = &zombies[i];
			break;
		}
	}
	if (e) {
		e->flags |= ACTIVE;
		e->angry = false;
		e->health = 4;
		e->x = x;
		e->y = y;
		if (qran() & 1) {
			e->velx = SPEED;
		} else {
			e->velx = -SPEED;
		}
		SetAnim(e, AnimZombieRun);
	}
	return e;
}


void zombies_update(void) {
	
	for (int i = 0; i < ZOMBIE_COUNT; i++) {
		entity_t *e = &zombies[i];
		if (!e->flags & ACTIVE) continue;
		
		e->velx = 0;
		e->vely = 0;
		// e->vely += ZOMBIE_GRAVITY;
		
		bool hit_x = entity_move_x(e, e->velx);
		bool hit_y = entity_move_y(e, e->vely);
		
		if (hit_x) {
			e->velx = -e->velx;
			set_hflip(e, e->velx < 0);
		}
		if (hit_y && e->vely > 0) {
			e->vely = 0;
			if (e->angry) {
				SetAnim(e, AnimZombieAngryRun);
			} else {
				SetAnim(e, AnimZombieRun);
			}
		} else {
			// if (e->angry) {
			// 	SetAnim(e, AnimZombieAngryFall);
			// } else {
			// 	SetAnim(e, AnimZombieFall);
			// }
		}
		
		if (e->y > (180 << FIX_SHIFT)) {
			e->y = -12 << FIX_SHIFT;
			if (!e->angry) {
				e->angry = true;
				e->velx = (e->flags & HFLIP) ? -SPEED_ANGRY : SPEED_ANGRY;
				SetAnim(e, AnimZombieAngryFall);
				mmEffect(SFX_ANGRY);
			}
		}
		
		if (global_tick % 5 == 0) {
			entity_animate(e);
		}
		
		if (e->health <= 0) {
			entity_deactivate(e);
		}
		
		int x = e->x >> FIX_SHIFT;
		int y = e->y >> FIX_SHIFT;
		
		obj_set_attr(&obj_mem[reserve_obj()],
			((y - 5) & ATTR0_Y_MASK) | ATTR0_SQUARE,
			((x - 4) & ATTR1_X_MASK) | (e->flags & (HFLIP|VFLIP)) | ATTR1_SIZE_16,
			(zombie_tid + e->anim[e->frame]*4) | ATTR2_PRIO(1) | ATTR2_PALBANK(0));
	}
	
}
