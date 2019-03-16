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

#include "ac/common.h"
#include "ac/display.h"
#include "ac/draw.h"
#include "ac/game_version.h"
#include "ac/gamesetup.h"
#include "ac/gamesetupstruct.h"
#include "ac/gamestate.h"
#include "ac/runtime_defines.h"
#include "ac/walkbehind.h"
#include "ac/dynobj/scriptsystem.h"
#include "debug/out.h"
#include "device/mousew32.h"
#include "font/fonts.h"
#include "gfx/ali3dexception.h"
#include "gfx/graphicsdriver.h"
#include "gui/guimain.h"
#include "gui/guiinv.h"
#include "main/graphics_mode.h"
#include "main/engine_setup.h"
#include "media/video/video.h"
#include "platform/base/agsplatformdriver.h"

using namespace AGS::Common;
using namespace AGS::Engine;

extern GameSetupStruct game;
extern ScriptSystem scsystem;
extern int _places_r, _places_g, _places_b;
extern int current_screen_resolution_multiplier;
extern IGraphicsDriver *gfxDriver;

int convert_16bit_bgr = 0;

// Convert guis position and size to proper game screen coordinates.
// Necessary for pre 3.1.0 games only.
void convert_gui_to_screen_coordinates(GameDataVersion filever)
{
    if (filever > kGameVersion_310)
        return;

    for (int i = 0; i < game.numcursors; ++i)
    {
        game.mcurs[i].hotx *= current_screen_resolution_multiplier;
        game.mcurs[i].hoty *= current_screen_resolution_multiplier;
    }

    for (int i = 0; i < game.numinvitems; ++i)
    {
        game.invinfo[i].hotx *= current_screen_resolution_multiplier;
        game.invinfo[i].hoty *= current_screen_resolution_multiplier;
    }

    for (int i = 0; i < game.numgui; ++i)
    {
        GUIMain*cgp = &guis[i];
        cgp->X *= current_screen_resolution_multiplier;
        cgp->Y *= current_screen_resolution_multiplier;
        if (cgp->Width < 1)
            cgp->Width = 1;
        if (cgp->Height < 1)
            cgp->Height = 1;
        if (cgp->Width == play.GetNativeSize().Width - 1)
            cgp->Width = play.GetNativeSize().Width;

        cgp->Width *= current_screen_resolution_multiplier;
        cgp->Height *= current_screen_resolution_multiplier;

        cgp->PopupAtMouseY *= current_screen_resolution_multiplier;

        for (int j = 0; j < cgp->GetControlCount(); ++j)
        {
            GUIObject *guio = cgp->GetControl(j);
            guio->X *= current_screen_resolution_multiplier;
            guio->Y *= current_screen_resolution_multiplier;
            guio->Width *= current_screen_resolution_multiplier;
            guio->Height *= current_screen_resolution_multiplier;
            guio->IsActivated = false;
        }
    }
}

// Convert objects position and size to proper room coordinates.
// Necessary for 3.1.0 and above games with legacy "low-res coordinates" setting.
void convert_objects_to_room_coordinates(GameDataVersion filever)
{
    if (! (filever >= kGameVersion_310 && (game.options[OPT_NATIVECOORDINATES] == 0) && game.IsHiRes()) )
        return;

    for (int i = 0; i < game.numcharacters; ++i) 
    {
        game.chars[i].x /= current_screen_resolution_multiplier;
        game.chars[i].y /= current_screen_resolution_multiplier;
    }

    // TODO: frankly doing this to inventory window props makes little sense,
    // find out if this is correct and why; fix or add comment here.
    for (int i = 0; i < numguiinv; ++i)
    {
        guiinv[i].ItemWidth /= current_screen_resolution_multiplier;
        guiinv[i].ItemHeight /= current_screen_resolution_multiplier;
    }
}

void engine_setup_system_gamesize()
{
    scsystem.width = game.size.Width;
    scsystem.height = game.size.Height;
    scsystem.viewport_width = divide_down_coordinate(play.GetMainViewport().GetWidth());
    scsystem.viewport_height = divide_down_coordinate(play.GetMainViewport().GetHeight());
}

void engine_init_resolution_settings(const Size game_size)
{
    Debug::Printf("Initializing resolution settings");

    // Initialize default viewports and room camera
    Rect viewport = RectWH(game_size);
    play.SetMainViewport(viewport);
    play.SetUIViewport(viewport);
    play.SetRoomViewport(viewport);
    play.SetRoomCameraSize(viewport.GetSize());

    Size native_size = game_size;
    if (game.IsHiRes())
    {
        if (!game.options[OPT_NATIVECOORDINATES])
        {
            native_size.Width = game_size.Width / 2;
            native_size.Height = game_size.Height / 2;
        }
        current_screen_resolution_multiplier = 2;
    }
    else
    {
        native_size.Width = game_size.Width;
        native_size.Height = game_size.Height;
        current_screen_resolution_multiplier = 1;
    }
    play.SetNativeSize(native_size);

    usetup.textheight = getfontheight_outlined(0) + 1;

    Debug::Printf(kDbgMsg_Init, "Game native resolution: %d x %d (%d bit)%s", game_size.Width, game_size.Height, game.color_depth * 8,
        game.IsLegacyLetterbox() ? " letterbox-by-design" : "");

    convert_gui_to_screen_coordinates(loaded_game_file_version);
    convert_objects_to_room_coordinates(loaded_game_file_version);

    engine_setup_system_gamesize();
}

// Setup gfx driver callbacks and options
void engine_post_gfxmode_driver_setup()
{
    gfxDriver->SetCallbackForPolling(update_polled_stuff_if_runtime);
    gfxDriver->SetCallbackToDrawScreen(draw_screen_callback);
    gfxDriver->SetCallbackForNullSprite(GfxDriverNullSpriteCallback);
}

// Reset gfx driver callbacks
void engine_pre_gfxmode_driver_cleanup()
{
    gfxDriver->SetCallbackForPolling(NULL);
    gfxDriver->SetCallbackToDrawScreen(NULL);
    gfxDriver->SetCallbackForNullSprite(NULL);
    gfxDriver->SetMemoryBackBuffer(NULL);
}

// Setup virtual screen
void engine_post_gfxmode_screen_setup(const DisplayMode &dm, bool recreate_bitmaps)
{
    if (recreate_bitmaps)
    {
        // TODO: find out if 
        // - we need to support this case at all;
        // - if yes then which bitmaps need to be recreated (probably only video bitmaps and textures?)
    }
}

void engine_pre_gfxmode_screen_cleanup()
{
}

// Release virtual screen
void engine_pre_gfxsystem_screen_destroy()
{
    delete sub_vscreen;
    sub_vscreen = NULL;
}

// Setup color conversion parameters
void engine_setup_color_conversions(int coldepth)
{
    // default shifts for how we store the sprite data1
#if defined(PSP_VERSION)
    // PSP: Switch b<>r for 15/16 bit.
    _rgb_r_shift_32 = 16;
    _rgb_g_shift_32 = 8;
    _rgb_b_shift_32 = 0;
    _rgb_b_shift_16 = 11;
    _rgb_g_shift_16 = 5;
    _rgb_r_shift_16 = 0;
    _rgb_b_shift_15 = 10;
    _rgb_g_shift_15 = 5;
    _rgb_r_shift_15 = 0;
#else
    _rgb_r_shift_32 = 16;
    _rgb_g_shift_32 = 8;
    _rgb_b_shift_32 = 0;
    _rgb_r_shift_16 = 11;
    _rgb_g_shift_16 = 5;
    _rgb_b_shift_16 = 0;
    _rgb_r_shift_15 = 10;
    _rgb_g_shift_15 = 5;
    _rgb_b_shift_15 = 0;
#endif
    // Most cards do 5-6-5 RGB, which is the format the files are saved in
    // Some do 5-6-5 BGR, or  6-5-5 RGB, in which case convert the gfx
    if ((coldepth == 16) && ((_rgb_b_shift_16 != 0) || (_rgb_r_shift_16 != 11)))
    {
        convert_16bit_bgr = 1;
        if (_rgb_r_shift_16 == 10) {
            // some very old graphics cards lie about being 16-bit when they
            // are in fact 15-bit ... get around this
            _places_r = 3;
            _places_g = 3;
        }
    }
    if (coldepth > 16)
    {
        // when we're using 32-bit colour, it converts hi-color images
        // the wrong way round - so fix that

#if defined(IOS_VERSION) || defined(ANDROID_VERSION) || defined(PSP_VERSION) || defined(MAC_VERSION)
        _rgb_b_shift_16 = 0;
        _rgb_g_shift_16 = 5;
        _rgb_r_shift_16 = 11;

        _rgb_b_shift_15 = 0;
        _rgb_g_shift_15 = 5;
        _rgb_r_shift_15 = 10;

        _rgb_r_shift_32 = 0;
        _rgb_g_shift_32 = 8;
        _rgb_b_shift_32 = 16;
#else
        _rgb_r_shift_16 = 11;
        _rgb_g_shift_16 = 5;
        _rgb_b_shift_16 = 0;
#endif
    }
    else if (coldepth == 16)
    {
        // ensure that any 32-bit graphics displayed are converted
        // properly to the current depth
#if defined(PSP_VERSION)
        _rgb_r_shift_32 = 0;
        _rgb_g_shift_32 = 8;
        _rgb_b_shift_32 = 16;

        _rgb_b_shift_15 = 0;
        _rgb_g_shift_15 = 5;
        _rgb_r_shift_15 = 10;
#else
        _rgb_r_shift_32 = 16;
        _rgb_g_shift_32 = 8;
        _rgb_b_shift_32 = 0;
#endif
    }
    else if (coldepth < 16)
    {
        // ensure that any 32-bit graphics displayed are converted
        // properly to the current depth
#if defined (WINDOWS_VERSION)
        _rgb_r_shift_32 = 16;
        _rgb_g_shift_32 = 8;
        _rgb_b_shift_32 = 0;
#else
        _rgb_r_shift_32 = 0;
        _rgb_g_shift_32 = 8;
        _rgb_b_shift_32 = 16;

        _rgb_b_shift_15 = 0;
        _rgb_g_shift_15 = 5;
        _rgb_r_shift_15 = 10;
#endif
    }

    set_color_conversion(COLORCONV_MOST | COLORCONV_EXPAND_256);
}

// Setup drawing modes and color conversions;
// they depend primarily on gfx driver capabilities and new color depth
void engine_post_gfxmode_draw_setup(const DisplayMode &dm)
{
    engine_setup_color_conversions(dm.ColorDepth);
    init_draw_method();
}

// Cleanup auxiliary drawing objects
void engine_pre_gfxmode_draw_cleanup()
{
    dispose_draw_method();
}

// Setup mouse control mode and graphic area
void engine_post_gfxmode_mouse_setup(const DisplayMode &dm, const Size &init_desktop)
{
    // Assign mouse control parameters.
    //
    // Whether mouse movement should be controlled by the engine - this is
    // determined based on related config option.
    const bool should_control_mouse = usetup.mouse_control == kMouseCtrl_Always ||
        usetup.mouse_control == kMouseCtrl_Fullscreen && !dm.Windowed;
    // Whether mouse movement control is supported by the engine - this is
    // determined on per platform basis. Some builds may not have such
    // capability, e.g. because of how backend library implements mouse utils.
    const bool can_control_mouse = platform->IsMouseControlSupported(dm.Windowed);
    // The resulting choice is made based on two aforementioned factors.
    const bool control_sens = should_control_mouse && can_control_mouse;
    if (control_sens)
    {
        Mouse::EnableControl(!dm.Windowed);
        Mouse::SetSpeedUnit(1.f);
        if (usetup.mouse_speed_def == kMouseSpeed_CurrentDisplay)
        {
            Size cur_desktop;
            if (get_desktop_resolution(&cur_desktop.Width, &cur_desktop.Height) == 0)
                Mouse::SetSpeedUnit(Math::Max((float)cur_desktop.Width / (float)init_desktop.Width,
                                              (float)cur_desktop.Height / (float)init_desktop.Height));
        }
        Mouse::SetSpeed(usetup.mouse_speed);
    }
    Debug::Printf(kDbgMsg_Init, "Mouse control: %s, base: %f, speed: %f", Mouse::IsControlEnabled() ? "on" : "off",
        Mouse::GetSpeedUnit(), Mouse::GetSpeed());

    on_coordinates_scaling_changed();

    // If auto lock option is set, lock mouse to the game window
    if (usetup.mouse_auto_lock && scsystem.windowed != 0)
        Mouse::TryLockToWindow();
}

// Reset mouse controls before changing gfx mode
void engine_pre_gfxmode_mouse_cleanup()
{
    // Always disable mouse control and unlock mouse when releasing down gfx mode
    Mouse::DisableControl();
    Mouse::UnlockFromWindow();
}

// Fill in scsystem struct with display mode parameters
void engine_setup_scsystem_screen(const DisplayMode &dm)
{
    scsystem.coldepth = dm.ColorDepth;
    scsystem.windowed = dm.Windowed;
    scsystem.vsync = dm.Vsync;
}

void engine_post_gfxmode_setup(const Size &init_desktop)
{
    DisplayMode dm = gfxDriver->GetDisplayMode();
    // If color depth has changed (or graphics mode was inited for the
    // very first time), we also need to recreate bitmaps
    bool has_driver_changed = scsystem.coldepth != dm.ColorDepth;

    engine_setup_scsystem_screen(dm);
    engine_post_gfxmode_driver_setup();
    engine_post_gfxmode_screen_setup(dm, has_driver_changed);
    if (has_driver_changed)
        engine_post_gfxmode_draw_setup(dm);
    engine_post_gfxmode_mouse_setup(dm, init_desktop);
    
    // TODO: the only reason this call was put here is that it requires
    // "windowed" flag to be specified. Find out whether this function
    // has anything to do with graphics mode at all. It is quite possible
    // that we may split it into two functions, or remove parameter.
    platform->PostAllegroInit(scsystem.windowed != 0);

    video_on_gfxmode_changed();
    invalidate_screen();
}

void engine_pre_gfxmode_release()
{
    engine_pre_gfxmode_mouse_cleanup();
    engine_pre_gfxmode_driver_cleanup();
    engine_pre_gfxmode_screen_cleanup();
}

void engine_pre_gfxsystem_shutdown()
{
    engine_pre_gfxmode_release();
    engine_pre_gfxmode_draw_cleanup();
    engine_pre_gfxsystem_screen_destroy();
}

void on_coordinates_scaling_changed()
{
    // Reset mouse graphic area and bounds
    Mouse::SetGraphicArea();
    // If mouse bounds do not have valid values yet, then limit cursor to viewport
    if (play.mboundx1 == 0 && play.mboundy1 == 0 && play.mboundx2 == 0 && play.mboundy2 == 0)
        Mouse::SetMoveLimit(play.GetMainViewport());
    else
        Mouse::SetMoveLimit(Rect(play.mboundx1, play.mboundy1, play.mboundx2, play.mboundy2));
}
