#pragma once
#include "g42itype.h"
/* ===================== MACROS =====================
 w can be a WindowPtr, a DialogPtr, or a GWorldPtr. In any of these
 cases, it returns the correct "bitmap" to send to CopyBits in the
 source or destination bitmap parameter.
 (w can also be a WindowPtr, DialogPtr, or GWorldPtr for any of these.)  */
//#include <CursorCtl.h>
#ifndef WINDOW_MACROS
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
#endif
// Miscellaneous MAC stuff
#if 0
extern void Msg (Str255 msg);
extern void ErrMsgCode (Str255 msg, short code);
extern void ErrMsg (Str255 msg);
#endif
// Others
class CommCursor
{
public:
	static Boolean SetWaitCursor (void) 
	{
		#if 1
		CursHandle cursHandle = GetCursor (watchCursor); 
		SetCursor (*cursHandle);
		#endif
	//	SpinCursor (1); 
		return (true);
	}
	static Boolean SetStdCursor (void) 
	{
		InitCursor();
		return (false);
	}
};
// Class for handling disparate color types G42Color & RGBColor
//-------------------------------
class CColorOps
{
public:
	CColorOps (RGBColor color) : Color(color) {}
	CColorOps (G42Color color)
	{
		Color.red 	= color.red << 8; 
		Color.green = color.green << 8;
		Color.blue 	= color.blue << 8;
	}
	operator RGBColor () {return Color;}
	operator G42Color ()
		{return G42Color (Color.red >> 8, Color.green >> 8, Color.blue >> 8);}
private:
	RGBColor Color;
};
