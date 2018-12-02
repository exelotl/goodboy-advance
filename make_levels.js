const fs = require('fs');
const path = require('path');

let outHeader = [`
#pragma once
#include "common.h"

`];

let outSource = [`
#include "common.h"
#include "levels.h"

`];

let outHeaderPath = 'source/assets/levels.h'
let outSourcePath = 'source/assets/levels.c'
if (!fs.existsSync('source/assets')){
	fs.mkdirSync('source/assets');
}

let tileset;
let tileInfos = {};

function loadTileset(tilesetJsonPath) {
	tileset = JSON.parse(fs.readFileSync(tilesetJsonPath));
	tileset.tiles.forEach(tile => {
		tileInfos[tile.id] = tile;
	});
}

function makeLevel(levelJsonPath) {
	let json = JSON.parse(fs.readFileSync(levelJsonPath));
	let {width, height} = json;
	
	let levelName = path.basename(levelJsonPath, '.json');
	let cells = [];
	let spawns = [];
	
	json.layers.forEach(layer => {
		if (layer.type === 'tilelayer' && layer.name === 'BG2') {
			layer.data.forEach(tile => {
				let tinfo = tileInfos[tile-1];
				switch (tinfo && tinfo.type) {
					case 'wall':
						cells.push(0x0001);
						break;
					case 'oneway':
						cells.push(0x0002);
						break;
					case 'spike':
						cells.push(0x0004);
						break;
					default:
						cells.push(0);
				}
			});
		} else if (layer.type === 'objectgroup') {
			layer.objects.forEach(obj => {
				let tinfo = tileInfos[obj.gid-1];
				let type;
				let x = obj.x;
				let y = obj.y;
				switch (tinfo ? tinfo.type : -1) {
					case 'startpos':
						type = 0;
						break;
					case 'breakable':
						type = 1;
						break;
					case 'altarjetpack':
						type = 2;
						break;
					case 'altarshield':
						type = 4;
						break;
					case 'altargun':
						type = 5;
						break;
					case 'gem':
						type = 6;
						break;
					case 'rocket':
						type = 7;
						break;
					default:
						console.warn('unrecognised type');
						return;
				}
				spawns.push(`{ .type = ${type}, .x = ${x}, .y = ${y} },\n`);
			});
		}
	})
	
	outHeader.push(`#include "assets/${levelName}.h"`);
	outHeader.push(`extern const level_t ${levelName};`);
	
	outSource.push(`const spawninfo_t ${levelName}Spawns[] = {\n${spawns.join('')}};\n`);
	outSource.push(
`const level_t ${levelName} = {
	.cells = {${cells.join(',')}},
	.tiles = ${levelName}Tiles,
	.tilesLen = ${levelName}TilesLen,
	.map = ${levelName}Map,
	.mapLen = ${levelName}MapLen,
	.pal = ${levelName}Pal,
	.palLen = ${levelName}PalLen,
	.spawns = ${levelName}Spawns,
	.spawnsLen = ${spawns.length},
};`
	);
}

loadTileset('assets/tileset.json');
makeLevel('assets/levels/Level1.json');

fs.writeFileSync(outHeaderPath, outHeader.join('\n'));
fs.writeFileSync(outSourcePath, outSource.join('\n'));
