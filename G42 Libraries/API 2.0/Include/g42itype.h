// g42itype.h - simple image type definitions
#ifndef G42ITYPE_H
#define G42ITYPE_H
/*************************************************************
	File:          g42itype.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   Basic configuration types for Group42Image Library
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
      Code   Date   Name and Description
***************************************************************/
#include <g42type.h>
// Microsoft Windows
#ifdef MSWIN
#ifdef __TURBOC__ // if borland
#define STRICT
#include <windows.h>
#else // everybody but borland
#include <windows.h>
#include <windowsx.h>
#define farmalloc(size) GlobalAllocPtr(GHND, size)
#define farfree(ptr)    GlobalFreePtr(ptr)
#endif // compiler specific
#endif // MSWIN
// X Windows
#ifdef XWIN
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <sys/types.h>
// my copy of gcc doesn't have min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif // XWIN
#ifdef MACOS	// (m.2) Moved some things to g42type.h
//#define DEBUG
#include <Palettes.h>	// I don't know of a single include to get everything aka windows.h D.I.
#include <Quickdraw.h>	
#include <QDOffscreen.h>
extern "C" {
int unlink(const char *path);	// Instead of having to include all of unistd.h
}
/* ===================== MACROS =====================
 w can be a WindowPtr, a DialogPtr, or a GWorldPtr. In any of these
 cases, it returns the correct "bitmap" to send to CopyBits in the
 source or destination bitmap parameter.
 (w can also be a WindowPtr, DialogPtr, or GWorldPtr for any of these.)  */
#define WINBITMAP(w)		(((GrafPtr)(w))->portBits)
#define WINPORTRECT(w)		((((WindowPeek)(w))->port).portRect)
#define WINDOWWIDTH(w)		(WINPORTRECT(w).right - WINPORTRECT(w).left)
#define WINDOWHEIGHT(w)		(WINPORTRECT(w).bottom - WINPORTRECT(w).top)
#define WINCONTENTRECT(w)	((**((WindowPeek)(w))->contRgn).rgnBBox)
#define WINCONTENTRGN(w)	(((WindowPeek)(w))->contRgn)
#define WINVISIBLERGN(w)	(((WindowPeek)(w))->port.visRgn)
#define WINSTRUCTRECT(w)	((**((WindowPeek)(w))->strucRgn).rgnBBox)
#define WINSTRUCTRGN(w)		(((WindowPeek)(w))->strucRgn)
#define WINUPDATERECT(w)	((**((WindowPeek)(w))->updateRgn).rgnBBox)
#define WINUPDATERGN(w)		(((WindowPeek)(w))->updateRgn)
// This takes a GDHandle (device handle)
#define PIXELDEPTH(x)		((**((**(x)).gdPMap)).pixelSize)
// r is a Rect
#define TOPLEFT(r)			(* (Point*) &(r.top))
#define BOTRIGHT(r)			(* (Point*) &(r.bottom))
#define WINDOW_MACROS	1
#endif		// MACOS
// This class simply removes the complexity of putting ifdef's
// around everything that refers to the palettes of each
// windowing system.  The operator should translate to the
// correct palette silently, so the user should never have
// to deal with it.
class G42Palette
{
	public:
#ifdef MSWIN
		HPALETTE ImagePalette;
		G42Palette(HPALETTE palette) : ImagePalette(palette) {}
		G42Palette() : ImagePalette(0) {}
		operator HPALETTE () {return ImagePalette;}
#endif
#ifdef XWIN
		Colormap ImagePalette;
		G42Palette(Colormap palette) : ImagePalette(palette) {}
		G42Palette() : ImagePalette(0) {}
		operator Colormap() {return ImagePalette;}
#endif
#ifdef MACOS
		PaletteHandle ImagePalette;
		G42Palette(PaletteHandle palette) : ImagePalette(palette) {}
		G42Palette() : ImagePalette(0) {}
		operator PaletteHandle () {return ImagePalette;}
#endif
};
// This class simply removes the complexity of putting ifdef's
// around everything that refers to the bitmaps of each
// windowing system.  The operator should translate to the
// correct bitmaps silently, so the user should never have
// to deal with it.
class G42Bitmap
{
	public:
#ifdef MSWIN
		HBITMAP Bitmap;
		G42Bitmap(HBITMAP bitmap) : Bitmap(bitmap) {}
		G42Bitmap() : Bitmap(0) {}
		operator HBITMAP () {return Bitmap;}
#endif
#ifdef XWIN
		XImage * Bitmap;
		G42Bitmap(XImage * bitmap) : Bitmap(bitmap) {}
		G42Bitmap() : Bitmap(0) {}
		operator XImage * () {return Bitmap;}
#endif
#ifdef MACOS
		GWorldPtr Bitmap;
		G42Bitmap (GWorldPtr bitmap) : Bitmap(bitmap) {}
		G42Bitmap() : Bitmap(0) {}
		operator GWorldPtr () {return Bitmap;}
#endif
};
// This class simply removes the complexity of putting ifdef's
// around everything that needs to get information about the
// display capabilities of each device.  The class should translate
// to and from the information silently, so the user should never
// have to deal with it
class G42Display
{
	public:
#ifdef MSWIN
		HDC DisplayInfo;
		G42Display(HDC display_info) : DisplayInfo(display_info) {}
		G42Display() : DisplayInfo(0) {}
		operator HDC () {return DisplayInfo;}
#endif
#ifdef XWIN
		Display * DisplayInfo;
		G42Display(Display * display_info) : DisplayInfo(display_info) {}
		G42Display() : DisplayInfo(0) {}
		operator Display * () {return DisplayInfo;}
#endif
#ifdef MACOS
		GWorldPtr * DisplayInfo;
		G42Display(GWorldPtr * display_info) : DisplayInfo(display_info) {}
		G42Display() : DisplayInfo(0) {}
		operator GWorldPtr * () {return DisplayInfo;}
#endif
};
// types of images
enum G42ImageType {BmpWin, BmpOS2, EpsL1, EpsL2, Gif, Jpeg, PbmAscii, PbmBinary,
	Pcx, Pict, Png, PsL1, PsL2, Ras, Tga, Tiff, Xwd, Unknown};
// basic color structure used for palettes.
class G42Color
{
	public:
		byte red;
		byte green;
		byte blue;
		G42Color(int r, int g, int b) :
      	red((byte)r), green((byte)g), blue((byte)b) {}
		G42Color(){}
		~G42Color(){}
		bool operator == (G42Color & c)
			{ return (c.red == red && c.green == green && c.blue == blue); }
		bool operator != (G42Color & c)
			{ return (c.red != red || c.green != green || c.blue != blue); }
};
// monochrome or true color
//enum G42PictureType {Monochrome, TrueColor};
enum G42PictureType {G42Monochrome, G42TrueColor};
// types of dithering available
enum G42DitherType {DitherNone, Single, Multiple};
// define to use exceptins instead of setjmp/longjmp
// note: the library itself may not throw an exception, but
// some of the code needs to do exceptions or longjmp, and
// this define controls which one it uses
// #define G42_USE_EXCEPTIONS
enum G42ButtonType{Raised = 0, Inverted = 1};
enum G42NewImageType{ImageNew = 0, ImagePaste = 1, ImageButtonBar = 3};
enum G42MethodType{MethodDither = 0, MethodMatching = 1};
enum G42PaletteType{Color = 0, Gray = 1, WinColor = 2};
enum G42JPEGStyle{StylePhoto = 0, StyleLineArt = 1};
const int HighColor = 257;
const int TrueColor = 258;
const int None = 0;
const int General = 1; // used for everything but Tiff, which uses the ones below
const int RLE = 1; // on Tiff, this means PackBits
const int CCITT = 2;
const int Group3 = 3;
const int Group4 = 4;
const int LZW = 5;
#endif // G42ITYPE_H
