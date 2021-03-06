// gicppsur.h - G42PPSurfaceController
#if !defined(GICPPSUR_H)
#define GICPPSUR_H
/*-----------------------------------------------------------------------------
	file:			gicppsur.h
	author:			David Ison
	date:			10/24/96
	revision:		3.0
	This is a subclass of G42SurfaceController that is "PowerPlant-aware".
	It activates certain things to make scrolling and other things work
	correctly.  This class only is used in PowerPlant applications.
----------------------------------------------------------------------------*/
#include "gicsur.h"
#ifdef MACOS	// Mac-only file
class LView;
class LScroller;
class G42PPScroller;
class G42PPSurfaceController : public G42SurfaceController
{
public:
	G42PPSurfaceController (LView &view, G42PPScroller &scroller, G42Surface surface, 
		G42Notifier *notifier = 0, bool default_area = true);
	virtual ~G42PPSurfaceController (void) {};
	virtual void QueryViewportOffset (void);
	virtual void ResetCanvasViewport (void);
	virtual void ScrollBy(int32 dx, int32 dy);
	#if 0
	virtual void SetViewportOffset (G42Point &offset);
	#endif
protected:
	LView			&PPView;
	G42PPScroller	&PPScroller;
};
#endif
#endif // GICPPSUR_H
