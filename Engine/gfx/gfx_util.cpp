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

#include "gfx/gfx_util.h"
#include "gfx/blender.h"
#include "../windows/include/allegro/internal/aintern.h"

// CHECKME: is this hack still relevant?
#if defined(IOS_VERSION) || defined(ANDROID_VERSION)
extern int psp_gfx_renderer;
#endif

namespace AGS
{
namespace Engine
{

using namespace Common;

namespace GfxUtil
{

// [AVD]naive custom blenders
#define BLEND(bpp, r, g, b)   _blender_trans##bpp(makecol##bpp(r, g, b), y, n)
// some formulas from AGSBlend
#define _BLENDOP_BURN(B,L) ((B + L < 255) ? 0:(B + L - 255)) // note: burn was called subtract
#define _BLENDOP_LIGHTEN(B,L) ((L > B) ? L:B)
#define _BLENDOP_DARKEN(B,L) ((L > B) ? B:L)
#define _BLENDOP_EXCLUSION(B,L) (B + L - 2 * B * L / 255)
#define _BLENDOP_SUBTRACT(B,L) (L - B < 0) ? 0:(L - B)
// not very pretty but forces the original alpha of "x" to "blender_result"
inline unsigned long _blender_mask_alpha24(unsigned long blender_result, unsigned long x, unsigned long y, unsigned long n)
{
  const unsigned long src_alpha = geta32(x);
  const unsigned long alphainv = 255-src_alpha;
  const unsigned long r = (getr24(blender_result)*src_alpha + alphainv*getr24(y))/255;
  const unsigned long g = (getg24(blender_result)*src_alpha + alphainv*getg24(y))/255;
  const unsigned long b = (getb24(blender_result)*src_alpha + alphainv*getb24(y))/255;
  return r | g<<8 | b<<16;
}
// alegro's dodge blend was wrong, mine is not perfect either
inline unsigned long _my_blender_dodge24(unsigned long x, unsigned long y, unsigned long n)
{
  const unsigned long h = (getr24(y)*n) / ( 256 - getr24(x) ); 
  const unsigned long j = (getg24(y)*n) / ( 256 - getg24(x) );
  const unsigned long k = (getb24(y)*n) / ( 256 - getb24(x) );
  return BLEND(24, h>255?255:h, j>255?255:j, k>255?255:k);
}
inline unsigned long _my_blender_burn24(unsigned long x, unsigned long y, unsigned long n)
{
  const unsigned long h = _BLENDOP_BURN(getr24(x), getr24(y)); 
  const unsigned long j = _BLENDOP_BURN(getg24(x), getg24(y)); 
  const unsigned long k = _BLENDOP_BURN(getb24(x), getb24(y)); 
  return BLEND(24, h, j, k);
}
inline unsigned long _my_blender_lighten24(unsigned long x, unsigned long y, unsigned long n)
{
  const unsigned long h = _BLENDOP_LIGHTEN(getr24(x), getr24(y)); 
  const unsigned long j = _BLENDOP_LIGHTEN(getg24(x), getg24(y)); 
  const unsigned long k = _BLENDOP_LIGHTEN(getb24(x), getb24(y)); 
  return BLEND(24, h, j, k);
}
inline unsigned long _my_blender_darken24(unsigned long x, unsigned long y, unsigned long n)
{
  const unsigned long h = _BLENDOP_DARKEN(getr24(x), getr24(y)); 
  const unsigned long j = _BLENDOP_DARKEN(getg24(x), getg24(y)); 
  const unsigned long k = _BLENDOP_DARKEN(getb24(x), getb24(y)); 
  return BLEND(24, h, j, k);
}
inline unsigned long _my_blender_exclusion24(unsigned long x, unsigned long y, unsigned long n)
{
  const unsigned long h = _BLENDOP_EXCLUSION(getr24(x), getr24(y)); 
  const unsigned long j = _BLENDOP_EXCLUSION(getg24(x), getg24(y)); 
  const unsigned long k = _BLENDOP_EXCLUSION(getb24(x), getb24(y)); 
  return BLEND(24, h, j, k);
}
inline unsigned long _my_blender_subtract24(unsigned long x, unsigned long y, unsigned long n)
{
  const unsigned long h = _BLENDOP_SUBTRACT(getr24(x), getr24(y)); 
  const unsigned long j = _BLENDOP_SUBTRACT(getg24(x), getg24(y)); 
  const unsigned long k = _BLENDOP_SUBTRACT(getb24(x), getb24(y)); 
  return BLEND(24, h, j, k);
}

inline unsigned long _blender_masked_add32(unsigned long x, unsigned long y, unsigned long n)
{
  return _blender_mask_alpha24(_blender_add24(x, y, n ), x, y, n);
}
inline unsigned long _blender_masked_dodge32(unsigned long x, unsigned long y, unsigned long n)
{
  return _blender_mask_alpha24(_my_blender_dodge24(x, y, n ), x, y, n);
}
inline unsigned long _blender_masked_burn32(unsigned long x, unsigned long y, unsigned long n)
{
  return _blender_mask_alpha24(_my_blender_burn24(x, y, n ), x, y, n);
}
inline unsigned long _blender_masked_lighten32(unsigned long x, unsigned long y, unsigned long n)
{
  return _blender_mask_alpha24(_my_blender_lighten24(x, y, n ), x, y, n);
}
inline unsigned long _blender_masked_darken32(unsigned long x, unsigned long y, unsigned long n)
{
  return _blender_mask_alpha24(_my_blender_darken24(x, y, n ), x, y, n);
}
inline unsigned long _blender_masked_exclusion32(unsigned long x, unsigned long y, unsigned long n)
{
  return _blender_mask_alpha24(_my_blender_exclusion24(x, y, n ), x, y, n);
}
inline unsigned long _blender_masked_subtract32(unsigned long x, unsigned long y, unsigned long n)
{
  return _blender_mask_alpha24(_my_blender_subtract24(x, y, n ), x, y, n);
}
inline unsigned long _blender_masked_screen32(unsigned long x, unsigned long y, unsigned long n)
{
  return _blender_mask_alpha24(_blender_screen24(x, y, n ), x, y, n);
}
inline unsigned long _blender_masked_multiply32(unsigned long x, unsigned long y, unsigned long n)
{
  return _blender_mask_alpha24(_blender_multiply24(x, y, n ), x, y, n);
}
// --



typedef BLENDER_FUNC PfnBlenderCb;

struct BlendModeSetter
{
    // Blender setter for destination with and without alpha channel;
    // assign null pointer if not supported
    PfnBlenderCb AllAlpha;       // src w alpha   -> dst w alpha
    PfnBlenderCb AlphaToOpaque;  // src w alpha   -> dst w/o alpha
    PfnBlenderCb OpaqueToAlpha;  // src w/o alpha -> dst w alpha
    PfnBlenderCb OpaqueToAlphaNoTrans; // src w/o alpha -> dst w alpha (opt-ed for no transparency)
    PfnBlenderCb AllOpaque;      // src w/o alpha -> dst w/o alpha
};

// Array of blender descriptions
// NOTE: set NULL function pointer to fallback to common image blitting
static const BlendModeSetter BlendModeSets[kNumBlendModes] =
{
    { NULL, NULL, NULL, NULL, NULL }, // kBlendMode_NoAlpha
    { _argb2argb_blender, _argb2rgb_blender, _rgb2argb_blender, _opaque_alpha_blender, NULL }, // kBlendMode_Alpha
    { NULL, _blender_masked_add32, _blender_add24, _blender_add24, NULL }, // kBlendMode_Add
    { NULL, _blender_masked_darken32, _my_blender_darken24, _my_blender_darken24, NULL }, // kBlendMode_Darken
    { NULL, _blender_masked_lighten32, _my_blender_lighten24, _my_blender_lighten24, NULL }, // kBlendMode_Lighten
    { NULL, _blender_masked_multiply32, _blender_multiply24, _blender_multiply24, NULL }, // kBlendMode_Multiply
    { NULL, _blender_masked_screen32, _blender_screen24, _blender_screen24, NULL }, // kBlendMode_Screen
    { NULL, _blender_masked_burn32, _my_blender_burn24, _my_blender_burn24, NULL }, // kBlendMode_Burn
    { NULL, _blender_masked_subtract32, _my_blender_subtract24, NULL }, // kBlendMode_Subtract
    { NULL, _blender_masked_exclusion32, _my_blender_exclusion24, _my_blender_exclusion24, NULL }, // kBlendMode_Exclusion
    { NULL, _blender_masked_dodge32, _my_blender_dodge24, _my_blender_dodge24, NULL }, // kBlendMode_Dodge
    // NOTE: add new modes here
};

bool SetBlender(BlendMode blend_mode, bool dst_has_alpha, bool src_has_alpha, int blend_alpha)
{
    if (blend_mode < 0 || blend_mode > kNumBlendModes)
        return false;
    const BlendModeSetter &set = BlendModeSets[blend_mode];
    PfnBlenderCb blender;
    if (dst_has_alpha)
        blender = src_has_alpha ? set.AllAlpha :
            (blend_alpha == 0xFF ? set.OpaqueToAlphaNoTrans : set.OpaqueToAlpha);
    else
        blender = src_has_alpha ? set.AlphaToOpaque : set.AllOpaque;

    if (blender)
    {
        set_blender_mode(NULL, NULL, blender, 0, 0, 0, blend_alpha);
        return true;
    }
    return false;
}

void DrawSpriteBlend(Bitmap *ds, const Point &ds_at, Bitmap *sprite,
                       BlendMode blend_mode,  bool dst_has_alpha, bool src_has_alpha, int blend_alpha)
{
    if (blend_alpha <= 0)
        return; // do not draw 100% transparent image

    if (// support only 32-bit blending at the moment
        ds->GetColorDepth() == 32 && sprite->GetColorDepth() == 32 &&
        // set blenders if applicable and tell if succeeded
        SetBlender(blend_mode, dst_has_alpha, src_has_alpha, blend_alpha))
    {
        ds->TransBlendBlt(sprite, ds_at.X, ds_at.Y);
    }
    else
    {
        GfxUtil::DrawSpriteWithTransparency(ds, sprite, ds_at.X, ds_at.Y, blend_alpha);
    }
}

void DrawSpriteWithTransparency(Bitmap *ds, Bitmap *sprite, int x, int y, int alpha)
{
    if (alpha <= 0)
    {
        // fully transparent, don't draw it at all
        return;
    }

    int surface_depth = ds->GetColorDepth();
    int sprite_depth  = sprite->GetColorDepth();

    if (sprite_depth < surface_depth
        // CHECKME: what is the purpose of this hack and is this still relevant?
#if defined(IOS_VERSION) || defined(ANDROID_VERSION)
        || (ds->GetBPP() < surface_depth && psp_gfx_renderer > 0) // Fix for corrupted speechbox outlines with the OGL driver
#endif
        )
    {
        // If sprite is lower color depth than destination surface, e.g.
        // 8-bit sprites drawn on 16/32-bit surfaces.
        if (sprite_depth == 8 && surface_depth >= 24)
        {
            // 256-col sprite -> truecolor background
            // this is automatically supported by allegro, no twiddling needed
            ds->Blit(sprite, x, y, kBitmap_Transparency);
            return;
        }

        // 256-col sprite -> hi-color background, or
        // 16-bit sprite -> 32-bit background
        Bitmap hctemp;
        hctemp.CreateCopy(sprite, surface_depth);
        if (sprite_depth == 8)
        {
            // only do this for 256-col -> hi-color, cos the Blit call converts
            // transparency for 16->32 bit
            color_t mask_color = hctemp.GetMaskColor();
            for (int scan_y = 0; scan_y < hctemp.GetHeight(); ++scan_y)
            {
                // we know this must be 1 bpp source and 2 bpp pixel destination
                const uint8_t *src_scanline = sprite->GetScanLine(scan_y);
                uint16_t      *dst_scanline = (uint16_t*)hctemp.GetScanLineForWriting(scan_y);
                for (int scan_x = 0; scan_x < hctemp.GetWidth(); ++scan_x)
                {
                    if (src_scanline[scan_x] == 0)
                    {
                        dst_scanline[scan_x] = mask_color;
                    }
                }
            }
        }

        if (alpha < 0xFF) 
        {
            set_trans_blender(0, 0, 0, alpha);
            ds->TransBlendBlt(&hctemp, x, y);
        }
        else
        {
            ds->Blit(&hctemp, x, y, kBitmap_Transparency);
        }
    }
    else
    {
        if (alpha < 0xFF && surface_depth > 8 && sprite_depth > 8) 
        {
            set_trans_blender(0, 0, 0, alpha);
            ds->TransBlendBlt(sprite, x, y);
        }
        else
        {
            ds->Blit(sprite, x, y, kBitmap_Transparency);
        }
    }
}

} // namespace GfxUtil

} // namespace Engine
} // namespace AGS
