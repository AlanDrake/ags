// Alan Drake's experimental filters of wonder
//----------------------------------------------------------
//Copyright (C) 2011 Alan v.Drake

//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU Lesser General Public
//License as published by the Free Software Foundation; either
//version 2.1 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
//Lesser General Public License for more details.
//


#ifndef __AC_ADEXFILTERS_H
#define __AC_ADEXFILTERS_H

#include <allegro.h>

void adex_scanline_2x(BITMAP *src, BITMAP *dest);
void adex_tvcrt_4x(BITMAP *src, BITMAP *dest);
void adex_monitorcrt_4x(BITMAP *src, BITMAP *dest);

#endif  // __AC_ADEXFILTERS_H