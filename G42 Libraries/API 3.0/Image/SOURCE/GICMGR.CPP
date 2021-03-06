// gicmgr.cpp - G42ControlManager
/*-----------------------------------------------------------------------------
	Modifications:
-----------------------------------------------------------------------------*/
#include "gicmgr.h"
#include "gicarea.h"
#include "gicsur.h"
#include "ginotify.h"
#include "gicmk.h"
#include "gmkmsg.h"
G42ControlManager::G42ControlManager (G42AreaController *controller,
	G42Surface surface, G42Notifier *notifier)
:
	ActiveController(0)
{
	/*  TODO: Check parameter errors (?)  */
	#if 0
	if (! controller)
		throw(parameter error)
	#endif
	AreaController		= controller;
//	ActiveController	= ???
	Notifier 			= notifier;
	Surface				= surface;	
}	
G42ControlManager::~G42ControlManager (void)
{
}
void
G42ControlManager::Draw (G42DrawLocation location)
{
	if (ActiveController) {
		ActiveController -> Draw (location);
	}
}
bool
G42ControlManager::IdleAction (void)
{
	bool value = false;
	if (ActiveController) {
		value = ActiveController -> IdleAction ();
	}
	return value;
}
bool
G42ControlManager::ProcessMessage (G42MouseKeyMessage *message)
{
	bool value = false;
	if (ActiveController) {	// MW-proofing
		value = ActiveController -> ProcessMessage (message);
	}
	return (value);
}
void
G42ControlManager::ResetDrawState (void)
{
	if (ActiveController) {	// MW-proofing
		ActiveController -> ResetDrawState ();
	}
}
void
G42ControlManager::SetCanvasSize (G42Size &size)
{
	if (ActiveController)
	{
		ActiveController -> SetCanvasSize (size);
	}
}
void
G42ControlManager::SetController (G42MouseKeyController *controller)
{
	ActiveController = controller;
	if (ActiveController)
	{
		ActiveController -> SetAreaController (AreaController);
	}
}
#ifdef MSWIN
void
G42ControlManager::WinSetCursor (void)
{
	if (ActiveController)
	{
		ActiveController -> WinSetCursor();
	}
	else
	{
		::SetCursor(LoadCursor(0, IDC_ARROW));
	}
}
#endif
void
G42ControlManager::SetViewport(const G42Rect & viewport)
{
	if (ActiveController)
	{
		ActiveController->SetViewport(viewport);
	}
}
void
G42ControlManager::ClearController(void)
{
	if (ActiveController)
	{
		ActiveController->Clear();
	}
}
