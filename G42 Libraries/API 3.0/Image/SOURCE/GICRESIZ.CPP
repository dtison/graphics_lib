// gicresiz.cpp	-- G42ImageResizeMKController
#include "gicresiz.h"
#include "gifresiz.h"
#include "gpath.h"
G42ImageResizeMKController::G42ImageResizeMKController (void)
:
	ButtonDown(false), MoveImage(true), ResizeImage(true), ZoomImage(false)
{
}
G42ImageResizeMKController::~G42ImageResizeMKController (void)
{
}
void
G42ImageResizeMKController::Draw(G42DrawLocation) // location)
{
	// only draw on mouse moves?
//	ActiveObject->Draw(location);
}
void
G42ImageResizeMKController::AdjustCursor(G42Point & point)
{
	G42CursorType cursor = G42ArrowCursor;
	int16 grabber = 0;
	if ((ResizeImage || ZoomImage) &&
		ActiveObject->TestHitGrabbers(grabber, point))
	{
		if (grabber == G42UpperLeft || grabber == G42LowerRight)
		{
			cursor = G42NWSEDiagCursor;
		}
		else if (grabber == G42UpperRight || grabber == G42LowerLeft)
		{
			cursor = G42NESWDiagCursor;
		}
		else if (grabber == G42Top || grabber == G42Bottom)
		{
			cursor = G42VertCursor;
		}
		else if (grabber == G42Left || grabber == G42Right)
		{
			cursor = G42HorzCursor;
		}
	}
	else if (MoveImage && ActiveObject->GetBounds().Contains(point))
	{
		cursor = G42MoveCursor;
	}
	SetCursor(cursor);
}
/*  Mouse Messages  */
bool
G42ImageResizeMKController::HandleMouseMessage (void)
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
G42ImageResizeMKController::HandleMouseDown (void)
{
	G42MouseDown * message = dynamic_cast<G42MouseDown *>(Message);
	if (Notifier)
	{
//		Notifier->ProcessMessage(G42NotifyMessage(G42CustomMessage, 0));
	}
	G42Point point = message->Point;
	ConvertPoint(point, false);
	// check to see if ActiveObject can handle mouse
	int16 grabber = 0;
	if (ActiveObject &&
		((ResizeImage || ZoomImage) &&
			ActiveObject->TestHitGrabbers(grabber, point)) ||
		(MoveImage && ActiveObject->GetBounds().Contains(point)))
	{
		if (message->Flags & G42ShiftDown)
		{
//			ActiveObject->SetProportional(true);
			ActiveObject->SetProportional(false);
		}
		else
		{
			ActiveObject->SetProportional(false);
		}
		ActiveObject->MouseDown(message);
		ButtonDown = true;
	}
	return true;
}
bool
G42ImageResizeMKController::HandleMouseMove (void)
{
	G42MouseMove * message = dynamic_cast<G42MouseMove *>(Message);
	/*  Do nothing is mouse button isn't down  */
	if (!ButtonDown || !ActiveObject)
	{
		G42Point point = message->Point;
		ConvertPoint(point, false);
		if (point != LastMouseMove)
		{
			AdjustCursor(point);
		}
		LastMouseMove = point;
		return false;
	}
	ActiveObject->MouseMove(message);
	return true;
}
bool
G42ImageResizeMKController::HandleMouseUp(void)
{
	if (!ButtonDown || !ActiveObject)
	{
		return false;
	}
	ButtonDown = false;
	G42MouseUp * message = dynamic_cast<G42MouseUp *>(Message);
	ActiveObject->MouseUp(message);
	SyncImage();
	return true;
}
bool
G42ImageResizeMKController::HandleMouseDoubleClick(void)
{
	if (!ActiveObject)
	{
		return false;
	}
	G42MouseDoubleClick * message =
		dynamic_cast<G42MouseDoubleClick *>(Message);
	ActiveObject->MouseDoubleClick(message);
	return true;
}
/*  Keyboard Messages  */
bool
G42ImageResizeMKController::HandleKeyMessage (void)
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
G42ImageResizeMKController::HandleKeyDown (void)
{
	bool value = false;
	G42KeyDown *message = dynamic_cast <G42KeyDown *> (Message);
	/*  TODO:  Want to process delete key - on MSWIN, this is a menu command */
	ActiveObject->KeyDown(message);
	return (value);
}
bool
G42ImageResizeMKController::HandleKeyUp (void)
{
	bool value = false;
	return (value);
}
bool
G42ImageResizeMKController::ProcessMessage (G42MouseKeyMessage *message)
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
G42ImageResizeMKController::IdleAction (void)
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
	if (ActiveObject)
	{
		if (ActiveObject->GetDefined())
		{
//			ActiveObject->DrawIdle();
//			value = true;
		}
	}
	return value;
}
/*  Creating the drawer in this function is a little wierd but not all information
	is available in the constructor for this class  */
void
G42ImageResizeMKController::SetAreaController(
	G42AreaController * area_controller)
{
	G42MouseKeyController::SetAreaController(area_controller);
	ResetDrawState();
	SetupActiveObject();
}
void
G42ImageResizeMKController::ResetDrawState(void)
{
	AreaController->SetDrawAnimated(true);
}
/*  This override is to set canvas size in all the grafobjects  */
void
G42ImageResizeMKController::SetCanvasSize(G42Size & size)
{
	G42MouseKeyController::SetCanvasSize(size);
	ActiveObject = 0;
	SetupActiveObject();
	if (ActiveObject)
	{
//		ActiveObject->SetCanvasSize(size);
		ActiveObject->SetUseAreaBounds(false);
	}
}
void
G42ImageResizeMKController::SyncImage(void)
{
	if (!ActiveObject || !AreaController || !AreaController->GetImage() ||
		!AreaController->GetImage()->HasInfo())
	{
		return;
	}
	G42Rect bounds(ActiveObject->GetBounds());
	G42ScaleTranslate scale_translate(AreaController->GetScaleTranslate());
//	bool reset_scale_trans = false;
	if (bounds.Left || bounds.Top)
	{
		// move image'
		G42Point p(bounds.Left, bounds.Top);
		scale_translate.Convert(p);
//		p.X = (int32)((double)p.X * scale_translate.GetScaleX());
		scale_translate = G42ScaleTranslate(scale_translate.GetScaleX(),
			scale_translate.GetScaleY(), p);
//		reset_scale_trans = true;
		AreaController->SetScaleTranslate(scale_translate);
	}
	if (bounds.GetWidth() !=
		AreaController->GetImage()->GetInfo().GetWidth() ||
		bounds.GetHeight() !=
		AreaController->GetImage()->GetInfo().GetHeight())
	{
		if (ResizeImage)
		{
			G42Size old_size(AreaController->GetImage()->GetInfo().GetWidth(),
				AreaController->GetImage()->GetInfo().GetHeight());
			G42Size new_size(bounds.GetWidth(), bounds.GetHeight());
			G42Path path(*AreaController);
			G42Resize * resize = new G42Resize(
				bounds.GetWidth(), bounds.GetHeight(),
				G42Resize::Interpolate);
			path.AddLink(*AreaController, *resize);
			path.AddLink(*resize, *AreaController);
			path.Start();
			path.Clear();
			G42NotifyImageResizedMessage msg(AreaController, old_size, new_size);
			Notifier->ProcessMessage(&msg);
			delete resize;
		}
		else if (ZoomImage)
		{
			// reset scale
		}
	}
	AreaController->Invalidate();
}
void
G42ImageResizeMKController::SetupActiveObject(void)
{
	if (ActiveObject || !AreaController || !AreaController->GetImage() ||
		!AreaController->GetImage()->HasInfo())
	{
		return;
	}
	G42Rect area(0, 0,
		AreaController->GetImage()->GetInfo().GetWidth() - 1,
		AreaController->GetImage()->GetInfo().GetHeight() - 1);
	ActiveObject.Set(new G42RectObject(AreaController, this));
//	ActiveObject->SetCanvasSize(CanvasSize);
	ActiveObject->InitObject(area, area);
	ActiveObject->SetSelected(false);
	ActiveObject->SetUseAreaBounds(false);
}
void
G42ImageResizeMKController::SetViewport(const G42Rect & viewport)
{
	G42MouseKeyController::SetViewport(viewport);
}
void
G42ImageResizeMKController::Clear(void)
{
	ActiveObject.Clear();
}
