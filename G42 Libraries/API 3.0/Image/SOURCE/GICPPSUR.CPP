// gicppsur.h - G42PPSurfaceController
/*-----------------------------------------------------------------------------
	Modifications:
-----------------------------------------------------------------------------*/
#include "gtype.h"
#ifdef MACOS	// Mac-only file
#include "gicppsur.h"
#include "gippscrl.h"
#include <math.h>
/*-----------------------------------------------------------------------------
	G42PPScroller member functions:
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
	Function - G42PPSurfaceController constructor
	Parameters:
		blah blah blah.
-----------------------------------------------------------------------------*/
G42PPSurfaceController::G42PPSurfaceController (LView &view,
	G42PPScroller &scroller, G42Surface surface, G42Notifier *notifier, 
	bool default_area) : 
	G42SurfaceController (surface, notifier, default_area), PPView (view), 
	PPScroller (scroller)
{}
/*-----------------------------------------------------------------------------
	Function - ResetCanvasViewport
	Description:
		Does the actual work of making the PP scrollers work with the library.
-----------------------------------------------------------------------------*/
/*  Queries scrollers for their current x and y position and offsets the viewport */
void 
G42PPSurfaceController::QueryViewportOffset (void)
{
 	#ifdef OLDWAY
	SPoint32 pos;	// SPoint32 is PowerPlant thing
	PPScroller.mScrollingView -> GetScrollPosition (pos);
	SetViewportOffset (G42Point (pos.h, pos.v));
 	#endif
 
	G42Point point;
 	PPScroller.GetScrollingViewPos (point);
	SetViewportOffset (point);
}
void
G42PPSurfaceController::ResetCanvasViewport (void)
{
	G42SurfaceController::ResetCanvasViewport();
	/*  Set up powerplant's view and scroller  */
	PPView.ResizeImageTo (Canvas.Width, Canvas.Height, true); 
	PPScroller.SetScrollBars (Viewport.Left, Viewport.Top);
}
void
G42PPSurfaceController::ScrollBy(int32 dx, int32 dy)
{
	if (abs (dx) > 1)
		dx >>= 1;
	if (abs (dy) > 1)	
		dy >>= 1;
		
	PPScroller.SetScrollBars (Viewport.Left + dx, Viewport.Top + dy);	
	PPScroller.Draw(nil);
}
#if 0
// TODO: This function may not be necessary
void
G42PPSurfaceController::SetViewportOffset (G42Point &offset)
{
	G42SurfaceController::SetViewportOffset (offset);
	/*  Set up powerplant's scroller */
	SysBeep (0);
	PPScroller.SetScrollBars (Viewport.Left, Viewport.Top);
}
#endif
#endif // MACOS
