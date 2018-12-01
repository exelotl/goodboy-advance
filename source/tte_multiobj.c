
// Tonc Text Engine custom renderer
// Allows you to use rows of sprites e.g. 32x16 or 64x32 as text surfaces

// Has a few limitations:
// - expects a 4bpp bitpacked font
// - you can only have one multiobj TTE context (not sure why you would want more)

#include <string.h>
#include <tonc.h>
#include "tte_multiobj.h"

#include "common.h"

// behind the scenes rendering functions used by TTE
static void multiobj_erase(int left, int top, int right, int bottom);
static void multiobj_draw(uint gid);

// Initialise text system to render to OVRAM
// You must set the target with tte_set_multiobj before attempting to render
void tte_init_multiobj(u32 cattrs, const TFont *font) {
	
	if (font == NULL) font = &vwf_default;	
	tte_init_base(font, multiobj_draw, multiobj_erase);
	
	TTC *context = tte_get_context();
	
	// --- Init color attribute ---
	u32 ink    =  cattrs        & 15;
	u32 shadow = (cattrs >> 8)  & 15;
	u32 paper  = (cattrs >> 16) & 15;
	//u32 spec = (cattrs >> 24) & 255;

	context->cattr[TTE_INK] = ink;
	context->cattr[TTE_SHADOW] = shadow;
	context->cattr[TTE_PAPER] = paper;
}


// The current working row of sprites

static TILE* dest = NULL;
static int base_w;
static int base_h;
static uint obj_count;    // how many consecutive sprites do we have?

// Set the working row of objects
// By default this renders to the top-left corner
// To reposition, you're better off moving the sprite itself
void tte_set_multiobj(OBJ_ATTR *base, uint count) {
	dest = &tile_mem_obj[0][base->attr2 & ATTR2_ID_MASK];
	base_w = obj_get_width(base);
	base_h = obj_get_height(base);
	obj_count = count;
	tte_set_pos(0, 0);
}

// Equivalent to the above, but useful when you don't have an object at hand
// however you know the tile ID and sprite size of the object
void tte_set_multiobj_dest(uint tid, uint w, uint h, uint count) {
	dest = &tile_mem_obj[0][tid];
	base_w = w;
	base_h = h;
	obj_count = count;
	tte_set_pos(0, 0);
}

// Update the attributes of all objects
// This includes making sure they have the correct successive positions and tile IDs
void multiobj_set_attr(OBJ_ATTR *base, uint count, u16 a0, u16 a1, u16 a2) {
	
	obj_set_attr(base, a0, a1, a2);
	int base_x  = a1 & ATTR1_X_MASK;
	int base_id = a2 & ATTR2_ID_MASK;
	a1 &= ~ATTR1_X_MASK;
	a2 &= ~ATTR2_ID_MASK;
	int base_w = obj_get_width(base);
	int base_h = obj_get_height(base);
	int base_size_tiles = base_w/8 * base_h/8;
	
	for (int i=1; i<count; i++) {
		obj_set_attr(base+i,
			a0,
			((base_x + base_w*i) & ATTR1_X_MASK) | a1,
			ATTR2_ID(base_id + base_size_tiles*i) | a2);
	}
}

// Position a row of objects so that they form a horizontal surface with no gaps
// x,y describes the top-left corner of the base object
void multiobj_set_pos(OBJ_ATTR *base, uint count, int x, int y) {
	int base_w = obj_get_width(base);
	for (int i=0; i<count; i++) {
		obj_set_pos(base+i, (x + base_w*i), y);
	}
}

#define TSIZE32 (sizeof(TILE) / sizeof(u32))

// Erase the tile contents of all sprites in the given multiobj
void multiobj_clear(OBJ_ATTR *base, uint count) {
	int w = obj_get_width(base) / 8;
	int h = obj_get_height(base) / 8;
	int tid = base->attr2 & ATTR2_ID_MASK;
	memset32(&tile_mem_obj[0][tid], 0, TSIZE32 * w * h * count);
}

// Erase the tile contents currently active multiobj
void tte_erase_multiobj(void) {
	memset32(dest, 0, TSIZE32 * base_w * base_h * obj_count);
}

// NOTE - this clears the entire sprite's VRAM (parameters are unused)
static void multiobj_erase(int left, int top, int right, int bottom) {
	memset32(dest, 0, TSIZE32 * base_w * base_h * obj_count);
}


// Warning - very ugly and probably inefficient adaption
// of the existing TTE 4bpp glyph drawing routines below:

static void multiobj_draw(uint gid) {
	
	//nocash_buffer[0] = gid + 32;
	//nocash_buffer[1] = '\0';
	//nocash_message();
	
	if (dest == NULL) {
		nocash_puts("warn: no multiobj dest set. use tte_set_multiobj or tte_set_multiobj_dest");
		return;
	}
	
	TTC *context = tte_get_context();
	TFont *font = context->font;
	
	u32 *srcD = (u32*)(font->data + gid * font->cellSize);
	u32 *srcL = srcD;
	uint charW = font->widths ? font->widths[gid] : font->charW;
	uint charH = font->charH;
	uint x = context->cursorX;
	uint y = context->cursorY;
	uint srcPitch = font->cellH;
	
	uint base_w_px = base_w * 8;
	
	uint dstPitch = sizeof(TILE) / sizeof(u32);
	uint sprPitch = dstPitch * base_w;
	uint sprSize = sprPitch * base_h;
	
	// primary and secondary destination tile pointers
	u32 *dstD = (u32*)dest + y + x/8*dstPitch;
	u32 *dstL;
	
	// if we are rendering onto an offset sprite, we need to
	//  increase the render destination to be relative to that sprite
	while (x >= base_w_px) {
		dstD += sprSize - sprPitch;
		x -= base_w_px;
	}

	// it might happen that we are rendering across the boundary between two sprites.
	// when we reach edge of this sprite, how many tiles must we jump?
	// answer: the size of 1 sprite, plus the number of rows needed to reach the end of the current sprite
	uint rightOffset = dstPitch;
	int jump = 0;
	
	if (x + charW > base_w_px) {
		rightOffset += sprSize - sprPitch;
		jump = (base_w_px - x - 1) / 8; // at which tile column does the glyph overlap onto the next sprite?
	}
	
	x %= 8;
	u32 lsl = 4*x;
	u32 lsr = 32-4*x;
	u32 endX = x+charW;
	
	u32 amask = 0x11111111;
	u32 px, pxmask, raw;
	u32 ink = context->cattr[TTE_INK];
	u32 shade = context->cattr[TTE_SHADOW];
	uint iy, iw;
	
	for(iw=0; iw < charW; iw += 8)	// Loop over vertical strips
	{
		dstL = dstD;
		srcL = srcD;
		dstD += dstPitch;
		srcD += srcPitch;
		
		iy = charH;
		
		while(iy--)					// Loop over scanlines
		{
			raw = *srcL++;
			
			px	   = (raw    & amask);
			raw	   = (raw>>1 & amask);
			pxmask = px | raw;
			if (pxmask)
			{
				px *= ink;
				px += raw*shade;
				pxmask *= 15;
				
				// Write left tile:
				dstL[0] = (dstL[0] &~ (pxmask<<lsl) ) | (px<<lsl);
				
				// Write right tile (if any)
				// also if we lie on the boundary (jump == 0), translate the right tile appropriately
				if (endX > 8) {
					if (jump == 0)
						dstL[rightOffset] = (dstL[rightOffset] &~ (pxmask>>lsr) ) | (px>>lsr);
					else
						dstL[dstPitch] = (dstL[dstPitch] &~ (pxmask>>lsr) ) | (px>>lsr);
				}
			}
			dstL++;
			
			// when we reach the next row, how many tiles must we jump?
			// answer: the size of 1 row (i.e. rows must be lined up horizontally in VRAM)
			// minus the size of the 1 tile that we just read
			
			if( ((u32)dstL)%32 == 0 ) {
				dstL += sprPitch - dstPitch;
			}
		}
		
		// permanently move the destination to the next sprite's VRAM
		// (after crossing the boundary)
		if (jump-- == 0) {
			dstD += rightOffset - dstPitch;
		}
	}
}
