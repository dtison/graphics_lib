// gicpaint.cpp -- G42ImagePaintController
#include "gicpaint.h"
#include "gpath.h"
#include "gifovl.h"
#include "giobrect.h"
#include "giobell.h"
#include "giobpoly.h"
#include "giobtext.h"
G42ImagePaintController::G42ImagePaintController (void)
:
	ButtonDown(false), AreaSelected(false), Shape(Rectangle)
{
}
G42ImagePaintController::~G42ImagePaintController (void)
{
}
void
G42ImagePaintController::Draw(G42DrawLocation location)
{
	if (!ActiveObject)
		return;
	ActiveObject->Draw(location);
}
void
G42ImagePaintController::AdjustCursor(G42Point & point)
{
	G42CursorType cursor = G42ArrowCursor;
	if (ActiveObject)
	{
		int16 grabber = 0;
		if (ActiveObject->TestHitGrabbers(grabber, point))
		{
			cursor = G42CrossCursor;
		}
		else if (ActiveObject->GetBounds().Contains(point))
		{
			cursor = G42MoveCursor;
		}
	}
	SetCursor(cursor);
}
/*  Mouse Messages  */
bool
G42ImagePaintController::HandleMouseMessage (void)
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
G42ImagePaintController::HandleMouseDown (void)
{
	G42MouseDown * message = dynamic_cast<G42MouseDown *>(Message);
	if (message->Button == G42RightButton)
	{
		if (AreaSelected)
		{
			PutAreaDown();
		}
		return true;
	}
	ButtonDown = true;
	if (Notifier)
	{
//		Notifier->ProcessMessage(G42NotifyMessage(G42CustomMessage, 0));
	}
	G42Point point = message->Point;
	ConvertPoint(point, false);
	// check to see if ActiveObject can handle mouse
	if (ActiveObject && (!ActiveObject->GetDefined() ||
		ActiveObject->GetResponseBounds().Contains(point)))
	{
		ActiveObject->MouseDown(message);
		return true;
	}
	if (AreaSelected)
	{
		PutAreaDown();
	}
	ActiveObject = CreateShape();
	G42Rect area_bounds = G42Rect(-1, -1,
		AreaController->GetImage()->GetInfo().GetWidth(),
		AreaController->GetImage()->GetInfo().GetHeight());
	ActiveObject->SetAreaBounds(area_bounds);
	ActiveObject->SetSelected(true);
	ActiveObject->MouseDown(message);
	Draw();
	return true;
}
G42GrafObject *
G42ImagePaintController::CreateShape (void)
{
	G42GrafObject * shape = 0;
	switch (Shape)
	{
		case Rectangle:
			shape = new G42RectObject(AreaController, this);
			break;
		case Polygon:
			shape = new G42PolyObject(AreaController, this);
			shape -> SetUseGrabbers (false);
			break;
		case Ellipse:
			shape = new G42EllipseObject(AreaController, this);
			break;
		case Freehand:
//			shape = new G42FreehandObject(AreaController, this);
			break;
		case Text:
			shape = new G42TextObject(AreaController, this);
			break;
	}
	return shape;
}
bool
G42ImagePaintController::HandleMouseMove (void)
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
	if (AreaSelected)
	{
		G42Rect old_rect = ActiveObject->GetResponseBounds();
		ActiveObject->MouseMove(message);
		G42Rect new_rect = ActiveObject->GetResponseBounds();
		old_rect |= new_rect;
		AreaController->GetDisplay()->GetScaleTranslate().Convert(old_rect);
		AreaController -> Invalidate (old_rect); // <Erase>
	}
	else
	{
		ActiveObject->MouseMove(message);
	}
	return true;
}
bool
G42ImagePaintController::HandleMouseUp(void)
{
	if (!ButtonDown || !ActiveObject)
	{
		return false;
	}
	ButtonDown = false;
	G42MouseUp * message = dynamic_cast<G42MouseUp *>(Message);
	ActiveObject->MouseUp(message);
	if (AreaSelected)
	{
		Draw();
	}
	else if (ActiveObject->GetDefined())
	{
		Paint();
	}
	return true;
}
bool
G42ImagePaintController::HandleMouseDoubleClick(void)
{
	if (!ActiveObject)
	{
		return false;
	}
	G42MouseDoubleClick * message =
		dynamic_cast<G42MouseDoubleClick *>(Message);
	bool was_defined = ActiveObject->GetDefined();
	ActiveObject->MouseDoubleClick(message);
	if (!was_defined && ActiveObject->GetDefined())
	{
		Paint();
	}
	return true;
}
/*  Keyboard Messages  */
bool
G42ImagePaintController::HandleKeyMessage (void)
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
G42ImagePaintController::HandleKeyDown (void)
{
	bool value = false;
	G42KeyDown *message = dynamic_cast <G42KeyDown *> (Message);
	/*  TODO:  Want to process delete key - on MSWIN, this is a menu command */
	ActiveObject->KeyDown(message);
	return (value);
}
bool
G42ImagePaintController::HandleKeyUp (void)
{
	bool value = false;
	return (value);
}
bool
G42ImagePaintController::ProcessMessage (G42MouseKeyMessage *message)
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
G42ImagePaintController::IdleAction (void)
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
			ActiveObject->DrawIdle();
			value = true;
		}
	}
	return value;
}
/*  Creating the drawer in this function is a little wierd but not all information
	is available in the constructor for this class  */
void
G42ImagePaintController::SetAreaController(
	G42AreaController * area_controller)
{
	G42MouseKeyController::SetAreaController(area_controller);
	ResetDrawState();
}
void
G42ImagePaintController::ResetDrawState(void)
{
	AreaController->SetDrawAnimated(false);
}
/*  This override is to set canvas size in all the grafobjects  */
void
G42ImagePaintController::SetCanvasSize(G42Size & size)
{
	G42MouseKeyController::SetCanvasSize(size);
	if (ActiveObject)
	{
//		ActiveObject->SetCanvasSize(size);
	}
}
void
G42ImagePaintController::PutAreaDown(void)
{
#if 0
	G42Image * image = AreaController->GetImage();
	if (!image || image->IsEmpty())
	{
		return;
	}
	// combine images
	G42PathComponent * area = AreaController;
	G42Point p = ActiveObject->GetBounds().GetTopLeft();
	p.X++;
	p.Y++;
//	if (image->IsModified() || p != OriginalUpperLeft)
	if (image->IsModified() || p != OriginalUpperLeft || Shape == Ellipse)
	{
		/*  Kludge for Heap in Region  */
		G42Region::SetHeap (& ((G42HeapManager &) image -> GetHeap()));
		G42Region * region = ActiveObject -> CreateObjectRegion();
		G42Ptr<G42Overlay> overlay(new G42Overlay(p, region));
		G42Path path(*area);
		path.AddHead(*(SelectedImageController->GetImage()));
		path.AddLink(*area, *overlay);
		path.AddLink(*(SelectedImageController->GetImage()), *overlay);
		path.AddLink(*overlay, *area);
		path.Start();
//	AreaController->ImageController->SetNeedRebuild();
	}
	DeleteArea();
#endif	
}
// this is the public one, needs a graph object also
void
G42ImagePaintController::SelectArea(const G42Rect &) // area)
{
#if 0
	if (area.IsEmpty())
	{
		return;
	}
	G42Image * image = AreaController->GetImage();
	if (!image || image->IsEmpty())
	{
		return;
	}
	GetArea(area);
	Draw();
#endif
}
// this is the private one used by mouse-up
void
G42ImagePaintController::GetArea(const G42Rect &) // area)
{
#if 0
	G42Rect active_rect(area);
	active_rect.Top--;
	active_rect.Bottom++;
	active_rect.Left--;
	active_rect.Right++;
	if (!ActiveObject)
	{
		G42RectObject * rect = new G42RectObject(AreaController, this);
		G42ThrowIfNull(rect);
		rect->InitObject(active_rect, active_rect);
		rect->SetSelected(true);
		G42Rect area_bounds = G42Rect(-1, -1,
			AreaController->GetImage()->GetInfo().GetWidth(),
			AreaController->GetImage()->GetInfo().GetHeight());
		rect->SetAreaBounds(area_bounds);
		ActiveObject = rect;
	}
	try
	{
		G42Image * image = AreaController->GetImage();
		G42Ptr<G42Crop> crop(new G42Crop(area));
		G42Path path(*image);
		path.AddLink(*image, *crop);
		path.AddLink(*crop, *(SelectedImageController->GetImage()));
		path.Start();
		SelectedImageController->SetNeedRebuild();
		image->SetModified(false); // if not modified, user hasn't changed image
		OriginalUpperLeft = area.GetTopLeft();
	}
	catch(...) // throws if no image in area
	{
		G42Rect rect = ActiveObject->GetResponseBounds();
		ActiveObject = 0;
		AreaController->GetDisplay()->GetScaleTranslate().Convert(rect);
		AreaController->Invalidate(rect);
		return;
	}
	AreaSelected = true;
	SetupActiveObject();
#endif
}
void
G42ImagePaintController::SetupActiveObject(void)
{
#if 0
	if (!ActiveObject)
	{
		G42Rect area(SelectedImageController->GetDisplay()->GetViewport().Left,
			SelectedImageController->GetDisplay()->GetViewport().Top,
			SelectedImageController->GetDisplay()->GetViewport().Left +
			SelectedImageController->GetImage()->GetInfo().GetWidth() - 1,
			SelectedImageController->GetDisplay()->GetViewport().Top +
			SelectedImageController->GetImage()->GetInfo().GetHeight() - 1);
		area.Top--;
		area.Bottom++;
		area.Left--;
		area.Right++;
		G42RectObject * rect = new G42RectObject(AreaController, this);
		rect->InitObject(area, area);
		rect->SetSelected(true);
		G42Rect area_bounds = G42Rect(-1, -1,
			AreaController->GetImage()->GetInfo().GetWidth(),
			AreaController->GetImage()->GetInfo().GetHeight());
		rect->SetAreaBounds(area_bounds);
		ActiveObject = rect;
	}
#if 0 // should never happen
	if (ActiveObject->GetBounds().GetWidth() !=
		SelectedImageController->GetImage()->GetInfo().GetWidth() + 2 ||
		ActiveObject->GetBounds().GetHeight() !=
		SelectedImageController->GetImage()->GetInfo().GetHeight() + 2)
	{
		G42Point top_left = ActiveObject->GetBounds().GetTopLeft();
		top_left.X++;
		top_left.Y++;
		G42Point image_top_left(0, 0);
		AreaController->GetDisplay()->GetScaleTranslate().Convert(image_top_left);
		if (top_left.X < image_top_left.X)
		{
			top_left.X = image_top_left.X;
		}
		if (top_left.Y < image_top_left.Y)
		{
			top_left.Y = image_top_left.Y;
		}
		G42Rect area(top_left.X, top_left.Y,
			top_left.X + SelectedImageController->GetImage()->GetInfo().GetWidth(),
			top_left.Y + SelectedImageController->GetImage()->GetInfo().GetHeight());
		area.Top--;
		area.Bottom++;
		area.Left--;
		area.Right++;
//		ActiveObject->SetCanvasSize(CanvasSize);
		ActiveObject->InitObject(area, area);
		ActiveObject->SetSelected(true);
		G42Rect area_bounds = G42Rect(-1, -1,
			AreaController->GetImage()->GetInfo().GetWidth(),
			AreaController->GetImage()->GetInfo().GetHeight());
		object->SetAreaBounds(area_bounds);
	}
#endif
#endif
}
void
G42ImagePaintController::DeleteArea(void)
{
#if 0
	if (!AreaSelected)
	{
		return;
	}
	G42Rect rect = ActiveObject->GetResponseBounds();
	AreaController->GetDisplay()->GetScaleTranslate().Convert(rect);
	AreaSelected = false;
	ActiveObject.Clear();
	SelectedImageController->GetImage()->Clear();
	SelectedImageController->GetDisplay()->Clear();
	AreaController->Invalidate(rect);
#endif
}
void
G42ImagePaintController::SetViewport(const G42Rect & viewport)
{
	G42MouseKeyController::SetViewport(viewport);
}
void
G42ImagePaintController::Clear(void)
{
	AreaSelected = false;
	ActiveObject.Clear();
}
void
G42ImagePaintController::SetShape(G42Shape shape)
{
	Shape = shape;
}
void
G42ImagePaintController::Paint(void)
{
}
