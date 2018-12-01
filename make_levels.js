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
	
	json.layers.forEach(layer => {
		if (layer.type === 'tilelayer') {
			layer.data.forEach(tile => {
				let tinfo = tileInfos[tile-1];
				switch (tinfo && tinfo.type) {
					case 'wall':
						cells.push(1);
						break;
					default:
						cells.push(0);
				}
			});
		}
	})
	
	outHeader.push(`#include "assets/${levelName}.h"`);
	outHeader.push(`extern const level_t ${levelName};`);
	
	outSource.push(
`const level_t ${levelName} = {
	.cells = {${cells.join(',')}},
	.tiles = ${levelName}Tiles,
	.tilesLen = ${levelName}TilesLen,
	.map = ${levelName}Map,
	.mapLen = ${levelName}MapLen,
	.pal = ${levelName}Pal,
	.palLen = ${levelName}PalLen,
};`
	);
}

loadTileset('assets/tileset.json');
makeLevel('assets/levels/Level1.json');

fs.writeFileSync(outHeaderPath, outHeader.join('\n'));
fs.writeFileSync(outSourcePath, outSource.join('\n'));
