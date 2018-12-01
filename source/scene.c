#include "common.h"

static void nothing(void) {}

static scene_t current_scene = {
	.show = nothing,
	.hide = nothing,
	.update = nothing,
};

void scene_set(scene_t next) {
	current_scene.hide();
	current_scene = next;
	current_scene.show();
}

void scene_update() {
	current_scene.update();
}
