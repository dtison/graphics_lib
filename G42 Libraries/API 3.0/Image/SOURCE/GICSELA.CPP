// gicsela.cpp	-- G42SelectedAreaController
#include "gicsela.h"
#include "gpath.h"
#include "gifcrop.h"
#include "gifovl.h"
#include "gifresiz.h"
#include "giobrect.h"
#include "giobell.h"
#include "giobpoly.h"
#include "giobfree.h"
G42SelectedAreaController::G42SelectedAreaController (void)
:
	ButtonDown(false), AreaSelected(false), Shape(Rectangle)
{}
G42SelectedAreaController::~G42SelectedAreaController (void)
{
}
void
G42SelectedAreaController::Draw(G42DrawLocation location)
{
	if (!IsAreaSelected())
		return;
	ActiveObject->Draw(location);
	if (SelectedImageController->GetNeedRebuild())
	{
		Rebuild(location);
	}
	else
	{
		SelectedImageController->GetDisplay()->Draw(location);
	}
}
void
G42SelectedAreaController::AdjustCursor(G42Point & point)
{
	G42CursorType cursor = G42ArrowCursor;
	if (AreaSelected)
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
G42SelectedAreaController::HandleMouseMessage (void)
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
G42SelectedAreaController::HandleMouseDown (void)
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
		ImageResized = false;
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
	SetSelectedScaleTranslate();
	Draw();
	return true;
}
G42GrafObject *
G42SelectedAreaController::CreateShape (void)
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
			shape = new G42FreehandObject(AreaController, this);
			break;
	}
	return shape;
}
bool
G42SelectedAreaController::HandleMouseMove (void)
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
		SyncImage(false);
		ActiveObject->SetDrawEnabled(false);
		ActiveObject->MouseMove(message);
		ActiveObject->SetDrawEnabled(true);
		SetSelectedScaleTranslate();
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
G42SelectedAreaController::HandleMouseUp(void)
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
		SetSelectedScaleTranslate();
		SyncImage();
		Draw();
	}
	else if (ActiveObject->GetDefined())
	{
		G42Rect area = ActiveObject->GetBounds();
		area.Top++;
		area.Left++;
		area.Right--;
		area.Bottom--;
		SelectArea(area);
	}
	return true;
}
bool
G42SelectedAreaController::HandleMouseDoubleClick(void)
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
		G42Rect area = ActiveObject->GetBounds();
		area.Top++;
		area.Left++;
		area.Right--;
		area.Bottom--;
		SelectArea(area);
	}
	return true;
}
/*  Keyboard Messages  */
bool
G42SelectedAreaController::HandleKeyMessage (void)
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
G42SelectedAreaController::HandleKeyDown (void)
{
	bool value = false;
	G42KeyDown *message = dynamic_cast <G42KeyDown *> (Message);
	/*  TODO:  Want to process delete key - on MSWIN, this is a menu command */
	ActiveObject->KeyDown(message);
	return (value);
}
bool
G42SelectedAreaController::HandleKeyUp (void)
{
	bool value = false;
	return (value);
}
bool
G42SelectedAreaController::ProcessMessage (G42MouseKeyMessage *message)
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
G42SelectedAreaController::IdleAction (void)
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
G42SelectedAreaController::SetAreaController(
	G42AreaController * area_controller)
{
	G42MouseKeyController::SetAreaController(area_controller);
	SelectedImageController.Set(new G42ImageController());
	SelectedImageController->SetAreaController(area_controller);
	#ifdef MSWIN
	SelectedImageController->SetSurfaceController(
		area_controller->GetSurfaceController());
	#endif
	ResetDrawState();
	/*  Kludge for Heap in Region  */
	G42Region::SetHeap (& ((G42HeapManager &) (AreaController->GetImage()) -> GetHeap()));
}
void
G42SelectedAreaController::ResetDrawState(void)
{
	AreaController->SetDrawAnimated(false);
}
/*  This override is to set canvas size in all the grafobjects  */
void
G42SelectedAreaController::SetCanvasSize(G42Size & size)
{
	G42MouseKeyController::SetCanvasSize(size);
	if (ActiveObject)
	{
//		ActiveObject->SetCanvasSize(size);
	}
}
void
G42SelectedAreaController::Rebuild(G42DrawLocation location)
{
	bool auto_draw = 	SelectedImageController->GetDisplay()->GetAutoDraw();
	SelectedImageController->GetDisplay()->SetAutoDraw(false);
	G42Path path(*(SelectedImageController->GetImage()));
	path.AddLink(*(SelectedImageController->GetImage()),
		*(SelectedImageController->GetDisplay()));
	path.Start();
	SelectedImageController->SetNeedRebuild(false);
	SelectedImageController->GetDisplay()->SetAutoDraw(auto_draw);
	if (auto_draw)
	{
		SelectedImageController->GetDisplay()->Draw(location);
	}
}
void
G42SelectedAreaController::SyncImage(bool final)
{
	SetSelectedScaleTranslate();
	G42Image * image;
	if (OriginalImage)
		image = OriginalImage;
	else
		image = SelectedImageController->GetImage();
	G42Rect bounds(ActiveObject->GetBounds());
	bounds.Normalize();
	if (bounds.GetWidth() - 2 != image->GetInfo().GetWidth() ||
		bounds.GetHeight() - 2 != image->GetInfo().GetHeight())
	{
		if (!OriginalImage)
		{
			OriginalImage = new G42Image(*SelectedImageController->GetImage());
		}
		// really need to stretch image, but this will work for now
		bool auto_draw = 	SelectedImageController->GetDisplay()->GetAutoDraw();
		SelectedImageController->GetDisplay()->SetAutoDraw(false);
		G42Resize * resize = new G42Resize(bounds.GetWidth() - 2,
			bounds.GetHeight() - 2, (final ? G42Resize::Interpolate :
				G42Resize::Average));
		G42Path path(*OriginalImage);
		path.AddLink(*OriginalImage, *resize);
		path.AddLink(*resize, *(SelectedImageController->GetImage()));
		path.Start();
		path.Clear();
		delete resize;
		SelectedImageController->GetDisplay()->SetAutoDraw(auto_draw);
		SelectedImageController->SetNeedRebuild();
		if (final)
		{
			Draw();
		}
		ImageResized = true;
	}
	else if (ImageResized && OriginalImage)
	{
		bool auto_draw = 	SelectedImageController->GetDisplay()->GetAutoDraw();
		SelectedImageController->GetDisplay()->SetAutoDraw(false);
		*(SelectedImageController->GetImage()) = *OriginalImage;
		SelectedImageController->GetDisplay()->SetAutoDraw(auto_draw);
		SelectedImageController->SetNeedRebuild();
		if (final)
		{
			Draw();
		}
		ImageResized = false;
	}
	else if (final)
	{
		Draw();
	}
	if (final)
	{
		OriginalImage = 0; // clear image saved for resize
	}
}
void
G42SelectedAreaController::PutAreaDown(void)
{
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
	if (image->IsModified() || p != OriginalUpperLeft)
//	if (image->IsModified() || p != OriginalUpperLeft || Shape == Ellipse)
	{
		G42Image * overlay_image = SelectedImageController->GetImage();
		overlay_image -> SetRegion (*ActiveObject -> CreateObjectRegion());
		G42Ptr<G42Overlay> overlay(new G42Overlay(p));
		G42Path path(*area);
		path.AddHead(*overlay_image);
		path.AddLink(*area, *overlay);
		path.AddLink(*overlay_image, *overlay);
		path.AddLink(*overlay, *area);
		path.Start();
//	AreaController->ImageController->SetNeedRebuild();
	}
	DeleteArea();
}
// this is the public one, needs a graph object also
void
G42SelectedAreaController::SelectArea(const G42Rect & area)
{
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
}
// this is the private one used by mouse-up
void
G42SelectedAreaController::GetArea(const G42Rect & area)
{
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
		image -> SetRegion (*ActiveObject -> CreateObjectRegion());
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
}
void
G42SelectedAreaController::SetupActiveObject(void)
{
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
	SetSelectedScaleTranslate();
}
void
G42SelectedAreaController::DeleteArea(void)
{
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
}
void
G42SelectedAreaController::SetSelectedScaleTranslate(void)
{
	G42ScaleTranslate display_trans(
		AreaController->GetDisplay()->GetScaleTranslate());
	G42Point translate(ActiveObject->GetBounds().GetTopLeft());
	translate.X++;
	translate.Y++;
	display_trans.Convert(translate);
	G42ScaleTranslate image_trans(display_trans.GetScaleX(),
		display_trans.GetScaleY(), translate);
	if (SelectedImageController.NotNull())
		SelectedImageController->SetCumlativeScaleTranslate(image_trans);
}
void
G42SelectedAreaController::SetViewport(const G42Rect & viewport)
{
	G42MouseKeyController::SetViewport(viewport);
	if (SelectedImageController.NotNull())
		SelectedImageController->GetDisplay()->SetViewport(viewport);
}
void
G42SelectedAreaController::BeginProcessing(void)
{
	SetupPath();
	PathImage->BeginProcessing();
	if (!Sender) // don't clear until loop finishes
	{
		PathImage = 0;
		ClearPath();
	}
	IsHead = false; // don't clear until now for Process
}
void
G42SelectedAreaController::Process(G42PathObject * object, G42PathComponent *)
{
	SetupPath();
	bool is_end = false;
	if (dynamic_cast<G42PathEnd *>(object))
	{
		is_end = true;
	}
	bool need_init = PathImage->IsEmpty();
	PathImage->Process(object, this);
	if (need_init && !PathImage->IsEmpty())
	{
		SetupActiveObject();
	}
	if (is_end)
	{
		if (PathImage->IsEmpty())
		{
			AreaSelected = false;
			ActiveObject = 0;
		}
		else
		{
			SelectedImageController->SetNeedRebuild();
		}
		if (IsHead) // don't clear out in case BeginProcessing hasn't been called yet
		{
			Sender = 0;
		}
		else
		{
			ClearPath();
			PathImage = 0;
		}
//		G42CanvasNeedsRedrawn msg(SurfaceController);
//		Notifier->ProcessMessage(&msg);
	}
}
void
G42SelectedAreaController::ClearPath(bool path_excepted)
{
	if (PathImage)
	{
		PathImage->ClearPath(path_excepted);
		PathImage = 0;
	}
	Sender = 0;
	Receiver = 0;
	G42PathComponent::ClearPath(path_excepted);
}
void
G42SelectedAreaController::SetupPath(void)
{
	if (!PathImage)
	{
		PathImage = SelectedImageController->GetImage();
		AddComponentToPath(PathImage);
		if (Receiver)
		{
			PathImage->SetReceiver(Receiver);
			Receiver->ReplaceSender(this, PathImage);
			Receiver = 0;
		}
		if (IsHead)
		{
			PathImage->SetHead();
		}
		if (Sender)
		{
			PathImage->SetSender(this);
		}
	}
}
void
G42SelectedAreaController::Clear(void)
{
	AreaSelected = false;
	ActiveObject.Clear();
	SelectedImageController->GetImage()->Clear();
	SelectedImageController->GetDisplay()->Clear();
}
void
G42SelectedAreaController::SetShape(G42Shape shape)
{
	Shape = shape;
}
