// gippscrl.h - G42PPScroller 
#if ! defined (GIPPSCRL_H)
#define GIPPSCRL_H
/*-----------------------------------------------------------------------------
	file:			gpcomp.h
	author:			David Ison
	date:			10/24/96
	revision:		3.0
	This is a subclass of MetroWerks PowerPlant LActiveScroller for setting 
	scroll positions and other scroller tasks in the library. It was 
	necessary because LActiveScroller doesn't allow access to its scrollbars.	
	This class is only used in PowerPlant applications.
----------------------------------------------------------------------------*/
#include "gtype.h"
#include "ggeomtry.h"
#ifdef MACOS	// Mac-only file
#include <LActiveScroller.h>
class G42PPSurfaceController;
class G42PPScroller : public LActiveScroller
{
public:
	enum {class_ID = 'gscr'};						// For PP Constructor
	G42PPScroller (void) {};
	G42PPScroller (LStream *inStream);
	G42PPScroller (const LScroller &inOriginal);
	virtual			~G42PPScroller() {};
	virtual void	AdjustScrollBars();
 	void 			GetScrollingViewPos (G42Point &point);
	void			SetScrollBars (const int32 xpos, const int32 ypos);
	/* Static things last  */
	static G42PPScroller *CreateG42PPScrollerStream (LStream *inStream);	
};
#endif	// MACOS
#endif	// GIPPSCRL_H