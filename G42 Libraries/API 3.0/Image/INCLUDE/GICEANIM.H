// giceanim.h - mouse key controller for editing animated images
#if !defined(GICEANIM_H)
#define GICEANIM_H
#include "gtools.h"
#include "gicmk.h"
#include "giobrect.h"
#include "gicimage.h"
#include "gpcomp.h"
#include "gicaanim.h"
class G42AnimatedEditController : public G42MouseKeyController,
	public G42PathComponent
{
public:
	// constructor and destructor
						G42AnimatedEditController(void);
	virtual 			~G42AnimatedEditController(void);
	void SelectAll(void);
	void ClearAllSelected(void);
	void DeleteSelected(void);
	uint32 GetNumSelected(void) const;
	// member functions
	virtual void		Draw (G42DrawLocation location = 0);	// Draws private UI
	virtual bool		ProcessMessage(G42MouseKeyMessage * message);
	virtual void 		SetAreaController(G42AreaController * area_controller);
	virtual void		SetCanvasSize(G42Size & size);
	virtual void		SetViewport(const G42Rect & viewport);
	virtual bool		IdleAction (void);
	virtual void		Clear(void);
	virtual void		ResetDrawState(void);
	virtual void BeginProcessing(void);
	virtual void Process(G42PathObject * object, G42PathComponent *);
	virtual void ClearPath(bool path_excepted = false);
private:
	// Instance variables
	G42Ptr<G42RectObject>
						ActiveObject; // current active graph object, null if none
	bool				ButtonDown; // true when button is down
	G42Point			LastMouseMove;
	uint32			LastIndex;
	bool				MovingThumbs;
	uint32			MovingIndex;
	void SelectArea(const G42Rect & rect, bool control = false,
		bool shift = false);
	void SelectIndex(uint32 index, bool control = false, bool shift = false);
	void				AdjustCursor (G42Point & point);
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
	void DrawMovingIndicator(G42DrawLocation location = 0);
	G42AreaAnimatedController * AnimatedArea; // dynamic casted pointer to area
};
#endif // GICEANIM_H
