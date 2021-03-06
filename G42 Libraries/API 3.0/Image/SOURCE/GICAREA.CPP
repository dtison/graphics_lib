// gicarea.cpp - G42AreaController
#define G42_EXPAND_PTR
#include "gicarea.h"
#include "gicsur.h"
#include "gpath.h"
#include "gtools.h"
G42AreaController::G42AreaController(G42Notifier * notifier)
:
	SurfaceController(0),
	NumFilters(0), Notifier(notifier),
	DeleteNotifier(false), ShouldDrawBackground(false),
	Area(0, 0, 0, 0), AutoDraw(true), ShouldDrawBorder(false)
{
	#ifdef MACOS
	AutoDraw = false;
	#endif
	// be sure to do this last so any exceptions thrown don't get here,
	// as it doesn't get cleared automatically
	if (!Notifier)
	{
		Notifier = new G42Notifier();
		DeleteNotifier = true;
	}
}
G42AreaController::~G42AreaController(void)
{
	if (DeleteNotifier && Notifier)
	{
		delete Notifier;
		DeleteNotifier = false;
		Notifier = 0;
	}
}
void
G42AreaController::Draw(G42DrawLocation location)
{
	Draw(Area, location);
}
void
G42AreaController::Rebuild(G42DrawLocation location)
{
	Rebuild(Area, location);
}
// This needs to use the new drawing objects to remove the MSWIN/MACOS
// dependency, as soon as I get around to cleaning up the drawing.
// It also needs to handle borders along with margins, and perhaps
// deal with the clipping rectangle.
#ifdef MSWIN
void
G42AreaController::DrawBackground(G42Rect & rect, G42DrawLocation location)
{
	if (ShouldDrawBackground)
	{
		G42Rect area(Area);
//		area &= rect;
		if (area.IsNull())
			return;
		G42DrawLocationVerifier dc(location, SurfaceController->GetSurface(),
			SurfaceController->GetWindowsPalette());
//		G42FillStyle style(dc, BackgroundColor, BackgroundStyleColor,
//			BackgroundStyle);
		SetBkMode(dc, OPAQUE);
		SetBkColor(dc, RGB(192, 192, 192));
//		SetBkColor(dc, RGB(255, 0, 0));
		LOGBRUSH b;
		b.lbStyle = BS_HATCHED;
		b.lbColor = RGB(128, 128, 128);
//		b.lbColor = RGB(0, 255, 0);
		b.lbHatch = HS_FDIAGONAL;
		HBRUSH brush = CreateBrushIndirect(&b);
		HBRUSH old_brush = (HBRUSH)SelectObject(dc, brush);
		HPEN old_pen = (HPEN)SelectObject(dc, GetStockObject(NULL_PEN));
#if 0
		HRGN clip = CreateRectRgn(
			area.Left - SurfaceController->GetViewport().Left,
			area.Top - SurfaceController->GetViewport().Top,
			area.Right - SurfaceController->GetViewport().Left + 2,
			area.Bottom - SurfaceController->GetViewport().Top + 2);
		SelectClipRgn(dc, clip);
#endif
//		area = Area;
		Rectangle(dc, area.Left - SurfaceController->GetViewport().Left,
			area.Top - SurfaceController->GetViewport().Top,
			area.Right - SurfaceController->GetViewport().Left + 2,
			area.Bottom - SurfaceController->GetViewport().Top + 2);
#if 0
		HRGN clip2 = CreateRectRgn(0, 0,
			SurfaceController->GetViewport().GetWidth(),
			SurfaceController->GetViewport().GetHeight());
		SelectClipRgn(dc, clip2);
		DeleteObject(clip);
#endif
		SelectObject(dc, old_pen);
		SelectObject(dc, old_brush);
		DeleteObject(brush);
#if 0
		bool delete_dc = false;
		if (!location)
		{
			location = GetDC(SurfaceController->GetSurface());
			delete_dc = true;
		}
//		HBRUSH brush = CreateSolidBrush(RGB(BackgroundColor.Red,
//			BackgroundColor.Green, BackgroundColor.Blue));
		LOGBRUSH lbrush;
		lbrush.lbStyle = BS_HATCHED;
		lbrush.lbColor = RGB(128, 128, 128);
		lbrush.lbHatch = HS_BDIAGONAL;
		HBRUSH brush = CreateBrushIndirect(&lbrush);
		SetBkColor(location, RGB(192, 192, 192));
		SetBkMode(location, OPAQUE);
		HBRUSH old_brush = (HBRUSH)SelectObject(location, brush);
//		HPEN old_pen = (HPEN)SelectObject(location, GetStockObject(NULL_PEN));
		HPEN old_pen = (HPEN)SelectObject(location, GetStockObject(BLACK_PEN));
		Rectangle(location, area.Left - SurfaceController->GetViewport().Left,
			area.Top - SurfaceController->GetViewport().Top,
			area.Right - SurfaceController->GetViewport().Left + 1, // 2,
			area.Bottom - SurfaceController->GetViewport().Top + 1); // 2);
		SelectObject(location, old_brush);
		DeleteObject(brush);
		SelectObject(location, old_pen);
		if (delete_dc && location)
		{
			ReleaseDC(SurfaceController->GetSurface(), location);
		}
#endif
	}
	if (ShouldDrawBorder)
	{
		G42DrawLocationVerifier dc(location, SurfaceController->GetSurface(),
			SurfaceController->GetWindowsPalette());
		G42FillStyle style(dc);
		HPEN pen = CreatePen(PS_SOLID, 0,
			RGB(BorderColor.Red, BorderColor.Green, BorderColor.Blue));
		HPEN old_pen = (HPEN)SelectObject(dc, pen);
		HBRUSH old_brush = (HBRUSH)SelectObject(dc, GetStockObject(NULL_BRUSH));
		Rectangle(dc, Area.Left - SurfaceController->GetViewport().Left,
			Area.Top - SurfaceController->GetViewport().Top,
			Area.Right - SurfaceController->GetViewport().Left + 1, // 2,
			Area.Bottom - SurfaceController->GetViewport().Top + 1); // 2);
		SelectObject(dc, old_brush);
		SelectObject(dc, old_pen);
		DeleteObject(pen);
	}
}
#else
void
G42AreaController::DrawBackground(G42Rect & rect, G42DrawLocation)
{
	/*  Temporary solution:  Draw [white] rectangle  */
	#if 0
	::ForeColor (whiteColor);
	::PaintRect (&((Rect) rect));
	::ForeColor (blackColor);
	#endif
	RGBColor color = BackgroundColor;
	::RGBForeColor (&color);
	::PaintRect (&((Rect) rect));
	::ForeColor (blackColor);
	
}
#endif
void
G42AreaController::SetScaleTranslate(G42ScaleTranslate & trans)
{
	ScaleTranslate = trans;
	SetImageScaleTranslate();
}
void
G42AreaController::SetCumlativeScaleTranslate(G42ScaleTranslate & trans)
{
	CumlativeScaleTranslate = trans;
	SetImageScaleTranslate();
}
void
G42AreaController::SetArea(void)
{
	Area.Left -= Margin.Left;
	Area.Right += Margin.Right;
	Area.Top -= Margin.Top;
	Area.Bottom += Margin.Bottom;
	if (ShouldDrawBorder)
	{
		Area.Left--;
		Area.Right++;
		Area.Top--;
		Area.Bottom++;
	}
	if (SurfaceController)
	{
		SurfaceController->AutoSizeCanvas();
	}
}
G42Notifier *
G42AreaController::SetNotifier(G42Notifier * notifier)
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
/*
	if (MouseKeyController)
	{
		MouseKeyController->SetNotifier(Notifier);
	}
*/
	return old_notifier;
}
void
G42AreaController::SetSurfaceController(G42SurfaceController * surface)
{
	SurfaceController = surface;
	if (DeleteNotifier)
		SetNotifier(SurfaceController->GetNotifier());
}
void
G42AreaController::SetMouseKeyController(G42MouseKeyController * mk)
{
	if (!ControlManager)
	{
		ControlManager.Set(new G42ControlManager(this,
			SurfaceController->GetSurface(), Notifier));
	}
	if (mk)
	{
		mk->SetAreaController(this);
		mk->SetCanvasSize(SurfaceController->GetCanvasSize());
		mk->SetViewport(SurfaceController->GetViewport());
	}
	ControlManager->SetController(mk);
}
G42Surface
G42AreaController::GetSurface(void)
{
	return (SurfaceController ? SurfaceController->GetSurface() : 0);
}
G42Size &
G42AreaController::GetCanvasSize(void)
{
	return SurfaceController->GetCanvasSize();
}
void
G42AreaController::MakeVisible(const G42Rect & rect)
{
	if (SurfaceController)
	{
		SurfaceController->MakeVisible(rect);
	}
}
void
G42AreaController::SetViewport(const G42Rect & rect)
{
	if (ControlManager.NotNull())
	{
		ControlManager->SetViewport(rect);
	}
}
void
G42AreaController::InsertFilter(const G42ImageFilter & filter, uintf index)
{
	for (uintf i = NumFilters; i > index; i--)
	{
		Filters[i].Set(Filters[i - 1].Release());
	}
	NumFilters++;
	Filters[index].Set(filter.MakeCopy());
	ResetImageFilters();
}
void
G42AreaController::RemoveFilter(uintf index)
{
	Filters[index].Clear();
	NumFilters--;
	for (uintf i = index; i < NumFilters; i++)
	{
		Filters[i].Set(Filters[i + 1].Release());
	}
	ResetImageFilters();
}
void
G42AreaController::ClearFilters(void)
{
	Filters.Clear();
	NumFilters = 0;
	ResetImageFilters();
}
G42Image *
G42AreaController::GetImage(void)
{
	return 0;
}
G42Display *
G42AreaController::GetDisplay(void)
{
	return 0;
}
void
G42AreaController::Invalidate(const G42Rect & rect, bool /* image_only */)
{
	if (SurfaceController)
	{
		SurfaceController->DrawInternal(rect);
	}
}
void
G42AreaController::Invalidate(bool image_only)
{
	Invalidate(SurfaceController->GetViewport(), image_only);
//	Invalidate(Area, image_only);
}
void
G42AreaController::Play(void)
{
}
void
G42AreaController::Pause(void)
{
}
void
G42AreaController::Stop(void)
{
}
void
G42AreaController::Forward(void)
{
}
void
G42AreaController::Backward(void)
{
}
void
G42AreaController::Rewind(void)
{
}
void
G42AreaController::GoToEnd(void)
{
}
bool
G42AreaController::IsPaused(void)
{
	return false;
}
bool
G42AreaController::IsPlaying(void)
{
	return false;
}
bool
G42AreaController::GetDrawAnimated(void)
{
	return false;
}
void
G42AreaController::SetDrawAnimated(bool)
{
}
