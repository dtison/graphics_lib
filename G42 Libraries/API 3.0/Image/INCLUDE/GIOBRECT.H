//  giobrect.h - G42RectObject
#if ! defined (GIOBRECT_H)
#define GIOBRECT_H
/*-----------------------------------------------------------------------------
	file:			giobrect.h
	author:			David Ison
	date:			10/25/96
	revision:		3.0
	Onscreen graphics object:  Rectangle
 ----------------------------------------------------------------------------*/
#include "giobquad.h"
class G42RectObject : public G42QuadObject
{
public:
								G42RectObject (G42AreaController *acontroller,
									G42MouseKeyController *mcontroller)
									: G42QuadObject (acontroller, mcontroller) {}
								
	virtual 					~G42RectObject (void) {}
	virtual G42Region 	* CreateObjectRegion (void);
protected:
	virtual void			DrawGrafObject (G42DrawLocation location = 0);
};
#endif
