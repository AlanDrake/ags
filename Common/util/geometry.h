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
// Geometry data structures and helper functions
//
//=============================================================================
#ifndef __AGS_CN_UTIL__GEOMETRY_H
#define __AGS_CN_UTIL__GEOMETRY_H

#include "util/math.h"

namespace AGSMath = AGS::Common::Math;
//namespace AGS
//{
//namespace Common
//{

enum FrameAlignment
{
    kAlignNone     = 0x00,
    kAlignLeft     = 0x01,
    kAlignRight    = 0x02,
    kAlignHCenter  = 0x04,
    kAlignTop      = 0x10,
    kAlignBottom   = 0x20,
    kAlignVCenter  = 0x40,

    kAlignTopLeft      = kAlignTop     | kAlignLeft,
    kAlignTopCenter    = kAlignTop     | kAlignHCenter,
    kAlignTopRight     = kAlignTop     | kAlignRight,
    kAlignCenterLeft   = kAlignVCenter | kAlignLeft,
    kAlignCentered     = kAlignVCenter | kAlignHCenter,
    kAlignCenterRight  = kAlignVCenter | kAlignRight,
    kAlignBottomLeft   = kAlignBottom  | kAlignLeft,
    kAlignBottomCenter = kAlignBottom  | kAlignHCenter,
    kAlignBottomRight  = kAlignBottom  | kAlignRight,

    // Masks
    kAlignHor      = kAlignLeft | kAlignRight | kAlignHCenter,
    kAlignVer      = kAlignTop | kAlignBottom | kAlignVCenter,
};

enum RectPlacement
{
    kPlaceOffset,
    kPlaceCenter,
    kPlaceStretch,
    kPlaceStretchProportional,
    kNumRectPlacement
};

struct Point
{
    int X;
    int Y;

    Point()
    {
        X = 0;
        Y = 0;
    }

    Point(int x, int y)
    {
        X = x;
        Y = y;
    }
};

struct Line
{
	int X1;
	int Y1;
	int X2;
	int Y2;

	Line()
	{
		X1 = 0;
		Y1 = 0;
		X2 = 0;
		Y2 = 0;
	}

	Line(int x1, int y1, int x2, int y2)
	{
		X1 = x1;
		Y1 = y1;
		X2 = x2;
		Y2 = y2;
	}
};

// Helper factory functions
inline Line HLine(int x1, int x2, int y)
{
	return Line(x1, y, x2, y);
}

inline Line VLine(int x, int y1, int y2)
{
	return Line(x, y1, x, y2);
}

struct Size
{
    int Width;
    int Height;

    Size()
    {
        Width = 0;
        Height = 0;
    }

    Size(int width, int height)
    {
        Width = width;
        Height = height;
    }

    inline bool IsNull() const
    {
        return Width <= 0 || Height <= 0;
    }

    inline void Clamp(const Size floor, const Size ceil)
    {
        Width = AGSMath::Clamp(Width, floor.Width, ceil.Width);
        Height = AGSMath::Clamp(Height, floor.Height, ceil.Height);
    }

    // Indicates if current size exceeds other size by any metric
    inline bool ExceedsByAny(const Size size) const
    {
        return Width > size.Width || Height > size.Height;
    }

    inline bool operator==(const Size size) const
    {
        return Width == size.Width && Height == size.Height;
    }

    inline bool operator!=(const Size size) const
    {
        return Width != size.Width || Height != size.Height;
    }

    inline bool operator<(const Size &other) const
    {
        return Width < other.Width || (Width == other.Width && Height < other.Height);
    }

    inline Size operator *(int x) const
    {
        return Size(Width * x, Height * x);
    }

    inline Size operator /(int x) const
    {
        return Size(Width / x, Height / x);
    }

    inline Size &operator *=(int x)
    {
        Width *= x;
        Height *= x;
        return *this;
    }

    inline Size &operator /=(int x)
    {
        Width /= x;
        Height /= x;
        return *this;
    }
};

// TODO: consider making Rect have right-bottom coordinate with +1 offset
// to comply with many other libraries (i.e. Right - Left == Width)
struct Rect
{
	int Left;
	int Top;
	int Right;
	int Bottom;

	Rect()
	{
		Left	= 0;
		Top		= 0;
		Right	= -1;
		Bottom	= -1;
	}

	Rect(int l, int t, int r, int b)
	{
		Left	= l;
		Top		= t;
		Right	= r;
		Bottom	= b;
	}

    inline Point GetLT() const
    {
        return Point(Left, Top);
    }

	inline int GetWidth() const
	{
		return Right - Left + 1;
	}

	inline int GetHeight() const
	{
		return Bottom - Top + 1;
	}

    inline Size GetSize() const
    {
        return Size(GetWidth(), GetHeight());
    }
    
    inline bool IsEmpty() const
    {
        return Right < Left || Bottom < Top;
    }

    inline bool IsInside(int x, int y) const
    {
        return x >= Left && y >= Top && (x <= Right) && (y <= Bottom);
    }

    inline bool IsInside(const Point &pt) const
    {
        return IsInside(pt.X, pt.Y);
    }

    inline void MoveToX(int x)
    {
        Right += x - Left;
        Left = x;
    }

    inline void MoveToY(int y)
    {
        Bottom += y - Top;
        Top = y;
    }

    inline void MoveTo(const Point &pt)
    {
        MoveToX(pt.X);
        MoveToY(pt.Y);
    }

    inline void SetWidth(int width)
    {
        Right = Left + width - 1;
    }

    inline void SetHeight(int height)
    {
        Bottom = Top + height - 1;
    }
};

// Helper factory function
inline Rect RectWH(int x, int y, int width, int height)
{
	return Rect(x, y, x + width - 1, y + height - 1);
}

inline Rect RectWH(const Size &sz)
{
    return Rect(0, 0, sz.Width - 1, sz.Height - 1);
}


struct Triangle
{
	int X1;
	int Y1;
	int X2;
	int Y2;
	int X3;
	int Y3;

	Triangle()
	{
		X1 = 0;
		Y1 = 0;
		X2 = 0;
		Y2 = 0;
		X3 = 0;
		Y3 = 0;
	}

	Triangle(int x1, int y1, int x2, int y2, int x3, int y3)
	{
		X1 = x1;
		Y1 = y1;
		X2 = x2;
		Y2 = y2;
		X3 = x3;
		Y3 = y3;
	}
};

struct Circle
{
	int X;
	int Y;
	int Radius;

	Circle()
	{
		X = 0;
		Y = 0;
		Radius = 0;
	}

	Circle(int x, int y, int radius)
	{
		X = x;
		Y = y;
		Radius = radius;
	}

};



int AlignInHRange(int x1, int x2, int off_x, int width, FrameAlignment align);
int AlignInVRange(int y1, int y2, int off_y, int height, FrameAlignment align);
Rect AlignInRect(const Rect &frame, const Rect &item, FrameAlignment align);

Size ProportionalStretch(int dest_w, int dest_h, int item_w, int item_h);
Size ProportionalStretch(const Size &dest, const Size &item);

Rect OffsetRect(const Rect &r, const Point off);
Rect CenterInRect(const Rect &place, const Rect &item);
Rect PlaceInRect(const Rect &place, const Rect &item, const RectPlacement &placement);
//} // namespace Common
//} // namespace AGS

#endif // __AGS_CN_UTIL__GEOMETRY_H
