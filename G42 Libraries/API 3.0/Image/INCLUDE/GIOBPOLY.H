//  giobpoly.h - G42PolyObject
#if ! defined (GIOBPOLY_H)
#define GIOBPOLY_H
/*-----------------------------------------------------------------------------
	file:				giobpoly.h
	author:			David Ison
	date:				11/18/96, mod 4-29-97
	revision:		3.0
	Onscreen graphics objects:  Polygon GrafObject
 ----------------------------------------------------------------------------*/
#include "giobpts.h"
class G42PolyObject : public G42PointsObject
{
public:
								G42PolyObject (G42AreaController *acontroller,
									G42MouseKeyController *mcontroller) :
								G42PointsObject (acontroller, mcontroller) {}
	void 						InitObject (const G42Vector <G42Point>  & points, 
									int16 count, const G42Rect & area_bounds);
protected:
	// Functions
	virtual void			DrawGrafObject (G42DrawLocation location = 0);
};
#endif
