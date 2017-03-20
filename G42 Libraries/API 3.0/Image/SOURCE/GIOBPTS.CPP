#include "giobpts.h"
G42PointsObject::G42PointsObject (G42AreaController *acontroller, 
	G42MouseKeyController *mcontroller) 
	: G42GrafObject (acontroller, mcontroller), 
	#ifdef ACCELERATE_KEYS
	KeyHeldDown (false), KeyDownTicks(0), 
	#endif
	NumPoints (0), SelectedGrabber (0)
{
	Origin.X = Origin.Y = 0;
	MoveOrigin.X = MoveOrigin.Y = 0;
	/*  Inherited from G42GrafObject  		 */
	XORDraw = true;
}
G42Region * 
G42PointsObject::CreateObjectRegion (void)
{
	/*  Normalize Bounds to 0, 0 */
	G42Vector <G42Point> points;
	for (int16 i = 0; i < NumPoints; i++)
	{
		points [i] = Points [i].OffsetBy (-Bounds.Left, -Bounds.Top);
	}
	G42Region * region = new G42Region (points, NumPoints);
	G42ThrowIfNull (region);
	return region;	
}
void
G42PointsObject::DrawPoints (G42DrawLocation location)
{
	if (NumPoints < 2)
		return;
		
	#ifdef MACOS
	/*  TODO:  Setup Solid line  */
	#endif
	#ifdef MSWIN
	G42DrawLocationVerifier dc(location,
		AreaController->GetSurfaceController()->GetSurface(),
		AreaController->GetSurfaceController()->GetWindowsPalette());
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
		MoveToEx (dc, point1.X, point1.Y, NULL);
		LineTo (dc, point2.X, point2.Y);
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
G42Point
G42PointsObject::GetPoint (int16 index)
{
	if (index < 0 || index > NumPoints) {
		throw (G42InvalidParameterException (0));
	}
	return Points [index];
}
void
G42PointsObject::KeyDown (G42KeyDown *message)
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
		/*  Refresh Bounds rectangles */
		UpdateBounds();
	}
}
void
G42PointsObject::KeyUp (G42KeyUp * /*  message  */)
{
	#ifdef ACCELERATE_KEYS
	KeyHeldDown = false;
	KeyDownTicks = 0;
	#endif
}
/*  For G42PointsObject, double-click actually defines */
void
G42PointsObject::MouseDoubleClick (G42MouseDoubleClick * /* message */)
{
	if (Defined)
		return;
		
	#if 0
	G42Point point (message -> Point);
	ConvertPoint (point, false);
	Points [NumPoints] = point;
	NumPoints++;
	#endif
	NumDrawGrabbers = NumGrabbers = NumPoints;
	PlaceGrabbers();
	Defined = true;
	AreaController -> Draw();		// TODO:  Draw this way or some other?
}
void
G42PointsObject::MouseDown (G42MouseDown *message)
{	
	G42Point point (message -> Point);
	ConvertPoint (point, false);
	
	if (! Defined)
	{
		/*  Add a new point  */
		Points [NumPoints] = point;
		NumPoints++;
		UpdateBounds();
		Draw();
	}
	else
	{
		/*  First check the grabbers, then the rectangle  */
		if (TestHitGrabbers (SelectedGrabber, point))
		{
			MousemoveMode = Resizing;
			RedrawHighlighted();
		}	
		else
			/*  Want to modify the object  */
			if (Bounds.Contains (point))
			{
				MousemoveMode = Moving;
				MoveOrigin = point;			
				RedrawHighlighted();
			}
	}
	Modified = true;
}
void
G42PointsObject::MouseMove (G42MouseMove *message)
{
	/*  Be sure point is in current canvas.  */
	G42Point point (message -> Point);
	ClipPointToRect (point, AreaBounds);	
	ConvertPoint (point, false);
	
	/*  Then erase current rectangle  */
	if (Defined && ! Bounds.IsNull()) {
		Draw();	// <Erase>
	}
	
	bool needs_drawn = false;	// Every mousemove does not need to be drawn 
	#if 0
	// TODO:  Need rubber bands stuff here
	if (! Defined)
	{
		Rectangle.Set (Origin.X, Origin.Y, point.X, point.Y);
		Rectangle.Normalize();
		if (Proportional)
		{
			G42Rect::G42RectCorner corner = FindCorner (point);
	//		Rectangle.ResizeToAspect (1.0, corner);
			Rectangle.MakeProportional (corner);
		}
		needs_drawn = true;
	}
	else
	#endif
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
			for (int16 i = 0; i < NumPoints; i++)
			{
				Points [i].Offset (dx, dy);
				Grabbers [i].Offset (dx, dy);
			}
			needs_drawn = true;
		}
		else
			if (MousemoveMode == Resizing)
			{
				Points [SelectedGrabber] = point;
				needs_drawn = true;
			}
		if (MousemoveMode != None) {
			PlaceGrabbers();
		}
//		if (! Defined || MousemoveMode == Resizing)
			
	}
	if (Defined) {
		Origin = Bounds.GetTopLeft();
	}
	/*  Now draw whatever the rectangle looks like  */
	if (needs_drawn)
		Draw();
	/*  Refresh Bounds Rectangles */
	UpdateBounds();
}
void
G42PointsObject::MouseUp (G42MouseUp *  /*  message  */)
{
	Draw(); // <erase>
		if (MousemoveMode != None) {
			MousemoveMode = None;
		}
	/*  Always do these on mouseup; simplifies things  */
	Highlighted = false;
	Draw();	
	/*  Refresh Bounds Rectangles */
	UpdateBounds();
	/*  GrafObjects are no longer modified after MouseUp  */
	Modified = false;
}
/*  Place the Grabbers on the current Rectangle */
void
G42PointsObject::PlaceGrabbers (void)
{
	int8 offset = G42GrabberSize >> 1;
	for (int16 i = 0; i < NumPoints; i++)
	{
		G42Point point = Points [i];
		G42Rect  rect (point.X - offset, point.Y - offset, point.X + offset, point.Y + offset);
		rect.Inflate (1, 1);
		Grabbers [i].GrabberRect  = rect;	
		Grabbers [i].GrabberPoint = point;
	}
}
void
G42PointsObject::UpdateBounds (void)
{
	int32 left 		= Points [0].X;
	int32	top  		= Points [0].Y;
	int32 right		= left;
	int32 bottom	= top; 
	for (int16 i = 1; i < NumPoints; i++)
	{
		G42Point point = Points [i];
		if (point.X < left) {
			left = point.X;
		}
		if (point.X > right) {
			right = point.X;
		}
		if (point.Y < top) {
			top = point.Y;
		}
		if (point.Y > bottom) {
			bottom = point.Y;
		}
	}
	Bounds.Set (left, top, right, bottom);	
	ResponseBounds = Bounds.InflatedBy (G42GrabberSize, G42GrabberSize);
}