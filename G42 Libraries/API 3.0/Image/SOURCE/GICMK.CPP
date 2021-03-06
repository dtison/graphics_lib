// gicmk.h - G42MouseKeyController
/*-----------------------------------------------------------------------------
	Modifications:
-----------------------------------------------------------------------------*/
#include "gicmk.h"
#include "gicmgr.h"
#include "gicsur.h"
#include "ginotify.h"
/*-----------------------------------------------------------------------------
	G42MouseKeyController member functions:
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
	Function - G42PPSurfaceController constructor
-----------------------------------------------------------------------------*/
G42MouseKeyController::G42MouseKeyController (void) 
:	
	#ifdef MSWIN
	WinCursor(0),
	#endif
	AreaController (0), Message (0), Notifier (0), Surface (0)
{
	CanvasSize.Set (0,0);
}
/*  This function is also hoping to do canvas handling for MSWIN */
void
G42MouseKeyController::ConvertPoint (G42Point &point, bool forward)
{
	if (forward)
		AreaController -> GetDisplay() -> GetScaleTranslate().Convert (point);
	else
	 	AreaController -> GetDisplay() -> GetScaleTranslate().ConvertBack (point);
}
bool
G42MouseKeyController::MouseButtonDown (void)
{
	#ifdef MSWIN
//	Is this possible?
// no, not really
	return false; // merely to keep the warning from appearing
	#endif
	#ifdef MACOS
	return Button();
	#endif
}
bool
G42MouseKeyController::ProcessMessage (G42MouseKeyMessage * /* message */)
{
	return false;
}
void
G42MouseKeyController::SetAreaController (G42AreaController *acontroller)
{
	AreaController = acontroller;
	Notifier = AreaController -> GetNotifier();
}
/*  Translates the G42CursorType into a specific cursor for each OS */
void
G42MouseKeyController::SetCursor (G42CursorType cursor)
{
	#ifdef MSWIN
	LPCTSTR cursor_id;
	bool standard_cursor = true;
	int cursor_int;
	switch (cursor)
	{
		case G42ArrowCursor:
			cursor_id = IDC_ARROW;
		break;
		case G42CrossCursor:
			cursor_id = IDC_CROSS;		
		break;
		case G42TextCursor:
			cursor_id = IDC_IBEAM;
		break;
		case G42MoveCursor:
			#if defined(WIN32)
			cursor_id = IDC_SIZEALL;
			#else
			cursor_id = IDC_SIZE;
			#endif
		break;
		case G42HorzCursor:
			cursor_id = IDC_SIZEWE;
		break;
		case G42VertCursor:
			cursor_id = IDC_SIZENS;
		break;
		case G42NWSEDiagCursor:
			cursor_id = IDC_SIZENWSE;
		break;
		case G42NESWDiagCursor:
			cursor_id = IDC_SIZENESW;
		break;
		case G42DropperCursor:
			cursor_int = G42_CURSOR_DROPPER;
			standard_cursor = false;
		break;
	}
	if (standard_cursor)
	{
		WinCursor = LoadCursor(0, cursor_id);
	}
	else
	{
		WinCursor = LoadCursor(ResourceIdentifier, MAKEINTRESOURCE(cursor_int));
	}
	if (!WinCursor)
		WinCursor = LoadCursor(0, IDC_ARROW);
	#endif
	#ifdef MACOS
	Cursor *cursor_id;	
	switch (cursor)
	{
		case G42ArrowCursor:
		case G42DropperCursor:
			cursor_id = &qd.arrow;
		break;
		case G42CrossCursor:
			cursor_id = *GetCursor (crossCursor);
		break;
		case G42TextCursor:
			cursor_id = *GetCursor (iBeamCursor);
		break;
		case G42MoveCursor:
			cursor_id = *GetCursor (plusCursor);
		break;
		case G42HorzCursor:
		case G42VertCursor:
		case G42NWSEDiagCursor:
		case G42NESWDiagCursor:
			cursor_id = *GetCursor (crossCursor);
		break;
	}
	::SetCursor (cursor_id);
	#endif
}
#ifdef MSWIN
void
G42MouseKeyController::WinSetCursor (void)
{
	if (!WinCursor)
		WinCursor = LoadCursor(0, IDC_ARROW);
	::SetCursor(WinCursor);
}
#endif
void
G42MouseKeyController::SetViewport (const G42Rect & viewport)
{
	Viewport = viewport;
}
void
G42MouseKeyController::Clear (void)
{
}
void
G42MouseKeyController::ResetDrawState (void)
{
}
