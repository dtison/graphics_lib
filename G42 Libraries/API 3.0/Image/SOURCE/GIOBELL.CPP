//  giobell.h - G42EllipseObject
	
#include "giobell.h"
#include "gidsetup.h"
G42Region * 
G42EllipseObject::CreateObjectRegion (void)
{
	/*  TODO:  What about mask parameter?  */
	/*  Normalize Bounds to 0, 0 */
	G42Rect rect = Bounds.OffsetBy (-Bounds.Left, -Bounds.Top);
	G42Region * region = new G42Region (rect, G42Region::Ellipse);
	G42ThrowIfNull (region);
	return region;	
}
/*  Draws private UI  */
void
G42EllipseObject::DrawGrafObject (G42DrawLocation location)
{
	if (Hot)
	{
		/*  Want to draw with a 2-wide solid line */
		G42WideLineStyle wide (location);
		DrawEllipse (Rectangle, location);
	}
	else
	{
		DrawEllipse (Rectangle, location);
	}
}
#include "girotate.h"
void
G42EllipseObject::DrawEllipse (G42Rect &rect, G42DrawLocation location)
{
	G42Rect rectg (rect);
	Display -> GetScaleTranslate().Convert (rectg);
	#ifdef MSWIN
	G42Rect viewport = AreaController -> GetDisplay() -> GetViewport();
	G42Rect rectw = rectg.OffsetBy (-viewport.Left, -viewport.Top);
	::Ellipse (location, rectw.Left, rectw.Top, rectw.Right, rectw.Bottom);
	#endif
	#ifdef MACOS
	Pattern pattern;
	::GetIndPattern (&pattern, sysPatListID, 4);
	::PenPat (&pattern);
	Rect rectm = rectg;
	::FrameOval (&rectm);
	/*  Quiet warnings  */
	if (location) {}
	#endif
	#ifdef ROTATION_SUPPORT	// For MACOS above
	#if 0
	Pattern pattern;
	::GetIndPattern (&pattern, sysPatListID, 1);
	::PenPat (&pattern);
	#endif
	/* Experimental rotation  */
	G42Rotation rotation (Bounds.GetSize(), 10);
	/*  Draw Ellipse  */
	int32 width  = rectg.GetWidth();
	int32 height = rectg.GetHeight();
	if (width == 0 || height == 0) {
		return;
	}
	int32 a = (width >> 1);
	int32 b = (height >> 1);		
	int32 asquared = a * a;
	int32 bsquared = b * b;
	G42Vector <G42Point> points;
	G42Point prev_point (0, 0);
	int16 num_points = 0;
	/*  Ellipse algorithm from Computer Graphics book */
	int32 x = 0;
	int32 y = b;
	points [num_points++] = G42Point (x, y);	
	double d1 = (double) (bsquared - (asquared * b) + (asquared >> 2));
	bool done = false;
	do
	{
		double v1 = (double) asquared * ((double) y - 0.5);	
		double v2 = (double) bsquared * (x + 1);
		if (v1 > v2)
		{
			if (d1 < 0)
			{
				d1 += (double) (bsquared * ((2 * x) + 3));
				x++;
			}
			else
			{
				d1 += (double) (bsquared * ((2 * x) + 3) + asquared * ((-2 * y) + 2));
				x++;
				y--;
			}
			points [num_points++] = G42Point (x, y);
		}
		else {
			done = true;
		}
	} while (! done);	
	double v1 = (double) x + 0.5;
	v1 *= v1;
	v1 *= (double) bsquared;
	double v2 = (double) y - 1;
	v2 *= v2;
	v2 *= (double) asquared;
	double v3 = (double) (asquared * bsquared);
	double d2 = v1 + v2 - v3; 
		
	while (y > 0)
	{
		if (d2 < 0)
		{
			v1 = (double) (bsquared * ((2 * x) + 2));
			v2 = (double) (asquared * ((-2 * y) + 3));
			d2 += (v1 + v2);
			x++;
			y--;
		}
		else
		{
			d2 += (double) (asquared * ((-2 * y) + 3));
			y--;
		}
		points [num_points++] = G42Point (x, y);
	}
	/*  Draw points  */
	int32 x_off = rectg.Left + a;
	int32 y_off = rectg.Top + b;
	for (int16 i = 0; i < num_points; i++)
	{
		G42Point se = points [i];
		G42Point sw (-se.X, se.Y);
		G42Point nw (sw.X, -sw.Y);
		G42Point ne (-nw.X, nw.Y);
		rotation.RotatePoint (se);
		rotation.RotatePoint (sw);
		rotation.RotatePoint (ne);
		rotation.RotatePoint (nw);
		se.Offset (x_off, y_off); 
		sw.Offset (x_off, y_off); 
		nw.Offset (x_off, y_off); 
		ne.Offset (x_off, y_off); 
		::MoveTo (se.X, se.Y);
		::LineTo (se.X, se.Y);
		::MoveTo (sw.X, sw.Y);
		::LineTo (sw.X, sw.Y);
		::MoveTo (ne.X, ne.Y);
		::LineTo (ne.X, ne.Y);
		::MoveTo (nw.X, nw.Y);
		::LineTo (nw.X, nw.Y);
	}
	#endif
}
