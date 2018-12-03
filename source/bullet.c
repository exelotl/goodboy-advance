#include "common.h"
#include "assets/SprBullet.h"

static const anim_t AnimBullet = { .speed = 3, .loop = 1, .len = 4, .frames = (int[]){0,1,2,3} };

static uint bullet_tid;

static const int BULLET_SPEED = Fix(6.0);

uint bullets_init(uint tid) {
	bullet_tid = tid;
	dma3_cpy(&tile_mem_obj[0][bullet_tid], SprBulletTiles, SprBulletTilesLen);
	
	for (int i=0; i<BULLET_COUNT; i++)
		bullets[i] = (entity_t){
			.w = 6,
			.h = 10,
		};
		
	return tid + 4 * 1;  // 4 frames
}

entity_t *bullet_spawn(int x, int y, int dir) {
	entity_t *e = NULL;
	for (int i = 0; i < BULLET_COUNT; i++) {
		if (!is_active(&bullets[i])) {
			e = &bullets[i];
			break;
		}
	}
	if (e) {
		e->flags = ACTIVE;
		e->velx = dir > 0 ? BULLET_SPEED : -BULLET_SPEED,
		set_center(e, (vec2){x, y});
		set_anim(e, &AnimBullet);
	}
	return e;
}

extern breakables_check_bullet(entity_t *bullet);

void bullets_update(void) {
	
	for (int i = 0; i < BULLET_COUNT; i++) {
		entity_t *e = &bullets[i];
		if (!e->flags & ACTIVE) continue;
		
		entity_animate(e);
		
		if (abs(player.x - e->x) > Fix(200)) {
			entity_deactivate(e);
			return;
		}
		
		// bool hit = entity_move_x(e, e->velx) || entity_move_y(e, e->vely);
		e->x += e->velx;
		e->y += e->vely;
		if (map_collide(e) & (CELL_SOLID | CELL_SPIKE)) {
			breakables_check_bullet(e);
			entity_deactivate(e);
			vec2 center = get_center(e);
			entity_t * fx = muzzle_spawn_impact(center.x, center.y, e->velx);
			if (fx) {
				fx->x += (e->velx > 0) ? Fix(-16) : Fix(16);
			}
		}
		
		int x = e->x >> FIX_SHIFT;
		int y = e->y >> FIX_SHIFT;
		
		obj_set_attr(&obj_mem[reserve_obj()],
			((y - scrolly + 1) & ATTR0_Y_MASK) | ATTR0_SQUARE,
			((x - scrollx - 3) & ATTR1_X_MASK) | ATTR1_SIZE_8,
			(bullet_tid + e->anim->frames[e->frame]*1) | ATTR2_PRIO(1) | ATTR2_PALBANK(3));
	}
		
}