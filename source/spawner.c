#include "common.h"

void spawn_all(const level_t *level) {
	
	for (int i = 0; i < level->spawnsLen; i++) {
		spawninfo_t spawn = level->spawns[i];
		int x = Fix(spawn.x);
		int y = Fix(spawn.y);
		switch (spawn.type) {
			case 0:
				// startpos
				player.x = x;
				player.y = y - Fix(16);
				break;
			case 1:
				// breakable
				breakable_spawn(x, y - Fix(16));
				break;
			case 2:
				// altarjetpack
				altarjetpack_spawn(x, y - Fix(16));
				break;
			case 4:
				// altarshield
				altarshield_spawn(x, y - Fix(16));
				break;
			case 5:
				// altargun
				altargun_spawn(x, y - Fix(16));
				break;
			case 6:
				// gem
				gem_spawn(x, y - Fix(16));
				break;
			case 7:
				// rocket
				rocket_spawn(x - Fix(16), y - Fix(32));
				break;
			default:
				break;
				// something bad happened...
		}
	}
	
}

