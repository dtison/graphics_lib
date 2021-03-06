// gictrans.cpp	-- G42ImageTransMKController
#include "gictrans.h"
#include "gpath.h"
#include "giimage.h"
#include "gicarea.h"
#include "gitweak.h"
#include "giftrans.h"
G42ImageTransMKController::G42ImageTransMKController (G42ResourceIdentifier id)
:
	ButtonDown(false)
{
	ResourceIdentifier = id;
}
G42ImageTransMKController::~G42ImageTransMKController (void)
{
}
void
G42ImageTransMKController::Draw(G42DrawLocation) // location)
{
}
/*  Mouse Messages  */
bool
G42ImageTransMKController::HandleMouseMessage (void)
{
	bool value = false;
	//  Do common processing for all mouse messages, then handle individual msg
	if (dynamic_cast<G42MouseDown *>(Message))
		value = HandleMouseDown();
	else if (dynamic_cast<G42MouseMove *>(Message))
		value = HandleMouseMove();
	else if (dynamic_cast<G42MouseUp *>(Message))
		value = HandleMouseUp();
	else if (dynamic_cast<G42MouseDoubleClick *>(Message))
		value = HandleMouseDoubleClick();
	return (value);
}
bool
G42ImageTransMKController::HandleMouseDown (void)
{
	if (!AreaController)
	{
		return false;
	}
	G42MouseDown * message = dynamic_cast<G42MouseDown *>(Message);
	G42Point point = message->Point;
	ConvertPoint(point, false);
	G42Image * image = AreaController->GetImage();
	if (!image || image->IsEmpty())
	{
		return false;
	}
	// setup image tweaker?  At least lock read
	G42Color color;
	// get color from tweaker
	G42NotifyTransChangedMessage msg(AreaController, true, color, false);
	ButtonDown = true;
	return true;
}
bool
G42ImageTransMKController::HandleMouseMove (void)
{
	G42MouseMove * message = dynamic_cast<G42MouseMove *>(Message);
	/*  Do nothing is mouse button isn't down  */
	if (!ButtonDown)
	{
		return false;
	}
	G42Point point = message->Point;
	ConvertPoint(point, false);
	G42Color color;
	// get color from tweaker
	G42NotifyTransChangedMessage msg(AreaController, true, color, false);
	return true;
}
bool
G42ImageTransMKController::HandleMouseUp(void)
{
	if (!ButtonDown)
	{
		return false;
	}
	ButtonDown = false;
	if (!AreaController || !AreaController->GetImage() ||
		!AreaController->GetImage()->HasInfo())
	{
		return false;
	}
	G42MouseUp * message = dynamic_cast<G42MouseUp *>(Message);
	G42Point point = message->Point;
	ConvertPoint(point, false);
	if (point.X < 0 || point.Y < 0 ||
		point.X >= AreaController->GetImage()->GetInfo().GetWidth() ||
		point.Y >= AreaController->GetImage()->GetInfo().GetHeight())
	{
		return true;
	}
	G42Color color;
	// get color from tweaker
	G42ImageTweaker tweaker(*AreaController->GetImage(),
		AreaController->GetDisplay()->GetActiveFrame()); // 2nd param is frame
	color = tweaker.GetColorAtPoint(point);
	// clear out tweaker (at least unlock read)
	// Set or clear transparency as appropriate
	G42Transparency * trans = new G42Transparency(color);
	G42Path path(*AreaController);
	path.AddLink(*AreaController, *trans);
	path.AddLink(*trans, *AreaController);
	path.Start();
	G42NotifyTransChangedMessage msg(AreaController, true, color, true);
	Notifier->ProcessMessage(&msg);
	
	return true;
}
bool
G42ImageTransMKController::HandleMouseDoubleClick(void)
{
	// not sure what to do with this
	return true;
}
/*  Keyboard Messages  */
bool
G42ImageTransMKController::HandleKeyMessage (void)
{
	bool value = false;
	if (dynamic_cast<G42KeyDown *>(Message))
	{
		value = HandleKeyDown();
	}
	else if (dynamic_cast<G42KeyUp *>(Message))
	{
		value = HandleKeyUp();
	}
	return (value);
}
bool
G42ImageTransMKController::HandleKeyDown (void)
{
	bool value = false;
//	G42KeyDown *message = dynamic_cast <G42KeyDown *> (Message);
	return (value);
}
bool
G42ImageTransMKController::HandleKeyUp (void)
{
	bool value = false;
	return (value);
}
bool
G42ImageTransMKController::ProcessMessage (G42MouseKeyMessage *message)
{
	G42Image * image = AreaController->GetImage();
	if (!image || image->IsEmpty())
	{
		return false;
	}
	bool value = false;
	Message = message;
	/*  Begin by broadcasting message to appropriate kind of handler  */
	if (dynamic_cast<G42MouseMessage *>(Message))
	{
		value = HandleMouseMessage();
	}
	else if (dynamic_cast<G42KeyMessage *>(Message))
	{
		value = HandleKeyMessage();
	}
	return (value);
}
bool
G42ImageTransMKController::IdleAction (void)
{
	G42Image * image = AreaController->GetImage();
	if (!image || image->IsEmpty())
	{
		return false;
	}
	if (ButtonDown)
	{
		return false;
	}
	bool value = false;
	return value;
}
/*  Creating the drawer in this function is a little wierd but not all information
	is available in the constructor for this class  */
void
G42ImageTransMKController::SetAreaController(
	G42AreaController * area_controller)
{
	G42MouseKeyController::SetAreaController(area_controller);
	SetCursor(G42DropperCursor);
	ResetDrawState();
}
void
G42ImageTransMKController::ResetDrawState(void)
{
	AreaController->SetDrawAnimated(false);
}
/*  This override is to set canvas size in all the grafobjects  */
void
G42ImageTransMKController::SetCanvasSize(G42Size & size)
{
	G42MouseKeyController::SetCanvasSize(size);
}
void
G42ImageTransMKController::SetViewport(const G42Rect & viewport)
{
	G42MouseKeyController::SetViewport(viewport);
}
void
G42ImageTransMKController::Clear(void)
{
}
