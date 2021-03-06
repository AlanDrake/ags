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
#ifndef __AGS_EE_AC__GLOBALDRAWINGSURFACE_H
#define __AGS_EE_AC__GLOBALDRAWINGSURFACE_H

//void RawSaveScreen ();
// RawRestoreScreen: copy backup bitmap back to screen; we
// deliberately don't free the Common::Bitmap *cos they can multiple restore
// and it gets freed on room exit anyway
//void RawRestoreScreen();
// Restores the backup bitmap, but tints it to the specified level
//void RawRestoreScreenTinted(int red, int green, int blue, int opacity);
//void RawDrawFrameTransparent (int frame, int translev);
//void RawClear (int clr);// [DEPRECATED]
//void RawSetColor (int clr);// [DEPRECATED]
//void RawSetColorRGB(int red, int grn, int blu);// [DEPRECATED]
//void RawPrint (int xx, int yy, const char *text);// [DEPRECATED]
//void RawPrintMessageWrapped (int xx, int yy, int wid, int font, int msgm);// [DEPRECATED]
//void RawDrawImageCore(int xx, int yy, int slot, int alpha = 0xFF);// [DEPRECATED]
//void RawDrawImage(int xx, int yy, int slot);// [DEPRECATED]
//void RawDrawImageOffset(int xx, int yy, int slot);// [DEPRECATED]
//void RawDrawImageTransparent(int xx, int yy, int slot, int opacity);// [DEPRECATED]
//void RawDrawImageResized(int xx, int yy, int gotSlot, int width, int height);// [DEPRECATED]
//void RawDrawLine (int fromx, int fromy, int tox, int toy);// [DEPRECATED]
//void RawDrawCircle (int xx, int yy, int rad);// [DEPRECATED]
//void RawDrawRectangle(int x1, int y1, int x2, int y2);// [DEPRECATED]
//void RawDrawTriangle(int x1, int y1, int x2, int y2, int x3, int y3);// [DEPRECATED]

#endif // __AGS_EE_AC__GLOBALDRAWINGSURFACE_H
