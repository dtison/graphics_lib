//  giobpts.h - G42PointsObject
#if ! defined (GIOBPTS_H)
#define GIOBPTS_H
/*-----------------------------------------------------------------------------
	file:				giobpts.h
	author:			David Ison
	date:				04/29/97
	revision:		3.0
	Onscreen graphics objects:  Points GrafObject [for Polygon and Freehand]
 ----------------------------------------------------------------------------*/
#include "giobgraf.h"
#include "gitype.h"
#define TBD
const G42MaxPoints = 4096;
class G42PointsObject : public G42GrafObject
{
public:
								G42PointsObject (G42AreaController *acontroller,
									G42MouseKeyController *mcontroller);
	G42Region * 			CreateObjectRegion (void);
	int16						GetNumPoints (void) {return NumPoints;}
	G42Point					GetPoint (int16 index);
	virtual void			KeyDown (G42KeyDown *message);
	virtual void			KeyUp (G42KeyUp *message);
	virtual void			MouseDoubleClick (G42MouseDoubleClick *message);
	virtual void			MouseDown (G42MouseDown *message);
	virtual void			MouseMove (G42MouseMove *message);
	virtual void			MouseUp (G42MouseUp *message);
protected:
	// Functions
	void						DrawPoints (G42DrawLocation location);
	void						PlaceGrabbers (void);
	void						UpdateBounds (void);
	// Instance variables
	#ifdef ACCELERATE_KEYS
	bool						KeyHeldDown;	// True when KeyDown msg rec'd
	int32						KeyDownTicks;	// For key move-accelerating
	#endif
	G42Point					MoveOrigin;		// For handling rectangle moves
	int16						NumPoints;
	G42Vector <G42Point>	Points;
	G42Point					Origin;			// Original mousedown point
	int16						SelectedGrabber;
};
#endif
