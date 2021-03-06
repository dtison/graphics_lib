//  giobgraf.h - G42GrafObject
#include "giobgraf.h"
#include "gicarea.h"
#include "gidsetup.h"
#include "gtools.h"
#include "gimarch.h"
#include "girotate.h"
#ifdef NOMACOS
Pattern	G42SingleImageViewer::AntsPattern = {0xF8, 0xF1, 0xE3, 0xC7, 0x8F, 0x1F, 
	0x3E, 0x7C};	
Pattern	G42SingleImageViewer::AntsPattern2 = {0x88, 0x44, 0x22, 0x11, 0x88, 0x44,
	0x22, 0x11};		
#endif
G42GrafObject::G42GrafObject (G42AreaController *acontroller, 
	G42MouseKeyController *mcontroller)
:
	AreaController (acontroller), Defined (false), Display (0), 
	DrawEnabled (true), Highlighted (false), Hot (false), 
	Modified (false), MousemoveMode (None), 
	MouseKeyController (mcontroller), NumDrawGrabbers (0), 
	NumGrabbers (0), 
	#ifdef ROTATION_SUPPORT
	RotateAngle (0), 
	#endif
	Selected (false), 
	Surface (0), UseAreaBounds (true), UseGrabbers (true), XORDraw (false)
{
	// Probably need to throw parameter exception if acontroller or mcontroller is null
	AreaBounds.SetNull();
	Bounds.SetNull();
	ResponseBounds.SetNull();
	Display = AreaController -> GetDisplay();
	Surface = Display -> GetSurface();
}
void
G42GrafObject::CleanupDraw (G42DrawLocation location, bool release)
{
	#ifdef MSWIN
	if (release)
		ReleaseDC (Surface, location);
	#endif
	#ifdef MACOS
	/*  Quiet warnings  */
	if (location || release) {}
	#endif
}
/*  Clips to canvas rectangle  */
void
G42GrafObject::ClipPointToRect (G42Point &point, const G42Rect &rect) const
{
	if (UseAreaBounds)
	{
		if (point.X < rect.Left)
			point.X = rect.Left;
		else
			if (point.X > rect.Right)
				point.X = rect.Right;
		if (point.Y < rect.Top)
			point.Y = rect.Top;
		else
			if (point.Y > rect.Bottom)
				point.Y = rect.Bottom;			
	}
}
/*  Clips a rectangle to fit completely within a specified bounds rectangle  */
void
G42GrafObject::ClipRectToRect (G42Rect &rect, G42Rect &bounds, int32 &xclip, 
	int32 &yclip) const
{
	if (UseAreaBounds)
	{
		int32 diff = rect.Top - bounds.Top;
		if (diff < 0)
		{
			yclip = -diff;
			rect.Top = bounds.Top;
		}
			
		diff = rect.Right - bounds.Right;
		if (diff > 0)
		{
			xclip = -diff;	
			rect.Right = bounds.Right;
		}
		diff = rect.Bottom - bounds.Bottom;
		if (diff > 0)
		{
			yclip = -diff;	
			rect.Bottom = bounds.Bottom;
		}
		diff = rect.Left - bounds.Left;
		if (diff < 0)
		{
			xclip = -diff;	
			rect.Left = bounds.Left;
		}
	}
}
/*  Draws private UI  */
void
G42GrafObject::Draw (G42DrawLocation location)
{
	if (Bounds.IsEmpty() || ! DrawEnabled) {
		return;
	}
	bool kill_dc = PrepareDraw (location);
	/*  Set up XOR drawing if needed  */
	G42Ptr <G42XORStyle> xor;
	if (XORDraw) {
		xor.Set (new G42XORStyle (location));
	}
	G42FrameStyle frame(location);
	G42DashedLineStyle dash(location);
	DrawGrafObject (location);
	dash.Clear();
	if (Selected)
		DrawGrabbers (location);
	xor.Clear();
	CleanupDraw (location, kill_dc);
}
void
G42GrafObject::DrawGrabbers (G42DrawLocation location)
{
	if (! Defined || ! UseGrabbers)
		return;
	/*  Set up wide drawing if needed  */
	G42Ptr <G42WideLineStyle> wide_line;
	if (Highlighted) {
		wide_line.Set (new G42WideLineStyle (location));
	}
	int8 offset = G42GrabberSize >> 1;
	for (int16 i = 0; i < NumDrawGrabbers; i++)
	{
		G42Point point = Grabbers [i].GrabberPoint;
		ConvertPoint (point, true);
		#ifdef MSWIN
		G42Rect viewport = AreaController -> GetDisplay() -> GetViewport();
		point.Offset (-viewport.Left, -viewport.Top);
		::Rectangle (location, point.X - offset, point.Y - offset,
			point.X + offset, point.Y + offset);
		#endif
		#ifdef MACOS
		Rect mrect;
		::SetRect (&mrect, point.X - offset, point.Y - offset, point.X + offset, point.Y + offset);
		::FrameRect (&mrect);
		#endif
	}
}
void
G42GrafObject::DrawIdle (void)
{
	// Draw Bounds rectangle
	G42Rect rectg = Bounds;
	Display -> GetScaleTranslate().Convert (rectg);
	G42MarchingAnts ants;
	#ifdef MSWIN
	G42Rect viewport = AreaController -> GetDisplay() -> GetViewport();
	rectg.Offset (-viewport.Left, -viewport.Top);
	HDC location = GetDC(Surface);
	{
		G42PrepareAntsToDraw p(ants, location);
		::Rectangle(location, rectg.Left, rectg.Top, rectg.Right, rectg.Bottom);
	}
	ReleaseDC(Surface, location);
	#endif
	#ifdef MACOS
	G42PrepareAntsToDraw p(ants, 0);
	Rect rectm = rectg;
	::FrameRect (&rectm);
	#endif
}
void
G42GrafObject::DrawRectangle (G42Rect &rect, G42DrawLocation location)
{
	G42Rect rectg (rect);
	Display -> GetScaleTranslate().Convert (rectg);
	#ifdef MSWIN
	G42Rect viewport = AreaController -> GetDisplay() -> GetViewport();
	G42Rect rectw = rectg.OffsetBy (-viewport.Left, -viewport.Top);
	::Rectangle (location, rectw.Left, rectw.Top, rectw.Right, rectw.Bottom);
	#endif
	#ifdef MACOS
	#ifdef ROTATION_SUPPORT
	/*  Draw Rotated outline  */
	if (MousemoveMode == Rotating)	
	{
		G42Rotation rotation (Bounds.GetSize(), RotateAngle, G42Rotation::Radians);
		G42Point source_axis = rotation.GetSourceAxis();
		G42Vector <G42Point> points;
		G42Rect rect = Bounds.OffsetBy (-Bounds.Left, -Bounds.Top);
		/*  Center rect on its axis  */
		rect.Offset (-source_axis.X, -source_axis.Y);
		/*  Rotate corner points  */
		points [0] = rect.GetTopLeft();
		points [1] = rect.GetTopRight();
		points [2] = rect.GetBottomRight();
		points [3] = rect.GetBottomLeft();
		for (int16 i = 0; i < 4; i++)
		{
			G42Point point = points [i];
			rotation.RotatePoint (point);
			point.Offset (Bounds.Left, Bounds.Top);
			points [i] = point;
		}
		::MoveTo (points [0].X, points [0].Y);
		::LineTo (points [1].X, points [1].Y);
		::LineTo (points [2].X, points [2].Y);
		::LineTo (points [3].X, points [3].Y);
		::LineTo (points [0].X, points [0].Y);
		return;
	}
	#endif
	::FrameRect (&((Rect) rectg));
	#ifdef ROTATION_SUPPORT
	/*  Draw Rotator thing  */
	if (Defined)
	{
		G42Point point;
		point.X = rect.Left + ((rect.Right - rect.Left) >> 1);
		point.Y = rect.Top;
		::MoveTo (point.X, point.Y);
		point = RotateGrabber.GrabberPoint;
		::LineTo (point.X, point.Y);
		::FrameRect (&((Rect) RotateGrabber.GrabberRect));
	}
	#endif
	/*  Quiet warnings  */
	if (location) {}
	#endif
}
#ifdef NOTYET
/*
	Parameters - 
	point  - Last mousemove point
	origin - Move origin
	rect	 - Current Rectangle (or bounds)
	bounds - Rectangle to confine movement to  */
void
G42GrafObject::HandleBoundedMove (G42Point &point, G42Point &origin, G42Rect &rect, 
	G42Rect &bounds)
{
	/*  First calculate the offset to try to move  */
	int32 dx = point.X - origin.X;	
	int32 dy = point.X - origin.X;
	/*  But test for off-canvas moves  */
	G42Rect test_rect = rect;
	test_rect.Offset (dx, dy);
	/*  xclip and yclip will tell us how much test_rect is out of bounds  */
	int32 xclip = 0, yclip = 0;
	ClipRectToRect (test_rect, bounds, xclip, yclip);
	point.Offset (xclip, yclip);	
	dx += xclip;
	dy += yclip;
	origin = point;
	/*  Now apply it to the object  */
	rect.Offset (dx, dy);
	origin.Offset (dx, dy);
}
#endif
bool
G42GrafObject::HandleArrowKeys (G42KeyDown *message, G42Rect rect, int32 &dx, int32 &dy)
{
	bool value = false;
	int32 total_ticks = 0;
	#ifdef ACCELERATE_KEYS
	if (! KeyHeldDown)
	{
		if (KeyDownTicks == 0) { // Initialize
			KeyDownTicks = ::TickCount();
		}
		KeyHeldDown = true;
	}
	else
		total_ticks = ::TickCount() - KeyDownTicks;	
	#endif	
	/*  Process the arrow keys  */
//	int32 dx = 0, dy = 0;
	G42Point clip_point;
	switch (message -> Key)
	{
		case G42UpArrow:
			dy = -1;
			if (total_ticks > 60)
				dy = -3;
			clip_point = rect.Top + dy;
			value = true;
		break;
		case G42RightArrow:
			dx = 1;		
			if (total_ticks > 60)
				dx = 3;
			clip_point = rect.Right + dx;	
			value = true;
		break;		
		case G42DownArrow:
			dy = 1;		
			if (total_ticks > 60)
				dy = 3;
			clip_point = rect.Bottom + dy;	
			value = true;
		break;		
		case G42LeftArrow:
			dx = -1;
			if (total_ticks > 60)
				dx = -3;					
			clip_point = rect.Left + dx;	
			value = true;
		break;				
	}
	// TODO:  Combine or improve these canvas clipping things (?)
	ConvertPoint (clip_point, true);
	if (! AreaBounds.Contains (clip_point))
		value = false;
	return value;
}
bool
G42GrafObject::PrepareDraw (G42DrawLocation &location)
{
	bool value = false;
	#ifdef MSWIN
	if (! location)
	{
		location = GetDC (Surface);
		value = true;
	}
  	#endif
  	#ifdef MACOS
  	if (location)	// Shut warnings up
  		return value;
  	#endif
  	return value;
}
/*  Redraws as highlighted (feedback for mousedown)  */
void
G42GrafObject::RedrawHighlighted (bool erase)
{
	if (erase)
		Draw();
	Highlighted = true;
	Draw(); 
}
/*  Set to draw private UI Hot */
void
G42GrafObject::SetHot (bool hot)
{
	Draw();		// <Erase>
	Hot = hot;
	Draw();		// <Refresh>
}
void
G42GrafObject::SetSelected (bool selected) 
{
	Draw();					// <Erase>
	Selected = selected;
	Draw();					// <Refresh>
}
bool
G42GrafObject::TestHitGrabbers (int16 &index, G42Point point)
{
	bool found = false;
	if (UseGrabbers)
	{
		index = 0;
		while (index < NumGrabbers && ! found)
		{
			if (Grabbers [index].GrabberRect.Contains (point))
				found = true;
			else
				index++;
		}
	}
	#ifdef ROTATION_SUPPORT
	/*  Rotate grabber  */
	if (! found)
	{
		if (RotateGrabber.GrabberRect.Contains (point))
		{
			found = true;
			index = -1;	// Funny kludge for indicating rotate grabber
		}
	}
	#endif
	return found;
}
