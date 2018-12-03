#include "common.h"

static void nothing(void) {}


static scene_t next_scene = {
	.show = nothing,
	.hide = nothing,
	.update = nothing,
};
static scene_t current_scene = {
	.show = nothing,
	.hide = nothing,
	.update = nothing,
};

static bool scene_changed = false;

void scene_set(scene_t next) {
	next_scene = next;
	scene_changed = true;
}

void scene_update() {
	if (scene_changed) {
		scene_changed = false;
		current_scene.hide();
		current_scene = next_scene;
		current_scene.show();
	}
	current_scene.update();
}
