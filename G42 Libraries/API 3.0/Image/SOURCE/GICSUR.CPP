// gicsur.cpp
#include "gicsur.h"
#include "gicarea.h"
#include "gifmode.h"
#define USE_SHADOW
G42SurfaceController::G42SurfaceController(G42Surface surface,
	G42Notifier * notifier, bool default_area, bool handle_scroll_bars)
:
	#ifdef MSWIN
	HScrollBar(0),	VScrollBar(0), Shadow(0), ShadowDC(0), DitherWindowPalette(0),
	OldShadowPal(0),
	#endif
	#ifdef MACOS
	SaveDevice (0), SavePort (0), Shadow (0),
	#endif
	Surface(surface), NumAreaControllers(0), Notifier(notifier),
	DeleteNotifier(false), AutoDraw(true),
	ShouldDrawBackground(true), DrawUnderAreas(true),
	Canvas(0, 0), Viewport(0, 0, 0, 0), UseShadowScreen(true),
	ActiveAreaIndex(0), HandleScrollBars(handle_scroll_bars),
	MouseButtonCount(0), CenterCanvas(true), CursorAreaIndex(0),
	AutoMouseScroll(handle_scroll_bars), BackgroundStyle(G42FillStyle::Solid),
	AdjustScrollBars(handle_scroll_bars), AutoCanvasSize(handle_scroll_bars),
	ScreenPalette(G42ScreenPalette::UseStandardPalette, surface), NumFilters(0)
{
	#ifdef MACOS
	AutoDraw = false;
	#endif
	try
	{
		#ifdef MSWIN
		if (handle_scroll_bars)
		{
			HScrollBar = new G42ScrollBar(surface, G42ScrollBar::Horizontal);
			VScrollBar = new G42ScrollBar(surface, G42ScrollBar::Vertical);
		}
		#endif
		if (!Notifier)
		{
			Notifier = new G42Notifier();
			DeleteNotifier = true;
		}
		HandleScreenPalette();
		if (default_area)
		{
			AreaControllers[0] = new G42AreaSingleController();
			NumAreaControllers++;
			AreaControllers[0]->SetSurfaceController(this);
			SetAreaFilters(0);
		}
	}
	catch (...)
	{
		Clear();
		throw;
	}
}
G42SurfaceController::~G42SurfaceController(void)
{
	Clear();
	#ifdef MSWIN
	if (MouseButtonCount > 0)
		ReleaseCapture();
	MouseButtonCount = 0;
	if (DitherWindowPalette)
	{
		DeleteObject(DitherWindowPalette);
		DitherWindowPalette = 0;
	}
	#endif
}
void
G42SurfaceController::Clear(void)
{
	#ifdef MSWIN
	if (HScrollBar)
	{
		delete HScrollBar;
		HScrollBar = 0;
	}
	if (VScrollBar)
	{
		delete VScrollBar;
		VScrollBar = 0;
	}
	#endif
	if (DeleteNotifier && Notifier)
	{
		delete Notifier;
		DeleteNotifier = false;
		Notifier = 0;
	}
	for (uintf index = 0; index < NumAreaControllers; index++)
	{
		if (AreaControllers[index])
		{
			delete AreaControllers[index];
		}
		AreaControllers[index] = 0;
	}
	NumAreaControllers = 0;
}
void
G42SurfaceController::DrawBackground(const G42Rect & rect,
	G42DrawLocation location)
{
	#ifdef MSWIN
	if (ShouldDrawBackground)
	{
		G42DrawLocationVerifier dc(location, Surface, DitherWindowPalette);
		G42FillStyle style(dc, BackgroundColor, BackgroundStyleColor, BackgroundStyle);
		HPEN old_pen = (HPEN)SelectObject(dc, GetStockObject(NULL_PEN));
		Rectangle(dc, rect.Left - Viewport.Left,
			rect.Top - Viewport.Top,
			rect.Right - Viewport.Left + 2,
			rect.Bottom - Viewport.Top + 2);
		SelectObject(dc, old_pen);
	}
	#endif
	#ifdef MACOS
	/*  Quiets warnings  */
	if (location) {}
	RGBColor color = BackgroundColor;
	::RGBForeColor (&color);
	::PaintRect (&((Rect) rect));
	::ForeColor (blackColor);
	#endif
}
void
G42SurfaceController::Draw(G42DrawLocation location)
{
	#if defined(MSWIN)
	uint32 x, y, width, height;
	if (Canvas.GetWidth() > Viewport.GetWidth())
	{
		x = 0;
		width = Canvas.GetWidth();
	}
	else
	{
		x = Viewport.Left;
		width = Viewport.GetWidth();
	}
	if (Canvas.GetHeight() > Viewport.GetHeight())
	{
		y = 0;
		height = Canvas.GetHeight();
	}
	else
	{
		y = Viewport.Top;
		height = Viewport.GetHeight();
	}
	DrawInternal(G42Rect(x, y, x + width - 1, y + height - 1), location);
//	DrawInternal(G42Rect(0, 0, Canvas.GetWidth() - 1, Canvas.GetHeight() - 1),
//		location);
//	DrawInternal(Viewport, location);
//	Draw(G42Rect(0, 0, Viewport.GetWidth() - 1, Viewport.GetHeight() - 1),
//		location);
	#else
	Draw(Viewport, location);
	#endif
}
void
G42SurfaceController::Draw(const G42Rect & rect, G42DrawLocation location)
{
	QueryViewportOffset();
	G42Rect r(rect);
	#if defined(MSWIN)
	r.Offset(Viewport.Left, Viewport.Top);
	#endif
	DrawInternal(r, location);
}
void
G42SurfaceController::DrawInternal(const G42Rect & rect, G42DrawLocation location,
	bool image_only)
{
	QueryViewportOffset();
	G42DrawLocationVerifier loc(location, Surface, DitherWindowPalette, true);
/*
	bool release_dc = false;
	if (!location)
	{
		location = GetDC(Surface);
		release_dc = true;
	}
	HPALETTE old_pal = 0;
	if (DitherWindowPalette)
	{
		old_pal = (HPALETTE)SelectPalette(location,
			DitherWindowPalette, false);
		RealizePalette(location);
	}
	#endif
*/
	G42DrawLocation shadow_location = SetupShadow(loc, rect);
	DrawBackground(rect, shadow_location);
	for (uintf area = 0; area < NumAreaControllers; area++)
	{
		if (AreaControllers[area])
		{
			G42Rect r(rect);
			AreaControllers[area]->Draw(r, shadow_location, image_only);
		}
	}
	PutShadow(loc, rect);
/*
	#if defined(MSWIN)
	if (old_pal)
	{
		SelectPalette(location, old_pal, false);
		RealizePalette(location);
	}
	if (release_dc)
	{
		ReleaseDC(Surface, location);
	}
	#endif
*/	
}
void
G42SurfaceController::Rebuild(G42DrawLocation location)
{
	Rebuild(G42Rect(0, 0, Canvas.GetWidth() - 1, Canvas.GetHeight() - 1),
		location);
}
void
G42SurfaceController::Rebuild(const G42Rect & rect, G42DrawLocation location)
{
	G42DrawLocationVerifier loc(location, Surface, DitherWindowPalette, true);
	QueryViewportOffset();
	G42DrawLocation shadow_location = SetupShadow(loc, rect);
	DrawBackground(rect, shadow_location);
	for (uintf area = 0; area < NumAreaControllers; area++)
	{
		if (AreaControllers[area])
		{
			G42Rect r(rect);
			AreaControllers[area]->Rebuild(r, shadow_location);
		}
	}
	PutShadow(loc, rect);
}
void
G42SurfaceController::ResizeViewport(void)
{
#ifdef MSWIN
	RECT rect;
	if (GetClientRect(Surface, &rect))
	{
		ResizeViewport(G42Size(rect.right, rect.bottom));
		if (AutoDraw)
		{
			Draw();
		}
	}
#endif
}
void
G42SurfaceController::ResizeViewport(const G42Size & size)
{
	if (size.Width != Viewport.GetWidth() || size.Height != Viewport.GetHeight())
	{
		bool need_redraw = false;
		if ((CenterCanvas && (Canvas.Width < Viewport.GetWidth() ||
			Canvas.Height < Viewport.GetHeight())) ||
			(Viewport.Left + size.Width > Canvas.Width) ||
			(Viewport.Top + size.Height > Canvas.Height))
		{
			need_redraw = true;
		}
		Viewport.Set(Viewport.Left, Viewport.Top,
			size.Width + Viewport.Left - 1, size.Height + Viewport.Top - 1);
		ResetCanvasViewport();
		if (need_redraw && AutoDraw)
		{
			Draw();
		}
	}
}
void
G42SurfaceController::SetViewportOffset(const G42Point & offset)
{
	Viewport.MoveTo(offset.X, offset.Y);
	// note: don't ResetCanvasViewport(), but be sure to tell those who
	// need to know about the viewport change
	for (uintf area = 0; area < NumAreaControllers; area++)
	{
		if (AreaControllers[area])
		{
			AreaControllers[area]->SetViewport(Viewport);
		}
	}
}
void
G42SurfaceController::SetScaleTranslate(const G42ScaleTranslate & scale_translate)
{
	ScaleTranslate = scale_translate;
	SetAreaScaleTranslate();
}
void
G42SurfaceController::SetAreaScaleTranslate(void)
{
	bool auto_draw = AutoDraw;
	SetAutoDraw(false);
	if (NumAreaControllers)
	{
		G42ScaleTranslate area_translate(
			ScaleTranslate.GetScaleX(), ScaleTranslate.GetScaleY(),
			G42Point(Margin.Left, Margin.Top));
		for (uintf area = 0; area < NumAreaControllers; area++)
		{
			if (AreaControllers[area])
			{
				AreaControllers[area]->SetCumlativeScaleTranslate(area_translate);
			}
		}
	}
	// either the AreaControllers just reset the canvas size, or the
	// user did (or will soon), so we don't have to do anything else
	if (auto_draw)
	{
		SetAutoDraw(true);
		Draw();
	}
}
void
G42SurfaceController::SetCanvasSize(const G42Size & size)
{
	bool need_redraw = false;
	if (CenterCanvas && ((size.Width != Canvas.Width &&
		(size.Width < Viewport.GetWidth() ||
		Canvas.Width < Viewport.GetWidth())) ||
		(size.Height != Canvas.Height &&
		(size.Height < Viewport.GetHeight() ||
		Canvas.Height < Viewport.GetHeight()))))
	{
		need_redraw = true;
	}
	Canvas = size;
	ResetCanvasViewport();
	for (uintf area = 0; area < NumAreaControllers; area++)
	{
		if (AreaControllers[area])
		{
			AreaControllers[area]->SetCanvasSize(Canvas);
		}
	}
	if (need_redraw && AutoDraw)
	{
		Draw();
	}
}
// David: you've got a valid Canvas and Viewport, and you now have to
// tell the OS about them.  One of the two has just changed, either
// because an image was loaded, a port resized, or a scaling change,
// or something similar (margins changed, etc).  This does not get
// called when the user moves the scrollbars.
void
G42SurfaceController::ResetCanvasViewport(void)
{
	if (HandleScrollBars && AdjustScrollBars)
	{
	if (Viewport.GetWidth() >= Canvas.Width)
	{
		if (CenterCanvas)
		{
			Viewport.MoveTo(-((Viewport.GetWidth() - Canvas.Width) >> 1),
				Viewport.Top);
		}
		else
		{
			Viewport.MoveTo(0, Viewport.Top);
		}
	}
	else
	{
		if (Viewport.Right >= Canvas.Width)
		{
			Viewport.Left -= Viewport.Right - Canvas.Width + 1;
			Viewport.Right = Canvas.Width - 1;
		}
		if (Viewport.Left < 0)
		{
			Viewport.Right += -(Viewport.Left);
			Viewport.Left = 0;
		}
	}
	if (Viewport.GetHeight() >= Canvas.Height)
	{
		if (CenterCanvas)
		{
			Viewport.MoveTo(Viewport.Left,
				-((Viewport.GetHeight() - Canvas.Height) >> 1));
		}
		else
		{
			Viewport.MoveTo(Viewport.Left, 0);
		}
	}
	else
	{
		if (Viewport.Bottom >= Canvas.Height)
		{
			Viewport.Top -= Viewport.Bottom - Canvas.Height + 1;
			Viewport.Bottom = Canvas.Height - 1;
		}
		if (Viewport.Top < 0)
		{
			Viewport.Bottom += -(Viewport.Top);
			Viewport.Top = 0;
		}
	}
	}
	for (uintf area = 0; area < NumAreaControllers; area++)
	{
		if (AreaControllers[area])
		{
			AreaControllers[area]->SetViewport(Viewport);
		}
	}
	// MSWIN can do this native, MACOS will have to override this function,
	// call it first, and then tell the framework about the Canvas and
	// Viewport
	#ifdef MSWIN
	if (HandleScrollBars)
	{
		if (Viewport.GetWidth() >= Canvas.Width)
		{
			HScrollBar->EnableScrollBar(false);
		}
		else
		{
			HScrollBar->EnableScrollBar(true);
			HScrollBar->SetInitPos(0, Canvas.Width - Viewport.GetWidth(),
				Viewport.Left, 1, Viewport.GetWidth() >> 1);
		}
		if (Viewport.GetHeight() >= Canvas.Height)
		{
			VScrollBar->EnableScrollBar(false);
		}
		else
		{
			VScrollBar->EnableScrollBar(true);
			VScrollBar->SetInitPos(0, Canvas.Height - Viewport.GetHeight(),
				Viewport.Top, 1, Viewport.GetHeight() >> 1);
		}
	}
	#endif
}
void
G42SurfaceController::AutoSizeCanvas(void)
{
	if (AutoCanvasSize && NumAreaControllers)
	{
		G42Size canvas(0, 0);
		for (uintf area = 0; area < NumAreaControllers; area++)
		{
			if (AreaControllers[area])
			{
				if (AreaControllers[area]->GetArea().Right >= canvas.Width)
					canvas.Width = AreaControllers[area]->GetArea().Right + 1;
				if (AreaControllers[area]->GetArea().Bottom >= canvas.Height)
					canvas.Height = AreaControllers[area]->GetArea().Bottom + 1;
			}
		}
		canvas.Width += Margin.Right;
		canvas.Height += Margin.Bottom;
		SetCanvasSize(canvas);
		G42CanvasSizeChanged msg(this, Canvas);
		Notifier->ProcessMessage(&msg);
	}
}
void
G42SurfaceController::SetMargins(const G42Margin & margin)
{
	Margin = margin;
	SetAreaScaleTranslate();
	AutoSizeCanvas();
}
void
G42SurfaceController::SetBackgroundColor(const G42Color & color)
{
	BackgroundColor = color;
	Draw();
}
void
G42SurfaceController::SetBackgroundStyle(G42FillStyle::G42Style style,
	const G42Color & color)
{
	BackgroundStyle = style;
	BackgroundStyleColor = color;
	Draw();
}
void
G42SurfaceController::SetDrawUnderAreas(bool draw)
{
	DrawUnderAreas = draw;
	Draw();
}
void
G42SurfaceController::RemoveAreaController(uintf index, bool delete_old)
{
	if (index >= NumAreaControllers)
	{
		return;
	}
	if (AreaControllers[index] && delete_old)
	{
		delete AreaControllers[index];
	}
	AreaControllers[index] = 0;
}
void
G42SurfaceController::RemoveAllAreaControllers(bool delete_old)
{
	for (uintf index = 0; index < NumAreaControllers; index++)
	{
		if (AreaControllers[index] && delete_old)
		{
			delete AreaControllers[index];
		}
		AreaControllers[index] = 0;
	}
	NumAreaControllers = 0;
}
void
G42SurfaceController::SetAreaController(G42AreaController * area,
	uintf index, bool delete_old)
{
	if (index >= NumAreaControllers)
	{
		for (uint32 i = NumAreaControllers; i <= index; i++)
		{
			AreaControllers[i] = 0;
		}
		NumAreaControllers = index + 1;
	}
	if (AreaControllers[index] && delete_old)
	{
		delete AreaControllers[index];
	}
	AreaControllers[index] = area;
	if (AreaControllers[index])
	{
		AreaControllers[index]->SetSurfaceController(this);
		AreaControllers[index]->SetViewport(Viewport);
		SetAreaScaleTranslate();
		SetAreaFilters(index);
	}
}
G42Notifier *
G42SurfaceController::SetNotifier(G42Notifier * notifier)
{
	G42Notifier * old_notifier;
	if (DeleteNotifier && Notifier)
	{
		delete Notifier;
		old_notifier = 0;
	}
	else
	{
		old_notifier = Notifier;
	}
	if (notifier)
	{
		Notifier = notifier;
		DeleteNotifier = false;
	}
	else
	{
		Notifier = new G42Notifier();
		DeleteNotifier = true;
	}
	for (uintf area = 0; area < NumAreaControllers; area++)
	{
		if (AreaControllers[area])
		{
			AreaControllers[area]->SetNotifier(Notifier);
		}
	}
	return old_notifier;
}
void
G42SurfaceController::SetMouseKeyController(G42MouseKeyController * mk)
{
	// note: this doesn't work well for some controllers that store
	// data.  We really should copy the controller to the 2nd, ect.
	for (uintf area = 0; area < NumAreaControllers; area++)
	{
		if (AreaControllers[area])
		{
			AreaControllers[area]->SetMouseKeyController(mk);
		}
	}
	Draw();
}
#ifdef MSWIN
void
G42SurfaceController::ProcessScrollMsg(G42ScrollBar::G42BarType bar,
	uintf code, uintf thumb_pos)
{
	int32 x = Viewport.Left;
	int32 y = Viewport.Top;
	switch (bar)
	{
		case G42ScrollBar::Horizontal:
			HScrollBar->ProcessScrollMsg(code, thumb_pos);
			x = HScrollBar->GetCurrentPos();
			break;
		case G42ScrollBar::Vertical:
			VScrollBar->ProcessScrollMsg(code, thumb_pos);
			y = VScrollBar->GetCurrentPos();
			break;
	}
	SetViewportOffset(G42Point(x, y));
}
bool
G42SurfaceController::WinSetCursor(HWND window, uintf hit_test)
{
	if (window == Surface && hit_test == HTCLIENT &&
		CursorAreaIndex >= 0 && NumAreaControllers &&
		AreaControllers[CursorAreaIndex])
	{
		AreaControllers[CursorAreaIndex]->WinSetCursor();
		return true;
	}
	return false;
}
#endif
G42DrawLocation
G42SurfaceController::SetupShadow(G42DrawLocation location, const G42Rect & rect) // in canvas coords
{
	#ifdef MSWIN
	#if defined(USE_SHADOW)
	Shadow = CreateCompatibleBitmap(location, rect.GetWidth(), rect.GetHeight());
	ShadowDC = CreateCompatibleDC(location);
	OldShadow = (HBITMAP)SelectObject(ShadowDC, Shadow);
	SetWindowOrgEx(ShadowDC, rect.Left - Viewport.Left,
		rect.Top - Viewport.Top, 0);
	intf org_x = -Viewport.Left - rect.Left;
	if (org_x < 0)
	{
		org_x += ((-org_x + 7) & ~7);
	}
	org_x &= 7;
	intf org_y = -Viewport.Top - rect.Top;
	if (org_y < 0)
	{
		org_y += ((-org_y + 7) & ~7);
	}
	org_y &= 7;
	SetBrushOrgEx(ShadowDC, org_x, org_y, 0);
	OldShadowPal = 0;
	if (DitherWindowPalette)
	{
		OldShadowPal = (HPALETTE)SelectPalette(ShadowDC,
			DitherWindowPalette, false);
		RealizePalette(ShadowDC);
	}
	return ShadowDC;
	#else
	return location;
	#endif // USE_SHADOW
	
	#endif // MSWIN
	#ifdef MACOS
	::GetGWorld (&SavePort, &SaveDevice);
	short depth = G42Display::MacGetScreenDepth();
	Rect rectm = rect;
	QDErr err = ::NewGWorld (&Shadow, depth, &rectm, nil, nil, useTempMem);
	if (! err)
	{
		::SetGWorld (Shadow, nil);
		::SetOrigin (rect.Left, rect.Top);
	}
	else
		Shadow = nil;
	return location;
	#endif
}
void
G42SurfaceController::PutShadow(G42DrawLocation location, const G42Rect & rect) // in canvas coords
{
	#ifdef MSWIN
	#if defined(USE_SHADOW)
	SetWindowOrgEx(ShadowDC, 0, 0, 0);
	BitBlt(location, rect.Left - Viewport.Left, rect.Top - Viewport.Top,
		rect.GetWidth(), rect.GetHeight(),
		ShadowDC, 0, 0, SRCCOPY);
	SelectObject(ShadowDC, OldShadow);
	if (OldShadowPal)
	{
		SelectPalette(ShadowDC, OldShadowPal, false);
		RealizePalette(ShadowDC);
		OldShadowPal = 0;
	}
	DeleteDC(ShadowDC);
	DeleteObject(Shadow);
	#endif // USE_SHADOW
	#endif
	#ifdef MACOS
	/*  Quiets warnings  */
	if (location) {}
	/*  Active port is the shadow  */
	/*  Refocus to saved (screen for now) port  */
	::SetGWorld (SavePort, SaveDevice);
	GWorldPtr screenPort = SavePort;
	if (Shadow)
	{
		/*  Transfer the Shadow bitmap to the screen (with dither)  */
		Rect sourceRect = rect;
		Rect destRect = sourceRect;
		short transferMode = srcCopy;
		short sourceDepth  = (**Shadow -> portPixMap).cmpCount * (**Shadow -> portPixMap).cmpSize;
		short destDepth 	 = (**screenPort -> portPixMap).cmpCount * (**screenPort -> portPixMap).cmpSize;
		if (sourceDepth > 8 && destDepth <= 8)
			transferMode += ditherCopy;
		::LockPixels (::GetGWorldPixMap (Shadow));
		::LockPixels (screenPort -> portPixMap);
		::CopyBits((BitMapPtr) &Shadow -> portPixMap,
				(BitMapPtr) &screenPort -> portPixMap,
				&sourceRect, &destRect, transferMode, nil);
		::UnlockPixels (screenPort -> portPixMap);
		::UnlockPixels (::GetGWorldPixMap (Shadow));
	}
	
	/*  Kill shadow  */
	if (Shadow)
	{
		DisposeGWorld (Shadow);
		Shadow = 0;
	}
	#endif
}
bool
G42SurfaceController::ProcessMessage (G42MouseKeyMessage * message)
{
	G42MouseMessage * mouse = dynamic_cast<G42MouseMessage *>(message);
	int32 index = ActiveAreaIndex;
	if (mouse)
	{
		#ifdef MSWIN
		mouse->Point.X += Viewport.Left;
		mouse->Point.Y += Viewport.Top;
		#endif
		if (!MouseButtonCount)
		{
			index = GetDispatchArea(mouse->Point);
			if (MKDispatchMode != DispatchToActiveArea)
			{
				CursorAreaIndex = index;
			}
		}
		if ((dynamic_cast<G42MouseDown *>(mouse) ||
			dynamic_cast<G42MouseDoubleClick *>(mouse)) &&
				mouse->Button == G42LeftButton)
		{
			MouseButtonCount++;
			#ifdef MSWIN
			if (MouseButtonCount == 1)
			{
				SetCapture(Surface);
				if (index >= 0 && ActiveAreaIndex != index &&
					MKDispatchMode != DispatchToActiveArea)
				{
					G42ActiveAreaIndexChanged msg(this, ActiveAreaIndex, index);
					ActiveAreaIndex = index;
					Notifier->ProcessMessage(&msg);
				}
			}
			#endif
		}
		else if (dynamic_cast<G42MouseUp *>(mouse))
		{
			#ifdef MSWIN
			if (MouseButtonCount == 1 && mouse->Button == G42LeftButton)
			{
				ReleaseCapture();
			}
			#endif
			if (MouseButtonCount && mouse->Button == G42LeftButton)
				MouseButtonCount--;
		}
		if (MouseButtonCount > 0 && HandleScrollBars && AutoMouseScroll)
		{
			QueryViewportOffset();
			if (!Viewport.Contains(mouse->Point))
			{
				int32 dx = 0;
				int32 dy = 0;
				if (mouse->Point.X < Viewport.Left && Viewport.Left > 0)
				{
					dx = mouse->Point.X - Viewport.Left;
					if (dx + Viewport.Left < 0)
					{
						dx = -Viewport.Left;
					}
				}
				else if (mouse->Point.X > Viewport.Right &&
					Viewport.Right < Canvas.Width - 1)
				{
					dx = mouse->Point.X - Viewport.Right;
					if (dx + Viewport.Right >= Canvas.Width)
					{
						dx = Canvas.Width - Viewport.Right - 1;
					}
				}
				if (mouse->Point.Y < Viewport.Top && Viewport.Top > 0)
				{
					dy = mouse->Point.Y - Viewport.Top;
					if (dy + Viewport.Top < 0)
					{
						dy = -Viewport.Top;
					}
				}
				else if (mouse->Point.Y > Viewport.Bottom &&
					Viewport.Bottom < Canvas.Height - 1)
				{
					dy = mouse->Point.Y - Viewport.Bottom;
					if (dy + Viewport.Bottom >= Canvas.Height)
					{
						dy = Canvas.Height - Viewport.Bottom - 1;
					}
				}
				if (dx || dy)
					ScrollBy(dx, dy);
			}
		}
	}
	if (index >= 0 && NumAreaControllers && AreaControllers[index])
	{
		return AreaControllers[index]->ProcessMessage(message);
	}
	return false;
}
void
G42SurfaceController::ScrollBy(int32 dx, int32 dy)
{
	#ifdef MSWIN
	if (dx)
		ProcessScrollMsg(G42ScrollBar::Horizontal, SB_THUMBPOSITION,
			Viewport.Left + dx);
	if (dy)
		ProcessScrollMsg(G42ScrollBar::Vertical, SB_THUMBPOSITION,
			Viewport.Top + dy);
	#endif
	#ifdef MACOS
	/*  Quiets warnings  */
	if (dx || dy) {}
	#endif
}
void
G42SurfaceController::MakeVisible(const G42Rect & rect)
{
	if (MouseButtonCount > 0 && HandleScrollBars && AutoMouseScroll)
	{
		QueryViewportOffset();
		if (!Viewport.Contains(rect))
		{
			int32 dx = 0;
			int32 dy = 0;
			if (rect.Left < Viewport.Left && Viewport.Left > 0)
			{
				dx = rect.Left - Viewport.Left;
				if (dx + Viewport.Left < 0)
				{
					dx = -Viewport.Left;
				}
			}
			else if (rect.Right > Viewport.Right &&
				Viewport.Right < Canvas.Width - 1)
			{
				dx = rect.Right - Viewport.Right;
				if (dx + Viewport.Right >= Canvas.Width)
				{
					dx = Canvas.Width - Viewport.Right - 1;
				}
			}
			if (rect.Top < Viewport.Top && Viewport.Top > 0)
			{
				dy = rect.Top - Viewport.Top;
				if (dy + Viewport.Top < 0)
				{
					dy = -Viewport.Top;
				}
			}
			else if (rect.Bottom > Viewport.Bottom &&
				Viewport.Bottom < Canvas.Height - 1)
			{
				dy = rect.Bottom - Viewport.Bottom;
				if (dy + Viewport.Bottom >= Canvas.Height)
				{
					dy = Canvas.Height - Viewport.Bottom - 1;
				}
			}
			if (dx || dy)
			{
				ScrollBy(dx, dy);
			}
		}
	}
}
void
G42SurfaceController::QueryViewportOffset(void)
{
}
void
G42SurfaceController::InsertFilter(const G42ImageFilter & filter, uintf index)
{
	for (uintf i = NumFilters; i > index; i--)
	{
		Filters[i].Set(Filters[i - 1].Release());
	}
	NumFilters++;
	Filters[index].Set(filter.MakeCopy());
	ResetAreaFilters();
}
void
G42SurfaceController::ReplaceFilter(const G42ImageFilter & filter, uintf index)
{
	if (index >= NumFilters)
	{
      InsertFilter(filter, index);
	}
	else
	{
		Filters[index].Set(filter.MakeCopy());
		ResetAreaFilters();
	}
}
void
G42SurfaceController::RemoveFilter(uintf index)
{
	Filters[index].Clear();
	NumFilters--;
	for (uintf i = index; i < NumFilters; i++)
	{
		Filters[i].Set(Filters[i + 1].Release());
	}
	ResetAreaFilters();
}
void
G42SurfaceController::ClearFilters(void)
{
	Filters.Clear();
	NumFilters = 0;
	ResetAreaFilters();
}
void
G42SurfaceController::ResetAreaFilters(void)
{
	for (uintf i_area = 0; i_area < NumAreaControllers; i_area++)
	{
		SetAreaFilters(i_area);
	}
}
void
G42SurfaceController::SetAreaFilters(uintf i_area)
{
	if (!AreaControllers[i_area])
		return;
		
	AreaControllers[i_area]->ClearFilters();
	for (uintf i_filter = 0; i_filter < NumFilters; i_filter++)
	{
		AreaControllers[i_area]->AppendFilter(
			Filters[i_filter].GetReference());
	}
//	AreaControllers[i_area]->SetPaletteInfo(PaletteInfo);
}
#ifdef MSWIN
bool
G42SurfaceController::QueryNewPalette(void)
{
	if (!DitherWindowPalette)
		return false;
	HDC dc = GetDC(Surface);
	HPALETTE old_pal = (HPALETTE)SelectPalette(dc, DitherWindowPalette, false);
	RealizePalette(dc);
	SelectPalette(dc, old_pal, false);
	RealizePalette(dc);
	ReleaseDC(Surface, dc);
	return true;
}
void
G42SurfaceController::PaletteChanged(HWND who)
{
	if (!DitherWindowPalette || who == Surface) // don't respond to ourselves
		return;
	HDC dc = GetDC(Surface);
	HPALETTE old_pal = (HPALETTE)SelectPalette(dc, DitherWindowPalette, false);
	RealizePalette(dc);
	SelectPalette(dc, old_pal, false);
	RealizePalette(dc);
	ReleaseDC(Surface, dc);
}
void
G42SurfaceController::SetWindowsDitherPalette(const G42Color * palette,
	uintf num_palette)
{
	LOGPALETTE * log_pal = (LOGPALETTE *)new byte [2048]; // just to be safe
	int add_white = 0;
	int add_black = 0;
	if (num_palette < 254)
		add_white++;
	if (num_palette < 255)
		add_black++;
	log_pal->palVersion = 0x300;
	log_pal->palNumEntries = (WORD)(num_palette + add_white + add_black);
	for (int i = 0; i < num_palette; i++)
	{
		log_pal->palPalEntry[i].peRed = palette[i].Red;
		log_pal->palPalEntry[i].peGreen = palette[i].Green;
		log_pal->palPalEntry[i].peBlue = palette[i].Blue;
		log_pal->palPalEntry[i].peFlags = 0;
	}
	if (add_white)
	{
		log_pal->palPalEntry[num_palette].peRed = 0xff;
		log_pal->palPalEntry[num_palette].peGreen = 0xff;
		log_pal->palPalEntry[num_palette].peBlue = 0xff;
		log_pal->palPalEntry[num_palette].peFlags = 0;
	}
	if (add_black)
	{
		log_pal->palPalEntry[num_palette + add_white].peRed = 0;
		log_pal->palPalEntry[num_palette + add_white].peGreen = 0;
		log_pal->palPalEntry[num_palette + add_white].peBlue = 0;
		log_pal->palPalEntry[num_palette + add_white].peFlags = 0;
	}
	if (DitherWindowPalette)
		DeleteObject(DitherWindowPalette);
	DitherWindowPalette = CreatePalette(log_pal);
	delete [] (byte *)log_pal;
	PaletteInfo.Pal = DitherWindowPalette;
}
#endif
void
G42SurfaceController::HandleScreenPalette(void)
{
	if (ScreenPalette.GetOptimizedPalette())
	{
		G42ImageMode dither(G42ImageMode::DitherColor8,
			ScreenPalette.GetOptimizedPalette());
		if (DitherFilterIndex < 0)
		{
			DitherFilterIndex = NumFilters;
			AppendFilter(dither);
		}
		else
		{
			ReplaceFilter(dither, DitherFilterIndex);
		}
		#ifdef MSWIN
		SetWindowsDitherPalette(ScreenPalette.GetScreenPalette(),
			ScreenPalette.GetNumColors());
		#endif
	}
}
bool
G42SurfaceController::IdleAction (void)
{
	bool another = false;
	if (NumAreaControllers)
	{
		for (uintf area = 0; area < NumAreaControllers; area++)
		{
			if (AreaControllers[area])
			{
				if (AreaControllers[area]->IdleAction())
				{
					another = true;
				}
			}
		}
	}
	return another;
}
void
G42SurfaceController::SetPaletteType(
	G42ScreenPalette::G42StandardPaletteType type, uintf num_palette)
{
	ScreenPalette.SetStandardPaletteType(type, num_palette);
	HandleScreenPalette();
}
void
G42SurfaceController::SetAutoDraw(bool draw)
{
	AutoDraw = draw;
	for (uintf area = 0; area < NumAreaControllers; area++)
	{
		if (AreaControllers[area])
		{
			AreaControllers[area]->SetAutoDraw(draw);
		}
	}
}
int32
G42SurfaceController::GetAreaIndexAtPoint(const G42Point & point,
	bool find_closest) const
{
	G42Point p(point);
	#ifdef MSWIN
	p.X += Viewport.Left;
	p.Y += Viewport.Top;
	#endif
	return InternalGetAreaIndexAtPoint(p, find_closest);
}
int32
G42SurfaceController::GetDispatchArea (const G42Point & point) const
{
	if (MKDispatchMode == DispatchToActiveArea)
	{
		return ActiveAreaIndex;
	}
	return InternalGetAreaIndexAtPoint(point,
		MKDispatchMode == DispatchToClosestArea);
}
int32
G42SurfaceController::InternalGetAreaIndexAtPoint(const G42Point & point,
	bool find_closest) const
{
	uint32 index = -1;
	uint32 dist = 0x7fffffff;
	for (int32 area = NumAreaControllers - 1; area >= 0; area--)
	{
		if (AreaControllers[area])
		{
			G42Rect rect = AreaControllers[area]->GetArea();
			if (rect.Contains(point))
			{
				index = area;
				break;
			}
			if (find_closest)
			{
				uint32 x;
				if (point.X < rect.Left)
				{
					x = rect.Left - point.X;
				}
				else if (point.X > rect.Right)
				{
					x = point.X - rect.Right;
				}
				else
				{
					x = 0;
				}
				uint32 y;
				if (point.Y < rect.Top)
				{
					y = rect.Top - point.Y;
				}
				else if (point.Y > rect.Bottom)
				{
					y = point.Y - rect.Bottom;
				}
				else
				{
					y = 0;
				}
				int32 d = x * x + y * y;
				if (d < dist)
				{
					index = area;
					dist = d;
				}
			}
		}
	}
	return index;
}
#ifdef MSWIN
bool
G42SurfaceController::WinMouseDown(const G42Point & point, uintf mod_keys,
	int16 button)
{
	uint16 flags = 0;
	if (mod_keys & MK_SHIFT)
		flags |= G42ShiftDown;
	if (mod_keys & MK_CONTROL)
		flags |= G42CtrlDown;
	return MouseDown(point, button, flags);
}
bool
G42SurfaceController::WinMouseUp(const G42Point & point, uintf mod_keys,
	int16 button)
{
	uint16 flags = 0;
	if (mod_keys & MK_SHIFT)
		flags |= G42ShiftDown;
	if (mod_keys & MK_CONTROL)
		flags |= G42CtrlDown;
	return MouseUp(point, button, flags);
}
bool
G42SurfaceController::WinMouseMove(const G42Point & point, uintf mod_keys)
{
	uint16 flags = 0;
	if (mod_keys & MK_SHIFT)
		flags |= G42ShiftDown;
	if (mod_keys & MK_CONTROL)
		flags |= G42CtrlDown;
	uint16 buttons = 0;
	if (mod_keys & MK_LBUTTON)
		buttons |= G42LeftButton;
	if (mod_keys & MK_MBUTTON)
		buttons |= G42MiddleButton;
	if (mod_keys & MK_RBUTTON)
		buttons |= G42RightButton;
	return MouseMove(point, buttons, flags);
}
bool
G42SurfaceController::WinMouseDoubleClick(const G42Point & point,
	uintf mod_keys, int16 button)
{
	uint16 flags = 0;
	if (mod_keys & MK_SHIFT)
		flags |= G42ShiftDown;
	if (mod_keys & MK_CONTROL)
		flags |= G42CtrlDown;
	return MouseDoubleClick(point, button, flags);
}
bool
G42SurfaceController::WinKeyDown(uintf key, uintf)
{
	if ((key >= 0x25 && key <= 0x28) || key == 0x9)
	{
		return KeyDown((char)key);
	}
	return false;
}
bool
G42SurfaceController::WinChar(uintf key, uintf)
{
	if (key >= ' ' && key != 0x7f)
	{
		return KeyDown((char)key);
	}
	return false;
}
#endif
