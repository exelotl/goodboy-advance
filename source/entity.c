#include <tonc.h>
#include "common.h"

// move by dx
// if we hit a wall, we move 1px at a time until we hit the wall
bool entity_move_x(entity_t *e, FIXED dx) {
	if (map_collide_at(e, dx, 0)) {
		int sign = SGN(dx) << FIX_SHIFT;
		while ((dx >> FIX_SHIFT) != 0 && !map_collide_at(e, sign, 0)) {
			e->x += sign;			
			dx -= sign;
		}
		return true;
	} else {
		e->x += e->velx;
		return false;
	}
}

bool entity_move_y(entity_t *e, FIXED dy) {
	if (map_collide_at(e, 0, dy)) {
		int sign = SGN(dy) << FIX_SHIFT;
		while ((dy >> FIX_SHIFT) != 0 && !map_collide_at(e, 0, sign)) {
			e->y += sign;
			dy -= sign;
		}
		return true;
	} else {
		e->y += e->vely;
		return false;
	}
}

// void entity_apply_drag(entity_t *e) {
// 	if (e->velx > 0)      e->velx = MAX(0, e->velx - e->dragx);
// 	else if (e->velx < 0) e->velx = MIN(0, e->velx + e->dragx);
// 	if (e->vely > 0)      e->vely = MAX(0, e->vely - e->dragy);
// 	else if (e->vely < 0) e->vely = MIN(0, e->vely + e->dragy);
// }

void entity_animate(entity_t *e) {
	if (++e->frame >= e->anim_len) e->frame = 0;
}
void entity_animate_noloop(entity_t *e) {
	if (e->frame < e->anim_len-1) {
		e->frame++;
	}
}