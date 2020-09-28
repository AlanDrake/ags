//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
//
//
//=============================================================================
#ifndef __AGS_EE_AC__SPRITELISTENTRY_H
#define __AGS_EE_AC__SPRITELISTENTRY_H

#include "gfx/ddb.h"

struct SpriteListEntry
{
    AGS::Engine::IDriverDependantBitmap *bmp;
    AGS::Common::Bitmap *pic;
    int baseline;
    int x,y;
    int transparent;
    bool takesPriorityIfEqual;
    bool hasAlphaChannel;
    int blendMode;

    SpriteListEntry();
};

#endif // __AGS_EE_AC__SPRITELISTENTRY_H
