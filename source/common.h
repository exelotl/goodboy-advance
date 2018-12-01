#pragma once
#include <tonc.h>
#include <maxmod.h>
#include <stdlib.h>
#include "soundbank.h"


typedef struct scene_t {
	void (*show)(void);
	void (*hide)(void);
	void (*update)(void);
} scene_t;

void scene_set(scene_t scene);
void scene_update();

extern const scene_t title_scene, game_scene;

#define CELL_SIZE 16
#define CELL_SHIFT 4
#define LEVEL_WIDTH_CELLS 32
#define LEVEL_HEIGHT_CELLS 32
#define LEVEL_CELL_COUNT (LEVEL_WIDTH_CELLS*LEVEL_HEIGHT_CELLS)

#define CELL_EMPTY 0
#define CELL_SOLID 1

typedef struct level_t {
	int cells[LEVEL_CELL_COUNT];
	unsigned char *tiles;
	unsigned short *map;
	unsigned short *pal;
	int tilesLen;
	int mapLen;
	int palLen;
} level_t;


typedef struct anim_t {
	int *frames;
	int len, speed;
	int loop;
} anim_t;


#define ACTIVE 0x0001  // does this entity exist in the world?
#define HFLIP 0x1000   // same as ATTR1_HFLIP
#define VFLIP 0x2000   // same as ATTR1_VFLIP

#define HVFLIP (HFLIP|VFLIP) // combined flag

typedef struct entity_t {
	int flags;
	FIXED x, y;
	FIXED velx, vely;
	int w, h;
	
	// tile index for the first frame of animation in VRAM
	// it's also possible that new frames are directly streamed to this tile
	// depends how you want to animate it
	int tid;
	
	union {
		// animated sprite fields
		struct {
			anim_t *anim;
			int anim_timer;
			int frame;
		};
		// text label fields
		struct {
			const TFont *font;
			uint ink, shadow;
			uint obj_count;
		};
	};
	
	// entity-specific fields
	// e.g.
	union {
		// player fields
		struct {
			int player_state;  // which ability is missing
			int player_anim;   // which frame of animation
		};
		// enemy fields
		struct {
			int health;
			bool angry;
		};
	};
} entity_t;


// general entity functions
bool entity_move_x(entity_t *e, FIXED velx);
bool entity_move_y(entity_t *e, FIXED vely);
void entity_animate(entity_t *e);

void set_anim(entity_t *e, anim_t *anim);
bool anim_finished(entity_t *e);

inline void entity_activate(entity_t *e) {
	e->flags |= ACTIVE;
}
inline void entity_deactivate(entity_t *e) {
	e->flags &= ~ACTIVE;
}
inline bool is_active(entity_t *e) {
	return e->flags & ACTIVE;
}
inline void set_hflip(entity_t *e, bool hflip) {
	if (hflip) e->flags |= HFLIP;
	else e->flags &= ~HFLIP;
}
inline void set_vflip(entity_t *e, bool vflip) {
	if (vflip) e->flags |= VFLIP;
	else e->flags &= ~VFLIP;
}

inline bool collide(int ax, int ay, int aw, int ah,
                    int bx, int by, int bw, int bh) {
	return ax + aw > bx
	    && ay + ah > by
		&& ax < bx + bw
		&& ay < by + bh;
}
inline bool entity_collide(entity_t *a, entity_t *b) {
	return collide(
		a->x >> FIX_SHIFT,
		a->y >> FIX_SHIFT,
		a->w, a->h,
		b->x >> FIX_SHIFT,
		b->y >> FIX_SHIFT,
		b->w, b->h);
}

bool map_collide_rect(int x, int y, int w, int h);

inline bool map_collide(entity_t *e) {
	return map_collide_rect(
		e->x >> FIX_SHIFT,
		e->y >> FIX_SHIFT,
		e->w, e->h);
}
inline bool map_collide_at(entity_t *e, FIXED dx, FIXED dy) {
	return map_collide_rect(
		(e->x + dx) >> FIX_SHIFT,
		(e->y + dy) >> FIX_SHIFT,
		e->w, e->h);
}

// some globals

extern int global_tick;
extern int scrollx, scrolly;  // camera

// sizes of various entity arrays

#define ZOMBIE_COUNT 20

// entity declarations

extern entity_t player;
extern entity_t label_hello;
extern entity_t zombies[ZOMBIE_COUNT];

uint label_init(entity_t *label, const TFont *font, uint ink, uint shadow, uint tid, uint obj_count);
void label_begin_write(entity_t *label);
void label_set_pos(entity_t *label, int x, int y);
void label_update(entity_t *label);
void label_update_all(void);

uint player_init(uint tid);
void player_update(void);
void player_draw(void);

uint zombies_init(uint tid);
void zombies_update(void);


entity_t *zombie_spawn(int x, int y);


// OAM management

int reserve_obj(void);
int reserve_obj_multi(int n);
int reserve_aff(void);
int reserve_aff_perm(void);
void oam_update(void);


// debugging utilities

inline void nocash_puti(int i) {
	itoa(i, nocash_buffer, 10);
	nocash_message();
}
inline void nocash_putx(int i) {
	itoa(i, nocash_buffer, 16);
	nocash_message();
}


// maths / fixed point utilities (on top of what tonc already offers)

inline FIXED fxceil(FIXED fx) {
	FIXED res = fx;
	res >>= FIX_SHIFT;
	res <<= FIX_SHIFT;
	if (res != fx) res += FIX_ONE;
	return res;
}

#define ANGLE(deg) ((deg<<16)/360)

inline int distance_sq(int x1, int y1, int x2, int y2) {
	int dx = x2 - x1;
	int dy = y2 - y1;
	return dx*dx + dy*dy;
}
