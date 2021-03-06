//  giobrect.h - G42RectangleObject
	
#include "giobrect.h"
#include "gidsetup.h"
/*  Draws private UI  */
void
G42RectObject::DrawGrafObject (G42DrawLocation location)
{
	if (Hot)
	{
		/*  Want to draw with a 2-wide solid line */
		G42WideLineStyle wide (location);
		DrawRectangle (Rectangle, location);
	}
	else
	{
		DrawRectangle (Rectangle, location);
	}
}
G42Region * 
G42RectObject::CreateObjectRegion (void)
{
	/*  TODO:  What about mask parameter?  */
	/*  Normalize Bounds to 0, 0 */
	G42Rect rect = Bounds.OffsetBy (-Bounds.Left, -Bounds.Top);
	G42Region * region = new G42Region (rect, G42Region::Rectangle);
	G42ThrowIfNull (region);
	return region;	
}
