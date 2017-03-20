// gicmk.h - G42MouseKeyController
#if ! defined (GICMK_H)
#define GICMK_H
#include "gitype.h"
/*-----------------------------------------------------------------------------
	file:				gicmk.h
	author:			David Ison
	date:				10/24/96
	revision:		3.0
	This is an abstract class for making mouse key controllers
----------------------------------------------------------------------------*/
#include "gtype.h"
#include "gitype.h"
#include "gmkmsg.h"
#include "gstrtab.h" // for G42ResourceIdentifier, should be in gtype.h
// Keyboard keys
#ifdef MSWIN
const int8 G42LeftArrow 			= VK_LEFT;
const int8 G42RightArrow 			= VK_RIGHT;
const int8 G42UpArrow 				= VK_UP;
const int8 G42DownArrow 			= VK_DOWN;
#endif
#ifdef MACOS
const int8 G42LeftArrow 			= 0x1C;
const int8 G42RightArrow 			= 0x1D;
const int8 G42UpArrow 				= 0x1E;
const int8 G42DownArrow 			= 0x1F;
#endif
enum G42CursorType {G42ArrowCursor, G42CrossCursor, G42TextCursor,
							G42MoveCursor, G42HorzCursor, G42VertCursor,
							G42NWSEDiagCursor, G42NESWDiagCursor, G42DropperCursor};
/*  Forward class declarations  */
class G42Notifier;
class G42Point;
class G42AreaController;
class G42MouseKeyController
{
public:
								G42MouseKeyController (void);
	virtual 					~G42MouseKeyController (void) {}
	virtual void			Clear (void);
	void						ConvertPoint (G42Point &point, bool forward);	// Tentative
	virtual void			Draw (G42DrawLocation = 0) {}
	virtual bool			IdleAction (void) {return false;}
	G42Size					GetCanvasSize (void) {return CanvasSize;}
	virtual bool			ProcessMessage (G42MouseKeyMessage *message);
	virtual void			ResetDrawState (void);
	virtual void 			SetAreaController (G42AreaController *acontroller);
	virtual void			SetCanvasSize (G42Size &size) {CanvasSize = size;}
	virtual void			SetCursor (G42CursorType cursor);
	virtual void			SetViewport (const G42Rect & viewport);
	// Windows only function to respond to WM_SETCURSOR message
	#ifdef MSWIN
	virtual void			WinSetCursor (void);
	#endif
protected:
	bool						MouseButtonDown (void);
	// Variables
	G42AreaController *  AreaController;
	G42MouseKeyMessage 	*Message;
	G42Size					CanvasSize;		// Size of current canvas
	G42Notifier				*Notifier;
	G42ResourceIdentifier ResourceIdentifier;
	G42Surface 				*Surface;
	G42Rect					Viewport;
	#ifdef MSWIN
	HCURSOR					WinCursor;
	#endif
};
#endif