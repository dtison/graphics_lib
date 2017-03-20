//  giobgraf.h - G42GrafObject
#if ! defined (GIOBGRAF_H)
#define GIOBGRAF_H
/*-----------------------------------------------------------------------------
	file:				giobgraf.h
	author:			David Ison
	date:				10/25/96
	revision:		3.0
	Parent class for onscreen graphics objects aka rectangles, circles etc.
 	(Helper class for mouse / key controllers) 
 ----------------------------------------------------------------------------*/
#include "gmkmsg.h"
#include "gicsur.h"
//#define ROTATION_SUPPORT
#ifdef ROTATION_SUPPORT
#include <math.h>
#endif
#define TBD
/*  Grabbers constants  */
const int8 G42NumGrabbers			= 8;
#ifdef MSWIN
const int8 G42NumDrawGrabbers		= 4;
const int8 G42GrabberSize			= 9;
#endif
#ifdef MACOS
const int8 G42NumDrawGrabbers		= 8;
const int8 G42GrabberSize			= 7;
#endif
// Grabbers on the four corners
const int8 G42UpperLeft				= 0;
const int8 G42UpperRight			= 1;
const int8 G42LowerRight			= 2;
const int8 G42LowerLeft				= 3;
// Extended, (Mac-style but emulated on MSWIN) grabbers
const int8 G42Top						= 4;
const int8 G42Right					= 5;
const int8 G42Bottom					= 6;
const int8 G42Left					= 7;
class G42Grabber
{
public:
								G42Grabber (void) 
									{GrabberPoint.X = GrabberPoint.Y = 0; GrabberRect.SetNull();}
								~G42Grabber (void) {};
	void						Offset (int32 dx, int32 dy) 
								{GrabberRect.Offset (dx, dy); GrabberPoint.Offset (dx, dy);}
	G42Point 				GrabberPoint;	// For drawing, cannot pre-calculate
	G42Rect 					GrabberRect;
};
class G42AreaController;
class G42Rect;
class G42Region;
class G42GrafObject
{
public:
									G42GrafObject (G42AreaController *acontroller,
									G42MouseKeyController *mcontroller);
	virtual						~G42GrafObject (void) {}
	void							ConvertPoint (G42Point &point, bool forward)
										{MouseKeyController -> ConvertPoint (point, forward);}
	virtual G42Region     * CreateObjectRegion (void) {return 0;}
	virtual void				Draw (G42DrawLocation location = 0);
	virtual void				DrawGrafObject (G42DrawLocation /* location */ = 0) {}	// Draws private UI
	virtual void				DrawIdle (void);
	const G42Rect &				GetBounds (void) const {return Bounds;}
	bool							GetDefined (void) {return Defined;}
	bool							GetModified (void) {return Modified;}
	const G42Rect &			GetResponseBounds (void) const {return ResponseBounds;}
	virtual	void				KeyDown (G42KeyDown * /* message */) {}
	virtual	void				KeyUp (G42KeyUp * /* message */) {}
	virtual	void				MouseDoubleClick (G42MouseDoubleClick * /* message */) {}
	virtual	void				MouseDown (G42MouseDown * /* message */) {}
	virtual	void				MouseMove (G42MouseMove * /* message */) {}
	virtual	void				MouseUp (G42MouseUp * /* message */) {}
	virtual	void				SetAreaBounds (G42Rect & area_bounds) 
										{AreaBounds = area_bounds;}
	virtual	void				SetDrawEnabled (bool enabled) {DrawEnabled = enabled;}		
	virtual  void				SetUseAreaBounds (bool use) {UseAreaBounds = use;}
	virtual	void				SetUseGrabbers	(bool use) {UseGrabbers = use;}
	virtual  void				SetHot (bool hot);		// Set to draw UI Hot
	void							SetSelected (bool selected);
	bool							TestHitGrabbers (int16 &, G42Point point);
protected:
	// Functions	
	void 							CleanupDraw (G42DrawLocation location, bool release);
	void							ClipPointToRect (G42Point &point, 
										const G42Rect &rect) const;
	void							ClipRectToRect (G42Rect &rect, G42Rect &bounds, 
										int32 &xclip, int32 &yclip) const;		
	void							DrawGrabbers (G42DrawLocation location);
	void							DrawRectangle (G42Rect &rect, G42DrawLocation location = 0);
	bool							HandleArrowKeys (G42KeyDown *message, G42Rect rect, 
										int32 &dx, int32 &dy);
	#ifdef NOTYET
	void							HandleBoundedMove (G42Point &point, G42Point &origin, 
										G42Rect &rect, G42Rect &bounds);
	#endif
	bool							PrepareDraw (G42DrawLocation &location);
	void							RedrawHighlighted (bool erase = true);	
	// Instance Variables
	G42Rect						AreaBounds;		// Rectangle this object lives in, usually canvas
	G42AreaController 		*AreaController;
	G42Rect						Bounds;			// Position and size in canvas
	bool							Defined;			// True if GrafObject has been defined
	G42Display					*Display;		// Convenience for Windows
	bool							DrawEnabled;	// Toggle for drawing
	G42Vector <G42Grabber>	Grabbers;		// Grabbers (if needed)
	bool							Highlighted;	// Object is moving or resizing
	bool							Hot;
	bool							Modified;		// True when GrafObject changes size, etc. 
	G42MouseKeyController 	*MouseKeyController; // Owner of this object
 	int16							NumDrawGrabbers;		// Number grabbers that will be drawn
	int16							NumGrabbers;			// Number of grabbers for this GrafObject
	G42Rect						ResponseBounds;
	#ifdef ROTATION_SUPPORT
	G42Grabber					RotateGrabber;	// Rotate grabber
	double						RotateAngle;	// 
	#endif
	bool							Selected;		// True if selected
	G42Surface					Surface;			// Convenience for Windows
	bool							UseAreaBounds;
	bool							UseGrabbers;	// Normally true, set false for no grabbers
	bool							XORDraw;
	// Enums
	#ifdef ROTATION_SUPPORT
	enum 							{None, Moving, Resizing, Rotating} MousemoveMode;
	#else
	enum 							{None, Moving, Resizing} MousemoveMode;
	#endif
};
#endif
