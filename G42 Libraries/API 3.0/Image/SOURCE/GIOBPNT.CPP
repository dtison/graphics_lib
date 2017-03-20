//  giobpnt.h - G42PointObject
	
#define RECT_SIZE 20
#define RECT_SIZE_HALF 10
#include "giobpnt.h"
#include "gidsetup.h"
G42PointObject::G42PointObject (G42AreaController *acontroller, 
	G42MouseKeyController *mcontroller) 
: 
	G42GrafObject (acontroller, mcontroller)
	#ifdef ACCELERATE_KEYS
	KeyHeldDown (false), KeyDownTicks(0)
	#endif
{
	Origin.X = Origin.Y = 0;
	MoveOrigin.X = MoveOrigin.Y = 0;
	/*  Inherited from G42GrafObject  		 */
	XORDraw = true;
}
void
G42PointObject::DrawGrafObject (G42DrawLocation location)
{
	DrawPoint (Bounds, location);
	if (Hot)
	{
		/*  Want to draw with a 2-wide solid line */
		G42WideLineStyle wide (location);
		DrawRectangle (Bounds, location);
	}
	/*  (No grabbers on point GrafObjects)  */
}
void
G42PointObject::DrawPoint (G42Rect &rect, G42DrawLocation location)
{
	const G42ScaleTranslate current_transform = Display -> GetScaleTranslate();
	G42Rect rectg (Origin.X - 2, Origin.Y - 2, Origin.X + 2, Origin.Y + 2);
	current_transform.Convert (rectg);
	
	/*  Bounds rectangle  */
	#ifdef MSWIN
	G42Rect viewport = AreaController -> GetDisplay() -> GetViewport();
	rectg.Offset (-viewport.Left, -viewport.Top);
	::Rectangle (location, rectg.Left, rectg.Top, rectg.Right, rectg.Bottom);
	#endif
	#ifdef MACOS
	#ifdef NOTYET
	/*  Setup Solid line  */
	Pattern pattern;
	::GetIndPattern (&pattern, sysPatListID, 1);
	::PenPat (&pattern);
	#endif
	Rect rectm = rectg;
	::FrameRect (&rectm);
	#endif
	
	/*  Cross */
	#ifdef NOTYET
	::GetIndPattern (&pattern, sysPatListID, 4);
	::PenPat (&pattern);
	#endif
 	rectg  = rect;
	/*  Setup cross points  */
	G42Point points [4];
	points [0] = G42Point (rectg.Left + RECT_SIZE_HALF, rectg.Top);
	points [1] = G42Point (rectg.Left + RECT_SIZE_HALF, rectg.Bottom);
	points [2] = G42Point (rectg.Left, rectg.Top + RECT_SIZE_HALF);
	points [3] = G42Point (rectg.Right, rectg.Top + RECT_SIZE_HALF);
	for (int8 i = 0; i < 4; i++)
	{
		current_transform.Convert (points [i]);
		#ifdef MSWIN
		points [i].Offset (-viewport.Left, -viewport.Top);
		#endif
	}
	#if 0
	/*  Setup rectg  */
	rectg = rect;
 	current_transform.Convert (rectg);
	#endif
	#ifdef NOMSWIN
	rectg.Offset (-viewport.Left, -viewport.Top);
	MoveToEx (location, rectg.Left + RECT_SIZE_HALF, rectg.Top, NULL);
	LineTo (location, rectg.Left + RECT_SIZE_HALF, rectg.Bottom);
	MoveToEx (location, rectg.Left, rectg.Top + RECT_SIZE_HALF, NULL);
	LineTo (location, rectg.Right, rectg.Top + RECT_SIZE_HALF);
	#endif
	#ifdef MSWIN
	MoveToEx (location,points [0].X, points [0].Y, NULL);
	LineTo (location, points [1].X, points [1].Y);
	MoveToEx (location, points [2].X, points [2].Y, NULL);
	LineTo (location, points [3].X, points [3].Y);
	#endif
	#ifdef MACOS
	::MoveTo (points [0].X, points [0].Y); 
	::LineTo (points [1].X, points [1].Y);
	::MoveTo (points [2].X, points [2].Y); 
	::LineTo (points [3].X, points [3].Y);
	/*  Quiet warnings  */
	if (location) {}
	#endif	
}
void
G42PointObject::InitObject (const G42Point &point, const G42Rect & area_bounds)
{
	AreaBounds = area_bounds;
	Origin 	= point;
	Bounds.Set (Origin.X - RECT_SIZE_HALF, Origin.Y - RECT_SIZE_HALF,
		Origin.X + RECT_SIZE_HALF, Origin.Y + RECT_SIZE_HALF);
	ResponseBounds = Bounds;
	Defined = true;
}
#if 0
void
G42PointObject::KeyDown (G42KeyDown *message)
{
	if (! Defined)
		return;
	int32 dx = 0;
	int32 dy = 0;
	if (HandleArrowKeys (message, Rectangle, dx, dy))
	{
		Draw();	// <Erase>
		Rectangle.Offset (dx, dy);
		Origin.Offset (dx, dy);
		Draw();
		/*  Refresh Bounds rectangles*/
		Bounds = ResponseBounds = Rectangle;
	}
}
#endif
void
G42PointObject::KeyDown (G42KeyDown *message)
{
	if (! Defined)
		return;
	int32 dx = 0;
	int32 dy = 0;
	if (HandleArrowKeys (message, Bounds, dx, dy))
	{
		/*  Erase  */
		G42Rect rectg = ResponseBounds.InflatedBy (3, 3); 
		Display -> GetScaleTranslate().Convert (rectg);
		AreaController -> Draw (rectg, 0, true);
		/*  Move & redraw  */
		Bounds.Offset (dx, dy);
		Origin.Offset (dx, dy);
		Draw();
		/*  Refresh Bounds rectangles*/
		ResponseBounds = Bounds;
	}
}
void
G42PointObject::KeyUp (G42KeyUp * /*  message  */)
{
	#ifdef ACCELERATE_KEYS
	KeyHeldDown = false;
	KeyDownTicks = 0;
	#endif
}
void
G42PointObject::MouseDown (G42MouseDown *message)
{
	G42Point point (message -> Point);
	ConvertPoint (point, false);
	if (! Defined)
	{
		/*  Create a new object  */
		Origin 	= point;
		Bounds.Set (Origin.X - RECT_SIZE_HALF, Origin.Y - RECT_SIZE_HALF, 
			Origin.X + RECT_SIZE_HALF, Origin.Y + RECT_SIZE_HALF);
		Draw();
	}
	else
	{
		/*  Want to modify the object  */
		if (Bounds.Contains (point))
		{
			MousemoveMode = Moving;
			MoveOrigin = point;			
			RedrawHighlighted();
		}
	}
}
void
G42PointObject::MouseMove (G42MouseMove *message)
{
	/*  Be sure point is in current canvas.  */
	G42Point point (message -> Point);
	ClipPointToRect (point, AreaBounds);	
	ConvertPoint (point, false);
	
	/*  Then erase current rectangle  */
	if (! Bounds.IsNull()) {
		Draw();	// <Erase>
	}
	
	bool needs_drawn = false;	// Every mousemove does not need to be drawn 
	if (! Defined)
	{
		Origin 	= point;
	 	Bounds.Set (Origin.X - RECT_SIZE_HALF, Origin.Y - RECT_SIZE_HALF, 
			Origin.X + RECT_SIZE_HALF, Origin.Y + RECT_SIZE_HALF);
		needs_drawn = true;
	}
	else
	{
		if (MousemoveMode == Moving)
		{
			/*  First calculate the offset to try to move  */
			int32 dx = point.X - MoveOrigin.X;	
			int32 dy = point.Y - MoveOrigin.Y;	
			/*  But test for off-canvas moves  */
			G42Rect test_rect = Bounds;
			test_rect.Offset (dx, dy);
			/*  xclip and yclip will tell us how much test_rect is out of bounds  */
			int32 xclip = 0, yclip = 0;
			ClipRectToRect (test_rect, AreaBounds, xclip, yclip);
			point.Offset (xclip, yclip);	
			dx += xclip;
			dy += yclip;
			MoveOrigin 	= point;
			/*  Now apply it to the object  */
			Bounds.Offset (dx, dy);
			Origin.Offset (dx, dy);
			needs_drawn = true;
		}
	}
	/*  Now draw whatever the rectangle looks like  */
	if (needs_drawn)
	{
		Modified = true;	
		Draw();
	}
}
void
G42PointObject::MouseUp (G42MouseUp *  /*  message  */)
{
	Draw(); // <erase>
	if (! Defined) 
	{
		Defined = true;
	}
	else
	{
		if (MousemoveMode != None)
		{
			Bounds.Normalize();	
			MousemoveMode = None;
		}
	}
	/*  Always do these on mouseup; simplifies things  */
	Highlighted = false;
	Draw();	
	/*  Refresh Bounds Rectangles */
	ResponseBounds = Bounds;
	/*  GrafObjects are no longer modified after MouseUp  */
	Modified = false;
}
