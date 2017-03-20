// gicresiz.h - G42ImageResizeMKController
#if !defined(GICRESIZ_H)
#define GICRESIZ_H
#include "gicmk.h"
#include "giobrect.h"
#include "gnotify.h"
class G42NotifyImageResizedMessage : public G42NotifyMessage
{
public:
	G42NotifyImageResizedMessage(G42AreaController * area,
		const G42Size & old_size, const G42Size & new_size)
	:
		Area(area), OldSize(old_size), NewSize(new_size) {}
	virtual ~G42NotifyImageResizedMessage(void) {}
	G42AreaController * GetAreaController(void) const
		{ return Area; }
	const G42Size & GetNewSize(void) const
		{ return NewSize; }
	const G42Size & GetOldSize(void) const
		{ return OldSize; }
private:
	G42AreaController *	Area;
	G42Size					NewSize;
	G42Size					OldSize;
};
class G42ImageResizeMKController : public G42MouseKeyController
{
public:
	G42ImageResizeMKController(void);
	virtual ~G42ImageResizeMKController(void);
	virtual void		Draw (G42DrawLocation location = 0);	// Draws private UI
	virtual bool		ProcessMessage(G42MouseKeyMessage * message);
	virtual void 		SetAreaController(G42AreaController * area_controller);
	virtual void		SetCanvasSize(G42Size & size);
	virtual void		SetViewport(const G42Rect & viewport);
	virtual bool		IdleAction (void);
	virtual void		Clear(void);
	virtual void		ResetDrawState(void);
private:
	// Instance variables
	G42Ptr<G42RectObject>
						ActiveObject; // current active graph object, null if none
	bool				ButtonDown; // true when button is down
	G42Point			LastMouseMove;
	G42Point			OriginalUpperLeft; // to see if user moved area
	bool				MoveImage;
	bool				ResizeImage;
	bool				ZoomImage;
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
	void				SyncImage(void);
	void				SetupActiveObject(void);
};
#endif // GICRESIZ_H
