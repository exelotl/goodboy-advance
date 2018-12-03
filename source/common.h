#pragma once
#include <tonc.h>
#include <maxmod.h>
#include <stdlib.h>
#include "soundbank.h"

#ifdef ISMAIN
	#define def
#else
	#define def extern
#endif

typedef struct scene_t {
	void (*show)(void);
	void (*hide)(void);
	void (*update)(void);
} scene_t;

void scene_set(scene_t scene);
void scene_update();

extern const scene_t title_scene, game_scene, fake_end_scene;

extern int slide_number;  // which image to display on title screen

#define CELL_SIZE 16
#define CELL_SHIFT 4
#define LEVEL_WIDTH_CELLS 64
#define LEVEL_HEIGHT_CELLS 64
#define LEVEL_CELL_COUNT (LEVEL_WIDTH_CELLS*LEVEL_HEIGHT_CELLS)
#define LEVEL_WIDTH_CHUNKS 4
#define LEVEL_HEIGHT_CHUNKS 4
#define LEVEL_WIDTH_PX (LEVEL_WIDTH_CELLS*CELL_SIZE)
#define LEVEL_HEIGHT_PX (LEVEL_HEIGHT_CELLS*CELL_SIZE)

#define CHUNK_WIDTH_PIXELS (8 * 32)
#define CHUNK_HEIGHT_PIXELS (8 * 32)

#define CELL_EMPTY  0x0000
#define CELL_SOLID  0x0001
#define CELL_ONEWAY 0x0002
#define CELL_SPIKE  0x0004


typedef void (*callback_t)(void);

// general purpose timeout  (only one fn at a time, for now)
def callback_t timeout_cb;
def int timeout_counter;

inline void timeout_clear() {
	timeout_cb = NULL;
	timeout_counter = 0;
}
inline void timeout_set(int t, callback_t cb) {
	timeout_cb = cb;
	timeout_counter = t;
}
inline void timeout_update() {
	if (timeout_counter > 0) {
		if (--timeout_counter == 0) {
			if (timeout_cb) {
				timeout_cb();
			}
		}
	}
}


typedef struct spawninfo_t {
	int type, x, y;
} spawninfo_t;

typedef struct level_t {
	int cells[LEVEL_CELL_COUNT];
	const unsigned char *tiles;
	const unsigned short *map;
	const unsigned short *pal;
	int tilesLen;
	int mapLen;
	int palLen;
	const spawninfo_t *spawns;
	int spawnsLen;
} level_t;

typedef struct anim_t {
	int *frames;
	int len, speed;
	int loop;
} anim_t;

typedef struct vec2 {
	FIXED x, y;
} vec2;


#define ACTIVE 0x0001  // does this entity exist in the world?
#define CAN_JET 0x0002
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
			const anim_t *anim;
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
		struct {
			int muzzle_aff;	
			int muzzle_attr1;
		};
		struct {
			int brk_timer;
		};
		struct {
			bool door_closed;
		};
		struct {
			bool icon_active;
		};
		struct {
			int gem_state;
			int gem_timer;
		};
		// unused
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

// these check whether an entity would be hitting a solid if it were to move by a certain amount
// (vely is used to decide if this includes oneway platforms)
bool did_hit_x(entity_t *e, FIXED dx);
bool did_hit_y(entity_t *e, FIXED dy);

void set_anim(entity_t *e, const anim_t *anim);
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
inline vec2 get_center(entity_t *e) {
	return (vec2) {
		.x = e->x + (e->w << FIX_SHIFT) / 2,
		.y = e->y + (e->h << FIX_SHIFT) / 2,
	};
}
inline void set_center(entity_t *e, vec2 pos) {
	e->x = pos.x - (e->w << FIX_SHIFT) / 2;
	e->y = pos.y - (e->h << FIX_SHIFT) / 2;
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

int map_collide_rect(int x, int y, int w, int h);

inline int map_collide(entity_t *e) {
	return map_collide_rect(
		e->x >> FIX_SHIFT,
		e->y >> FIX_SHIFT,
		e->w, e->h);
}
inline int map_collide_at(entity_t *e, FIXED dx, FIXED dy) {
	return map_collide_rect(
		(e->x + dx) >> FIX_SHIFT,
		(e->y + dy) >> FIX_SHIFT,
		e->w, e->h);
}

// some globals

def int global_tick;
def int scrollx, scrolly;  // camera
def int parallax_x, parallax_y;

// sizes of various entity arrays

#define ZOMBIE_COUNT 1
#define MUZZLE_COUNT 4
#define BULLET_COUNT 8
#define BREAKABLE_COUNT 20
#define GEM_COUNT 3


// entity declarations

def entity_t player;
def entity_t rocket; // for collision tests, updated in altar.c?
def entity_t shield;
def entity_t label_dialog;
def entity_t bullets[BULLET_COUNT];
def entity_t muzzles[MUZZLE_COUNT];
def entity_t zombies[ZOMBIE_COUNT];
def entity_t breakables[BREAKABLE_COUNT];
def entity_t gems[GEM_COUNT];

// gem states
#define GEM_AVAILABLE 0
#define GEM_DISAPPEARING 1
#define GEM_TAKEN 2
#define GEM_DEPOSITING 3
#define GEM_DEPOSITED 4

// for respawning only?
def int spawn_x, spawn_y;

def int shake_timer;
def int shake_x;
def int shake_y;

def int dialog_has_input;
def int dialog_visible;

def int gems_deposited;

// reserved affine indices
def int aff_rotate_270;


void dialog_init(void);
void dialog_say(const char *str, int duration, int x);
void dialog_say_next(const char *str, int duration, int x);
void dialog_update(void);

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

uint muzzles_init(uint tid);
void muzzles_update(void);

uint bullets_init(uint tid);
void bullets_update(void);

uint shield_init(uint tid);
void shield_update(void);

uint breakables_init(uint tid);
void breakables_update(void);

uint altars_init(uint tid);
void altars_update(void);

uint gems_init(uint tid);
void gems_update(void);

entity_t *zombie_spawn(int x, int y);
entity_t *muzzle_spawn(int x, int y, int aff, int attr1);
entity_t *muzzle_spawn_impact(int x, int y, int dir);
entity_t *shield_spawn(int x, int y);
entity_t *bullet_spawn(int x, int y, int dir);
entity_t *breakable_spawn(int x, int y);
entity_t *gem_spawn(int x, int y);

void altargun_spawn(int x, int y);
void altarshield_spawn(int x, int y);
void altarjetpack_spawn(int x, int y);

void spawn_all(const level_t *level);
void map_set_cell(int x, int y, int val); 

// OAM management

int reserve_obj(void);
int reserve_obj_multi(int n);
int reserve_aff(void);
int reserve_aff_perm(void);
void oam_update(void);

// fades
void fader_init(void);
void fader_update(void);
void fader_fade_out(int spd, callback_t cb);
void fader_fade_in(int spd, callback_t cb);

#define PALBANK_NUM_COLORS (PAL_BG_SIZE/sizeof(COLOR))
def EWRAM_DATA COLOR pal_bg_target[PALBANK_NUM_COLORS];
def EWRAM_DATA COLOR pal_obj_target[PALBANK_NUM_COLORS];

#define pal_bg_target_mem (pal_bg_target)
#define pal_obj_target_mem (pal_obj_target)
#define pal_bg_target_bank ((PALBANK*)pal_bg_target)
#define pal_obj_target_bank ((PALBANK*)pal_obj_target)

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

#define Fix(f) (int)(f * FIX_SCALE)

inline int distance_sq(int x1, int y1, int x2, int y2) {
	int dx = x2 - x1;
	int dy = y2 - y1;
	return dx*dx + dy*dy;
}
