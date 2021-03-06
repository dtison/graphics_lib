#include "giobpoly.h"
void
G42PolyObject::DrawGrafObject (G42DrawLocation location)
{
	DrawPoints (location);
	if (Hot)
	{
		/*  Want to draw with a 2-wide solid line */
		G42WideLineStyle wide (location);
		DrawRectangle (Bounds, location);
	}
}
/*  Allows caller to initialize GrafObject instead of having to use mouse  */
void
G42PolyObject::InitObject (const G42Vector <G42Point>  & points, int16 count,
	const G42Rect & area_bounds)
{
	if (count <= 0) {
		throw (G42InvalidParameterException (0));
	}
	AreaBounds	 		= area_bounds;
	Origin 				= points [0];
	NumGrabbers 		= NumPoints = count;
	NumDrawGrabbers 	= NumPoints;
	for (int16 i = 0; i < NumPoints; i++)
		Points [i] = points [i];
	PlaceGrabbers();
	Defined = true;
	/*  Refresh Bounds Rectangles */
	UpdateBounds();
}
