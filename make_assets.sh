
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
# BG_OPTIONS="$BG_OPTIONS -gT00ff00" # transparent colour
BG_OPTIONS="$BG_OPTIONS -m"         # produce map data
BG_OPTIONS="$BG_OPTIONS -mR8"       # entries for 4bpp tiles
BG_OPTIONS="$BG_OPTIONS -mLs"       # regular bg broken into screenblocks

BG4_OPTIONS=""
BG4_OPTIONS="$BG4_OPTIONS -ftc"       # output to C files
BG4_OPTIONS="$BG4_OPTIONS -gt"        # output tiled graphics
BG4_OPTIONS="$BG4_OPTIONS -gB4"       # output 8bpp graphics
BG4_OPTIONS="$BG4_OPTIONS -gu8"       # output data as byte array
BG4_OPTIONS="$BG4_OPTIONS -gT 000000" # transparent colour
BG4_OPTIONS="$BG4_OPTIONS -m"         # produce map data
BG4_OPTIONS="$BG4_OPTIONS -mR4"       # entries for 4bpp tiles
BG4_OPTIONS="$BG4_OPTIONS -mLs"       # regular bg broken into screenblocks

# BGAFF_OPTIONS=""
# BGAFF_OPTIONS="$BGAFF_OPTIONS -ftc"       # output to C files
# BGAFF_OPTIONS="$BGAFF_OPTIONS -gt"        # output tiled graphics
# BGAFF_OPTIONS="$BGAFF_OPTIONS -gB8"       # output 4bpp graphics
# BGAFF_OPTIONS="$BGAFF_OPTIONS -gu8"       # output data as byte array
# BGAFF_OPTIONS="$BGAFF_OPTIONS -gT 000000" # transparent colour
# BGAFF_OPTIONS="$BGAFF_OPTIONS -m"         # produce map data
# BGAFF_OPTIONS="$BGAFF_OPTIONS -mRa"       # entries for affine tiles
# BGAFF_OPTIONS="$BGAFF_OPTIONS -mLa"       # affine map layout

SPRITE_8x8=""
SPRITE_8x8="$SPRITE_8x8 -ftc"       # output to C files
SPRITE_8x8="$SPRITE_8x8 -gt"        # output tiled graphics
SPRITE_8x8="$SPRITE_8x8 -gB4"       # output 4bpp graphics
SPRITE_8x8="$SPRITE_8x8 -gu8"       # output data as byte array
SPRITE_8x8="$SPRITE_8x8 -gT 000000" # transparent colour
SPRITE_8x8="$SPRITE_8x8 -pe 16"     # up to 16 colours in the palette

SPRITE_16x16="$SPRITE_8x8 -Mw 2 -Mh 2"
SPRITE_32x32="$SPRITE_8x8 -Mw 4 -Mh 4"
SPRITE_32x64="$SPRITE_8x8 -Mw 4 -Mh 8"

grit $ASSETS/BgTalkie.png $BG4_OPTIONS
grit $ASSETS/BgPlanet.png $BG4_OPTIONS
grit $ASSETS/levels/Level1.png $BG_OPTIONS
grit $ASSETS/levels/Level2.png $BG_OPTIONS

grit $ASSETS/SprPlayer.png $SPRITE_32x32
grit $ASSETS/SprMuzzle.png $SPRITE_32x32
grit $ASSETS/SprShield.png $SPRITE_32x32
grit $ASSETS/SprBullet.png $SPRITE_8x8
grit $ASSETS/SprBreakable.png $SPRITE_32x32
grit $ASSETS/SprBarrier.png $SPRITE_32x64
grit $ASSETS/SprSacrificedItems.png $SPRITE_32x32
grit $ASSETS/SprGem.png $SPRITE_32x32

# a bunch of 16x16 graphics that are able to use a single palette
# note: first graphic must contain transparent (#408000) pixels
grit $ASSETS/SprPlayer1.png \
	$ASSETS/SprZombie.png \
	-O SprShared1 $SPRITE_16x16 -pS

cd "../.."
node make_levels.js