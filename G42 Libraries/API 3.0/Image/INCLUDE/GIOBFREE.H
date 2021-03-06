//  giobfree.h - G42FreehandObject
#if ! defined (GIOBFREE_H)
#define GIOBFREE_H
/*-----------------------------------------------------------------------------
	file:				giobfree.h
	author:			David Ison
	date:				04/29/97
	revision:		3.0
	Onscreen graphics objects:  Freehand GrafObject
 ----------------------------------------------------------------------------*/
#include "giobpts.h"
class G42FreehandObject : public G42PointsObject
{
public:
								G42FreehandObject (G42AreaController *acontroller,
									G42MouseKeyController *mcontroller) :
									#ifdef MSWIN
									Location(0),
									#endif
									G42PointsObject (acontroller, mcontroller) {}
	virtual					~G42FreehandObject (void);
#ifdef NOTYET
	void 						InitObject (const G42Vector <G42Point>  & points,
									int16 count, const G42Rect & area_bounds);
#endif
	virtual void			MouseDown (G42MouseDown *message);
	virtual void			MouseMove (G42MouseMove *message);
	virtual void			MouseUp (G42MouseUp *message);
protected:
	// Functions
	virtual void			DrawGrafObject (G42DrawLocation location = 0);
	virtual void			DrawFreehand (G42DrawLocation location  = 0);
	#ifdef MSWIN
	G42DrawLocation		Location;
	#endif
};
#endif
