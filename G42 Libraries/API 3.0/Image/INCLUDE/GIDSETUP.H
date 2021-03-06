// gidsetup.h - drawing setup classes
#if !defined (GIDSETUP_H)
#define GIDSETUP_H
#include "gitype.h"
class G42FillStyle
{
public:
	enum G42Style { Solid, None, Horizontal, Vertical, NESW, NWSE, CrossHatch,
		DiagCrossHatch };
	G42FillStyle(G42DrawLocation location) // none only
	:
		#ifdef MSWIN
		NeedDelete(false), Brush(0), NeedOldBkMode(false), NeedOldBkColor(false),
		#endif
		Location(location)
	{
		#ifdef MSWIN
		OldBrush = (HBRUSH)SelectObject(Location, GetStockObject(NULL_BRUSH));
		#endif
		#ifdef MACOS
		#endif
	}
	G42FillStyle(G42DrawLocation location, const G42Color & forground) // solid only
	:
		#ifdef MSWIN
		NeedDelete(true), Brush(0), NeedOldBkMode(false), NeedOldBkColor(false),
		#endif
		Location(location)
	{
		#ifdef MSWIN
		Brush = CreateSolidBrush(
			RGB(forground.Red, forground.Green, forground.Blue));
		OldBrush = (HBRUSH)SelectObject(Location, Brush);
		#endif
		#ifdef MACOS
		::RGBForeColor (&((RGBColor) forground));
		#endif
	}
	G42FillStyle(G42DrawLocation location, const G42Color & forground,
		G42Style style)
	:
		#ifdef MSWIN
		NeedDelete(true), Brush(0), NeedOldBkMode(true), NeedOldBkColor(false),
		#endif
		Location(location)
	{
		#ifdef MSWIN
		OldBkMode = SetBkMode(location, TRANSPARENT);
		#endif
		#ifdef MACOS
		::RGBForeColor (&((RGBColor) forground));
		#endif
		SetStyle(forground, style);
	}
	G42FillStyle(G42DrawLocation location, const G42Color & forground,
		const G42Color & background, G42Style style)
	:
		#ifdef MSWIN
		NeedDelete(true), Brush(0), NeedOldBkMode(true), NeedOldBkColor(true),
		#endif
		Location(location)
	{
		#ifdef MSWIN
		OldBkMode = SetBkMode(location, OPAQUE);
		OldBkColor = SetBkColor(location,
			RGB(background.Red, background.Green, background.Blue));
		SetStyle(forground, style);
		#endif
		#ifdef MACOS
		if (forground.Red) {};	// QUIET WARNINGS
		if (background.Red) {};
		if (style) {}
		#endif
	}
	void SetStyle(const G42Color & c, G42Style s)
	{
		#ifdef MSWIN
		if (s == None)
		{
			NeedDelete = false;
			OldBrush = (HBRUSH)SelectObject(Location, GetStockObject(NULL_BRUSH));
		}
		else if (s == Solid)
		{
			Brush = CreateSolidBrush(
				RGB(c.Red, c.Green, c.Blue));
			OldBrush = (HBRUSH)SelectObject(Location, Brush);
		}
		else
		{
			LOGBRUSH b;
			b.lbStyle = BS_HATCHED;
			b.lbColor = RGB(c.Red, c.Green, c.Blue);
			if (s == Horizontal)
			{
				b.lbHatch = HS_HORIZONTAL;
			}
			else if (s == Vertical)
			{
				b.lbHatch = HS_VERTICAL;
			}
			else if (s == NESW)
			{
				b.lbHatch = HS_BDIAGONAL;
			}
			else if (s == NWSE)
			{
				b.lbHatch = HS_FDIAGONAL;
			}
			else if (s == CrossHatch)
			{
				b.lbHatch = HS_CROSS;
			}
			else
			{
				b.lbHatch = HS_DIAGCROSS;
			}
			Brush = CreateBrushIndirect(&b);
			OldBrush = (HBRUSH)SelectObject(Location, Brush);
		}
		#endif
		#ifdef MACOS
		if (c.Red) {}
		if (s) {}
		#endif
	}
	void Clear(void) // removes style before destructor
	{
		#ifdef MSWIN
		SelectObject(Location, OldBrush);
		if (NeedDelete)
		{
			DeleteObject(Brush);
			NeedDelete = false;
		}
		if (NeedOldBkMode)
		{
			SetBkMode(Location, OldBkMode);
		}
		if (NeedOldBkColor)
		{
			SetBkMode(Location, OldBkColor);
		}
		#endif
		#ifdef MACOS
		#endif
	}
	~G42FillStyle(void)
		{ Clear(); }
protected:
	G42DrawLocation Location;
	#ifdef MSWIN
	HBRUSH Brush;
	HBRUSH OldBrush;
	bool NeedDelete;
	bool NeedOldBkMode;
	int OldBkMode;
	bool NeedOldBkColor;
	DWORD OldBkColor;
	#endif
};
class G42DashedLineStyle
{
public:
	G42DashedLineStyle (G42DrawLocation location)
	:
	#ifdef MSWIN
	NeedDelete(true), OldBkMode (0), OldPen(0), Pen(0),
	#endif
	Location (location)
	{
		#ifdef MSWIN
		Pen = CreatePen (PS_DOT, 0, RGB (0, 0, 0));
		OldPen = (HPEN)SelectObject (Location, Pen);
		OldBkMode = SetBkMode(Location, TRANSPARENT);
		#endif
		#ifdef MACOS
		Pattern pattern;
		::GetIndPattern (&pattern, sysPatListID, 4);
		::PenPat (&pattern);		
		#endif
	}
	void Clear(void) // removes style before destructor
	{
		#ifdef MSWIN
		if (NeedDelete)
		{
			SelectObject(Location, OldPen);
			DeleteObject(Pen);
			SetBkMode(Location, OldBkMode);
			NeedDelete = false;
		}
		#endif
		#ifdef MACOS
		Pattern pattern;
		::GetIndPattern (&pattern, sysPatListID, 1);
		::PenPat (&pattern);		
		#endif
	}
	~G42DashedLineStyle(void)
		{ Clear(); }
protected:
	/*  (Location isn't used on the Mac)  */
	G42DrawLocation	Location;
	#ifdef MSWIN
	bool 					NeedDelete;
	int 					OldBkMode;
	HPEN 					OldPen;
	HPEN 					Pen;
	#endif
};
class G42WideLineStyle
{
public:
	G42WideLineStyle (G42DrawLocation location)
		:
		#ifdef MSWIN
		NeedDelete(true), Pen(0), OldPen(0),
		#endif
		Location (location)
	{
		#ifdef MSWIN
		Pen = CreatePen (PS_SOLID, 2, RGB (0, 0, 0));
		OldPen = (HPEN)SelectObject (Location, Pen);
		#endif
		#ifdef MACOS
		PenState penState;
		GetPenState (&penState);
		Point point;
		point.h = point.v = 2;
		penState.pnSize = point;
		::SetPenState (&penState);
		#endif
	}
	void Clear(void) // removes style before destructor
	{
		#ifdef MSWIN
		if (NeedDelete)
		{
			SelectObject(Location, OldPen);
			DeleteObject(Pen);
			NeedDelete = false;
		}
		#endif
		#ifdef MACOS
		PenState penState;
		GetPenState (&penState);
		Point point;
		point.h = point.v = 1;
		penState.pnSize = point;
		::SetPenState (&penState);
		#endif
	}
	~G42WideLineStyle(void)
		{ Clear(); }
protected:
	G42DrawLocation 	Location;
	#ifdef MSWIN
	bool 					NeedDelete;
	HPEN 					OldPen;
	HPEN 					Pen;
	#endif
};
class G42XORStyle
{
public:
	G42XORStyle(G42DrawLocation location)
		:
		#ifdef MSWIN
		NeedDelete(true),
		#endif
		Location(location)
	{
		#ifdef MSWIN
		OldRop2 = SetROP2(Location, R2_NOT);
		#endif
		#ifdef MACOS
		PenState penState;
		GetPenState (&penState);
		penState.pnMode = srcXor;
		::SetPenState (&penState);
		#endif
	}
	void Clear(void) // removes style before destructor
	{
		#ifdef MSWIN
		if (NeedDelete)
		{
			SetROP2(Location, OldRop2);
			NeedDelete = false;
		}
		#endif
		#ifdef MACOS
		PenState penState;
		GetPenState (&penState);
		penState.pnMode = srcCopy;
		::SetPenState (&penState);
		#endif
	}
	~G42XORStyle(void)
		{ Clear(); }
protected:
	G42DrawLocation Location;
	#ifdef MSWIN
	int OldRop2;
	bool NeedDelete;
	#endif
};
class G42FrameStyle // null brush, black pen
{
public:
	G42FrameStyle(G42DrawLocation location)
		:
		#ifdef MSWIN
		NeedDelete(true), OldBrush(0), OldPen(0),
		#endif
		Location(location)
	{
		#ifdef MSWIN
		OldPen = (HPEN)SelectObject (Location, GetStockObject(BLACK_PEN));
		OldBrush = (HBRUSH)SelectObject (Location, GetStockObject(NULL_BRUSH));
		#endif
	}
	void Clear(void) // removes style before destructor
	{
		#ifdef MSWIN
		if (NeedDelete)
		{
			SelectObject(Location, OldPen);
			SelectObject(Location, OldBrush);
			NeedDelete = false;
		}
		#endif
	}
	~G42FrameStyle(void)
		{ Clear(); }
protected:
	G42DrawLocation Location;
	#ifdef MSWIN
	HBRUSH OldBrush;
	HPEN OldPen;
	bool NeedDelete;
	#endif
};
#endif // GIDSETUP_H
