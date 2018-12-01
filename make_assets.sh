
OUTPATH="source/assets"
ASSETS="../../assets"

rm -rf $OUTPATH
mkdir -p $OUTPATH

cd $OUTPATH

BG_OPTIONS=""
BG_OPTIONS="$BG_OPTIONS -ftc"       # output to C files
BG_OPTIONS="$BG_OPTIONS -gt"        # output tiled graphics
BG_OPTIONS="$BG_OPTIONS -gB8"       # output 8bpp graphics
BG_OPTIONS="$BG_OPTIONS -gu8"       # output data as byte array
BG_OPTIONS="$BG_OPTIONS -gT 000000" # transparent colour
BG_OPTIONS="$BG_OPTIONS -m"         # produce map data
BG_OPTIONS="$BG_OPTIONS -mR4"       # entries for 4bpp tiles
BG_OPTIONS="$BG_OPTIONS -mLs"       # regular bg broken into screenblocks

SPRITE_8x8=""
SPRITE_8x8="$SPRITE_8x8 -ftc"       # output to C files
SPRITE_8x8="$SPRITE_8x8 -gt"        # output tiled graphics
SPRITE_8x8="$SPRITE_8x8 -gB4"       # output 4bpp graphics
SPRITE_8x8="$SPRITE_8x8 -gu8"       # output data as byte array
SPRITE_8x8="$SPRITE_8x8 -gT 000000" # transparent colour
SPRITE_8x8="$SPRITE_8x8 -pe 16"     # up to 16 colours in the palette

SPRITE_16x16="$SPRITE_8x8 -Mw 2 -Mh 2"
SPRITE_32x32="$SPRITE_8x8 -Mw 4 -Mh 4"

grit $ASSETS/levels/Level1.png $BG_OPTIONS

grit $ASSETS/SprPlayer.png $SPRITE_32x32
grit $ASSETS/SprMuzzle.png $SPRITE_32x32

# a bunch of 16x16 graphics that are able to use a single palette
# note: first graphic must contain transparent (#408000) pixels
grit $ASSETS/SprPlayer1.png \
	$ASSETS/SprZombie.png \
	-O SprShared1 $SPRITE_16x16 -pS

cd "../.."
node make_levels.js