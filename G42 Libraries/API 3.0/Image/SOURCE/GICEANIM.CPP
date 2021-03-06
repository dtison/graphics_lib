// giceanim.cpp
#include "giceanim.h"
#include "gifasplt.h"
#include "gpath.h"
G42AnimatedEditController::G42AnimatedEditController (void)
:
	ButtonDown(false), LastIndex(0), MovingThumbs(false),
	MovingIndex(0)
{
}
G42AnimatedEditController::~G42AnimatedEditController (void)
{
}
void
G42AnimatedEditController::Draw(G42DrawLocation location)
{
	if (ActiveObject)
	{
		ActiveObject->Draw(location);
	}
	if (MovingThumbs)
	{
		DrawMovingIndicator(location);
	}
}
void
G42AnimatedEditController::DrawMovingIndicator(G42DrawLocation location)
{
	intf row;
	intf column;
	if (AnimatedArea->GetLimitDirection() ==
		G42MultiAreaBaseController::LimitAcross)
	{
		row = MovingIndex / AnimatedArea->GetNumThumbsAcross();
		column = MovingIndex % AnimatedArea->GetNumThumbsAcross();
	}
	else
	{
		row = MovingIndex % AnimatedArea->GetNumThumbsDown();
		column = MovingIndex / AnimatedArea->GetNumThumbsDown();
	}
	uint32 thumb_width = AnimatedArea->GetThumbWidth() +
		AnimatedArea->GetThumbSpaceBetween();
	uint32 thumb_height = AnimatedArea->GetThumbHeight() +
		AnimatedArea->GetThumbSpaceBelow() + AnimatedArea->GetThumbTextSpace() +
		AnimatedArea->GetThumbTextSpaceAbove();
	G42Point point;
	point.X = column * thumb_width -
		((AnimatedArea->GetThumbSpaceBetween()) >> 1);
	point.Y = row * thumb_height -
		((AnimatedArea->GetThumbSpaceBelow()) >> 1);
	ConvertPoint(point, true);
	#ifdef MSWIN
	G42DrawLocationVerifier dc(location, AreaController->GetSurface(), 0);
//		(WindowPalette ? WindowPalette->Pal : 0));
	G42XORStyle xor(dc);
	G42FrameStyle frame(dc);
	point -= Viewport.GetTopLeft();
	Rectangle(dc, point.X - 2, point.Y, point.X + 3, point.Y + thumb_height);
	#endif // MSWIN
}
void
G42AnimatedEditController::AdjustCursor(G42Point & /* point */)
{
	G42CursorType cursor = G42ArrowCursor;
#if 0
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
#endif
	SetCursor(cursor);
}
/*  Mouse Messages  */
bool
G42AnimatedEditController::HandleMouseMessage (void)
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
G42AnimatedEditController::HandleMouseDown (void)
{
	G42MouseDown * message = dynamic_cast<G42MouseDown *>(Message);
	if (message->Button == G42RightButton)
	{
		// remove all selections
		AnimatedArea->ClearAllSelected();
		return true;
	}
	ButtonDown = true;
	G42Point point = message->Point;
	ConvertPoint(point, false);
	// check to see if it is inside a previously selected thumb
	if (AnimatedArea->GetNumSelected())
	{
		uint32 index = AnimatedArea->GetThumbAtPoint(point);
		if (AnimatedArea->IsSelected(index))
		{
			MovingThumbs = true;
			MovingIndex = AnimatedArea->GetInsertIndexAtPoint(point);
			DrawMovingIndicator();
			return true;
		}
	}
	// check to see if ActiveObject can handle mouse
	ActiveObject.Set(new G42RectObject(AreaController, this));
	ActiveObject->SetSelected(true);
//	ActiveObject->SetCanvasSize(CanvasSize);
	ActiveObject->MouseDown(message);
	ActiveObject->SetUseAreaBounds(false);
	return true;
}
bool
G42AnimatedEditController::HandleMouseMove (void)
{
	G42MouseMove * message = dynamic_cast<G42MouseMove *>(Message);
	if (MovingThumbs)
	{
		G42Point point = message->Point;
		ConvertPoint(point, false);
		uint32 new_index = AnimatedArea->GetInsertIndexAtPoint(point);
		if (new_index != MovingIndex)
		{
			DrawMovingIndicator();
			MovingIndex = new_index;
			DrawMovingIndicator();
		}
		return true;
	}
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
G42AnimatedEditController::HandleMouseUp(void)
{
	G42MouseUp * message = dynamic_cast<G42MouseUp *>(Message);
	if (message->Button == G42RightButton)
	{
		return false;
	}
	if (MovingThumbs)
	{
		G42Point point = message->Point;
		ConvertPoint(point, false);
		DrawMovingIndicator();
		MovingIndex = AnimatedArea->GetInsertIndexAtPoint(point);
		uint32 i;
		// set up path stuff here
		// move thumbs
		i = 0;
		uint32 insert_index = MovingIndex;
		while (i < AnimatedArea->GetNumImages())
		{
			if (AnimatedArea->IsSelected(i) &&
				(i < MovingIndex || i >= insert_index))
			{
				AnimatedArea->MoveThumb(i, insert_index, false);
				insert_index++;
			}
			else
			{
				i++;
			}
		}
		MovingThumbs = false;
      AnimatedArea->Invalidate();
		return true;
	}
	if (!ButtonDown || !ActiveObject)
	{
		return false;
	}
	ButtonDown = false;
	ActiveObject->MouseUp(message);
	G42Rect area = ActiveObject->GetBounds();
	if (!ActiveObject->GetDefined()) // area.IsEmpty())
	{
		G42Point point = message->Point;
		ConvertPoint(point, false);
		area = G42Rect(point.X, point.Y, point.X, point.Y);
	}
	ActiveObject = 0;
	SelectArea(area, message->Flags & G42CtrlDown,
		message->Flags & G42ShiftDown);
	return true;
}
bool
G42AnimatedEditController::HandleMouseDoubleClick(void)
{
	// what to do here?
	
	return true;
}
/*  Keyboard Messages  */
bool
G42AnimatedEditController::HandleKeyMessage (void)
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
G42AnimatedEditController::HandleKeyDown (void)
{
	bool value = false;
	G42KeyDown *message = dynamic_cast <G42KeyDown *> (Message);
	/*  TODO:  Want to process delete key - on MSWIN, this is a menu command */
	if (ActiveObject)
	{
		ActiveObject->KeyDown(message);
	}
	return (value);
}
bool
G42AnimatedEditController::HandleKeyUp (void)
{
	bool value = false;
	return (value);
}
bool
G42AnimatedEditController::ProcessMessage (G42MouseKeyMessage *message)
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
G42AnimatedEditController::IdleAction (void)
{
#if 0
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
	#endif
	return false;
}
/*  Creating the drawer in this function is a little wierd but not all information
	is available in the constructor for this class  */
void
G42AnimatedEditController::SetAreaController(
	G42AreaController * area_controller)
{
	G42MouseKeyController::SetAreaController(area_controller);
	AnimatedArea = dynamic_cast<G42AreaAnimatedController *>(area_controller);
	ResetDrawState();
}
void
G42AnimatedEditController::ResetDrawState(void)
{
	if (AnimatedArea)
	{
		AnimatedArea->SetThumbnailMode(true);
	}
}
/*  This override is to set canvas size in all the grafobjects  */
void
G42AnimatedEditController::SetCanvasSize(G42Size & size)
{
	G42MouseKeyController::SetCanvasSize(size);
	if (ActiveObject)
	{
//		ActiveObject->SetCanvasSize(size);
	}
}
void
G42AnimatedEditController::SelectIndex(uint32 index, bool control, bool shift)
{
	if (!control && !shift)
	{
		AnimatedArea->ClearAllSelected();
	}
	uint32 start = index;
	uint32 end = index;
	if (shift)
	{
		start = min(index, LastIndex);
		end = max(index, LastIndex);
	}
	for (uint32 i = start; i <= end; i++)
	{
		if (control)
		{
			AnimatedArea->ToggleSelected(i);
		}
		else
		{
			AnimatedArea->SetSelected(i);
		}
	}
	LastIndex = start;
	AnimatedArea->Invalidate();
}
void
G42AnimatedEditController::SelectAll(void)
{
	if (!AnimatedArea)
	{
		return;
	}
	AnimatedArea->SelectAll();
	LastIndex = 0;
	AnimatedArea->Invalidate();
}
void
G42AnimatedEditController::ClearAllSelected(void)
{
	if (!AnimatedArea)
	{
		return;
	}
	AnimatedArea->ClearAllSelected();
	LastIndex = 0;
	AnimatedArea->Invalidate();
}
void
G42AnimatedEditController::DeleteSelected(void)
{
	if (!AnimatedArea || !AnimatedArea->GetNumSelected())
	{
		return;
	}
	G42Image * image = AnimatedArea->GetImage();
	if (!image)
	{
		return;
	}
	G42Ptr<G42ImageAnimatedSplitter> splitter;
	G42Vector<uint32> split_array;
	uint32 num_split = 0;
	uint32 i;
	for (i = 0; i < AnimatedArea->GetNumImages(); i++)
	{
		if (AnimatedArea->IsSelected(i))
		{
			split_array[num_split++] = i;
		}
	}
	splitter = new G42ImageAnimatedSplitter(split_array, num_split);
	G42Path path(*image);
	path.AddLink(*image, *splitter);
	path.AddLink(*splitter, *image);
	path.Start();
	// now reset thumbs
	for (i = 0; i < AnimatedArea->GetNumImages();)
	{
		if (AnimatedArea->IsSelected(i))
		{
			AnimatedArea->DeleteThumb(i, false);
		}
		else
		{
			i++;
		}
	}
	LastIndex = 0;
	AnimatedArea->Invalidate();
}
uint32
G42AnimatedEditController::GetNumSelected(void) const
{
	if (AnimatedArea)
	{
		return AnimatedArea->GetNumSelected();
	}
	return 0;
}
void
G42AnimatedEditController::SelectArea(const G42Rect & area, bool control,
	bool shift)
{
	uint32 top_left = AnimatedArea->GetThumbAtPoint(area.GetTopLeft());
	uint32 bottom_right = AnimatedArea->GetThumbAtPoint(area.GetBottomRight());
	if (top_left == bottom_right)
	{
		SelectIndex(top_left, control, shift);
		return;
	}
	AnimatedArea->ClearAllSelected();
	for (uint32 i = top_left; i <= bottom_right; i++)
	{
		AnimatedArea->ToggleSelected(i);
	}
	AnimatedArea->Invalidate();
}
void
G42AnimatedEditController::SetViewport(const G42Rect & viewport)
{
	G42MouseKeyController::SetViewport(viewport);
}
// replace animated area into path, set UseSelectedInPath, start it
void
G42AnimatedEditController::BeginProcessing(void)
{
}
// replace animated area into path, set UseSelectedInPath, start it
void
G42AnimatedEditController::Process(G42PathObject * object, G42PathComponent *)
{
}
void
G42AnimatedEditController::ClearPath(bool path_excepted)
{
	G42PathComponent::ClearPath(path_excepted);
}
void
G42AnimatedEditController::Clear(void)
{
#if 0
	AreaSelected = false;
	ActiveObject.Clear();
	SelectedImageController->GetImage()->Clear();
	SelectedImageController->GetDisplay()->Clear();
#endif
}
