// gicmgr.h	--	G42Controller Manager
#if ! defined (GICMGR_H)
#define GICMGR_H
/*-----------------------------------------------------------------------------
	file:			gicmgr.h
	author:			David Ison
	date:			10/24/96
	revision:		3.0
	Manages the active G42MouseKeyController.
----------------------------------------------------------------------------*/
#include "gtype.h"
#include "gitype.h"
/*  Forward class declarations  */
class G42AreaController;
class G42MouseKeyController;
class G42Notifier;
class G42Image;
class G42MouseKeyMessage;
class G42ControlManager
{
public:
								G42ControlManager (G42AreaController *controller,
								G42Surface surface, G42Notifier *notifier);
	virtual 					~G42ControlManager (void);
	void						ClearController(void);
	void						Draw (G42DrawLocation location = 0);
	G42AreaController		*GetAreaController (void) {return AreaController;}
	bool 						IdleAction (void);
	bool						ProcessMessage (G42MouseKeyMessage *message);
	void						ResetDrawState(void);
	void						SetCanvasSize (G42Size &size);
	void						SetViewport (const G42Rect & viewport);
	void						SetController (G42MouseKeyController *controller);
	#ifdef MSWIN
	void						WinSetCursor (void);
	#endif
	
private:
	G42MouseKeyController	*ActiveController;
	G42AreaController		*AreaController;
	G42Image					*Image;
	G42Notifier				*Notifier;
	G42Surface 				Surface;
};
#endif