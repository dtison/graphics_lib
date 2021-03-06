//  giobquad.h - G42QuadObject
// TODO:  Put in HandleBoundedMove (point, MoveOrigin, Rectangle, AreaBounds); in this and G42PointObject
#include "giobquad.h"
G42QuadObject::G42QuadObject (G42AreaController *acontroller,
	G42MouseKeyController *mcontroller)  
:
 	G42GrafObject (acontroller, mcontroller), 
	#ifdef ACCELERATE_KEYS
	KeyHeldDown (false), KeyDownTicks(0), 
	#endif
	Proportional (false), SelectedGrabber (-1), 
	ResizeChangeLocation(G42RectChangeWidth)
{
	PinRect.SetNull();
	Rectangle.SetNull();
	Origin.X = Origin.Y = 0;
	MoveOrigin.X = MoveOrigin.Y = 0;
	/* Inherited from G42GrafObject  */
	NumGrabbers 		= G42NumGrabbers;
	NumDrawGrabbers 	= G42NumDrawGrabbers;
	XORDraw				= true;
}
G42Rect::G42RectCorner
G42QuadObject::FindCorner (G42Point &point)
{
	G42Rect::G42RectCorner corner;
	if (point.Y > Origin.Y)	{
		if (point.X > Origin.X) {
			corner = G42Rect::BottomRight;
		}
		else {
			corner = G42Rect::BottomLeft;
		}
	}
	else {
		if (point.X > Origin.X) {
			corner = G42Rect::TopRight;
		}
		else {
			corner = G42Rect::TopLeft;
		}
	}
	return corner;
}
uintf
G42QuadObject::GetProportionalChangeLocation (const G42Point & point)
{
	uintf change;
	if (point.Y > Origin.Y)
	{
		if (point.X > Origin.X)
		{
			change = G42RectChangeBottomRight;
		}
		else if (point.X < Origin.X)
		{
			change = G42RectChangeBottomLeft;
		}
		else
		{
			change = G42RectChangeWidth;
		}
	}
	else if (point.Y < Origin.Y)
	{
		if (point.X > Origin.X)
		{
			change = G42RectChangeTopRight;
		}
		else if (point.X < Origin.X)
		{
			change = G42RectChangeTopLeft;
		}
		else
		{
			change = G42RectChangeWidth;
		}
	}
	else
	{
		change = G42RectChangeHeight;
	}
	return change;
}
#if 0
G42Rect::G42RectCorner
G42QuadObject::Grabber2Corner (int16 grabber)
{
	// TODO:  Need to deal with  grabber > 3
	return (G42Rect::G42RectCorner) grabber;
}
#endif
void
G42QuadObject::HandleLowerLeft (G42Point point)
{
	ResizeChangeLocation = 0;
	/*  Check for out of bounds on horizontal axis  */
	if (point.X > PinRect.Right)
	{
		Rectangle.Left = PinRect.Right;
		Rectangle.Right = point.X;
		ResizeChangeLocation |= G42RectChangeRight;
	}
	else
	{
		Rectangle.Right = PinRect.Right;
		Rectangle.Left = point.X;
		ResizeChangeLocation |= G42RectChangeLeft;
	}
	/*  Check for out of bounds on vertical axis  */
	if (point.Y < PinRect.Top)
	{
		Rectangle.Bottom = PinRect.Top;
		Rectangle.Top = point.Y;
		ResizeChangeLocation |= G42RectChangeTop;
	}
	else
	{
		Rectangle.Top = PinRect.Top;
		Rectangle.Bottom = point.Y;
		ResizeChangeLocation |= G42RectChangeBottom;
	}
}
void
G42QuadObject::HandleLowerRight (G42Point point)
{
	/*  Check for out of bounds on horizontal axis  */
	ResizeChangeLocation = 0;
	if (point.X < PinRect.Left)
	{
		Rectangle.Right = PinRect.Left;
		Rectangle.Left = point.X;
		ResizeChangeLocation |= G42RectChangeLeft;
	}
	else
	{
		Rectangle.Left = PinRect.Left;
		Rectangle.Right = point.X;
		ResizeChangeLocation |= G42RectChangeRight;
	}
	/*  Check for out of bounds on vertical axis  */
	if (point.Y < PinRect.Top)
	{
		Rectangle.Bottom = PinRect.Top;
		Rectangle.Top = point.Y;
		ResizeChangeLocation |= G42RectChangeTop;
	}
	else
	{
		Rectangle.Top = PinRect.Top;
		Rectangle.Bottom = point.Y;
		ResizeChangeLocation |= G42RectChangeBottom;
	}
}
void
G42QuadObject::HandleUpperLeft (G42Point point)
{
	/*  Check for out of bounds on horizontal axis  */
	ResizeChangeLocation = 0;
	if (point.X > PinRect.Right)
	{
		Rectangle.Left = PinRect.Right;
		Rectangle.Right = point.X;
		ResizeChangeLocation |= G42RectChangeRight;
	}
	else
	{
		Rectangle.Right = PinRect.Right;
		Rectangle.Left = point.X;
		ResizeChangeLocation |= G42RectChangeLeft;
	}
	/*  Check for out of bounds on vertical axis  */
	if (point.Y > PinRect.Bottom)
	{
		Rectangle.Top = PinRect.Bottom;
		Rectangle.Bottom = point.Y;
		ResizeChangeLocation |= G42RectChangeBottom;
	}
	else
	{
		Rectangle.Bottom = PinRect.Bottom;
		Rectangle.Top = point.Y;
		ResizeChangeLocation |= G42RectChangeTop;
	}
}
void
G42QuadObject::HandleUpperRight (G42Point point)
{
	/*  Check for out of bounds on horizontal axis  */
	ResizeChangeLocation = 0;
	if (point.X < PinRect.Left)
	{
		Rectangle.Right = PinRect.Left;
		Rectangle.Left = point.X;
		ResizeChangeLocation |= G42RectChangeLeft;
	}
	else
	{
		Rectangle.Left = PinRect.Left;
		Rectangle.Right = point.X;
		ResizeChangeLocation |= G42RectChangeRight;
	}
	/*  Check for out of bounds on vertical axis  */
	if (point.Y > PinRect.Bottom)
	{
		Rectangle.Top = PinRect.Bottom;
		Rectangle.Bottom = point.Y;
		ResizeChangeLocation |= G42RectChangeBottom;
	}
	else 
	{
		Rectangle.Bottom = PinRect.Bottom;
		Rectangle.Top = point.Y;
		ResizeChangeLocation |= G42RectChangeTop;
	}
}
/*  Allows caller to initialize GrafObject instead of having to use mouse  */
void
G42QuadObject::InitObject (const G42Rect &rect, const G42Rect &bounds)
{
	if (rect.IsNull()) {
		throw (G42InvalidParameterException (0));
	}
	AreaBounds	 	= bounds;
	Rectangle 		= rect;
	Origin 			= Rectangle.GetTopLeft();
	UpdateBounds();
	PlaceGrabbers();
	Defined = true;
}
void
G42QuadObject::HandleTop (G42Point point)
{
	/*  Check for out of bounds on vertical axis  */
	Rectangle.Left = PinRect.Left;
	Rectangle.Right = PinRect.Right;
	if (point.Y > PinRect.Bottom)
	{
		Rectangle.Top = PinRect.Bottom;
		Rectangle.Bottom = point.Y;
	}
	else
	{
		Rectangle.Bottom = PinRect.Bottom;
		Rectangle.Top = point.Y;
	}
	ResizeChangeLocation = G42RectChangeWidth;
}
void
G42QuadObject::HandleRight (G42Point point)
{
	/*  Check for out of bounds on horizontal axis  */
	Rectangle.Top = PinRect.Top;
	Rectangle.Bottom = PinRect.Bottom;
	if (point.X < PinRect.Left)
	{
		Rectangle.Right = PinRect.Left;
		Rectangle.Left = point.X;
	}
	else
	{
		Rectangle.Left = PinRect.Left;
		Rectangle.Right = point.X;
	}
	ResizeChangeLocation = G42RectChangeHeight;
}
void
G42QuadObject::HandleBottom (G42Point point)
{
	/*  Check for out of bounds on vertical axis  */
	Rectangle.Left = PinRect.Left;
	Rectangle.Right = PinRect.Right;
	if (point.Y < PinRect.Top)
	{
		Rectangle.Bottom = PinRect.Top;
		Rectangle.Top = point.Y;
	}
	else
	{
		Rectangle.Top = PinRect.Top;
		Rectangle.Bottom = point.Y;
	}
	ResizeChangeLocation = G42RectChangeWidth;
}
void
G42QuadObject::HandleLeft (G42Point point)
{
	/*  Check for out of bounds on horizontal axis  */
	Rectangle.Top = PinRect.Top;
	Rectangle.Bottom = PinRect.Bottom;
	if (point.X > PinRect.Right) 
	{
		Rectangle.Left = PinRect.Right;
		Rectangle.Right = point.X;
	}
	else
	{
		Rectangle.Right = PinRect.Right;
		Rectangle.Left = point.X;
	}
	ResizeChangeLocation = G42RectChangeHeight;
}
void
G42QuadObject::KeyDown (G42KeyDown *message)
{
	if (! Defined)
		return;
	
	int32 dx = 0;
	int32 dy = 0;
	if (HandleArrowKeys (message, Rectangle, dx, dy))
	{
		/*  Erase  */
//		G42Rect rectg = ResponseBounds.InflatedBy (3, 3); 
		G42Rect rectg = ResponseBounds;
		Display -> GetScaleTranslate().Convert (rectg);
//		AreaController -> Draw (rectg, 0, true);
//		AreaController -> Invalidate (rectg, true);
		/*  Move & redraw  */
		Rectangle.Offset (dx, dy);
		Origin.Offset (dx, dy);
		PlaceGrabbers();
		Draw();
		UpdateBounds();
	}
}
void
G42QuadObject::KeyUp (G42KeyUp * /*  message  */)
{
	#ifdef ACCELERATE_KEYS
	KeyHeldDown = false;
	KeyDownTicks = 0;
	#endif
}
void
G42QuadObject::MouseDown (G42MouseDown *message)
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
	}
	else
	{
		/*  First <erase> ants, then redraw *** TODO: Make a function out of some of this?? */
		G42Rect rectg = ResponseBounds;
		Display -> GetScaleTranslate().Convert (rectg);
		#ifdef MSWIN
		rectg.Inflate (1, 1);
//		AreaController -> Draw (rectg, 0, true);
		AreaController -> Invalidate (rectg, true);
		Draw();
		#endif
		#ifdef MACOS
		::InvalRect (&((Rect) rectg));
		#endif
		/*  Want to modify the object  */
		/*  First check the grabbers, then the rectangle  */
		if (TestHitGrabbers (SelectedGrabber, point))
		{
			#ifdef ROTATION_SUPPORT
			/*  Rotate grabber selected  */
			if (SelectedGrabber == -1) {
				MousemoveMode = Rotating;
			}
			else
			#endif
			{
				MousemoveMode = Resizing;
				PinRect = Rectangle;
				OriginalAspect = (double)Rectangle.GetHeight() /
					(double)Rectangle.GetWidth();
				RedrawHighlighted();
			}
		}	
		else			 	
			if (Rectangle.Contains (point))
			{
				MousemoveMode = Moving;
				MoveOrigin = point;			
				RedrawHighlighted();
			}
	}
}
void
G42QuadObject::MouseMove (G42MouseMove *message)
{
	/*  Be sure point is in current canvas.  */
	G42Point point (message -> Point);
	ConvertPoint (point, false);
	ClipPointToRect (point, AreaBounds);	
	/*  Then erase current rectangle  */
	if (! Rectangle.IsNull()) {
		Draw();	// <Erase>
	}
	
	bool needs_drawn = false;	// Every mousemove does not need to be drawn 
	if (! Defined)
	{
		Rectangle.Set (Origin.X, Origin.Y, point.X, point.Y);
		Rectangle.Normalize();
		if (Proportional || (message->Flags & G42ShiftDown))
		{
//			G42Rect::G42RectCorner corner = FindCorner (point);
//			Rectangle.MakeProportional (corner);
			Rectangle.ResizeProportional (1.0,
				GetProportionalChangeLocation(point));
		}
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
			G42Rect test_rect = Rectangle;
			test_rect.Offset (dx, dy);
			/*  xclip and yclip will tell us how much test_rect is out of bounds  */
			int32 xclip = 0, yclip = 0;
			ClipRectToRect (test_rect, AreaBounds, xclip, yclip);
			point.Offset (xclip, yclip);
			dx += xclip;
			dy += yclip;
			MoveOrigin 	= point;
			/*  Now apply it to the object  */
			Rectangle.Offset (dx, dy);
			Origin.Offset (dx, dy);
			for (int8 i = 0; i < NumGrabbers; i++)
				Grabbers [i].Offset (dx, dy);
			needs_drawn = true;
		}
		else if (MousemoveMode == Resizing)
		{
			switch (SelectedGrabber)
			{
				case G42UpperLeft:
					HandleUpperLeft (point);
				break;
				case G42UpperRight:
					HandleUpperRight (point);
				break;
				case G42LowerRight:
					HandleLowerRight (point);
				break;
				case G42LowerLeft:
					HandleLowerLeft (point);
				break;
				case G42Top:
					HandleTop (point);
				break;
				case G42Right:
					HandleRight (point);
				break;
				case G42Bottom:
					HandleBottom (point);
				break;
				case G42Left:
					HandleLeft (point);
				break;
			}
			needs_drawn = true;
			if (Proportional || (message->Flags & G42ShiftDown))
			{
				Rectangle.ResizeProportional (OriginalAspect,
					ResizeChangeLocation);
			}
		}
		#ifdef ROTATION_SUPPORT
		else if (MousemoveMode == Rotating)
		{
			/*  Find rotation angle  */
			#if 0
			G42Rect rectg = ResponseBounds;
			Display -> GetScaleTranslate().Convert (rectg);
			rectg.Inflate (1, 1);
			AreaController -> Invalidate (rectg, true);
			Draw();
			#endif
			G42Point middle (Rectangle.Left + (Rectangle.GetWidth() >> 1),
				 Rectangle.Top + (Rectangle.GetHeight() >> 1));
			G42Point test_point (point.X - middle.X, point.Y - middle.Y);
	//		double r = sqrt ((test_point.X * test_point.X) + (test_point.Y * test_point.Y));
			double angle = atan2f ((double) test_point.Y, (double) test_point.X);
			angle += (3.14159 / 2);
			RotateAngle = angle;
		}
		#endif
		if (MousemoveMode != None) {
			PlaceGrabbers();
		}
//		if (! Defined || MousemoveMode == Resizing)
	}
	if (Defined) {
		Origin = Rectangle.GetTopLeft();
	}
	/*  Now draw whatever the rectangle looks like  */
	if (needs_drawn)
	{
		Modified = true;
		Draw();
	}
	UpdateBounds();
}
void
G42QuadObject::MouseUp (G42MouseUp *  /*  message  */)
{
	Draw(); // <erase>
	if (! Defined) 
	{
		if (!Rectangle.IsEmpty())
		{
			PlaceGrabbers();
			Defined = true;
		}
	}
	else
	{
		if (MousemoveMode != None)
		{
			PinRect.SetNull();
			Rectangle.Normalize();	
			SelectedGrabber = -1;
			MousemoveMode = None;
		}
	}
	/*  Always do these on mouseup; simplifies things  */
	Highlighted = false;
	Draw();	
	UpdateBounds();
	/*  GrafObjects are no longer modified after MouseUp  */
	Modified = false;
}
/*  Place the Grabbers on the current Rectangle */
void
G42QuadObject::PlaceGrabbers (void)
{
	int8 offset = G42GrabberSize >> 1;
	G42Rect  rect;
	G42Point point;
	/*  Upper left point and rectangle  */
	point = Rectangle.GetTopLeft();
	rect.Set (point.X - offset, point.Y - offset, point.X + offset, point.Y + offset);
	#ifdef NOTYET
	rect.Offset (1, 1);
	#endif
	rect.Inflate (1, 1);
	Grabbers [G42UpperLeft].GrabberRect  = rect;
	Grabbers [G42UpperLeft].GrabberPoint = point;
	/*  Upper right point and rectangle  */
	point = Rectangle.GetTopRight();
	rect.Set (point.X - offset, point.Y - offset, point.X + offset, point.Y + offset);
	#ifdef NOTYET
	rect.Offset (-1, 1);
	#endif
	rect.Inflate (1, 1);
	Grabbers [G42UpperRight].GrabberRect 	= rect;
	Grabbers [G42UpperRight].GrabberPoint	= point;
	/*  Lower right point and rectangle  */
	point = Rectangle.GetBottomRight();
	rect.Set (point.X - offset, point.Y - offset, point.X + offset, point.Y + offset);
	#ifdef NOTYET
	rect.Offset (-1, -1);
	#endif
	rect.Inflate (1, 1);
	Grabbers [G42LowerRight].GrabberRect 	= rect;
	Grabbers [G42LowerRight].GrabberPoint	= point;
	/*  Lower left point and rectangle  */
	point = Rectangle.GetBottomLeft();
	rect.Set (point.X - offset, point.Y - offset, point.X + offset, point.Y + offset);
	#ifdef NOTYET
	rect.Offset (1, -1);
	#endif
	rect.Inflate (1, 1);
	Grabbers [G42LowerLeft].GrabberRect 	= rect;
	Grabbers [G42LowerLeft].GrabberPoint	= point;
	#ifdef MSWIN
	/*  For MSWIN, the next four "grabbers" are for point-testing only  */
	G42Rect rect1 = Grabbers [G42UpperLeft].GrabberRect;
	G42Rect rect2 = Grabbers [G42UpperRight].GrabberRect;
	G42Rect top (rect1.Right, rect1.Top, rect2.Left, rect2.Bottom);
	Grabbers [G42Top].GrabberRect = top;
	rect1 = Grabbers [G42UpperRight].GrabberRect;
	rect2 = Grabbers [G42LowerRight].GrabberRect;
	G42Rect right (rect1.Left, rect1.Bottom, rect2.Right, rect2.Top);
	Grabbers [G42Right].GrabberRect = right;
	rect1 = Grabbers [G42LowerRight].GrabberRect;
	rect2 = Grabbers [G42LowerLeft].GrabberRect;
	G42Rect bottom (rect2.Right, rect2.Top, rect1.Left, rect1.Bottom);
	Grabbers [G42Bottom].GrabberRect = bottom;
	rect1 = Grabbers [G42LowerLeft].GrabberRect;
	rect2 = Grabbers [G42UpperLeft].GrabberRect;
	G42Rect left (rect2.Left, rect2.Bottom, rect1.Right, rect1.Top);
	Grabbers [G42Left].GrabberRect = left;
	#endif
	#ifdef MACOS
	/*  Next do the sides  */
	G42Size size = Rectangle.GetSize();
	int32 half_width 	= size.Width >> 1;
	int32 half_height = size.Height >> 1;
	/*  Top point and rectangle  */
	point.Set (Rectangle.Left + half_width, Rectangle.Top);
	rect.Set (point.X - offset, point.Y - offset, point.X + offset, point.Y + offset);
	#ifdef NOTYET
	rect.Offset (0, 1);
	#endif
	rect.Inflate (1, 1);
	Grabbers [G42Top].GrabberRect 	= rect;
	Grabbers [G42Top].GrabberPoint	= point;
	/*  Right point and rectangle  */
	point.Set (Rectangle.Right, Rectangle.Top + half_height);
	rect.Set (point.X - offset, point.Y - offset, point.X + offset, point.Y + offset);
	#ifdef NOTYET
	rect.Offset (-1, 0);
	#endif
	rect.Inflate (1, 1);
	Grabbers [G42Right].GrabberRect 	= rect;
	Grabbers [G42Right].GrabberPoint	= point;
	/*  Bottom point and rectangle  */
	point.Set (Rectangle.Left + half_width, Rectangle.Bottom);
	rect.Set (point.X - offset, point.Y - offset, point.X + offset, point.Y + offset);
	#ifdef NOTYET
	rect.Offset (0, -1);
	#endif
	rect.Inflate (1, 1);
	Grabbers [G42Bottom].GrabberRect 	= rect;
	Grabbers [G42Bottom].GrabberPoint	= point;
	/*  Left point and rectangle  */
	point.Set (Rectangle.Left, Rectangle.Top + half_height);
	rect.Set (point.X - offset, point.Y - offset, point.X + offset, point.Y + offset);
	#ifdef NOTYET
	rect.Offset (1, 0);
	#endif
	rect.Inflate (1, 1);
	Grabbers [G42Left].GrabberRect 	= rect;
	Grabbers [G42Left].GrabberPoint	= point;
	#endif	// MACOS
	#ifdef ROTATION_SUPPORT
	/*  Place Rotate grabber  */
	point.X = Rectangle.Left + ((Rectangle.Right - Rectangle.Left) >> 1);
	point.Y = Rectangle.Top - 10;
	rect.Set (point.X - offset, point.Y - offset, point.X + offset, point.Y + offset);
	RotateGrabber.GrabberRect 	= rect;
	RotateGrabber.GrabberPoint	= point;
	#endif
}
void
G42QuadObject::UpdateBounds (void)
{
	/*  Refresh Bounds Rectangles */
	Bounds = Rectangle;
	ResponseBounds = Rectangle.InflatedBy (G42GrabberSize, G42GrabberSize);
	#ifdef ROTATION_SUPPORT
	/*  Expand for rotation  */
	ResponseBounds.Top -= (10 + (G42GrabberSize >> 1));
	#endif
}
