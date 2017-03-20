//  giobell.h - G42EllipseObject
#if ! defined (GIOBELL_H)
#define GIOBELL_H
/*-----------------------------------------------------------------------------
	file:			giobell.h
	author:			David Ison
	date:			11/08/96
	revision:		3.0
	Onscreen graphics object:  Ellipse
 ----------------------------------------------------------------------------*/
#include "giobquad.h"
class G42EllipseObject : public G42QuadObject
{
public:
								G42EllipseObject (G42AreaController *acontroller,
									G42MouseKeyController *mcontroller)
									: G42QuadObject (acontroller, mcontroller) {}
	virtual 					~G42EllipseObject (void) {}
	G42Region * 			CreateObjectRegion (void);
protected:
	virtual void			DrawGrafObject (G42DrawLocation location = 0);
	void						DrawEllipse (G42Rect &rect, G42DrawLocation location = 0);
};
#endif
