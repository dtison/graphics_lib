// gicsur.h - G42SurfaceController
#if !defined(GICSUR_H)
#define GICSUR_H
#include "gitype.h"
#include "gpcomp.h"
#include "ggeomtry.h"
#include "gicasing.h"
#if defined(MSWIN)
#include "gscrollb.h"
#endif
#include "gicmk.h"
#include "gtools.h"
#include "gispal.h"
#include "gidsetup.h"
#include "gnotify.h"
class G42SurfaceController
{
public:
	G42SurfaceController(G42Surface surface, G42Notifier * notifier = 0,
		bool default_area = true, bool handle_scrollbars = true);
	virtual ~G42SurfaceController(void);
	void Draw(G42DrawLocation location = 0);
	void Draw(const G42Rect & rect, G42DrawLocation location = 0);
	void DrawInternal(const G42Rect & rect, G42DrawLocation location = 0,
		bool image_only = false);
	void Rebuild(G42DrawLocation location = 0);
	void Rebuild(const G42Rect & rect, G42DrawLocation location = 0);
	void ResizeViewport(void);
	void ResizeViewport(const G42Size & size);
	virtual void SetViewportOffset(const G42Point & offset);
	G42Rect & GetViewport(void) { return Viewport; }
	void SetScaleTranslate(const G42ScaleTranslate & scale_translate);
	void SetAreaScaleTranslate(void);
	void SetCanvasSize(const G42Size & size);
	void SetAreaController(G42AreaController * area, uintf index = 0,
		bool delete_old = true);
	void RemoveAreaController(uintf index = 0, bool delete_old = true);
	void RemoveAllAreaControllers(bool delete_old = true);
	uintf GetNumAreaControllers(void)
		{ return NumAreaControllers; }
	G42ScaleTranslate & GetScaleTranslate(void) { return ScaleTranslate; }
	G42Size & GetCanvasSize(void) { return Canvas; }
	G42AreaController * GetAreaController()
		{ return AreaControllers[ActiveAreaIndex]; }
	uintf GetActiveAreaIndex(void)
		{ return ActiveAreaIndex; }
	G42AreaController * GetAreaController(uintf index)
		{ return AreaControllers[index]; }
	void SetActiveAreaIndex(uintf area)
		{ ActiveAreaIndex = area; }
	G42Surface GetSurface(void) { return Surface; }
	operator G42PathComponent & (void)
		{ return *(AreaControllers[ActiveAreaIndex]); }
	G42Notifier * GetNotifier() { return Notifier; }
	G42Notifier * SetNotifier(G42Notifier * notifier);
	void SetMargins(const G42Margin & margin);
	const G42Margin & GetMargins(void) const
		{ return Margin; }
	void SetAreaMargins(const G42Margin & margin)
		{ if (AreaControllers[ActiveAreaIndex])
			AreaControllers[ActiveAreaIndex]->SetMargins(margin); }
	void SetMouseKeyController(G42MouseKeyController * mk);
	#ifdef MSWIN
	void ProcessScrollMsg(G42ScrollBar::G42BarType bar,
		uintf code, uintf thumb_pos);
	bool QueryNewPalette(void);
	void PaletteChanged(HWND who);
	HPALETTE GetWindowsPalette(void)
		{ return DitherWindowPalette; }
	void SetWindowsDitherPalette(const G42Color * palette, uintf num_palette);
	bool WinSetCursor(HWND window, uintf hit_test);
	bool WinMouseDown(const G42Point & point, uintf mod_keys,
		int16 button = G42DefaultButton);
	bool WinMouseUp(const G42Point & point, uintf mod_keys,
		int16 button = G42DefaultButton);
	bool WinMouseMove(const G42Point & point, uintf mod_keys);
	bool WinMouseDoubleClick(const G42Point & point, uintf mod_keys,
		int16 button = G42DefaultButton);
	bool WinKeyDown(uintf key, uintf flags);
	bool WinChar(uintf key, uintf flags);
	#endif
	void Clear(void);
	virtual void ResetCanvasViewport(void);
	void AutoSizeCanvas(void);
	void SetAreaBackgroundColor(const G42Color & color)
		{ if (AreaControllers[ActiveAreaIndex])
			AreaControllers[ActiveAreaIndex]->SetBackgroundColor(color); }
	void SetAreaBackgroundStyle(G42FillStyle::G42Style style,
		const G42Color & color)
		{ if (AreaControllers[ActiveAreaIndex])
			AreaControllers[ActiveAreaIndex]->SetBackgroundStyle(style, color); }
	void SetAreaShouldDrawBackground(bool draw = true)
		{ if (AreaControllers[ActiveAreaIndex])
			AreaControllers[ActiveAreaIndex]->SetShouldDrawBackground(draw); }
	void SetAreaBorderColor(const G42Color & color)
		{ if (AreaControllers[ActiveAreaIndex])
			AreaControllers[ActiveAreaIndex]->SetBorderColor(color); }
	void SetAreaShouldDrawBorder(bool draw = true)
		{ if (AreaControllers[ActiveAreaIndex])
			AreaControllers[ActiveAreaIndex]->SetShouldDrawBorder(draw); }
	void SetBackgroundColor(const G42Color & color);
	void SetBackgroundStyle(G42FillStyle::G42Style style,
		const G42Color & background);
	void SetDrawUnderAreas(bool draw);
	void SetShouldDrawBackground(bool draw = true)
		{ ShouldDrawBackground = draw; }
	void SetPaletteType(G42ScreenPalette::G42StandardPaletteType type,
		uintf num_palette = 256);
	bool GetUseShadowScreen(void)
		{ return UseShadowScreen; }
	void SetUseShadowScreen(bool use_shadow_screen = true)
		{ UseShadowScreen = use_shadow_screen; }
	bool GetAutoMouseScroll(void)
		{ return AutoMouseScroll; }
	void SetAutoMouseScroll(bool auto_mouse_scroll = true)
		{ AutoMouseScroll = auto_mouse_scroll; }
	bool GetCenterCanvas(void)
		{ return CenterCanvas; }
	void SetCenterCanvas(bool center_canvas = true)
		{ CenterCanvas = center_canvas; }
	bool GetAutoCanvasSize(void)
		{ return AutoCanvasSize; }
	void SetAutoCanvasSize(bool auto_canvas_size = true)
		{ AutoCanvasSize = auto_canvas_size; }
	bool GetAdjustScrollBars(void)
		{ return AdjustScrollBars; }
	void SetAdjustScrollBars(bool adjust_scroll_bars = true)
		{ AdjustScrollBars = adjust_scroll_bars; }
	bool GetAutoDraw(void)
		{ return AutoDraw; }
	void SetAutoDraw(bool state);
//		{ AutoDraw = state; }
	void DrawBackground(const G42Rect & rect,
		G42DrawLocation location);
	bool ProcessMessage (G42MouseKeyMessage * message);
	bool MouseDown(const G42Point & point,
		int16 button = G42DefaultButton, int16 flags = 0)
	{
		G42MouseDown msg;
		msg.Point = point;
		msg.Button = button;
		msg.Flags = flags;
		return ProcessMessage(&msg);
	}
	bool MouseUp(const G42Point & point,
		int16 button = G42DefaultButton, int16 flags = 0)
	{
		G42MouseUp msg;
		msg.Point = point;
		msg.Button = button;
		msg.Flags = flags;
		return ProcessMessage(&msg);
	}
	bool MouseMove(const G42Point & point,
		int16 button = G42DefaultButton, int16 flags = 0)
	{
		G42MouseMove msg;
		msg.Point = point;
		msg.Button = button;
		msg.Flags = flags;
		return ProcessMessage(&msg);
	}
	bool KeyDown(int16 key, int16 flags = 0)
	{
		G42KeyDown msg;
		msg.Key = key;
		msg.Flags = flags;
		return ProcessMessage(&msg);
	}
	bool KeyUp(int16 key, int16 flags = 0)
	{
		G42KeyUp msg;
		msg.Key = key;
		msg.Flags = flags;
		return ProcessMessage(&msg);
	}
	bool MouseDoubleClick(const G42Point & point,
		int16 button = G42DefaultButton, int16 flags = 0)
	{
		G42MouseDoubleClick msg;
		msg.Point = point;
		msg.Button = button;
		msg.Flags = flags;
		return ProcessMessage(&msg);
	}
	virtual void ScrollBy(int32 dx, int32 dy);
	void MakeVisible(const G42Rect & rect);
	virtual void QueryViewportOffset(void);
	bool IdleAction(void);
	uintf GetNumFilters(void) { return NumFilters; }
	void AppendFilter(const G42ImageFilter & filter)
		{ InsertFilter(filter, NumFilters); }
	void InsertFilter(const G42ImageFilter & filter, uintf index = 0);
	G42ImageFilter * GetFilter(uintf index)
		{ return Filters[index]; }
	void ReplaceFilter(const G42ImageFilter & filter, uintf index);
	void RemoveFilter(uintf index);
	void ClearFilters(void);
	void ResetAreaFilters(void);
	void SetAreaFilters(uintf i_area);
	enum G42MouseKeyDispatchMode { DispatchToActiveArea, DispatchToClosestArea,
		DispatchToContainingArea };
	G42MouseKeyDispatchMode GetMouseKeyDispatchMode (void) const
		{ return MKDispatchMode; }
	void SetMouseKeyDispatchMode (G42MouseKeyDispatchMode mode)
		{ MKDispatchMode = mode; }
	int32 GetAreaIndexAtPoint(const G42Point & point,
		bool find_closest = true) const;
protected:
	G42Surface Surface;
	G42Notifier * Notifier;
	bool DeleteNotifier;
	G42ExpandableArray<G42AreaController *> AreaControllers;
	uintf NumAreaControllers;
	intf ActiveAreaIndex;
	intf CursorAreaIndex;
	bool DeleteAreaController;
	bool AutoDraw;
	G42ExpandableArray< G42Ptr <G42ImageFilter> > Filters;
	int NumFilters;
	bool UseShadowScreen;
	bool HandleScrollBars;
	bool AutoMouseScroll;
	bool CenterCanvas;
	bool AdjustScrollBars;
	bool AutoCanvasSize;
	#ifdef MSWIN
	G42ScrollBar * HScrollBar;
	G42ScrollBar * VScrollBar;
	HPALETTE DitherWindowPalette;
	#endif
	#ifdef MACOS	// TODO:  Finish this
	int		DitherWindowPalette;
	#endif
	G42ScaleTranslate ScaleTranslate;
	G42Rect Viewport;
	G42Size Canvas;
//	bool AutoSizeCanvas;
	G42Size UnconvertedCanvas;
	bool ConvertCanvas;
	bool CenterInViewport;
	G42Margin Margin;
	bool DrawUnderAreas;
	bool ShouldDrawBackground;
	G42Color BackgroundColor;
	G42Color BackgroundStyleColor;
	G42FillStyle::G42Style BackgroundStyle;
	bool UseShadow;
	uintf MouseButtonCount;
	G42ScreenPalette ScreenPalette;
//	G42PaletteInfo PaletteInfo;
	intf DitherFilterIndex; 
	#ifdef MSWIN
	HBITMAP Shadow;
	HDC ShadowDC;
	HBITMAP OldShadow;
	HPALETTE OldShadowPal;
	#endif
	#ifdef MACOS
	GDHandle		SaveDevice;
	GWorldPtr	SavePort;
	GWorldPtr	Shadow;
	#endif
	G42DrawLocation SetupShadow(G42DrawLocation location, const G42Rect & rect); // in canvas coords
	void PutShadow(G42DrawLocation location, const G42Rect & rect); // in canvas coords
	virtual void HandleScreenPalette(void);
	G42MouseKeyDispatchMode MKDispatchMode;
	int32 GetDispatchArea (const G42Point & point) const;
	int32 InternalGetAreaIndexAtPoint(const G42Point & point,
		bool find_closest) const;
};
class G42ActiveAreaIndexChanged : public G42NotifyMessage
{
public:
	G42ActiveAreaIndexChanged(G42SurfaceController * surface, uintf old_index,
		uintf new_index)
	:
		Surface(surface), OldIndex(old_index), NewIndex(new_index) {}
	~G42ActiveAreaIndexChanged(void) {}
	uintf GetNewActiveAreaIndex(void) const
		{ return NewIndex; }
	uintf GetOldActiveAreaIndex(void) const
		{ return OldIndex; }
	G42SurfaceController * GetSurfaceController(void) const
		{ return Surface; }
private:
	uintf							NewIndex;
	uintf							OldIndex;
	G42SurfaceController *	Surface;
};
class G42CanvasSizeChanged : public G42NotifyMessage
{
public:
	G42CanvasSizeChanged(G42SurfaceController * surface,
		const G42Size & size)
	:
		Surface(surface), Size(size) {}
	~G42CanvasSizeChanged(void) {}
	const G42Size & GetCanvasSize(void) const
		{ return Size; }
	G42SurfaceController * GetSurfaceController(void) const
		{ return Surface; }
private:
	G42Size						Size;
	G42SurfaceController *	Surface;
};
#endif // GICSUR_H
