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

#include "gfx/bitmap.h"
#include "gfx/gfxfilter_adexfilter.h"
#include "gfx/adexfilters.h"

namespace AGS
{
namespace Engine
{
namespace ALSW
{

using namespace Common;

#pragma region AdexScaline2x

const GfxFilterInfo AdexScanline2xGFXFilter::FilterInfo = GfxFilterInfo("AdexScaline2x", "Adex Scanline 2x filter (32-bit only)", 2, 2);

AdexScanline2xGFXFilter::AdexScanline2xGFXFilter()
    : _adexScalingBuffer(NULL)
{
}

AdexScanline2xGFXFilter::~AdexScanline2xGFXFilter()
{
    delete _adexScalingBuffer;
}

const GfxFilterInfo &AdexScanline2xGFXFilter::GetInfo() const
{
    return FilterInfo;
}

bool AdexScanline2xGFXFilter::Initialize(const int color_depth, String &err_str)
{
    if (color_depth < 32)
    {
        err_str = "Only supports 32-bit colour games";
        return false;
    }
    return AllegroGfxFilter::Initialize(color_depth, err_str);
}

Bitmap* AdexScanline2xGFXFilter::InitVirtualScreen(Bitmap *screen, const Size src_size, const Rect dst_rect)
{
    Bitmap *virtual_screen = AllegroGfxFilter::InitVirtualScreen(screen, src_size, dst_rect);

    int min_scaling = 2;

    _adexScalingBuffer = BitmapHelper::CreateBitmap(src_size.Width * min_scaling, src_size.Height * min_scaling);

    return virtual_screen;
}

Bitmap *AdexScanline2xGFXFilter::ShutdownAndReturnRealScreen(Bitmap *currentScreen)
{
    Bitmap *real_screen = AllegroGfxFilter::ShutdownAndReturnRealScreen(currentScreen);
    delete _adexScalingBuffer;
    _adexScalingBuffer = NULL;
    return real_screen;
}

Bitmap *AdexScanline2xGFXFilter::PreRenderPass(Bitmap *toRender)
{
    _adexScalingBuffer->Acquire();
    adex_scanline_2x(toRender->GetAllegroBitmap(), _adexScalingBuffer->GetAllegroBitmap());
    _adexScalingBuffer->Release();
    return _adexScalingBuffer;
}

#pragma endregion AdexScaline2x

#pragma region AdexPCCRT4x

const GfxFilterInfo AdexPCCRT4xGFXFilter::FilterInfo = GfxFilterInfo("AdexPCCRT4x", "Adex PC CRT 4x filter (32-bit only)", 4, 4);

AdexPCCRT4xGFXFilter::AdexPCCRT4xGFXFilter()
    : _adexScalingBuffer(NULL)
{
}

AdexPCCRT4xGFXFilter::~AdexPCCRT4xGFXFilter()
{
    delete _adexScalingBuffer;
}

const GfxFilterInfo &AdexPCCRT4xGFXFilter::GetInfo() const
{
    return FilterInfo;
}

bool AdexPCCRT4xGFXFilter::Initialize(const int color_depth, String &err_str)
{
    if (color_depth < 32)
    {
        err_str = "Only supports 32-bit colour games";
        return false;
    }
    return AllegroGfxFilter::Initialize(color_depth, err_str);
}

Bitmap* AdexPCCRT4xGFXFilter::InitVirtualScreen(Bitmap *screen, const Size src_size, const Rect dst_rect)
{
    Bitmap *virtual_screen = AllegroGfxFilter::InitVirtualScreen(screen, src_size, dst_rect);

    int min_scaling = 4;

    _adexScalingBuffer = BitmapHelper::CreateBitmap(src_size.Width * min_scaling, src_size.Height * min_scaling);

    return virtual_screen;
}

Bitmap *AdexPCCRT4xGFXFilter::ShutdownAndReturnRealScreen(Bitmap *currentScreen)
{
    Bitmap *real_screen = AllegroGfxFilter::ShutdownAndReturnRealScreen(currentScreen);
    delete _adexScalingBuffer;
    _adexScalingBuffer = NULL;
    return real_screen;
}

Bitmap *AdexPCCRT4xGFXFilter::PreRenderPass(Bitmap *toRender)
{
    _adexScalingBuffer->Acquire();
    adex_monitorcrt_4x(toRender->GetAllegroBitmap(), _adexScalingBuffer->GetAllegroBitmap());
    _adexScalingBuffer->Release();
    return _adexScalingBuffer;
}

#pragma endregion AdexPCCRT4x

#pragma region AdexTVCRT4x

const GfxFilterInfo AdexTVCRT4xGFXFilter::FilterInfo = GfxFilterInfo("AdexTVCRT4x", "Adex TV CRT 4x filter (32-bit only)", 4, 4);

AdexTVCRT4xGFXFilter::AdexTVCRT4xGFXFilter()
    : _adexScalingBuffer(NULL)
{
}

AdexTVCRT4xGFXFilter::~AdexTVCRT4xGFXFilter()
{
    delete _adexScalingBuffer;
}

const GfxFilterInfo &AdexTVCRT4xGFXFilter::GetInfo() const
{
    return FilterInfo;
}

bool AdexTVCRT4xGFXFilter::Initialize(const int color_depth, String &err_str)
{
    if (color_depth < 32)
    {
        err_str = "Only supports 32-bit colour games";
        return false;
    }
    return AllegroGfxFilter::Initialize(color_depth, err_str);
}

Bitmap* AdexTVCRT4xGFXFilter::InitVirtualScreen(Bitmap *screen, const Size src_size, const Rect dst_rect)
{
    Bitmap *virtual_screen = AllegroGfxFilter::InitVirtualScreen(screen, src_size, dst_rect);

    int min_scaling = 4;

    _adexScalingBuffer = BitmapHelper::CreateBitmap(src_size.Width * min_scaling, src_size.Height * min_scaling);

    return virtual_screen;
}

Bitmap *AdexTVCRT4xGFXFilter::ShutdownAndReturnRealScreen(Bitmap *currentScreen)
{
    Bitmap *real_screen = AllegroGfxFilter::ShutdownAndReturnRealScreen(currentScreen);
    delete _adexScalingBuffer;
    _adexScalingBuffer = NULL;
    return real_screen;
}

Bitmap *AdexTVCRT4xGFXFilter::PreRenderPass(Bitmap *toRender)
{
    _adexScalingBuffer->Acquire();
    adex_tvcrt_4x(toRender->GetAllegroBitmap(), _adexScalingBuffer->GetAllegroBitmap());
    _adexScalingBuffer->Release();
    return _adexScalingBuffer;
}

#pragma endregion AdexTVCRT4x

} // namespace ALSW
} // namespace Engine
} // namespace AGS
