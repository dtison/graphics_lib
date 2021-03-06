//  gicsela.h - G42SelectedAreaController (Mouse Key Controller that handles
//					selected areas)
#if !defined(GICSELA_H)
#define GICSELA_H
#include "gtools.h"
#include "gicmk.h"
#include "giobgraf.h"
#include "gicimage.h"
#include "gpcomp.h"
class G42SelectedAreaController : public G42MouseKeyController,
	public G42PathComponent
{
public:
	// constructor and descructor
						G42SelectedAreaController(void);
	virtual 			~G42SelectedAreaController(void);
	// member functions
	virtual void		Draw (G42DrawLocation location = 0);	// Draws private UI
	virtual bool		ProcessMessage(G42MouseKeyMessage * message);
	virtual void 		SetAreaController(G42AreaController * area_controller);
	virtual void		SetCanvasSize(G42Size & size);
	virtual void		SetViewport(const G42Rect & viewport);
	virtual bool		IdleAction (void);
	virtual void		Clear(void);
	virtual void		ResetDrawState(void);
	void				Rebuild(G42DrawLocation location = 0);
	void				PutAreaDown(void);
	void				SelectArea(const G42Rect & area);
	void				DeleteArea(void);
	bool				IsAreaSelected(void) const
							{ return AreaSelected; }
	const G42Image *	GetSelectedImage(void) const
							{ return SelectedImageController->GetImage(); }
	const G42Rect &		GetSelectedRect(void) const
							{ return ActiveObject->GetBounds(); }
	virtual void BeginProcessing(void);
	virtual void Process(G42PathObject * object, G42PathComponent *);
	virtual void ClearPath(bool path_excepted = false);
	enum G42Shape { Rectangle, Polygon, Ellipse, Freehand }; // more to be added later
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
	G42Image *		PathImage;
	G42Point			OriginalUpperLeft; // to see if user moved area
	G42Ptr <G42Image>
						OriginalImage; // image before resizing
	bool				ImageResized; // whether the image has been resized or not
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
	void						SyncImage(bool final = true);
	void						GetArea(const G42Rect & area);
	void						SetupActiveObject(void);
	void						SetupPath(void);
	G42GrafObject *		CreateShape(void);
};
#endif // GICSELA_H
