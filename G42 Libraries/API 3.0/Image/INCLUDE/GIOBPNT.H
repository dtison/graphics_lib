//  giobpnt.h - G42PointObject
#if ! defined (GIOBPNT_H)
#define GIOBPNT_H
/*-----------------------------------------------------------------------------
	file:				giobpnt.h
	author:			David Ison
	date:				11/14/96
	revision:		3.0
	Onscreen graphics objects:  Point GrafObject
 ----------------------------------------------------------------------------*/
#include "giobgraf.h"
#include "gitype.h"
#define TBD
class G42PointObject : public G42GrafObject
{
public:
								G42PointObject (G42AreaController *acontroller,
									G42MouseKeyController *mcontroller);
	virtual 					~G42PointObject (void) {}
	void						InitObject (const G42Point & point, 
									const G42Rect & area_bounds);	
	virtual void			KeyDown (G42KeyDown *message);
	virtual void			KeyUp (G42KeyUp *message);
	virtual void			MouseDown (G42MouseDown *message);
	virtual void			MouseMove (G42MouseMove *message);
	virtual void			MouseUp (G42MouseUp *message);
protected:
	// Functions
	virtual void			DrawGrafObject (G42DrawLocation location = 0);
	virtual void			DrawPoint (G42Rect &rect, G42DrawLocation location  = 0);
	// Instance variables
	#ifdef ACCELERATE_KEYS
	bool						KeyHeldDown;	// True when KeyDown msg rec'd
	int32						KeyDownTicks;	// For key move-accelerating
	#endif
	G42Point					MoveOrigin;		// For handling rectangle moves
	G42Point					Origin;			// Original mousedown point
//	G42Rect					Rectangle;		// The actual rectangle 
};
#endif
