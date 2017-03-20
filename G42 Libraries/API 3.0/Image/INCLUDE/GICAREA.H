// gicarea.h - G42AreaController
#if !defined(GICAREA_H)
#define GICAREA_H
#include "gitype.h"
#include "gicimage.h"
#include "ginotify.h"
#include "giimage.h"
#include "gidisp.h"
#include "gtools.h"
#include "gisdup.h"
#include "gicmgr.h"
#include "gicmk.h"
#include "gifilter.h"
#include "gidsetup.h"
class G42SurfaceController;
class G42AreaController : public G42PathComponent
{
public:
	G42AreaController(G42Notifier * notifier = 0);
	~G42AreaController(void);
	virtual G42Image * GetImage(void);
	virtual G42Display * GetDisplay(void);
	void Draw(G42DrawLocation location = 0);
	virtual void Draw(G42Rect & rect, G42DrawLocation location = 0,
		bool image_only = false) = 0;
	void Rebuild(G42DrawLocation location = 0);
	virtual void Rebuild(G42Rect & rect, G42DrawLocation location = 0,
		bool image_only = false) = 0;
	G42ScaleTranslate & GetScaleTranslate(void) { return ScaleTranslate; }
	virtual void SetScaleTranslate(G42ScaleTranslate & trans);
	G42Surface GetSurface(void);
	G42ScaleTranslate & GetCumlativeScaleTranslate(void) { return CumlativeScaleTranslate; }
	virtual void SetCumlativeScaleTranslate(G42ScaleTranslate & trans);
	virtual void SetImageScaleTranslate(void) = 0;
	virtual void SetArea(void) = 0;
	virtual void SetSurfaceController(G42SurfaceController * surface);
	G42SurfaceController * GetSurfaceController(void) const
		{ return SurfaceController; }
	virtual void SetMouseKeyController(G42MouseKeyController * mk);
	G42Notifier * GetNotifier() { return Notifier; }
	virtual G42Notifier * SetNotifier(G42Notifier * notifier);
/*
	// functions from G42PathComponent
	virtual void	BeginProcessing(void) = 0; // output data as a head of a path
	virtual void	ClearPath(void) = 0;  // user calls this to clear a path that
	virtual void	Process(G42PathObject * object) = 0;
												// receive a path object from Sender
*/
	const G42Rect & GetArea(void)
		{ return Area; }
	void SetMargins(const G42Margin & margin)
		{ Margin = margin; }
	bool ProcessMessage (G42MouseKeyMessage * message)
		{ return (ControlManager.NotNull() ?
			ControlManager->ProcessMessage(message) : 0); }
	void SetCanvasSize (G42Size & size)
		{ if (ControlManager.NotNull())
			ControlManager->SetCanvasSize(size); }
	virtual void SetViewport (const G42Rect & viewport) = 0;
	void SetBackgroundColor(const G42Color & color)
		{ BackgroundColor = color; ShouldDrawBackground = true; }
	void SetBackgroundStyle(G42FillStyle::G42Style style,
		const G42Color & background)
		{ BackgroundStyleColor = background; BackgroundStyle = style;
			ShouldDrawBackground = true; }
	void SetShouldDrawBackground(bool draw = true)
		{ ShouldDrawBackground = draw; }
	void SetBorderColor(const G42Color & border)
		{ BorderColor = border; ShouldDrawBorder = true; }
	void SetShouldDrawBorder(bool draw = true)
		{ ShouldDrawBorder = draw; }
	G42Size & GetCanvasSize(void);
	void MakeVisible(const G42Rect & rect);
	uintf GetNumFilters(void) { return NumFilters; }
	void AppendFilter(const G42ImageFilter & filter)
		{ InsertFilter(filter, NumFilters); }
	void InsertFilter(const G42ImageFilter & filter, uintf index = 0);
	G42ImageFilter * GetFilter(uintf index)
		{ return Filters[index]; }
	void RemoveFilter(uintf index);
	void ClearFilters(void);
	virtual void ResetImageFilters(void) = 0;
	#ifdef MSWIN
	void WinSetCursor (void)
		{ if (ControlManager.NotNull())
			ControlManager->WinSetCursor(); }
	#endif
	bool GetAutoDraw(void)
		{ return AutoDraw; }
	void SetAutoDraw(bool state)
		{ AutoDraw = state; }
	bool 	IdleAction (void)
	{ return (ControlManager.NotNull() ?
			ControlManager->IdleAction() : false); }
	virtual bool GetDrawAnimated(void);
	virtual void SetDrawAnimated(bool state = true);
	void Invalidate(const G42Rect & rect, bool image_only = false);
	void Invalidate(bool image_only = false);
	virtual void Play(void);
	virtual void Pause(void);
	virtual void Stop(void);
	virtual void Forward(void);
	virtual void Backward(void);
	virtual void Rewind(void);
	virtual void GoToEnd(void);
	virtual bool IsPaused(void);
	virtual bool IsPlaying(void);
protected:
	void DrawBackground(G42Rect & rect, G42DrawLocation location);
	G42Ptr<G42ControlManager> ControlManager;
	G42SurfaceController * SurfaceController;
	G42ScaleTranslate ScaleTranslate;
	G42ScaleTranslate CumlativeScaleTranslate;
	G42ExpandableArray< G42Ptr <G42ImageFilter> > Filters;
	int NumFilters;
	G42Notifier * Notifier;
	bool DeleteNotifier;
	G42Rect Area;
	G42Margin Margin; // note: does not get scaled by surfaces scale
	G42Color BackgroundColor;
	G42Color BackgroundStyleColor;
	G42FillStyle::G42Style BackgroundStyle;
	G42Color BorderColor;
	bool ShouldDrawBackground;
	bool ShouldDrawBorder;
	bool AutoDraw;
};
#endif // GICAREA_H
