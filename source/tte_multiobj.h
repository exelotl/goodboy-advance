#pragma once

#include <tonc.h>

void tte_init_multiobj(u32 cattrs, const TFont *font);
void tte_set_multiobj(OBJ_ATTR* base, uint count);
void tte_set_multiobj_dest(uint tid, uint w, uint h, uint count);
void tte_erase_multiobj(void);
void multiobj_set_attr(OBJ_ATTR* base, uint count, u16 attr0, u16 attr1, u16 attr2);
void multiobj_set_pos(OBJ_ATTR* base, uint count, int x, int y);
void multiobj_clear(OBJ_ATTR* base, uint count);
