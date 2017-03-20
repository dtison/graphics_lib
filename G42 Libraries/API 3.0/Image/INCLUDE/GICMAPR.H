//  gicmapr.h - G42MapController (Mouse Key Controller that handles image maps)
#if ! defined (GICMAPR_H)
#define GICMAPR_H
#define TBD
/*** No time for comments yet.  D.I.  ***/
#include "gicmk.h"
#include "gimap.h"
class G42GrafObject;
class G42RectObject;
class G42QueryURL;
const	int16		G42MapperNone	= -1;
// Keyboard keys
#ifdef MSWIN
const int8 G42DeleteKey 		= VK_DELETE;
#endif
#ifdef MACOS
const int8 G42DeleteKey 		= 0x08;
#endif
/*  An actual mouse key controller  */
class G42MapController : public G42MouseKeyController
{
public:
	enum 						G42MapTool {MapRectangleTool, MapCircleTool, MapPolygonTool,
									MapPointTool, MapEditTool};
								G42MapController (void);
	virtual 					~G42MapController (void);
	#ifdef NOTYET
	void 						AddElement (const G42MapElement &element) {ImageMap -> AddElement (element);}
	#endif
	void 						AddElement (const G42MapElement &element, int16 index = G42MapperNone);
	void						CBTextToMap (const char * buffer, uintf buffer_len);
	void 						ClearSelectedElement (void) {ImageMap -> ClearSelectedElement();}
	void 						DeleteElement (int16 index);
	virtual void			Draw (G42DrawLocation location = 0);		// Draws private UI
	const char 				*GetClientSideName(void) const {return ImageMap -> GetClientSideName();}
	int16 					GetDefaultIndex (void) const {return ImageMap -> GetDefaultIndex();}
	const 					G42MapElement *GetElement (int16 index);
	G42MapTool 				GetMapTool (void) {return MapTool;}
	G42MapType 				GetMapType (void) const {return ImageMap -> GetMapType();}
	int16						GetNumElements (void) const {return NumElements;}
	int16 					GetSelectedElement (void) const {return ImageMap -> GetSelectedElement();}
	bool 						HasDefault (void) const {return ImageMap -> HasDefault();}
	bool 						HasSelectedElement (void) const {return ImageMap -> HasSelectedElement();}
	virtual bool			IdleAction (void);
	bool						IsModified (void) const {return (ImageMap -> IsDirty());}
	bool 						IsValid (void) const {return ImageMap -> IsValid();}
	void						MapToCBText (char * buffer, uintf max_buffer_len);
	void 						MoveElementPos (int16 new_pos, int16 old_pos); 
	void						NewImageMap (bool refresh = true);
	bool						OpenImageMap (const G42FilePath & path);
	virtual bool			ProcessMessage (G42MouseKeyMessage *message);
	virtual void			ResetDrawState (void);	
	virtual void 			SetAreaController (G42AreaController *acontroller);
	virtual void			SetCanvasSize (G42Size &size);
	void						SetImageMap (G42ImageMap *map, bool refresh = true);
	void 						Save (const G42FilePath & file_path, G42MapType type)
									{ImageMap -> Save (file_path, type);}
	void 						Save (char *map_buffer, uintf max_buffer_length, G42MapType type)
									{ImageMap -> Save (map_buffer, max_buffer_length, type);}
	void						SelectElement (int16 index);
	void						SelectedElementToCBText (char * buffer, uintf max_buffer_len);
	void 						SetClientSideName (const char *name) {ImageMap -> SetClientSideName (name);}
	void 						SetElement (int16 index, const G42MapElement &element) 
									{ImageMap -> SetElement (index, element);}
	void 						SetElementAltText (int16 index, const char *text) 
									{ImageMap -> SetElementAltText (index, text);}
	void 						SetElementComment (int16 index, const char *comment) 
									{ImageMap -> SetElementComment (index, comment);}
	void 						SetElementURL (int16 index, const char *url)
									{ImageMap -> SetElementURL (index, url);}
			
	void 						SetMapTool (G42MapTool tool);
	void						ResizeImageMap (const G42Size & prev_size,
									const G42Size & new_size, bool refresh);
									
private:
	void						AdjustCursor (G42Point &point);
	void						AddMapElement (G42QueryURL &query, const G42GrafObject *object);
	void						DeleteSelectedElement (void);
	void						FinishActiveObject (void);
	/*  Mouse & Keyboard Messages  */
	bool 						HandleMouseMessage (void);
	bool						HandleMouseDoubleClick (void);	
	bool 						HandleMouseDown (void);
	bool 						HandleMouseMove (void);
	bool 						HandleMouseUp (void);
	void						HandleNewMapTool (G42MouseDown *message);
	bool						HandleKeyMessage (void);
	bool 						HandleKeyDown (void);
	bool 						HandleKeyUp (void);
	G42GrafObject			*NewGrafObject (const G42MapElement::G42ElementType type, 
									const G42MapElement *element = 0);
	void 						SetElementInfo (G42MapElement &element, G42MapTool tool);
	G42MapElement			::G42ElementType Tool2ElementType (const G42MapTool tool) const;
	void						UpdateCoords (const int16 index, const G42GrafObject *object);
	void 						UpdateCoords (G42MapElement &element, const G42GrafObject *object);
	/*  Instance variables  */
	G42GrafObject			*ActiveObject;
	bool						ButtonDown;
	G42Vector 				<G42GrafObject *> GrafObjects; // GrafObjects for map elements
	G42ImageMap				*ImageMap;
	int16						LastHotIndex;
	G42Point					LastMouseMove;
	G42MapTool				MapTool;
	int16						NumElements;					
	int16						SelectedElement;
};
#endif
