// gitype.h - Image Types
#if !defined(GITYPE_H)
#define GITYPE_H
/*-----------------------------------------------------------------------------
	file:			gpath.h
	author:		Guy Schalnat
	date:			10/09/96
	revision:	3.0
	This file handles any types that are specific to the image side of the
	library.  It also includes ggeomtry.h and, on MSWIN, windows.h (after
	defining STRICT).  It defines the following classes:
	G42Surface - a window surface typedef
	G42DrawLocation - where to draw
	G42Color - Red, Green, Blue structure
	G42Margin - Left, Top, Right, Bottom structure
	G42ScaleTranslate - a class that scales by doubles (horizontal and vertical)
		then adds in an offset.  It is completely inline.
	
-----------------------------------------------------------------------------*/
// get basic types along with OS define
#include "gtype.h"
// include windows.h
#ifdef MSWIN
#define STRICT
#include <windows.h>
#endif
// we'll need this for ScaleTranslate
#include "ggeomtry.h"
// G42DrawLocation - place to draw
#ifdef MSWIN
typedef HDC G42DrawLocation;
#endif
#ifdef MACOS
typedef GrafPtr G42DrawLocation;
#endif
// G42Surface - basically a window or pane
#ifdef MSWIN
typedef HWND G42Surface;
#endif
#ifdef MACOS
typedef GrafPtr G42Surface;
#endif
// G42Palette - a screen palette
#ifdef MSWIN
typedef HPALETTE G42Palette;
#endif
#ifdef MACOS
typedef int G42Palette; // temporary until Mac starts using palettes
#endif
class G42DrawLocationVerifier
{
public:
	G42DrawLocationVerifier(G42DrawLocation original, G42Surface surface,
		G42Palette pal, bool force_palette = false)
	:
		Location(original), Surface(surface), Pal(pal), OldPal(0),
		NeedDelete(false)
	{
		#ifdef MSWIN
		if (!Location)
		{
			Location = GetDC(Surface);
			NeedDelete = true;
		}
		if (Pal && (NeedDelete || force_palette))
		{
			OldPal = (HPALETTE)SelectPalette(Location, Pal, false);
			RealizePalette(Location);
		}
		#endif
		#ifdef MACOS
		if (force_palette)
		{
		
		}
		#endif
	}
	void Clear(void)
	{
		#ifdef MSWIN
		if (OldPal)
		{
			SelectPalette(Location, OldPal, false);
			RealizePalette(Location);
		}
		if (NeedDelete)
		{
			ReleaseDC(Surface, Location);
		}
		#endif
	}
	~G42DrawLocationVerifier(void)
	{
		Clear();
	}
	operator G42DrawLocation (void) const
		{ return Location; }
private:
	G42DrawLocation Location;
	bool NeedDelete;
	G42Palette Pal;
	G42Palette OldPal;
	G42Surface Surface;
};
// a simple structure to hold a color
struct G42Color
{
public:
	byte Red;
	byte Green;
	byte Blue;
	G42Color() {}
	G42Color(byte red, byte green, byte blue) :
		Red(red), Green(green), Blue(blue) {}
	G42Color(const G42Color & ref)
		: Red(ref.Red), Green(ref.Green), Blue(ref.Blue) {}
//	~G42Color() {}
#ifdef MACOS
	G42Color (const RGBColor & color);
	operator RGBColor (void) const;
#endif
	bool operator == (const G42Color & ref)
		{ return Red == ref.Red && Green == ref.Green && Blue == ref.Blue; }
	G42Color & operator = (const G42Color & ref)
		{ Red = ref.Red; Green = ref.Green; Blue = ref.Blue; return *this; }
};
#ifdef MACOS
inline G42Color::G42Color (const RGBColor & color) 
{
	Red = color.red >> 8;
	Green = color.green >> 8;
	Blue = color.blue >> 8;
}
inline G42Color::operator RGBColor (void) const 
{
	RGBColor color;
	color.red = Red << 8;
	color.green = Green << 8;
	color.blue = Blue << 8;
	return color;
}
#endif
// a structure to hold colors from 0 to 65535 - used in gilab.cpp for one place
struct G42Color16
{
public:
	uint16 Red;
	uint16 Green;
	uint16 Blue;
	G42Color16() : Red(0), Green(0), Blue(0) {}
	G42Color16(uint16 r, uint16 g, uint16 b) : Red(r), Green(g), Blue(b) {}
	G42Color16(const G42Color & c) :
		Red((uint16)((uint16)c.Red | ((uint16)c.Red << 8))),
		Green((uint16)((uint16)c.Green | ((uint16)c.Green << 8))),
		Blue((uint16)((uint16)c.Blue | ((uint16)c.Blue << 8))) {}
	G42Color16(const G42Color16 & c) :
		Red((uint16)c.Red), Green((uint16)c.Green), Blue((uint16)c.Blue) {}
	operator G42Color (void) { return G42Color((byte)(Red >> 8),
		(byte)(Green >> 8), (byte)(Blue >> 8)); }
	G42Color16 & operator = (const G42Color16 & c)
		{ Red = c.Red; Green = c.Green; Blue = c.Blue; return *this; }
	G42Color16 & operator = (const G42Color & c)
		{ Red = (uint16)((uint16)c.Red | ((uint16)c.Red << 8));
		Red = (uint16)((uint16)c.Green | ((uint16)c.Green << 8));
		Red = (uint16)((uint16)c.Blue | ((uint16)c.Blue << 8)); return *this; }
};
// a simple structure to hold margins
struct G42Margin
{
	int32 Left;
	int32 Top;
	int32 Right;
	int32 Bottom;
	G42Margin() : Left(0), Top(0), Right(0), Bottom(0) {}
	G42Margin(int32 left, int32 top, int32 right, int32 bottom) :
		Left(left), Top(top), Right(right), Bottom(bottom) {}
};
// A class to do transformations from one coordinate system to another.
// Note that the scaling is done first, then the translate.  Also, if
// a rectangle is given, it converts Right + 1 and Bottom + 1
class G42ScaleTranslate
{
public:
	G42ScaleTranslate(double ScaleX, double ScaleY, const G42Point & Translate);
		// throw(G42InvalidParameterException)
	G42ScaleTranslate(double Scale, const G42Point & Translate);
		// throw(G42InvalidParameterException)
	G42ScaleTranslate(double ScaleX, double ScaleY);
		// throw(G42InvalidParameterException)
	G42ScaleTranslate(double Scale);
		// throw(G42InvalidParameterException)
	G42ScaleTranslate(const G42Point & Translate);
	G42ScaleTranslate();
	~G42ScaleTranslate();
	void ConvertX(long & x) const;
	void ConvertY(long & y) const;
	void ConvertX(int & x) const;
	void ConvertY(int & y) const;
	void ConvertX(signed char & x) const;
	void ConvertY(signed char & y) const;
	void ConvertX(unsigned long & x) const;
	void ConvertY(unsigned long & y) const;
	void ConvertX(unsigned int & x) const;
	void ConvertY(unsigned int & y) const;
	void ConvertX(unsigned char & x) const;
	void ConvertY(unsigned char & y) const;
	void ConvertBackX(long & x) const;
	void ConvertBackY(long & y) const;
	void ConvertBackX(int & x) const;
	void ConvertBackY(int & y) const;
	void ConvertBackX(signed char & x) const;
	void ConvertBackY(signed char & y) const;
	void ConvertBackX(unsigned long & x) const;
	void ConvertBackY(unsigned long & y) const;
	void ConvertBackX(unsigned int & x) const;
	void ConvertBackY(unsigned int & y) const;
	void ConvertBackX(unsigned char & x) const;
	void ConvertBackY(unsigned char & y) const;
	void Convert(G42Point & p) const;
	void ConvertBack(G42Point & p) const;
	void Convert(G42Rect & p) const;
	void ConvertBack(G42Rect & p) const;
	G42Point GetTranslate(void) const
		{ return G42Point(Translate); }
	double GetScaleX(void) const
		{ return ScaleX; }
	double GetScaleY(void) const
		{ return ScaleY; }
private:
	double ScaleX;
	double ScaleY;
	G42Point Translate;
};
// *** Peter I had to move some functions to the top of the list. D.I.
inline void
G42ScaleTranslate::ConvertX(long & x) const
{
	double xd = (double)x * ScaleX;
	x = (long)(Translate.X + (int32)(xd /* + (xd >= 0 ? .5 : - .5)) */ ));
}
inline void
G42ScaleTranslate::ConvertY(long & y) const
{
	double yd = (double)y * ScaleY;
	y = (long)(Translate.Y + (int32)(yd /* + (yd >= 0 ? .5 : - .5)) */ ));
}
inline void
G42ScaleTranslate::ConvertX(int & x) const
{
	double xd = (double)x * ScaleX;
	x = (int)(Translate.X + (int32)(xd /* + (xd >= 0 ? .5 : - .5)) */ ));
}
inline void
G42ScaleTranslate::ConvertY(int & y) const
{
	double yd = (double)y * ScaleY;
	y = (int)(Translate.Y + (int32)(yd /* + (yd >= 0 ? .5 : - .5)) */ ));
}
inline void
G42ScaleTranslate::ConvertX(signed char & x) const
{
	double xd = (double)x * ScaleX;
	x = (signed char)(Translate.X + (int32)(xd /* + (xd >= 0 ? .5 : - .5)) */ ));
}
inline void
G42ScaleTranslate::ConvertY(signed char & y) const
{
	double yd = (double)y * ScaleY;
	y = (signed char)(Translate.Y + (int32)(yd /* + (yd >= 0 ? .5 : - .5)) */ ));
}
inline void
G42ScaleTranslate::ConvertX(unsigned long & x) const
{
	double xd = (double)x * ScaleX;
	x = (unsigned long)(Translate.X + (int32)(xd /* + (xd >= 0 ? .5 : - .5)) */ ));
}
inline void
G42ScaleTranslate::ConvertY(unsigned long & y) const
{
	double yd = (double)y * ScaleY;
	y = (unsigned long)(Translate.Y + (int32)(yd /* + (yd >= 0 ? .5 : - .5)) */ ));
}
inline void
G42ScaleTranslate::ConvertX(unsigned int & x) const
{
	double xd = (double)x * ScaleX;
	x = (unsigned int)(Translate.X + (int32)(xd /* + (xd >= 0 ? .5 : - .5)) */ ));
}
inline void
G42ScaleTranslate::ConvertY(unsigned int & y) const
{
	double yd = (double)y * ScaleY;
	y = (unsigned int)(Translate.Y + (int32)(yd /* + (yd >= 0 ? .5 : - .5)) */ ));
}
inline void
G42ScaleTranslate::ConvertX(unsigned char & x) const
{
	double xd = (double)x * ScaleX;
	x = (unsigned char)(Translate.X + (int32)(xd /* + (xd >= 0 ? .5 : - .5)) */ ));
}
inline void
G42ScaleTranslate::ConvertY(unsigned char & y) const
{
	double yd = (double)y * ScaleY;
	y = (unsigned char)(Translate.Y + (int32)(yd /* + (yd >= 0 ? .5 : - .5)) */ ));
}
inline void
G42ScaleTranslate::ConvertBackX(long & x) const
{
	double xd = (double)(x - Translate.X) / ScaleX;
	x = (long)((int32)(xd /* + (xd >= 0 ? .5 : - .5)) */ ));
}
inline void
G42ScaleTranslate::ConvertBackY(long & y) const
{
	double yd = (double)(y - Translate.Y) / ScaleY;
	y = (long)((int32)(yd /* + (yd >= 0 ? .5 : - .5)) */ ));
}
inline void
G42ScaleTranslate::ConvertBackX(int & x) const
{
	double xd = (double)(x - Translate.X) / ScaleX;
	x = (int)((int32)(xd /* + (xd >= 0 ? .5 : - .5)) */ ));
}
inline void
G42ScaleTranslate::ConvertBackY(int & y) const
{
	double yd = (double)(y - Translate.Y) / ScaleY;
	y = (int)((int32)(yd /* + (yd >= 0 ? .5 : - .5)) */ ));
}
inline void
G42ScaleTranslate::ConvertBackX(signed char & x) const
{
	double xd = (double)(x - Translate.X) / ScaleX;
	x = (signed char)((int32)(xd /* + (xd >= 0 ? .5 : - .5)) */ ));
}
inline void
G42ScaleTranslate::ConvertBackY(signed char & y) const
{
	double yd = (double)(y - Translate.Y) / ScaleY;
	y = (signed char)((int32)(yd /* + (yd >= 0 ? .5 : - .5)) */ ));
}
inline void
G42ScaleTranslate::ConvertBackX(unsigned long & x) const
{
	double xd = (double)(x - Translate.X) / ScaleX;
	x = (unsigned long)((int32)(xd /* + (xd >= 0 ? .5 : - .5)) */ ));
}
inline void
G42ScaleTranslate::ConvertBackY(unsigned long & y) const
{
	double yd = (double)(y - Translate.Y) / ScaleY;
	y = (unsigned long)((int32)(yd /* + (yd >= 0 ? .5 : - .5)) */ ));
}
inline void
G42ScaleTranslate::ConvertBackX(unsigned int & x) const
{
	double xd = (double)(x - Translate.X) / ScaleX;
	x = (unsigned int)((int32)(xd /* + (xd >= 0 ? .5 : - .5)) */ ));
}
inline void
G42ScaleTranslate::ConvertBackY(unsigned int & y) const
{
	double yd = (double)(y - Translate.Y) / ScaleY;
	y = (unsigned int)((int32)(yd /* + (yd >= 0 ? .5 : - .5)) */ ));
}
inline void
G42ScaleTranslate::ConvertBackX(unsigned char & x) const
{
	double xd = (double)(x - Translate.X) / ScaleX;
	x = (unsigned char)((int32)(xd /* + (xd >= 0 ? .5 : - .5)) */ ));
}
inline void
G42ScaleTranslate::ConvertBackY(unsigned char & y) const
{
	double yd = (double)(y - Translate.Y) / ScaleY;
	y = (unsigned char)((int32)(yd /* + (yd >= 0 ? .5 : - .5)) */ ));
}
/***  End of functions moved to top  ***/
inline
G42ScaleTranslate::G42ScaleTranslate(double scale_x, double scale_y,
	const G42Point & translate)
:
	ScaleX(scale_x), ScaleY(scale_y), Translate(translate)
{
	if (scale_x == 0.0 || scale_y == 0.0)
		throw G42InvalidParameterException();
}
inline
G42ScaleTranslate::G42ScaleTranslate(double scale, const G42Point & translate)
:
	ScaleX(scale), ScaleY(scale), Translate(translate)
{
	if (scale == 0.0)
		throw G42InvalidParameterException();
}
inline
G42ScaleTranslate::G42ScaleTranslate(double scale_x, double scale_y)
:
	ScaleX(scale_x), ScaleY(scale_y), Translate(0, 0)
{
	if (scale_x == 0.0 || scale_y == 0.0)
		throw G42InvalidParameterException();
}
inline
G42ScaleTranslate::G42ScaleTranslate(double scale)
:
	ScaleX(scale), ScaleY(scale), Translate(0, 0)
{
	if (scale == 0.0)
		throw G42InvalidParameterException();
}
inline
G42ScaleTranslate::G42ScaleTranslate(const G42Point & translate)
:
	ScaleX(1), ScaleY(1), Translate(translate)
{
}
inline
G42ScaleTranslate::G42ScaleTranslate()
:
	ScaleX(1), ScaleY(1), Translate(0, 0)
{
}
inline
G42ScaleTranslate::~G42ScaleTranslate()
{
}
inline void
G42ScaleTranslate::Convert(G42Point & p) const
{
	ConvertX(p.X);
	ConvertY(p.Y);
}
inline void
G42ScaleTranslate::ConvertBack(G42Point & p) const
{
	ConvertBackX(p.X);
	ConvertBackY(p.Y);
}
inline void
G42ScaleTranslate::Convert(G42Rect & r) const
{
	ConvertX(r.Left);
	ConvertY(r.Top);
	r.Right++;
	r.Bottom++;
	ConvertX(r.Right);
	ConvertY(r.Bottom);
	r.Right--;
	r.Bottom--;
}
inline void
G42ScaleTranslate::ConvertBack(G42Rect & r) const
{
	ConvertBackX(r.Left);
	ConvertBackY(r.Top);
	r.Right++;
	r.Bottom++;
	ConvertBackX(r.Right);
	ConvertBackY(r.Bottom);
	r.Right--;
	r.Bottom--;
}
#endif // GITYPE_H
