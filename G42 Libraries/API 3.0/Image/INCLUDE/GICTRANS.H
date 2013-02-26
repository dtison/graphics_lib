// gictrans.h - G42ImageTransMKController
#if !defined(GICTRANS_H)
#define GICTRANS_H

#include "gicmk.h"
#include "gnotify.h"

class G42NotifyTransChangedMessage : public G42NotifyMessage
{
public:
	G42NotifyTransChangedMessage(G42AreaController * area, bool transparent,
		const G42Color & transparent_color, bool final = true)
		: Area(area), Transparent(transparent),
			TransparentColor(transparent_color), Final(final) {}
	virtual ~G42NotifyTransChangedMessage(void) {}

	G42AreaController * GetAreaController(void) const
		{ return Area; }
	bool IsFinal(void) const
		{ return Final; }
	bool IsTransparent(void) const
		{ return Transparent; }
	const G42Color & GetTransparentColor(void) const
		{ return TransparentColor; }

private:
	G42AreaController * Area;
	bool Final;
	bool Transparent;
	G42Color TransparentColor;
};

class G42ImageTransMKController : public G42MouseKeyController
{
public:
	G42ImageTransMKController(G42ResourceIdentifier id);
	virtual ~G42ImageTransMKController(void);

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
	bool				ButtonDown; // true when button is down

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
};

#endif // GICTRANS_H

