#include "common.h"
#include "soundbank_bin.h"
#include "tte_multiobj.h"
#include "fonts/Volter.h"

int global_tick;
int scrollx, scrolly;

int aff_rotate_270;

int main(void) {
	irq_init(NULL);
	irq_enable(II_VBLANK);
	
	// add maxmod vblank interrupt handler
	// initialise maxmod with soundbank and 8 channels
	irq_add(II_VBLANK, mmVBlank);
    mmInitDefault((mm_addr)soundbank_bin, 8);
		
	REG_DISPCNT = DCNT_MODE0 // 4 regular backgrounds
				| DCNT_BG0
				// | DCNT_BG1
				// | DCNT_BG2
				// | DCNT_BG3
				| DCNT_OBJ     // enable sprites
				| DCNT_OBJ_1D; // 1D tile mapping for sprites
	
	tte_init_multiobj(
		bytes2word(1,2,0,0),  // ink,shadow,paper,special color indices
		&VolterFont           // intitial font structure
	);
	
	aff_rotate_270 = reserve_aff_perm();
	obj_aff_rotate(&obj_aff_mem[aff_rotate_270], ANGLE(270));
	
	// scene_set(title_scene);
	scene_set(game_scene);
	
	while (1) {
		
		qran(); // introduce some nondeterminism to the RNG
		
		VBlankIntrWait();
		mmFrame();
		key_poll();
	
		scene_update();
		oam_update(); // clear unused entries, reset allocation counters
		
		global_tick++;
	}
}

