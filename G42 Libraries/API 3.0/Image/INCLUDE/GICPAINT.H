//  gicpaint.h - G42ImagePaintController (Mouse Key Controller that handles
//					painting)
#if !defined(GICPAINT_H)
#define GICPAINT_H
#include "gtools.h"
#include "gicmk.h"
#include "giobgraf.h"
class G42ImagePaintController : public G42MouseKeyController
{
public:
	// constructor and descructor
						G42ImagePaintController(void);
	virtual 			~G42ImagePaintController(void);
	// member functions
	virtual void		Draw (G42DrawLocation location = 0);	// Draws private UI
	virtual bool		ProcessMessage(G42MouseKeyMessage * message);
	virtual void 		SetAreaController(G42AreaController * area_controller);
	virtual void		SetCanvasSize(G42Size & size);
	virtual void		SetViewport(const G42Rect & viewport);
	virtual bool		IdleAction (void);
	virtual void		Clear(void);
	virtual void		ResetDrawState(void);
	void				PutAreaDown(void);
	void				SelectArea(const G42Rect & area);
	void				DeleteArea(void);
	bool				IsAreaSelected(void) const
							{ return AreaSelected; }
	const G42Rect &		GetSelectedRect(void) const
							{ return ActiveObject->GetBounds(); }
	enum G42Shape { Rectangle, Polygon, Ellipse, Freehand, Text }; // more to be added later
	void SetShape (G42Shape shape);
	G42Shape GetShape (void) const
		{ return Shape; }
private:
	// Instance variables
	G42Ptr<G42GrafObject>
						ActiveObject; // current active graph object, null if none
	bool				ButtonDown; // true when button is down
	G42Ptr<G42ImageController>
						SelectedImageController; // current selected image
	bool				AreaSelected; // true if area is already selected
	G42Point			LastMouseMove;
	G42Point			OriginalUpperLeft; // to see if user moved area
	G42Shape			Shape; // shape to select
	void						AdjustCursor (G42Point & point);
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
	// Other useful private functions
	void						SetSelectedScaleTranslate(void);
	void						GetArea(const G42Rect & area);
	void						SetupActiveObject(void);
	void						SetupPath(void);
	G42GrafObject *		CreateShape(void);
	void						Paint(void);
};
#endif // GICPAINT_H
