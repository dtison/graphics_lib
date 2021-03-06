//  giobquad.h - G42QuadObject
#if ! defined (GIOBQUAD_H)
#define GIOBQUAD_H
/*-----------------------------------------------------------------------------
	file:			giobquad.h
	author:			David Ison
	date:			10/25/96
	revision:		3.0
	Onscreen graphics objects:  Abstract class for Rectangle and Circle
 ----------------------------------------------------------------------------*/
#include "giobgraf.h"
#include "gitype.h"
#define TBD
#if 0
#define ACCELERATE_KEYS
#endif
class G42QuadObject : public G42GrafObject
{
public:
								G42QuadObject (G42AreaController *acontroller,
									G42MouseKeyController *mcontroller);
	virtual 					~G42QuadObject (void) {}
	void						InitObject (const G42Rect &rect, 
									const G42Rect & area_bounds);	
	virtual void			KeyDown (G42KeyDown *message);
	virtual void			KeyUp (G42KeyUp *message);
	virtual void			MouseDown (G42MouseDown *message);
	virtual void			MouseMove (G42MouseMove *message);
	virtual void			MouseUp (G42MouseUp *message);
	void						SetProportional (bool proportional) 
									{Proportional = proportional;}
protected:
	// Functions
	G42Rect					::G42RectCorner FindCorner (G42Point &point);
	uintf						GetProportionalChangeLocation (const G42Point & point);
	#if 0
	G42Rect					::G42RectCorner Grabber2Corner (int16 grabber);
	#endif
	void						PlaceGrabbers (void);
	// Instance variables
	void						HandleLowerLeft (G42Point point);
	void						HandleLowerRight (G42Point point);
	void						HandleUpperLeft (G42Point point);
	void						HandleUpperRight (G42Point point);
	void						HandleTop (G42Point point);
	void						HandleRight (G42Point point);
	void						HandleBottom (G42Point point);
	void						HandleLeft (G42Point point);
	void						UpdateBounds (void);
	#ifdef ACCELERATE_KEYS
	bool						KeyHeldDown;	// True when KeyDown msg rec'd
	int32						KeyDownTicks;	// For key move-accelerating
	#endif
	G42Point					MoveOrigin;		// For handling rectangle moves
	G42Point					Origin;			// Original mousedown point
	double					OriginalAspect;// For resizing proportionally
	G42Rect					PinRect;			// For pinning axes
	bool						Proportional;	// Always make width == height
	G42Rect					Rectangle;		// The actual rectangle
	uintf						ResizeChangeLocation;
	int16						SelectedGrabber;
};
#endif
