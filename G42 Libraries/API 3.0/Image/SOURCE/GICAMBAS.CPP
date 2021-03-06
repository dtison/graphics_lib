// gicamult.cpp - G42AreaMultiController
#include "gicambas.h"
#include "gicsur.h"
#include "gpath.h"
#include "gtools.h"
#include "gpobj.h"
G42MultiAreaBaseController::G42MultiAreaBaseController(G42Notifier * notifier)
:
	G42AreaSingleController(notifier), NumImages(0), ThumbWidth(100),
	ThumbHeight(100), SpaceBetween(25), SpaceBelow(25), TextSpace(50),
	TextSpaceAbove(25), GrowDown(true), NumThumbsAcross(1), NumThumbsDown(1),
	DrawThumbs(true), AutoArrangeThumbs(true), FitToViewport(true),
	NumSelected(0)
{
}
G42MultiAreaBaseController::~G42MultiAreaBaseController(void)
{
}
void
G42MultiAreaBaseController::DrawSelected(uintf i, G42DrawLocation location)
{
	if (Selected[i])
	{
		#ifdef MSWIN
		intf row;
		intf column;
		if (GrowDown)
		{
			row = i / NumThumbsAcross;
			column = i % NumThumbsAcross;
		}
		else
		{
			row = i % NumThumbsDown;
			column = i / NumThumbsDown;
		}
		uint32 x = column * (ThumbWidth + SpaceBetween) - (SpaceBetween >> 1);
		uint32 y = row * (ThumbHeight + SpaceBelow + TextSpace +
			TextSpaceAbove) - (SpaceBelow >> 1);
		G42FillStyle fs(location, BorderColor);
		HPEN old_pen = (HPEN)SelectObject(location, GetStockObject(NULL_PEN));
		G42Rect rect(x, y, x + ThumbWidth + SpaceBetween,
			y + ThumbHeight + SpaceBelow + TextSpace + TextSpaceAbove);
		G42Point translate(ScaleTranslate.GetTranslate());
		CumlativeScaleTranslate.Convert(translate);
		G42ScaleTranslate image_trans(
			CumlativeScaleTranslate.GetScaleX() * ScaleTranslate.GetScaleX(),
			CumlativeScaleTranslate.GetScaleY() * ScaleTranslate.GetScaleY(),
			translate);
		image_trans.Convert(rect);
		Rectangle(location, rect.Left - SurfaceController->GetViewport().Left,
			rect.Top - SurfaceController->GetViewport().Top,
			rect.Right - SurfaceController->GetViewport().Left + 1, // 2,
			rect.Bottom - SurfaceController->GetViewport().Top + 1); // 2);
		SelectObject(location, old_pen);
		#endif
	}
}
void
G42MultiAreaBaseController::DrawText(uintf i, G42DrawLocation location)
{
	#ifdef MSWIN
	intf row;
	intf column;
	if (GrowDown)
	{
		row = i / NumThumbsAcross;
		column = i % NumThumbsAcross;
	}
	else
	{
		row = i % NumThumbsDown;
		column = i / NumThumbsDown;
	}
	uint32 x = column * (ThumbWidth + SpaceBetween);
	uint32 y = row * (ThumbHeight + SpaceBelow + TextSpace + TextSpaceAbove);
	G42FillStyle style(location);
	HPEN pen;
	if (Selected[i])
	{
		pen = CreatePen(PS_SOLID, 0,
			RGB(BackgroundColor.Red, BackgroundColor.Green, BackgroundColor.Blue));
	}
	else
	{
		pen = CreatePen(PS_SOLID, 0,
			RGB(BorderColor.Red, BorderColor.Green, BorderColor.Blue));
	}
	HPEN old_pen = (HPEN)SelectObject(location, pen);
	HBRUSH old_brush = (HBRUSH)SelectObject(location, GetStockObject(NULL_BRUSH));
	G42Rect rect(x - 1, y - 1, x + ThumbWidth, y + ThumbHeight);
	G42Point translate(ScaleTranslate.GetTranslate());
	CumlativeScaleTranslate.Convert(translate);
	G42ScaleTranslate image_trans(
		CumlativeScaleTranslate.GetScaleX() * ScaleTranslate.GetScaleX(),
		CumlativeScaleTranslate.GetScaleY() * ScaleTranslate.GetScaleY(),
		translate);
	image_trans.Convert(rect);
	Rectangle(location, rect.Left - SurfaceController->GetViewport().Left,
		rect.Top - SurfaceController->GetViewport().Top,
		rect.Right - SurfaceController->GetViewport().Left + 1, // 2,
		rect.Bottom - SurfaceController->GetViewport().Top + 1); // 2);
	if (StringArray[i])
	{
		if (Selected[i])
		{
			SetTextColor(location,
				RGB(BackgroundColor.Red, BackgroundColor.Green, BackgroundColor.Blue));
		}
		else
		{
			SetTextColor(location,
				RGB(BorderColor.Red, BorderColor.Green, BorderColor.Blue));
		}
		G42Rect rect(x, y + ThumbHeight + TextSpaceAbove, x + ThumbWidth,
			y + ThumbHeight + TextSpaceAbove + TextSpace);
		image_trans.Convert(rect);
		RECT r;
		r.left = rect.Left - SurfaceController->GetViewport().Left;
		r.top = rect.Top - SurfaceController->GetViewport().Top;
		r.right = rect.Right - SurfaceController->GetViewport().Left;
		r.bottom = rect.Bottom - SurfaceController->GetViewport().Top;
		::DrawText(location, (char *)StringArray[i], -1, &r,
			DT_CENTER | DT_NOPREFIX | DT_WORDBREAK);
//		Rectangle(location, r.left, r.top, r.right, r.bottom);
	}
	SelectObject(location, old_brush);
	SelectObject(location, old_pen);
	DeleteObject(pen);
	#endif
}
void
G42MultiAreaBaseController::Draw(G42Rect & rect, G42DrawLocation location,
	bool image_only)
{
	if (DrawThumbs)
	{
//		DrawBackground(rect, location);
		if (!NumImages)
		{
			return;
		}
		for (uint32 i = 0; i < NumImages; i++)
		{
			DrawSelected(i, location);
			ImageControllers[i]->GetDisplay()->Draw(rect, location);
			DrawText(i, location);
		}
		if (ControlManager && !image_only)
		{
			ControlManager->Draw(location);
		}
	}
	else
	{
		G42AreaSingleController::Draw(rect, location, image_only);
	}
}
void
G42MultiAreaBaseController::Rebuild(G42Rect & rect, G42DrawLocation location,
	bool image_only)
{
	if (DrawThumbs)
	{
//		DrawBackground(rect, location);
		if (!NumImages)
		{
			return;
		}
		for (uint32 i = 0; i < NumImages; i++)
		{
			ImageControllers[i]->GetDisplay()->SetAutoDraw(false);
			G42Path path(*(ImageControllers[i]->GetImage()));
			path.AddLink(*(ImageControllers[i]->GetImage()),
				*(ImageControllers[i]->GetDisplay()));
			path.Start();
			path.Clear();
			ImageControllers[i]->GetDisplay()->SetAutoDraw(true);
			DrawSelected(i, location);
			ImageControllers[i]->GetDisplay()->Draw(rect, location);
			DrawText(i, location);
		}
		if (ControlManager && !image_only)
		{
			ControlManager->Draw(location);
		}
	}
	else
	{
		G42AreaSingleController::Rebuild(rect, location, image_only);
	}
}
/*
void
G42MultiAreaBaseController::SetScaleTranslate(G42ScaleTranslate & trans)
{
}
void
G42MultiAreaBaseController::SetCumlativeScaleTranslate(G42ScaleTranslate & trans)
{
}
*/
void
G42MultiAreaBaseController::SetImageScaleTranslate(void)
{
	G42Point translate(ScaleTranslate.GetTranslate());
	CumlativeScaleTranslate.Convert(translate);
	G42ScaleTranslate image_trans(
		CumlativeScaleTranslate.GetScaleX() * ScaleTranslate.GetScaleX(),
		CumlativeScaleTranslate.GetScaleY() * ScaleTranslate.GetScaleY(),
		translate);
//	if (DrawThumbs)
	{
		for (uint32 i = 0; i < NumImages; i++)
		{
			if (ImageControllers[i])
			{
				ImageControllers[i]->SetCumlativeScaleTranslate(image_trans);
			}
		}
	}
	ImageController->SetCumlativeScaleTranslate(image_trans);
}
void
G42MultiAreaBaseController::SetArea(void)
{
	if (DrawThumbs)
	{
		if (NumImages > 0)
		{
			Area.Left = 0;
			Area.Top = 0;
			Area.Right = Area.Left +
				NumThumbsAcross * (ThumbWidth + SpaceBetween) - SpaceBetween;
			Area.Bottom = Area.Top +
				NumThumbsDown * (ThumbHeight + SpaceBelow + TextSpace +
				TextSpaceAbove) - SpaceBelow;
			G42Point translate(ScaleTranslate.GetTranslate());
			CumlativeScaleTranslate.Convert(translate);
			G42ScaleTranslate image_trans(
				CumlativeScaleTranslate.GetScaleX() * ScaleTranslate.GetScaleX(),
				CumlativeScaleTranslate.GetScaleY() * ScaleTranslate.GetScaleY(),
				translate);
			image_trans.Convert(Area);
		}
		else
		{
			Area.Left = 0;
			Area.Right = 0;
			Area.Top = 0;
			Area.Bottom = 0;
			G42Point translate(ScaleTranslate.GetTranslate());
			CumlativeScaleTranslate.Convert(translate);
			G42ScaleTranslate image_trans(
				CumlativeScaleTranslate.GetScaleX() * ScaleTranslate.GetScaleX(),
				CumlativeScaleTranslate.GetScaleY() * ScaleTranslate.GetScaleY(),
				translate);
			image_trans.Convert(Area);
		}
		G42AreaController::SetArea();
	}
	else
	{
		G42AreaSingleController::SetArea();
	}
}
void
G42MultiAreaBaseController::SetMouseKeyController(G42MouseKeyController * mk)
{
	G42AreaController::SetMouseKeyController(mk);
}
void
G42MultiAreaBaseController::SetViewport(const G42Rect & viewport)
{
	G42AreaController::SetViewport(viewport);
	for (uint32 i = 0; i < NumImages; i++)
	{
		if (ImageControllers[i].NotNull())
		{
			ImageControllers[i]->GetDisplay()->SetViewport(viewport);
		}
	}
	if (DrawThumbs)
	{
		ResetThumbsAcross();
	}
//	else
	{
		G42AreaController::SetViewport(viewport);
	}
}
G42Notifier *
G42MultiAreaBaseController::SetNotifier(G42Notifier * notifier)
{
	return G42AreaController::SetNotifier(notifier);
}
void
G42MultiAreaBaseController::ResetGrowth(void)
{
	if (NumImages == 1)
	{
		ResetThumbsAcross();
	}
	if (GrowDown)
	{
		NumThumbsDown = (NumImages - 1) / NumThumbsAcross + 1;
	}
	else
	{
		NumThumbsAcross = (NumImages - 1) / NumThumbsDown + 1;
	}
	SetArea();
}
void
G42MultiAreaBaseController::ResetThumbsAcross(void)
{
	bool need_reset = false;
	if (GrowDown)
	{
		int32 width;
		int32 new_thumbs_across = NumThumbsAcross;
		bool recalculate = false;
		if (FitToViewport)
		{
			width = SurfaceController->GetViewport().Right -
				SurfaceController->GetMargins().Right - Area.Left + 1;
			if (width < 0)
			{
				width = 0;
			}
			recalculate = true;
		}
		else if (AutoArrangeThumbs)
		{
			width = AreaWidth;
			recalculate = true;
		}
		if (recalculate)
		{
			new_thumbs_across = (width + SpaceBetween) /
				(ThumbWidth + SpaceBetween);
		}
		
		if (new_thumbs_across <= 0)
		{
			new_thumbs_across = 1;
		}
		if (new_thumbs_across != NumThumbsAcross)
		{
			NumThumbsAcross = new_thumbs_across;
			need_reset = true;
		}
		NumThumbsDown = (NumImages - 1) / NumThumbsAcross + 1;
	}
	else
	{
		int32 height;
		int32 new_thumbs_down = NumThumbsDown;
		bool recalculate = false;
		if (FitToViewport)
		{
			height = SurfaceController->GetViewport().Bottom -
				SurfaceController->GetMargins().Bottom - Area.Top + 1;
			if (height < 0)
			{
				height = 0;
			}
			recalculate = true;
		}
		else if (AutoArrangeThumbs)
		{
			height = AreaHeight;
			recalculate = true;
		}
		if (recalculate)
		{
			new_thumbs_down = (height + SpaceBelow) /
				(ThumbHeight + TextSpace + TextSpaceAbove + SpaceBelow);
		}
		if (new_thumbs_down <= 0)
		{
			new_thumbs_down = 1;
		}
		if (new_thumbs_down != NumThumbsDown)
		{
			NumThumbsDown = new_thumbs_down;
			need_reset = true;
		}
		NumThumbsAcross = (NumImages - 1) / NumThumbsDown + 1;
	}
	if (need_reset)
	{
		for (uint32 i = 0; i < NumImages; i++)
		{
			ResetThumbLocation(i);
		}
		SetArea();
	}
}
void
G42MultiAreaBaseController::ResetThumbLocation(uint32 index)
{
	if (NumImages == 0 || NumThumbsAcross == 0)
		return;
	uint32 row;
	uint32 column;
	if (GrowDown)
	{
		row = index / NumThumbsAcross;
		column = index % NumThumbsAcross;
	}
	else
	{
		row = index % NumThumbsDown;
		column = index / NumThumbsDown;
	}
	uint32 x = column * (ThumbWidth + SpaceBetween);
	uint32 y = row * (ThumbHeight + SpaceBelow + TextSpace + TextSpaceAbove);
	if (ImageControllers[index].NotNull() &&
		ImageControllers[index]->GetImage() &&
		!ImageControllers[index]->GetImage()->IsEmpty())
	{
		uint32 width = ImageControllers[index]->GetImage()->GetInfo().GetWidth();
		uint32 height = ImageControllers[index]->GetImage()->GetInfo().GetHeight();
		if (!width || !height)
			return;
		double sx = (double)ThumbWidth / (double)width;
		double sy = (double)ThumbHeight / (double)height;
		double scale;
		if (sx >= 1.0 && sy >= 1.0)
		{
			scale = 1.0;
			x += ((ThumbWidth - width) >> 1);
			y += ((ThumbHeight - height) >> 1);
		}
		else if (sx < sy)
		{
			scale = sx;
			y += ((ThumbHeight - (uintf)(scale * (double)height)) >> 1);
		}
		else
		{
			scale = sy;
			x += ((ThumbWidth - (uintf)(scale * (double)width)) >> 1);
		}
		ImageControllers[index]->SetScaleTranslate(
			G42ScaleTranslate(scale, G42Point(x, y)));
	}
}
void
G42MultiAreaBaseController::SetThumbnailMode(bool thumbnail_mode)
{
	DrawThumbs = thumbnail_mode;
	SetArea();
//	SetImageScaleTranslate();
	G42AreaController::Rebuild();
}
void
G42MultiAreaBaseController::ResetImageFilters(void)
{
/*
	if (ImageController.NotNull())
	{
		ImageController->ClearFilters();
		for (uintf i = 0; i < NumFilters; i++)
		{
			ImageController->AppendFilter(Filters[i].GetReference());
		}
	}
*/
}
void
G42MultiAreaBaseController::SetThumbLimit(G42LimitDirection direction,
	G42LimitType type, uintf num)
{
	GrowDown = (direction == LimitAcross);
	if (type == LimitViewport)
	{
		FitToViewport = true;
		AutoArrangeThumbs = false;
	}
	else if (type == LimitPixels)
	{
		FitToViewport = false;
		AutoArrangeThumbs = true;
		if (GrowDown)
		{
			AreaWidth = num;
		}
		else
		{
			AreaHeight = num;
		}
	}
	else
	{
		FitToViewport = false;
		AutoArrangeThumbs = false;
		if (GrowDown)
		{
			NumThumbsAcross = num;
		}
		else
		{
			NumThumbsDown = num;
		}
	}
	ResetThumbsAcross();
	SetArea();
}
uint32
G42MultiAreaBaseController::GetThumbAtPoint(const G42Point & p) const
{
	G42Point translate(ScaleTranslate.GetTranslate());
	CumlativeScaleTranslate.Convert(translate);
	G42ScaleTranslate image_trans(
		CumlativeScaleTranslate.GetScaleX() * ScaleTranslate.GetScaleX(),
		CumlativeScaleTranslate.GetScaleY() * ScaleTranslate.GetScaleY(),
		translate);
	G42Point pt = p;
//	image_trans.ConvertBack(pt);
	intf row = (pt.Y + (SpaceBelow >> 1)) / (ThumbHeight + SpaceBelow +
		TextSpace + TextSpaceAbove);
	if (row < 0)
	{
		row = 0;
	}
	if (row >= NumThumbsDown)
	{
		row = NumThumbsDown - 1;
	}
	intf column = (pt.X + (SpaceBetween >> 1)) / (ThumbWidth + SpaceBetween);
	if (column < 0)
	{
		column = 0;
	}
	if (column >= NumThumbsAcross)
	{
		column = NumThumbsAcross - 1;
	}
	uintf thumb;
	if (GrowDown)
	{
		thumb = column + row * NumThumbsAcross;
	}
	else
	{
		thumb = row + column * NumThumbsDown;
	}
	if (thumb >= NumImages)
	{
		thumb = NumImages - 1;
	}
	return thumb;
}
uint32
G42MultiAreaBaseController::GetInsertIndexAtPoint(const G42Point & p) const
{
	G42Point translate(ScaleTranslate.GetTranslate());
	CumlativeScaleTranslate.Convert(translate);
	G42ScaleTranslate image_trans(
		CumlativeScaleTranslate.GetScaleX() * ScaleTranslate.GetScaleX(),
		CumlativeScaleTranslate.GetScaleY() * ScaleTranslate.GetScaleY(),
		translate);
	G42Point pt = p;
//	image_trans.ConvertBack(pt);
	intf row = (pt.Y + (SpaceBelow >> 1)) / (ThumbHeight + SpaceBelow +
		TextSpace + TextSpaceAbove);
	if (row < 0)
	{
		row = 0;
	}
	if (row > NumThumbsDown)
	{
		row = NumThumbsDown;
	}
	intf column = (pt.X + ((ThumbWidth + SpaceBetween) >> 1)) /
		(ThumbWidth + SpaceBetween);
	if (column < 0)
	{
		column = 0;
	}
	if (column > NumThumbsAcross)
	{
		column = NumThumbsAcross;
	}
	uintf thumb;
	if (GrowDown)
	{
		thumb = column + row * NumThumbsAcross;
	}
	else
	{
		thumb = row + column * NumThumbsDown;
	}
	if (thumb > NumImages)
	{
		thumb = NumImages;
	}
	return thumb;
}
void
G42MultiAreaBaseController::SetSelected(uintf index, bool selected)
{
	if (selected == Selected[index])
	{
		return;
	}
	ToggleSelected(index);
}
bool
G42MultiAreaBaseController::IsSelected(uintf index)
{
	return Selected[index];
}
void
G42MultiAreaBaseController::ToggleSelected(uintf index)
{
	if (Selected[index])
	{
		Selected[index] = false;
		NumSelected--;
	}
	else
	{
		Selected[index] = true;
		NumSelected++;
	}
	// Need Invalidate for this one area?
}
void
G42MultiAreaBaseController::ClearAllSelected(void)
{
	NumSelected = 0;
	for (uintf i = 0; i < NumImages; i++)
	{
		Selected[i] = false;
	}
	// Need Invalidate
}
void
G42MultiAreaBaseController::SelectAll(void)
{
	for (uintf i = 0; i < NumImages; i++)
	{
		Selected[i] = true;
	}
	NumSelected = NumImages;
	// Need Invalidate
}
uintf
G42MultiAreaBaseController::GetNumSelected(void)
{
	return NumSelected;
}
void
G42MultiAreaBaseController::DeleteThumb(uint32 thumb, bool copy_strings)
{
	NumImages--;
	for (uint32 i = thumb; i < NumImages; i++)
	{
		ImageControllers[i] = ImageControllers[i + 1].Release();
		Selected[i] = Selected[i + 1];
		if (copy_strings)
		{
			StringArray[i] = StringArray[i + 1].Release();
		}
	}
	if (!copy_strings)
	{
		StringArray[NumImages] = 0;
	}
	ResetThumbsAcross();
	for (uint32 j = thumb; j < NumImages; j++)
	{
		ResetThumbLocation(j);
	}
}
void
G42MultiAreaBaseController::MoveThumb(
	uint32 old_index, uint32 new_index, bool copy_strings)
{
	if (old_index == new_index)
	{
		return;
	}
	G42Ptr<G42ImageController> image;
	image = ImageControllers[old_index].Release();
	bool selected = Selected[old_index];
	G42APtr<char> string;
	if (copy_strings)
	{
		string = StringArray[old_index].Release();
	}
	if (old_index < new_index)
	{
		for (uint32 i = old_index; i < new_index; i++)
		{
			ImageControllers[i] = ImageControllers[i + 1].Release();
			Selected[i] = Selected[i + 1];
			if (copy_strings)
			{
				StringArray[i] = StringArray[i + 1].Release();
			}
			ResetThumbLocation(i);
		}
	}
	else
	{
		for (uint32 i = old_index; i > new_index; i--)
		{
			ImageControllers[i] = ImageControllers[i - 1].Release();
			Selected[i] = Selected[i - 1];
			if (copy_strings)
			{
				StringArray[i] = StringArray[i - 1].Release();
			}
			ResetThumbLocation(i);
		}
	}
	ImageControllers[new_index] = image.Release();
	Selected[new_index] = selected;
	if (copy_strings)
	{
		StringArray[new_index] = string.Release();
	}
	ResetThumbLocation(new_index);
}
