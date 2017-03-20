// gicmapr.cpp	-- G42MapController
	
#include "gicmapr.h"
#include "giobpnt.h"
#include "giobrect.h"
#include "giobell.h"
#include "giobpoly.h"
#include "gexcept.h"
/*  TODO's and notes:
	1.  Crash on new map [done]
	2.  Crash on open map [done]
	3.  Drag listbox reorder bug [done]
	4.  Dbl-click not query url [done]
	5.  Add default button [done]
	6.  Crash on Delete element
	
	[Default elements are modeled as vanilla GrafObjects]
	if (! ImageMap) {
		throw (G42InternalException (0));
	}
///		ImageMap->SetImageSize(
//			(int)(Image->GetInfo().Width), (int)(Image->GetInfo().Height));	
*/
G42MapController::G42MapController (void)
: 
	ActiveObject (0), ButtonDown (false), ImageMap (0), LastHotIndex (G42MapperNone), 
	MapTool (MapEditTool), NumElements (0), SelectedElement (G42MapperNone)
{
	LastMouseMove.Set (0, 0);
	NewImageMap (false);
	G42ThrowIfNull (ImageMap);
}
G42MapController::~G42MapController (void)
{
	if (ActiveObject)
	{
		delete ActiveObject;
		ActiveObject = 0;
	}
	if (ImageMap)
	{
		delete ImageMap;
		ImageMap = 0;
	}
	/*  Delete all the G42GrafObjects  */
	for (int16 i = 0; i < NumElements; i++)
		delete GrafObjects [i];
}
// TODO:  Need something like this for SetElement()
void
G42MapController::AddElement (const G42MapElement &element, int16 index)
{
	if (index > (NumElements + 1)) {
		throw (G42InvalidParameterException (0));
	}
	if (element.Type == G42MapElement::Default)
	{
		/*  See if map already has a default  */
		if (ImageMap -> HasDefault()) {
			return;
		}
		/*  Make sure index is 0  */	
		index = 0;
	}
	/*  Add to image map  */
	ImageMap -> AddElement (element, index);
	/*  Move GrafObjects down  */
	for (int16 i = (NumElements - 1); i >= index; i--)
		GrafObjects [i + 1] = GrafObjects [i];
	/*  Make a new GrafObject  */
	GrafObjects [index] = NewGrafObject (element.Type, &element);	
	NumElements++;
	/*  Send off an add element notification  */
	if (Notifier) {
		Notifier -> ProcessMessage (&G42AddElement (index));
	}
}
void
G42MapController::AddMapElement (G42QueryURL &query, const G42GrafObject *object)
{
	G42MapElement element;
	element.URL 	 = (char *) query.GetURL();
	element.Comment = (char *) query.GetComment();
	element.AltText = (char *) query.GetAltText();
	SetElementInfo (element, MapTool);
	UpdateCoords (element, object);
	ImageMap -> AddElement (element);
}
void
G42MapController::AdjustCursor (G42Point &point)
{
	G42CursorType cursor = G42ArrowCursor;
	if (MapTool == MapEditTool)
	{
		if (SelectedElement != G42MapperNone)
		{
			bool check_bounds = false;	// To check if point is in selected element's bounds
			/*  Get the element type  */
			int16 index = ImageMap -> GetSelectedElement();
			const G42MapElement * element = GetElement (index);
			if (element -> Type == G42MapElement::Polygon)
			{
				check_bounds = true;
			}
			else
			{
				/*  Rectangle and Ellipse handler (also Point, by default) */
				int16 grabber = 0;
				if (GrafObjects [SelectedElement] -> TestHitGrabbers (grabber, point))
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
				else
				{
					check_bounds = true;
				}
			}
			if (check_bounds)
			{
				if (GrafObjects [SelectedElement] -> GetBounds().Contains (point))
				{
					cursor = G42MoveCursor;
				}
			}
		}
	}
	else
		cursor = G42CrossCursor;
	SetCursor (cursor);
}
void
G42MapController::CBTextToMap (const char * buffer, uintf buffer_len)
{
	if (buffer == 0) {
		throw (G42InvalidParameterException (0));
	}
	G42ImageMap * map = new G42ImageMap (buffer, buffer_len);
	if (map)
	{
		int16 num_elements =  map -> GetNumElements();
		for (int16 i = 0; i < num_elements; i++)
		{
			ImageMap -> AddElement ((G42MapElement) *ImageMap -> GetElement (i));
		}
	}
	delete map;
}
void  
G42MapController::DeleteElement (int16 index) 
{
	if (index < 0 || index >= NumElements)
		throw (G42InvalidParameterException (0));
	/*  Delete it from the map  */
	ImageMap -> DeleteElement (index);
	/*  Update the user interface  */
	G42GrafObject * object = GrafObjects [index];
	/*  Save GrafObject's rectangle  */
	G42Rect rect = object -> GetResponseBounds();
	delete object;
	NumElements--;
	for (int i = index; i < NumElements; i++)
		GrafObjects [i] = GrafObjects [i+1];
	/*  Reset SelectedElement  */
	SelectedElement = ImageMap -> HasSelectedElement() ?
		ImageMap -> GetSelectedElement() : G42MapperNone;
	LastHotIndex = G42MapperNone;
	/*  Refresh  */
	AreaController -> Invalidate (rect); // <Erase>
	
}
void
G42MapController::DeleteSelectedElement (void)
{
	ImageMap -> DeleteElement (SelectedElement);
	/*  Update GrafObjects  */
	AreaController -> Invalidate (GrafObjects [SelectedElement] -> 
		GetResponseBounds()); // <Erase>
	delete GrafObjects [SelectedElement];
	NumElements--;
	for (int16 i = SelectedElement; i < NumElements; i++)
		GrafObjects [i] = GrafObjects [i + 1];
	/*  Reset SelectedElement  */
	SelectedElement = ImageMap -> HasSelectedElement() ?
		ImageMap -> GetSelectedElement() : G42MapperNone;
	LastHotIndex = G42MapperNone;
// ges: I don't think we need to do this?
	AreaController -> Invalidate();
}
/*  Invokes drawing   */
void
G42MapController::Draw (G42DrawLocation location)
{
	// Different modes, call Draw(), DrawHot(), turn select on, whatever...
	/*  Draw GrafObject under construction  */
	if (ActiveObject) {
		ActiveObject -> Draw (location);
	}
	/*  Draw all the map elements  */
	for (int16 i = 0; i < NumElements; i++)
		GrafObjects [i] -> Draw (location);
}
const G42MapElement 
*G42MapController::GetElement (int16 index)
{
	if (index < 0 || index > (NumElements - 1)) {
		throw (G42InvalidParameterException (0));
	}		
	return ImageMap -> GetElement (index);
}
/*  Mouse Messages  */
bool
G42MapController::HandleMouseMessage (void)
{
	bool value = false;
	/*  Do common processing for all mouse messages, then handle individual msg  */
	if (dynamic_cast <G42MouseDown *> (Message))
		value = HandleMouseDown();		
	else
		if (dynamic_cast <G42MouseMove *> (Message))
			value = HandleMouseMove();
		else
			if (dynamic_cast <G42MouseUp *> (Message))
				value = HandleMouseUp();
			else
				if (dynamic_cast <G42MouseDoubleClick *> (Message))
					value = HandleMouseDoubleClick();
	return (value);
}
bool
G42MapController::HandleMouseDoubleClick (void)
{
	bool value;
  	G42MouseDoubleClick *message = dynamic_cast <G42MouseDoubleClick *> (Message);
	/*  If an element is selected, pass msg on to grafobject and send URL query  */
	if (SelectedElement != G42MapperNone) 
	{
		GrafObjects [SelectedElement] -> MouseDoubleClick (message);	
		const G42MapElement * element = ImageMap -> GetElement (SelectedElement);
		G42QueryURL query (element -> URL, element -> Comment, element -> AltText);
		if (Notifier && Notifier -> ProcessMessage (&query))
		{
			G42MapElement new_element;
			new_element.URL 	 = (char *) query.GetURL();
			new_element.Comment = (char *) query.GetComment();
			new_element.AltText = (char *) query.GetAltText();
			ImageMap -> SetElement (SelectedElement, new_element);
		}
	}
	else
	{
		if (MapTool != MapEditTool) 
		{
			ActiveObject -> MouseDoubleClick (message);
			FinishActiveObject();
		}
	}
	value = true;
	return (value);
}
bool
G42MapController::HandleMouseDown (void)
{
	bool value;
	G42MouseDown *message = dynamic_cast <G42MouseDown *> (Message);
	ButtonDown = true;
	#if 0
	if (Notifier)
		Notifier -> ProcessMessage (G42NotifyMessage (G42CustomMessage, 0));
	#endif
	
	G42Point point = message -> Point;
	ConvertPoint (point, false);
	int16 index = G42MapperNone;
	/*  ActiveObject gets priority for msgs passed to it (only polygon tool) */
	if (ActiveObject)
	{
		ActiveObject ->  MouseDown (message);
		return true;
	}	
	
	/*  See if there is an element pointed to by the mouse  */
	if (SelectedElement != G42MapperNone) {
		if (GrafObjects [SelectedElement] -> GetResponseBounds().Contains (point))
			index = SelectedElement;
	}
	/*  If not, ask map code to find it  */
	if (index == G42MapperNone) {
		index = ImageMap -> GetIndexAtPoint (point.X, point.Y);
	}
	/*  Un-select previously selected grafobject  */
	if (SelectedElement != G42MapperNone)
	{
		GrafObjects [SelectedElement] -> SetHot (false);
		GrafObjects [SelectedElement] -> SetSelected (false);
		SelectedElement = G42MapperNone;
	}
	if (MapTool == MapEditTool)
	{
		/*  Select a new grafobject  */
		if (index != G42MapperNone)
		{
			GrafObjects [index] -> SetHot (false);
			GrafObjects [index] -> SetSelected (true);
			GrafObjects [index] -> MouseDown (message); 
			SelectedElement = index;
		}
	}
	else {
		HandleNewMapTool (message);
	}
	ImageMap -> SelectElement (SelectedElement);
	/*  Send off a selected element notification  */
	if (Notifier) {
		Notifier -> ProcessMessage (&G42SelectedElement (index));
	}
	AdjustCursor (point);
	value = true;
	return (value);
}
bool
G42MapController::HandleMouseMove (void)
{
	G42MouseMove *message = dynamic_cast <G42MouseMove *> (Message);
	G42Point point = message -> Point;
	ConvertPoint (point, false);
	if (point == LastMouseMove)
		return false;
	/*  Dispatch to AdjustCursor if a button isn't down  */
	if (! message -> Button)
	{
		AdjustCursor (point);
	}
	
	/*  Ordinary case is when button isn't down  */
	if (! ButtonDown)
	{
		int16 index = ImageMap -> GetIndexAtPoint (point.X, point.Y);
		if (index != LastHotIndex)
		{
			/*  First cool the previous hot GrafObject */
			if (LastHotIndex != G42MapperNone) {
				GrafObjects [LastHotIndex] -> SetHot (false);
			}
			if (MapTool == MapEditTool && index != G42MapperNone)
			{
				/*  Heat the index at cursor position  */
				GrafObjects [index] -> SetHot (true);
				LastHotIndex = index;
				/*  Send off a hot element notification  */
				if (Notifier) {
					Notifier -> ProcessMessage (&G42HotElement (index));
				}
			}
			else {
				LastHotIndex = G42MapperNone;
			}
		}
	}	
	else
	{
		if (SelectedElement != G42MapperNone) {
			GrafObjects [SelectedElement] -> MouseMove (message);	
		}
		else
		{
			/*  If active MapTool is not edit, we have a valid ActiveObject  */
			if (MapTool != MapEditTool) {
				ActiveObject -> MouseMove (message);
			}
		}
	}
	LastMouseMove = point;	
	return true;
}
bool
G42MapController::HandleMouseUp (void)
{	
	bool value = false;
	ButtonDown = false;
	G42MouseUp *message = dynamic_cast <G42MouseUp *> (Message);
	G42GrafObject * object = 0;
	/*  If a GrafObject's selected, pass the msg on to it  */
	if (SelectedElement != G42MapperNone)
	{
		object = GrafObjects [SelectedElement];
		bool modified = object -> GetModified();
		object -> MouseUp (message);
		if (modified)	{
			UpdateCoords (SelectedElement, GrafObjects [SelectedElement] );
		}
	}
	else
	{
		/*  If active MapTool is not edit, we have a valid ActiveObject  */
		// GES: unless we got a mouse up without a mouse down (possible on MSWIN)
		if (MapTool != MapEditTool && ActiveObject)
		{
			object = ActiveObject;
			object -> MouseUp (message);
			/*  Don't finish object if it's a Polygon  */
			if (dynamic_cast <G42PolyObject *> (ActiveObject) == 0) {
				FinishActiveObject();
			}
		}
	}
	#ifdef NOTYET
	if (object) {
		AreaController -> Invalidate (object -> GetResponseBounds());
	}
	#else
	AreaController -> Invalidate();
	#endif
	
	return (value);
}
void
G42MapController::FinishActiveObject (void)
{
	/*  Ask notifier for URL information  */
	G42QueryURL query;
	if (Notifier && Notifier -> ProcessMessage (&query))
	{
		/*  Update our internal list  */
		NumElements++;
		SelectedElement = (int16) ((int16) NumElements - (int16) 1);
		GrafObjects [SelectedElement] = ActiveObject;
		GrafObjects [SelectedElement] -> SetSelected (true);
		/*  Add it to the the map  */
		AddMapElement (query, ActiveObject);
		/*  Select in the map  */
		ImageMap -> SelectElement (SelectedElement);
		/*  Send off an add element notification  */
		if (Notifier) {
			Notifier -> ProcessMessage (&G42AddElement (SelectedElement));
		}
	}
	else {
		delete ActiveObject;
	}
	ActiveObject = 0;
}
void
G42MapController::HandleNewMapTool (G42MouseDown *message)
{
	if (MapTool == MapEditTool)
		return;
	/*  Create new GrafObject for (under construction) new element  */
	G42MapElement::G42ElementType type = Tool2ElementType (MapTool);
	G42GrafObject *object = NewGrafObject (type);
	G42Rect area_bounds = G42Rect (0, 0, 
		AreaController->GetImage()->GetInfo().GetWidth() - 1,
		AreaController->GetImage()->GetInfo().GetHeight() - 1);
	object -> SetAreaBounds (area_bounds);
	object -> MouseDown (message);
	ActiveObject = object;
}
/*  Keyboard Messages  */
bool
G42MapController::HandleKeyMessage (void)
{
	bool value = false;
	/*  Do common processing for all key messages, then handle individual msg  */
	if (dynamic_cast <G42KeyDown *> (Message))
		value = HandleKeyDown();
	else
		if (dynamic_cast <G42KeyUp *> (Message))
			value = HandleKeyUp();
	return (value);
}
bool
G42MapController::HandleKeyDown (void)
{
	if (! NumElements) {
		return false;
	}
	bool value = false;
	G42KeyDown *message = dynamic_cast <G42KeyDown *> (Message);
	if (SelectedElement != G42MapperNone)
	{
		/*  First try to process delete key  */
		if (message -> Key == G42DeleteKey) {
			DeleteSelectedElement();
		}
		else
		{
			/*  Let the GrafObject have it  */
			GrafObjects [SelectedElement] -> KeyDown (message); 
//			if (true)	// Can't test for GrafObject modified (doesn't work with keystrokes yet)
			{
				UpdateCoords (SelectedElement, GrafObjects [SelectedElement]);
				AreaController -> MakeVisible (GrafObjects [SelectedElement] -> GetBounds());
			}
		}
	}
	return (value);
}
bool
G42MapController::HandleKeyUp (void)
{
	if (! NumElements)	{
		return false;
	}
		
	bool value = false;
	return (value);
}
bool
G42MapController::IdleAction (void)
{
	bool value = false;
	if (ButtonDown)
		return false;
//return false;
	#ifdef OLDWAY
	if (ImageMap -> HasSelectedElement())
	{
		if (ImageMap -> GetElement (ImageMap->GetSelectedElement()) -> Type != 
			G42MapElement::Default)
		{
		}
	}
	#endif
		
	if (SelectedElement != G42MapperNone) 
	{
		G42GrafObject *object = GrafObjects [SelectedElement];
		if (object -> GetDefined()) 
		{
			object -> DrawIdle();
			value = true;	
		}
	}
	return value;
}
// TODO:  Check for bogus values in the element parameter  */
G42GrafObject
*G42MapController::NewGrafObject (const G42MapElement::G42ElementType type, 
	const G42MapElement *element)
{
	G42GrafObject *object = 0;
	G42Rect area_bounds = G42Rect (0, 0, 
		AreaController->GetImage()->GetInfo().GetWidth() - 1,
		AreaController->GetImage()->GetInfo().GetHeight() - 1);
	switch (type)
	{
		case G42MapElement::Rectangle:
      {
			G42RectObject *rect_object = new G42RectObject (AreaController, this);
			G42ThrowIfNull (rect_object);
			if (element) {
				rect_object -> InitObject (G42Rect (element -> Points [0], 	
					element -> Points [1]), area_bounds);
			}
			object = rect_object;
		}
		break;
		case G42MapElement::Circle:
      {
			G42EllipseObject *ellipse_object = new G42EllipseObject (AreaController, this);
			G42ThrowIfNull (ellipse_object);
			ellipse_object -> SetProportional (true);
			if (element) {
				ellipse_object -> InitObject (G42Rect (element -> Points [0], 	
					element -> Points [1]), area_bounds);
			}
			object = ellipse_object;
		}
		break;
		case G42MapElement::Point:
      {
			G42PointObject *point_object = new G42PointObject (AreaController, this);
			G42ThrowIfNull (point_object);
			if (element) {
				point_object -> InitObject (element -> Points [0], area_bounds);
			}
			object = point_object;
		}
		break;
		case G42MapElement::Polygon:
      {
			G42PolyObject *poly_object = new G42PolyObject (AreaController, this);
			G42ThrowIfNull (poly_object);
			if (element) {				
				poly_object -> InitObject (element -> Points, 
					element -> NumPoints, area_bounds);
			}
	
			object = poly_object;
 		}
		break;
		case G42MapElement::Default:
      {
			G42GrafObject *graf_object = new G42GrafObject (AreaController, this);
			G42ThrowIfNull (graf_object);
			object = graf_object;
		}
		break;
	}
	return object;
}
void
G42MapController::MapToCBText (char * buffer, uintf max_buffer_len)
{
	if (buffer == 0) {
		throw (G42InvalidParameterException (0));
	}
	ImageMap -> Save (buffer, max_buffer_len, ClientSide);
}
void
G42MapController::MoveElementPos (int16 new_pos, int16 old_pos) 
{
	if (old_pos < 0 || old_pos >= NumElements 
		|| new_pos < 0 || new_pos >= NumElements 
		|| old_pos == new_pos)
		throw (G42InvalidParameterException (0));
		
	/*  Reorder the map  */
	ImageMap -> MoveElementPos (new_pos, old_pos);
	/*  Update the user interface  */
	G42GrafObject * object = GrafObjects [old_pos];
	if (old_pos > new_pos)
	{
		for (uintf i = old_pos; i > new_pos; i--) {
			GrafObjects [i] = GrafObjects [i - 1];
		}
	}
	else
	{
		for (uintf i = old_pos; i < new_pos; i++)
		{
			GrafObjects [i] = GrafObjects [i + 1];
		}
	}
	GrafObjects [new_pos] = object;
	SelectedElement = ImageMap -> GetSelectedElement();
	/*  Reset SelectedElement  */
	SelectedElement = ImageMap -> HasSelectedElement() ?
		ImageMap -> GetSelectedElement() : G42MapperNone;
	LastHotIndex = G42MapperNone;
	/*  Refresh  */
	AreaController -> Invalidate();
}
void
G42MapController::NewImageMap (bool refresh)
{
	G42ImageMap * map = new G42ImageMap;
	/*  We may need to do clobber map notification  */
	if (NumElements > 0 && Notifier) {
		Notifier -> ProcessMessage (&G42ClobberMap());	
	}
	SetImageMap (map, refresh);
}
bool
G42MapController::OpenImageMap (const G42FilePath & path)
{
	bool value = false;
	G42ImageMap *map = new G42ImageMap (path);
	G42ThrowIfNull (map);
	if (! map -> IsValid())
		throw (G42InternalException (0));	// TODO:  What exception is this?
 	SetImageMap (map);							// TODO:  Need a return value on this?
	return value;
}
bool
G42MapController::ProcessMessage (G42MouseKeyMessage *message)
{
	bool value = false;
	Message = message;
	/*  Begin by broadcasting message to appropriate kind of handler  */
	if (dynamic_cast <G42MouseMessage *> (Message))
		value = HandleMouseMessage();		
	else
		if (dynamic_cast <G42KeyMessage *> (Message))
			value = HandleKeyMessage();
	return (value);
}
void
G42MapController::SelectedElementToCBText (char * buffer, uintf max_buffer_len)
{
	if (buffer == 0) {
		throw (G42InvalidParameterException (0));
	}
	int16 index = ImageMap -> GetSelectedElement();	
	if (index > 0)
	{
		G42ImageMap map;
		map.AddElement ((G42MapElement) *ImageMap -> GetElement (index));
		map.Save (buffer, max_buffer_len, ClientSide);
	}
}
void
G42MapController::SelectElement (int16 index)
{
	if (index < 0 || index > ImageMap -> GetNumElements()) {
		throw (G42InvalidParameterException (0));
	}
	/*  Select element in map  */
	ImageMap -> SelectElement (index);
	/*  Un-select previously selected grafobject  */
	if (SelectedElement != G42MapperNone)
	{
		GrafObjects [SelectedElement] -> SetHot (false);
		GrafObjects [SelectedElement] -> SetSelected (false);
		SelectedElement = G42MapperNone;
	}
	if (index != G42MapperNone)
	{
		/*  Select the grafobject  */
		GrafObjects [index] -> SetHot (false);
		GrafObjects [index] -> SetSelected (true);
		SelectedElement = index;
	}
	/*  Send off a selected element notification  */
	if (Notifier) {
		Notifier -> ProcessMessage (&G42SelectedElement (index));
	}
}
/*  Creating the drawer in this function is a little weird but not all information 
	is available in the constructor for this class  */
void
G42MapController::SetAreaController (G42AreaController *acontroller)
{
	G42MouseKeyController::SetAreaController (acontroller);
	ResetDrawState ();
}
void
G42MapController::ResetDrawState (void)
{
	AreaController -> SetDrawAnimated (true);
}
/*  For some reason, the selected element had to be saved and set afterwards.  
	Q:  What else might need to be saved ?  */
void					
G42MapController::ResizeImageMap (const G42Size & prev_size, 
	const G42Size & new_size, bool refresh)
{
	double scale_x = (double) new_size.Width / (double) prev_size.Width;
	double scale_y = (double) new_size.Height / (double) prev_size.Height;
	/*  Save the selected element  */
	int16 selected_element = G42MapperNone;
	if (ImageMap -> HasSelectedElement()) {
		selected_element = ImageMap -> GetSelectedElement();
	}
	G42Vector <G42Point> points;
	G42Rect area_bounds = G42Rect (0, 0, 
		AreaController->GetImage()->GetInfo().GetWidth() - 1,
		AreaController->GetImage()->GetInfo().GetHeight() - 1);
	for (int16 i = 0; i < NumElements; i++)
	{
		const G42MapElement * element = ImageMap -> GetElement (i);
		G42MapElement new_element (*element);
		/*  Scale the points and update the element  */
		int16 num_points = element -> NumPoints;
		for (int16 j = 0; j < num_points; j++)
		{
			points [j] = element -> Points [j];			
			points [j].X = (int32) ((double) points [j].X * scale_x);
			points [j].Y = (int32) ((double) points [j].Y * scale_y);
			new_element.Points [j] = points [j];
		}
		ImageMap -> SetElement (i, new_element);						
		/*  Update the GrafObject  */
		G42MapElement::G42ElementType type = element -> Type;
		switch (type)
		{
			case G42MapElement::Rectangle:
			{
				G42RectObject * object = (G42RectObject *) GrafObjects [i];
				object -> InitObject (G42Rect (points [0], points [1]), area_bounds);
			}
			break;
			case G42MapElement::Circle:
			{
				G42EllipseObject * object = (G42EllipseObject *) GrafObjects [i];
				object -> InitObject (G42Rect (points [0], points [1]), area_bounds);
			}
			break;
			case G42MapElement::Point:
			{
				G42PointObject * object = (G42PointObject *) GrafObjects [i];
				object -> InitObject (points [0], area_bounds);
			}
			break;
			case G42MapElement::Polygon:
			{
				G42PolyObject * object = (G42PolyObject *) GrafObjects [i];
				object -> InitObject (points, num_points, area_bounds);
			}
			break;
		}
	}
	if (NumElements)
	{
		if (selected_element != G42MapperNone) {
			ImageMap -> SelectElement (selected_element);
		}
		if (refresh) {
			AreaController -> Invalidate();
		}
	}
}
/*  This override is to set the area bounds for the grafobjects  */
void
G42MapController::SetCanvasSize (G42Size &size) 
{
	G42MouseKeyController::SetCanvasSize (size);
	if (AreaController && AreaController->GetImage() && AreaController->GetImage() -> HasInfo())
	{
		G42Rect area_bounds = G42Rect (0, 0, 
			AreaController->GetImage()->GetInfo().GetWidth() - 1,
			AreaController->GetImage()->GetInfo().GetHeight() - 1);
		/*  Go thru array and SetCanvasSize() for all the G42GrafObjects...  */
		for (int16 i = 0; i < NumElements; i++)
			GrafObjects [i] -> SetAreaBounds (area_bounds);
	}
}
void
G42MapController::SetElementInfo (G42MapElement &element, G42MapTool tool)
{
	switch (tool)
	{
		case MapRectangleTool:
			element.Type = G42MapElement::Rectangle;
			element.NumPoints = 2;
		break;
		case MapCircleTool:
			element.Type = G42MapElement::Circle;
			element.NumPoints = 2;
		break;
		case MapPointTool:
			element.Type = G42MapElement::Point;
			element.NumPoints = 1;
		break;
		case MapPolygonTool:
			element.Type = G42MapElement::Polygon;
			element.NumPoints = 0;	// Done in UpdateCoords() 
		break;
	}
}
void
G42MapController::SetImageMap (G42ImageMap *map, bool refresh)
{
	if (! map) {
		throw (G42InvalidParameterException (0));
	}
	/*  Kill the old one first  */
	if (ImageMap) 
	{
		/*  Delete current Map and all the G42GrafObjects  */
		delete ImageMap;
		for (int16 i = 0; i < NumElements; i++)
			delete GrafObjects [i];		
		NumElements = 0;
		SelectedElement = G42MapperNone;
	}
	ImageMap = map;
	/*  Make GrafObjects list match new map */
	NumElements = ImageMap -> GetNumElements();
	for (int16 i = 0; i < NumElements; i++)
	{
		const G42MapElement *element = ImageMap -> GetElement (i);
		GrafObjects [i] = NewGrafObject (element -> Type, element);
	}
	if (refresh) {
		AreaController -> Invalidate();
	}
}
void
G42MapController::SetMapTool (G42MapTool tool)
{
	#ifdef SVMAP_STUFF
	if (MapTool == MapEditTool)
		SetCursor(ArrowCursor);
	else
		SetCursor(CrossCursor);
	#endif
	MapTool = tool;
	/*  This applies to polygons only  */
	if (ActiveObject)	
	{
		delete ActiveObject;
		ActiveObject = 0;
		AreaController -> Invalidate();
	}
}
/*  Converts G42MapController tool type into G42ImageMap Element Type  */
G42MapElement::G42ElementType
G42MapController::Tool2ElementType (const G42MapTool tool) const
{
	if (tool == MapEditTool)
		throw (G42InvalidParameterException (0));
		
	G42MapElement::G42ElementType value = G42MapElement::Rectangle;
	switch (tool)
	{
		case MapCircleTool:
			value = G42MapElement::Circle;
		break;
		case MapPolygonTool:
			value = G42MapElement::Polygon;
		break;
		case MapPointTool:
			value = G42MapElement::Point;
		break;
	}
	return value;
}
void
G42MapController::UpdateCoords (const int16 index, const G42GrafObject *object)
{
	G42MapElement element (*ImageMap -> GetElement (index));
	UpdateCoords (element, object);
	ImageMap -> SetElement (index, element);
	/*  Partial kludge because SetElement deletes previous & then re-adds  */
	{
		ImageMap -> SelectElement (index);
		/*  Send off a selected element notification  */
		if (Notifier) {
			Notifier -> ProcessMessage (&G42SelectedElement (index));
		}
	}
}
void
G42MapController::UpdateCoords (G42MapElement &element, const G42GrafObject *object)
{
	/*  If it's a PolyObject we need the points  */
	G42PolyObject *poly_object = dynamic_cast <G42PolyObject *> ((G42GrafObject *) object);
	if (poly_object) 
	{
		int16 num_points = poly_object -> GetNumPoints();
		for (int16 i = 0; i < num_points; i++)
			element.Points [i] = poly_object -> GetPoint (i);
		element.NumPoints = num_points;
	}
	else
	{
		G42Rect rect = object -> GetBounds();
		element.Points [0] = rect.GetTopLeft();
		element.Points [1] = rect.GetBottomRight();
	}
}
