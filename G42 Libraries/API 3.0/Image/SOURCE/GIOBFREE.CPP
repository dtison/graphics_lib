#include "giobfree.h"
void
G42FreehandObject::DrawGrafObject (G42DrawLocation location)
{
	DrawPoints (location);
	if (Hot)
	{
		/*  Want to draw with a 2-wide solid line */
		G42WideLineStyle wide (location);
		DrawRectangle (Bounds, location);
	}
}
G42FreehandObject::~G42FreehandObject (void)
{
	#ifdef MSWIN
	if (Location)
	{
		ReleaseDC(Surface, Location);
		Location = 0;
	}
	#endif
}
void
G42FreehandObject::DrawFreehand (G42DrawLocation location)
{
	if (NumPoints < 2)
		return;
		
	#ifdef MACOS
	/*  TODO:  Setup Solid line  */
	#endif
	for (int16 i = 0; i < NumPoints - 1; i++)
	{
		G42Point point1 = Points [i];
		G42Point point2 = Points [i + 1];
		ConvertPoint (point1, true);
		ConvertPoint (point2, true);
		#ifdef MSWIN
		G42Rect viewport = AreaController -> GetDisplay() -> GetViewport();
		point1.Offset (-viewport.Left, -viewport.Top);
		point2.Offset (-viewport.Left, -viewport.Top);
		MoveToEx (location, point1.X, point1.Y, NULL);
		LineTo (location, point2.X, point2.Y);
		#endif
		#ifdef MACOS
		::MoveTo (point1.X, point1.Y);
		::LineTo (point2.X, point2.Y);
		#endif
	}
	if (Defined)
	{
		G42Point point = Points [0];
		ConvertPoint (point, true);
		#ifdef MSWIN
		G42Rect viewport = AreaController -> GetDisplay() -> GetViewport();
		point.Offset (-viewport.Left, -viewport.Top);
		LineTo (location, point.X, point.Y);
		#endif
		#ifdef MACOS
		::LineTo (point.X, point.Y);
		#endif
	}
	#ifdef MACOS
	/*  Quiet warnings  */
	if (location) {}
	#endif
}
#ifdef NOTYET
/*  Allows caller to initialize GrafObject instead of having to use mouse  */
void
G42FreehandObject::InitObject (const G42Vector <G42Point>  & points, int16 count,
	const G42Rect & area_bounds)
{
	if (count <= 0) {
		throw (G42InvalidParameterException (0));
	}
	AreaBounds	 		= area_bounds;
	Origin 				= points [0];
	NumGrabbers 		= NumPoints = count;
	NumDrawGrabbers 	= NumPoints;
	for (int16 i = 0; i < NumPoints; i++)
		Points [i] = points [i];
	PlaceGrabbers();
	Defined = true;
	/*  Refresh Bounds Rectangles */
	UpdateBounds();
}
#endif
void
G42FreehandObject::MouseDown (G42MouseDown *message)
{
	G42Point point (message -> Point);
	ConvertPoint (point, false);
	ClipPointToRect (point, AreaBounds);	
	
	if (! Defined)
	{
		/*  Create a new object  */
		G42Rect test_rect(point.X, point.Y, point.X, point.Y);
		int32 xclip = 0, yclip = 0;
		ClipRectToRect (test_rect, AreaBounds, xclip, yclip);
		point.Offset (xclip, yclip);
		Origin 	= point;
		Bounds.Set (point.X, point.Y, point.X, point.Y);	// Have to set Bounds for Draw to work
		/*  Store the point  */
		Points [NumPoints] = point;
		NumPoints++;
		Origin = point;
		Modified = true;
		/*  Move there  */
		#ifdef MSWIN
		G42Rect viewport = AreaController -> GetDisplay() -> GetViewport();
		Location = GetDC(Surface);
		SetROP2(Location, R2_NOT);
		::MoveToEx (Location, point.X - viewport.Left, point.Y - viewport.Top, 0);
		#endif
		#ifdef MACOS
		::MoveTo (point.X, point.Y);
		#endif
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
G42FreehandObject::MouseMove (G42MouseMove *message)
{
	if (! Defined)
	{
		G42Point point (message -> Point);
		ClipPointToRect (point, AreaBounds);
		ConvertPoint (point, false);
		/*  Store the point  */
		Points [NumPoints] = point;
		NumPoints++;
		/*  Draw to point  */
		#ifdef MSWIN
		G42Rect viewport = AreaController -> GetDisplay() -> GetViewport();
		::LineTo (Location, point.X - viewport.Left, point.Y - viewport.Top);
		#endif
		#ifdef MACOS
		::LineTo (point.X, point.Y);
		#endif
	}
	G42PointsObject::MouseMove (message);
}
void
G42FreehandObject::MouseUp (G42MouseUp *  /*  message  */)
{
	#ifdef MSWIN
	if (Location)
	{
		ReleaseDC(Surface, Location);
		Location = 0;
	}
	#endif
	if (MousemoveMode != None) {
		MousemoveMode = None;
	}
	/*  Always do these on mouseup; simplifies things  */
	Highlighted = false;
//	Draw();
	/*  Refresh Bounds Rectangles */
	UpdateBounds();
	/*  GrafObjects are no longer modified after MouseUp  */
	Modified = false;
	Defined = true;
	AreaController -> Draw();		// TODO:  Draw this way or some other?
}
