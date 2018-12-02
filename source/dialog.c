#include "common.h"
#include "fonts/Acknowledge.h"
#include "fonts/Volter.h"
#include "assets/BgTalkie.h"

static int dialog_scroll_x = 0;
static int dialog_scroll_y = 0;

void dialog_init(void) {
	// tte_init_chr4c
	dialog_has_input = false;
	dialog_visible = false;
}

void dialog_say(char *str) {
	
}

void dialog_update(void) {
	if (dialog_visible) {
		
	}
	
	// REG_BG0HOFS = dialog_scroll_x;
	// REG_BG0VOFS = dialog_scroll_y;
}
