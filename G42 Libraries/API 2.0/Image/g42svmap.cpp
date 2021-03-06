
#include <g42itype.h>
#include <g42sview.h>
#include <g42image.h>
#include <math.h>
//#include "gesdebug.h"
#ifdef MACOS
Pattern	G42SingleImageViewer::AntsPattern = {0xF8, 0xF1, 0xE3, 0xC7, 0x8F, 0x1F, 
	0x3E, 0x7C};	
Pattern	G42SingleImageViewer::AntsPattern2 = {0x88, 0x44, 0x22, 0x11, 0x88, 0x44, 
	0x22, 0x11};		
#endif
void G42SingleImageViewer::DrawImageMapElement(G42DrawLocation location, uint index)
{
	const G42ImageMap::G42MapElement * element = ImageMap->GetElement(index);
	if (!element)
		return;
	DrawImageMapElement(location, element, false, index == CurrentElement,
		ImageMap->HasSelectedElement() &&
			index == ImageMap->GetSelectedElement());
}
void G42SingleImageViewer::DrawMap(G42DrawLocation location)
{
	if (!ImageMap || !ImageMapVisible ||
		!(ImageMap->GetNumElements() || NewMapElementStarted))
		return;
	for (int i = 0; i < ImageMap->GetNumElements(); i++)
	{
		DrawImageMapElement(location, i);
	}
	if (ImageMap->HasSelectedElement() && ImageMap->GetElement(
		ImageMap->GetSelectedElement())->Type != G42ImageMap::Default)
	{
		PrepareDrawArea(location);
		DrawSelectedHandles (location);
		DrawSelectedRectangle(location);
		CleanupDrawArea(location);
	}
	if (NewMapElementStarted || StretchingElement || EditingPolygon ||
		MovingElement)
	{
		PrepareDrawRubber(location);
		DrawImageMapElement(location, &NewElement, true);
		CleanupDrawRubber(location);
	}
}
void G42SingleImageViewer::PrepareDrawMap(G42DrawLocation location)
{
	#ifdef MSWIN
	SelectObject(location, GetStockObject(NULL_BRUSH));
	MapNormalPen = CreatePen(PS_DOT, 1, RGB(0xff, 0xff, 0xff));
	MapSelectedPen = CreatePen(PS_DASH, 1, RGB(0xff, 0xff, 0xff));
	SetBkMode(location, OPAQUE);
	SetBkColor(location, RGB(0, 0, 0));
	OldMapPen = (HPEN)SelectObject(location, MapNormalPen);
	SetROP2(location, R2_COPYPEN);
	SetStretchBltMode(location, STRETCH_DELETESCANS);
	#endif
	#ifdef MACOS	
	/*  Use marching ants pattern  */
	if (PrepareDrawMapCnt == 0)	//  Sometimes prepares weren't balanced with cleanups
	{
		::GetGWorld ((GWorldPtr *) &OldPrepDrawMapPort, &OldPrepDrawMapDevice);
		::SetGWorld ((GWorldPtr) location, nil);
		::LockPixels (::GetGWorldPixMap ((GWorldPtr) location));	
		::PenNormal();
		::PenPat (&AntsPattern);
		PrepareDrawMapCnt++;
	}
	#endif
}
void G42SingleImageViewer::CleanupDrawMap(G42DrawLocation location)
{
#ifdef MSWIN
	SelectObject(location, OldMapPen);
	DeleteObject(MapNormalPen);
	DeleteObject(MapSelectedPen);
#endif
#ifdef MACOS
	if (PrepareDrawMapCnt > 0)	//  Sometimes prepares weren't balanced with cleanups
	{
		::UnlockPixels (::GetGWorldPixMap ((GWorldPtr) location));
		::SetGWorld ((GWorldPtr) OldPrepDrawMapPort, OldPrepDrawMapDevice);
		::PenNormal();
		PrepareDrawMapCnt--;
	}
#endif
}
void G42SingleImageViewer::SetImageMapVisible(bool visible)
{
	if (visible != ImageMapVisible)
	{
		ImageMapVisible = visible;
		ImageMapVisible = visible;
		#ifndef MACOS
		Draw();
		#endif
		if (ImageMapVisible)
			SetCursor(CrossCursor);
		else
			SetCursor(ArrowCursor);
	}
}
// actual draw of the element, different on MSWIN and MACOS
// if is_new, we need to draw as a rubber object.  This call
// will be sandwiched between PrepareDrawRubber() and
// CleanupDrawRubber() calls, and is_current and is_selected
// will both be false.  Otherwise, this call will be between
// PrepareDrawMap() and CleanupDrawMap() calls.  is_selected
// means the object is selected, and will be drawn with the
// selected rectangle afterwards, which must be set up here.
// If is_current, the mouse is over this object and the object
// should be "highlighted" somehow 
#ifdef MSWIN
void G42SingleImageViewer::DrawImageMapElement(G42DrawLocation location,
	const G42ImageMap::G42MapElement * element, bool is_new,
	bool is_current, bool is_selected)
{
	if (!element)
		return;
	switch (element->Type)
	{
		case G42ImageMap::Rectangle:
		{
			if (is_selected)
			{
				SelX = (int)(ImageToWindow(min(element->Coords[0].x,
					element->Coords[1].x)) - PanX) - 1;
				SelY = (int)(ImageToWindow(min(element->Coords[0].y,
					element->Coords[1].y)) - PanY) - 1;
				SelWidth = (int)(ImageToWindow(max(element->Coords[0].x,
					element->Coords[1].x)) - PanX - SelX + 2);
				SelHeight = (int)(ImageToWindow(max(element->Coords[0].y,
					element->Coords[1].y)) - PanY - SelY + 2);
				SelStretch = true;
				SelectObject(location, MapSelectedPen);
			}
			else if (is_current)
			{
				SelectObject(location, MapSelectedPen);
			}
			Rectangle(location,
				(int)(ImageToWindow(element->Coords[0].x) - PanX),
				(int)(ImageToWindow(element->Coords[0].y) - PanY),
				(int)(ImageToWindow(element->Coords[1].x) - PanX),
				(int)(ImageToWindow(element->Coords[1].y) - PanY));
			if (is_selected || is_current)
				SelectObject(location, MapNormalPen);
			break;
		}
		case G42ImageMap::Circle:
		{
			if (is_selected)
			{
				SelX = (int)(ImageToWindow(min(element->Coords[0].x,
					element->Coords[1].x)) - PanX - 1);
				SelY = (int)(ImageToWindow(min(element->Coords[0].y,
					element->Coords[1].y)) - PanY - 1);
				SelWidth = (int)(ImageToWindow(max(element->Coords[0].x,
					element->Coords[1].x)) - PanX) - SelX + 2;
				SelHeight = (int)(ImageToWindow(max(element->Coords[0].y,
					element->Coords[1].y)) - PanY) - SelY + 2;
				SelStretch = true;
				SelectObject(location, MapSelectedPen);
			}
			else if (is_current)
			{
				SelectObject(location, MapSelectedPen);
			}
			Ellipse(location,
				(int)(ImageToWindow(element->Coords[0].x) - PanX),
				(int)(ImageToWindow(element->Coords[0].y) - PanY),
				(int)(ImageToWindow(element->Coords[1].x) - PanX),
				(int)(ImageToWindow(element->Coords[1].y) - PanY));
			if (is_selected || is_current)
				SelectObject(location, MapNormalPen);
			break;
		}
		case G42ImageMap::Polygon:
		{
			if (is_selected)
			{
				SelX = element->Coords[0].x;
				SelY = element->Coords[0].y;
				SelWidth = SelX;
				SelHeight = SelY;
				SelStretch = false;
				for (uint i = 1; i < element->NumCoords; i++)
				{
					if (element->Coords[i].x < SelX)
						SelX = element->Coords[i].x;
					if (element->Coords[i].x > SelWidth)
						SelWidth = element->Coords[i].x;
					if (element->Coords[i].y < SelY)
						SelY = element->Coords[i].y;
					if (element->Coords[i].y > SelHeight)
						SelHeight = element->Coords[i].y;
				}
				SelX = (int)(ImageToWindow(SelX) - PanX);
				SelY = (int)(ImageToWindow(SelY) - PanY);
				SelWidth = (int)(ImageToWindow(SelWidth) - PanX);
				SelHeight = (int)(ImageToWindow(SelHeight) - PanY);
				SelWidth -= SelX - 1;
				SelHeight -= SelY - 1;
				int box_side = ((SelectMargin + 1) >> 1);
				SelX -= box_side + 1;
				SelY -= box_side + 1;
				SelWidth += (box_side << 1) + 2;
				SelHeight += (box_side << 1) + 2;
				SelectObject(location, MapSelectedPen);
			}
			else if (is_current)
			{
				SelectObject(location, MapSelectedPen);
			}
			uint num_point_array = element->NumCoords;
			if (is_new && NewPolygonStarted)
				num_point_array++;
			POINT * point_array = new POINT [num_point_array];
			for (uint i = 0; i < num_point_array; i++)
			{
				point_array[i].x = (int)(ImageToWindow(element->Coords[i].x) - PanX);
				point_array[i].y = (int)(ImageToWindow(element->Coords[i].y) - PanY);
			}
			if (is_new && !NewPolygonClosed)
				Polyline(location, point_array, num_point_array);
			else
				Polygon(location, point_array, num_point_array);
			if (is_selected)
			{
				int box_side = ((SelectMargin + 1) >> 1);
				for (uint i = 0; i < num_point_array; i++)
				{
					Rectangle(location, point_array[i].x - box_side,
						point_array[i].y - box_side,
						point_array[i].x + box_side,
						point_array[i].y + box_side);
				}
			}
			delete [] point_array;
			if (is_selected || is_current)
				SelectObject(location, MapNormalPen);
			break;
		}
		case G42ImageMap::Point:
		{
			if (is_selected)
			{
				SelX = (int)(ImageToWindow(element->Coords[0].x) - PanX) -
					MapPointSize - 2;
				SelY = (int)(ImageToWindow(element->Coords[0].y) - PanY) -
					MapPointSize - 2;
				SelWidth = (MapPointSize << 1) + 6;
				SelHeight = (MapPointSize << 1) + 6;
				SelStretch = false;
				SelectObject(location, MapSelectedPen);
			}
			else if (is_current)
			{
				SelectObject(location, MapSelectedPen);
			}
			MoveToEx(location,
				(int)(ImageToWindow(element->Coords[0].x + MapPointSize) - PanX),
				(int)(ImageToWindow(element->Coords[0].y) - PanY), 0);
			LineTo(location,
				(int)(ImageToWindow(element->Coords[0].x - MapPointSize) - PanX),
				(int)(ImageToWindow(element->Coords[0].y) - PanY));
			MoveToEx(location,
				(int)(ImageToWindow(element->Coords[0].x) - PanX),
				(int)(ImageToWindow(element->Coords[0].y + MapPointSize) - PanY), 0);
			LineTo(location,
				(int)(ImageToWindow(element->Coords[0].x) - PanX),
				(int)(ImageToWindow(element->Coords[0].y - MapPointSize) - PanY));
			if (is_selected || is_current)
				SelectObject(location, MapNormalPen);
			break;
		}
#if 0
		case G42ImageMap::Default:
		{
			if (is_selected)
			{
				SelX = (int)(-PanX) - 1;
				SelY = (int)(-PanY) - 1;
				SelWidth = (int)(ImageToWindow(Image->GetInfo().Width - 1) + 4);
				SelHeight = (int)(ImageToWindow(Image->GetInfo().Height - 1) + 4);
				SelStretch = false;
			}
			if (is_current || is_selected)
				SelectObject(location, MapSelectedPen);
			Rectangle(location,
				(int)(ImageToWindow(0) - PanX),
				(int)(ImageToWindow(0) - PanY),
				(int)(ImageToWindow(Image->GetInfo().Width) - PanX),
				(int)(ImageToWindow(Image->GetInfo().Height) - PanY));
			if (is_current || is_selected)
				SelectObject(location, MapNormalPen);
			break;
		}
#endif
		default:
			break;
	}
}
#endif
// actual draw of the element, different on MSWIN and MACOS
// if is_new, we need to draw as a rubber object.  This call
// will be sandwiched between PrepareDrawRubber() and
// CleanupDrawRubber() calls, and is_current and is_selected
// will both be false.  Otherwise, this call will be between
// PrepareDrawMap() and CleanupDrawMap() calls.  is_selected
// means the object is selected, and will be drawn with the
// selected rectangle afterwards, which must be set up here.
// If is_current, the mouse is over this object and the object
// should be "highlighted" somehow
#ifdef MACOS
void G42SingleImageViewer::DrawImageMapElement(G42DrawLocation location,
	const G42ImageMap::G42MapElement * element, bool is_new,
	bool is_current, bool is_selected)
{
	Rect rect;
	if (!element)
		return;
	switch (element->Type)
	{
		case G42ImageMap::Rectangle:
		{
			if (is_selected)
			{
				SelX = ImageToWindow(min(element->Coords[0].x,
					element->Coords[1].x)) - PanX;
				SelY = ImageToWindow(min(element->Coords[0].y,
					element->Coords[1].y)) - PanY;
				SelWidth = ImageToWindow(max(element->Coords[0].x,
					element->Coords[1].x)) - PanX - SelX + 1;
				SelHeight = ImageToWindow(max(element->Coords[0].y,
					element->Coords[1].y)) - PanY - SelY + 1;
				SelStretch = true;
				::PenPat (&AntsPattern2);	// mapselectedpen
			}
			else
				if (is_current)
					::PenPat (&AntsPattern2);	// mapselectedpen
			SetRectMac (&rect,
				(int)(ImageToWindow(element->Coords[0].x) - PanX),
				(int)(ImageToWindow(element->Coords[0].y) - PanY),
				(int)(ImageToWindow(element->Coords[1].x) - PanX),
				(int)(ImageToWindow(element->Coords[1].y) - PanY));
			FrameRect (&rect);
			if (is_selected || is_current)
				::PenPat (&AntsPattern);	// mapnormalpen
		}
		break;
		case G42ImageMap::Circle:
		{
			if (is_selected)
			{
				SelX = ImageToWindow(min(element->Coords[0].x,
					element->Coords[1].x)) - PanX;
				SelY = ImageToWindow(min(element->Coords[0].y,
					element->Coords[1].y)) - PanY;
				SelWidth = ImageToWindow(max(element->Coords[0].x,
					element->Coords[1].x)) - PanX - SelX + 1;
				SelHeight = ImageToWindow(max(element->Coords[0].y,
					element->Coords[1].y)) - PanY - SelY + 1;
				SelStretch = true;
				::PenPat (&AntsPattern2);	// mapselectedpen
			}
			else 
				if (is_current)
					::PenPat (&AntsPattern2);	// mapselectedpen
			SetRectMac (&rect, ImageToWindow (element->Coords[0].x) - PanX,
				ImageToWindow (element->Coords[0].y) - PanY,
				ImageToWindow (element->Coords[1].x) - PanX,
				ImageToWindow (element->Coords[1].y) - PanY);
			FrameOval (&rect);
			if (is_selected || is_current)
				::PenPat (&AntsPattern);	// mapnormalpen
		}
		break;
		case G42ImageMap::Polygon:
			DrawImageMapElementPolygon (location, element, is_new, is_current, is_selected);
		break;
		case G42ImageMap::Point:
		{
			if (is_selected)
			{
				SelX = ImageToWindow(element->Coords[0].x) - PanX - MapPointSize - 2;
				SelY = ImageToWindow(element->Coords[0].y) - PanY - MapPointSize - 2;
				SelWidth = (MapPointSize << 1) + 6;
				SelHeight = (MapPointSize << 1) + 6;
				SelStretch = false;
				::PenPat (&AntsPattern2);	// mapselectedpen
			}
			else 
				if (is_current)
					::PenPat (&AntsPattern2);	// mapselectedpen
			MoveTo (ImageToWindow(element->Coords[0].x + MapPointSize) - PanX + 1,
				ImageToWindow(element->Coords[0].y) - PanY + 1);
			LineTo (ImageToWindow(element->Coords[0].x - MapPointSize) - PanX + 1,
				ImageToWindow(element->Coords[0].y) - PanY + 1);
			MoveTo (ImageToWindow(element->Coords[0].x) - PanX + 1,
				ImageToWindow(element->Coords[0].y + MapPointSize) - PanY + 1);
			LineTo (ImageToWindow(element->Coords[0].x) - PanX + 1,
				ImageToWindow(element->Coords[0].y - MapPointSize) - PanY + 1);
			if (is_current || is_selected)
				::PenPat (&AntsPattern);	// mapnormalpen
		}
		break;
		default:
			break;
	}
}
#endif	// MACOS
/*  To fix the following error from CodeWarrior 10:  
Error   : ran out of registers--turn on Global Optimization for this function
g42svmap.cpp line 516  [But, it still needed the pragma after splitting out] */
#pragma global_optimizer on
#ifdef MACOS
void G42SingleImageViewer::DrawImageMapElementPolygon(G42DrawLocation location,
	const G42ImageMap::G42MapElement * element, bool is_new,
	bool is_current, bool is_selected)
{
	if (is_selected)
	{
		SelX = element->Coords[0].x;
		SelY = element->Coords[0].y;
		SelWidth = SelX;
		SelHeight = SelY;
		SelStretch = false;
		for (uint i = 1; i < element->NumCoords; i++)
		{
			if (element->Coords[i].x < SelX)
				SelX = element->Coords[i].x;
			if (element->Coords[i].x > SelWidth)
				SelWidth = element->Coords[i].x;
			if (element->Coords[i].y < SelY)
				SelY = element->Coords[i].y;
			if (element->Coords[i].y > SelHeight)
				SelHeight = element->Coords[i].y;
		}
		SelX = ImageToWindow(SelX) - PanX;
		SelY = ImageToWindow(SelY) - PanY;
		SelX -= 2;
		SelY -= 3;
		SelWidth = ImageToWindow(SelWidth) - PanX;
		SelHeight = ImageToWindow(SelHeight) - PanY;
		SelWidth -= SelX - 1;
		SelHeight -= SelY - 1;
		SelWidth += 4;
		SelHeight += 4;
		::PenPat (&AntsPattern2);	// mapselectedpen
	}
	else 
		if (is_current)
			::PenPat (&AntsPattern2);	// mapselectedpen
	uint num_point_array = element->NumCoords;
	if (is_new && NewPolygonStarted)
		num_point_array++;
	PolyHandle polyHandle;
	if (! is_new || NewPolygonClosed)	// Want to close polygon
	{
		polyHandle = OpenPoly ();
		/*  Move to the 1st point, lineto the rest of them, then lineto back to the 1st */
		MoveTo (ImageToWindow(element->Coords[0].x) - PanX,
			ImageToWindow(element->Coords[0].y) - PanY);
		for (uint i = 1; i < num_point_array; i++)
			LineTo (ImageToWindow(element->Coords[i].x) - PanX,
				ImageToWindow(element->Coords[i].y) - PanY);
		LineTo (ImageToWindow(element->Coords[0].x) - PanX,
			ImageToWindow(element->Coords[0].y) - PanY);
		ClosePoly();
		FramePoly (polyHandle);
	}
	else	// Only want to draw lines
	{
		MoveTo (ImageToWindow(element->Coords[0].x) - PanX,
			ImageToWindow(element->Coords[0].y) - PanY);
		for (uint i = 1; i < num_point_array; i++)
			LineTo (ImageToWindow(element->Coords[i].x) - PanX,
				ImageToWindow(element->Coords[i].y) - PanY);			
	}			
	
	if (is_selected)
	{
		int box_side = ((SelectMargin + 1) >> 1);
		for (uint i = 0; i < num_point_array; i++)
		{
			Rect rect;
			::SetRect (&rect, ImageToWindow(element->Coords[i].x) - PanX - box_side,
				ImageToWindow(element->Coords[i].y) - PanY - box_side,
				ImageToWindow(element->Coords[i].x) - PanX + box_side,			
				ImageToWindow(element->Coords[i].y) - PanY + box_side);
			::OffsetRect (&rect, 1, 0);
			::InvertRect (&rect);
		}
	}
	
	if (is_selected || is_current)
		::PenPat (&AntsPattern);	// mapnormalpen
}
#endif
#pragma global_optimizer off
// now used only when selected area is destroyed
void G42SingleImageViewer::EraseSelectedArea(G42DrawLocation location)
{
	if (AreaSelected)
	{
//		DrawSelectedRectangle(location);
//		AreaSelected = false; // turn off selected area to draw area underneath
		Draw(location, SelX - SelectMargin, SelY - SelectMargin,
			SelWidth + (SelectMargin << 1), SelHeight + (SelectMargin << 1));
//		AreaSelected = true;
	}
}
void G42SingleImageViewer::DrawSelectedArea(G42DrawLocation location)
{
	PrepareDrawArea(location);
	DrawArea(location);
	CleanupDrawArea(location);
}
void G42SingleImageViewer::DrawArea(G42DrawLocation location)
{
	if (AreaSelected)
	{
		SelX = (int)(ImageToWindow(SelectedAreaX) - PanX);
		SelY = (int)(ImageToWindow(SelectedAreaY) - PanY);
		SelWidth = (int)(ImageToWindow(SelectedAreaX + SelectedAreaWidth) -
			PanX) - SelX + 1;
		SelHeight = (int)(ImageToWindow(SelectedAreaY + SelectedAreaHeight) -
			PanY) - SelY + 1;
		SelStretch = true;
		DrawSelectedRectangle(location);
		DrawSelectedHandles (location);
		if (SelectedImage) // this better not be null
		{
			int32 pan_x = PanX - ImageToWindow(SelectedAreaX);
			int32 pan_y = PanY - ImageToWindow(SelectedAreaY);
			#ifdef MACOS
			/*  For drawing the selected area, we want to ensure calling InitPixmap() in iview.cpp  */
			ChangedForDraw = true;
			#endif
			DrawImage(location, SelectedImage, SelectedDitherImage,
         	SelectedStretchImage, SelectedDitherBaseImage, pan_x, pan_y,
            0, 0, WindowWidth, WindowHeight);
		}
	}
}
void G42SingleImageViewer::PrepareDrawArea(G42DrawLocation location)
{
#ifdef MSWIN
	SelectObject(location, GetStockObject(NULL_BRUSH));
	AreaPen = CreatePen(PS_DOT, 1, RGB(0xff, 0xff, 0xff));
	SetBkMode(location, OPAQUE);
	SetBkColor(location, RGB(0, 0, 0));
	OldAreaPen = (HPEN)SelectObject(location, AreaPen);
	SetROP2(location, R2_COPYPEN);
#endif
#ifdef MACOS
	/*  Use marching ants pattern  */
	if (PrepareDrawImgCnt == 0)
	{
		::GetGWorld ((GWorldPtr *) &OldPrepDrawImgPort, &OldPrepDrawImgDevice);
		::SetGWorld ((GWorldPtr) location, nil);
		::LockPixels (::GetGWorldPixMap ((GWorldPtr) location));	
		::PenNormal();
		::PenPat (&AntsPattern);
		PrepareDrawImgCnt++;
	}
#endif
}
void G42SingleImageViewer::CleanupDrawArea(G42DrawLocation location)
{
#ifdef MSWIN
	SelectObject(location, OldAreaPen);
	DeleteObject(AreaPen);
#endif
#ifdef MACOS
	if (PrepareDrawImgCnt > 0)
	{
		::UnlockPixels (::GetGWorldPixMap ((GWorldPtr) location));
		::SetGWorld ((GWorldPtr) OldPrepDrawImgPort, OldPrepDrawImgDevice);
		::PenNormal();
		PrepareDrawImgCnt--;
	}
#endif
}
void
G42SingleImageViewer::DrawSelectedHandles (G42DrawLocation location)
{
	#ifdef MSWIN
	if (SelStretch)
	{
		HPEN old_pen = (HPEN)SelectObject(location, GetStockObject(WHITE_PEN));
		HBRUSH old_brush =
			(HBRUSH)SelectObject(location, GetStockObject(BLACK_BRUSH));
		Rectangle(location, SelX - SelectMargin, SelY - SelectMargin,
			SelX, SelY);
		Rectangle(location, SelX + SelWidth, SelY - SelectMargin,
			SelX + SelWidth + SelectMargin, SelY);
		Rectangle(location, SelX - SelectMargin, SelY + SelHeight,
			SelX, SelY + SelHeight + SelectMargin);
		Rectangle(location, SelX + SelWidth, SelY + SelHeight,
			SelX + SelWidth + SelectMargin, SelY + SelHeight + SelectMargin);
		SelectObject(location, old_pen);
		SelectObject(location, old_brush);
	}
	#endif
	#ifdef MACOS // Future TODO:  Remove OffsetRect's and put into calculations instead
	if (SelStretch)
	{
		Rect rect;
		SetRectMac (&rect, SelX - SelectMargin, SelY - SelectMargin, SelX, SelY);
		::OffsetRect (&rect, 2, 2);
		::InvertRect (&rect);
		SetRectMac (&rect, SelX + SelWidth - 1, SelY - SelectMargin,
			SelX + SelWidth + SelectMargin - 1, SelY);
		::OffsetRect (&rect, -2, 2);
		::InvertRect (&rect);
		SetRectMac (&rect, SelX - SelectMargin, SelY + SelHeight - 1,
			SelX, SelY + SelHeight + SelectMargin - 1);
		::OffsetRect (&rect, 2, -2);
		::InvertRect (&rect);
		SetRectMac (&rect, SelX + SelWidth - 1, SelY + SelHeight - 1,
			SelX + SelWidth + SelectMargin - 1, SelY + SelHeight + SelectMargin - 1);
		::OffsetRect (&rect, -2, -2);
		::InvertRect (&rect);
	}
	#endif
}
void G42SingleImageViewer::DrawSelectedRectangle(G42DrawLocation location)
{
	#ifdef MSWIN
	Rectangle(location, SelX - 1, SelY - 1,
		SelX + SelWidth, SelY + SelHeight);
	#endif
	#ifdef MACOS
	Rect rect;
	SetRectMac (&rect, SelX - 1, SelY - 1, SelX + SelWidth, SelY + SelHeight);
	#if 0
	InsetRect (&rect, -2, -2);
	#endif
//	if (AreaSelected && !StretchingArea)
		FrameRect (&rect);	// Future TODO: Use dotted lines
//	else	
//		InvertRect (&rect);
	#endif	// MACOS
}
void G42SingleImageViewer::SetImageMap(G42ImageMap * image_map)
{
	ImageMap = image_map;
	if (ImageMap && Image && Image->HasInfo())
	{
		ImageMap->SetImageSize(
			(int)(Image->GetInfo().Width), (int)(Image->GetInfo().Height));
	}
	#ifndef MACOS
	Draw();
	#endif
}
void
G42SingleImageViewer::MakeProportional(void)
{
	if (NewMapElementStarted)
	{
		int width = abs(CurrentX - StartX);
		int height = abs(CurrentY - StartY);
		switch (MapTool)
		{
			case MapCircleTool:
			case MapPolygonTool:
				if (width > height)
					CurrentY = StartY;
				else
					CurrentX = StartX;
				break;
			case MapRectangleTool:
				if (width > height)
				{
					if (CurrentY > StartY)
						CurrentY = StartY + width;
					else
						CurrentY = StartY - width;
				}
				else
				{
					if (CurrentX > StartX)
						CurrentX = StartX + height;
					else
						CurrentX = StartX - height;
				}
				break;
			default:
				break;
		}
	}
	else if (StretchingElement || (AreaSelected && StretchingArea))
	{
		int old_width = StartX - OtherX;
		int old_height = StartY - OtherY;
		int new_width = CurrentX - OtherX;
		int new_height = CurrentY - OtherY;
		if ((int32)abs(old_width) * (int32)abs(new_height) >
			(int32)abs(new_width) * (int32)abs(old_height))
		{
			CurrentX = (int)(((int32)new_height * (int32)old_width) /
				(int32)old_height) + OtherX;
		}
		else
		{
			CurrentY = (int)(((int32)new_width * (int32)old_height) /
				(int32)old_width) + OtherY;
		}
	}
}
void
G42SingleImageViewer::SetMouseString(int32 x, int32 y)
{
	if (!Image || !Image->HasInfo())
		return;
	char buf[10];
	if (x < 0)
		x = 0;
	if (x >= (int)Image->GetInfo().Width)
		x = (int)Image->GetInfo().Width - 1;
	if (y < 0)
		y = 0;
	if (y >= (int)Image->GetInfo().Height)
		y = (int)Image->GetInfo().Height - 1;
	if (x != MouseX || y != MouseY)
	{
		sprintf(buf, "%d, %d", x, y);
		SetStatusString(buf);
		MouseX = x;
		MouseY = y;
	}
}
void
G42SingleImageViewer::PrepareDrawRubber(G42DrawLocation location)
{
	#ifdef MSWIN
	SetROP2(location, R2_NOT);
	SelectObject(location, GetStockObject(NULL_BRUSH));
	SelectObject(location, GetStockObject(BLACK_PEN));
	#endif
	#ifdef MACOS	
	/*  Set up "invert" for drawing rubber bands  */
	if (PrepareDrawRubberCnt == 0)	// Somehow prepares aren't balanced with cleanups 
	{
		::GetGWorld ((GWorldPtr *) &OldPrepDrawRubberPort, &OldPrepDrawRubberDevice);
		::SetGWorld ((GWorldPtr) location, nil);
		#ifdef SETORIGIN
		::SetOrigin (XOrigin, YOrigin);
		#endif
		::LockPixels (::GetGWorldPixMap ((GWorldPtr) location));
		PenState penState;
		::GetPenState (&penState);
		penState.pnMode = srcXor;
		::SetPenState (&penState);			
		PrepareDrawRubberCnt++;
	}
	#endif
}
void
G42SingleImageViewer::CleanupDrawRubber(G42DrawLocation location)
{
	#ifdef MACOS
	if (PrepareDrawRubberCnt > 0) // Somehow prepares aren't balanced with cleanups 
	{
		::UnlockPixels (::GetGWorldPixMap ((GWorldPtr) location));
		::SetGWorld ((GWorldPtr) OldPrepDrawRubberPort, OldPrepDrawRubberDevice);
		::PenNormal();
		PrepareDrawRubberCnt--;
	}
	#endif
}
void
G42SingleImageViewer::MouseMove(int x, int y,
	bool shift_down, bool control_down)
{
	#ifdef MSWIN
	if (SettingCursor)
	{
		return;
	}
	if (control_down || OldControlDown)
	{
		if (control_down)
		{
			if (OldControlDown)
			{
				int new_x = x;
				int new_y = y;
				CursorXFraction += x - OldCursorX;
				if (CursorXFraction > 0)
				{
					x = OldCursorX + (CursorXFraction >> 2);
					CursorXFraction &= 3;
				}
				else if (CursorXFraction < 0)
				{
					x = OldCursorX - ((3 - CursorXFraction) >> 2);
					CursorXFraction = 3 - ((-CursorXFraction + 3) & 3);
				}
				CursorYFraction += y - OldCursorY;
				if (CursorYFraction > 0)
				{
					y = OldCursorY + (CursorYFraction >> 2);
					CursorYFraction &= 3;
				}
				else if (CursorYFraction < 0)
				{
					y = OldCursorY - ((3 - CursorYFraction) >> 2);
					CursorYFraction = 3 - ((-CursorYFraction + 3) & 3);
				}
				if (x != new_x || y != new_y)
				{
					POINT p;
					p.x = x;
					p.y = y;
					ClientToScreen(Window, &p);
//					GetCursorPos(&p);
//					SettingCursor++;
//					SetCursorPos(p.x + x - new_x, p.y + y - new_y);
//					SettingCursor--;
					SetCursorPos(p.x, p.y);
				}
				OldCursorX = x;
				OldCursorY = y;
			}
			else
			{
				OldControlDown = true;
				OldCursorX = x;
				OldCursorY = y;
				CursorXFraction = 0;
            CursorYFraction = 0;
			}
		}
		else
		{
			OldControlDown = false;
		}
	}
	#endif
	if (TransparentMode)
	{
		CurrentX = (int)WindowToImage(x + PanX);
		CurrentY = (int)WindowToImage(y + PanY);
		SetMouseString(CurrentX, CurrentY);
		if (ReportingTransColors)
		{
			if (CurrentX >= 0 && CurrentX < Image->GetInfo().Width &&
				CurrentY >= 0 && CurrentY < Image->GetInfo().Height)
					TransparentPointClick(CurrentX, CurrentY, false);
		}
		return;
	}
	if (SelectingArea)
	{
		PrepareDrawRubber(DC);
		DrawRubberImageRect(StartX, StartY, CurrentX, CurrentY);
		CurrentX = WindowToImage(x + PanX);
		CurrentY = WindowToImage(y + PanY);
		DrawRubberImageRect(StartX, StartY, CurrentX, CurrentY);
		SetMouseString(CurrentX, CurrentY);
		CleanupDrawRubber(DC);
	}
	else if (AreaSelected)
	{
		if (MovingArea)
		{
			int32 old_x = CurrentX;
			int32 old_y = CurrentY;
			CurrentX = (int32)WindowToImage(x + PanX);
			CurrentY = (int32)WindowToImage(y + PanY);
			SetMouseString(CurrentX, CurrentY);
			SelectedAreaX += CurrentX - old_x;
			SelectedAreaY += CurrentY - old_y;
			int dx = abs((int)(x + PanX - ImageToWindow(old_x))) + 1;
			int dy = abs((int)(y + PanY - ImageToWindow(old_y))) + 1;
			Draw(SelX - SelectMargin - dx, SelY - SelectMargin - dy,
				SelWidth + ((SelectMargin + dx) << 1),
				SelHeight + ((SelectMargin + dy) << 1));
		}
		else if (StretchingArea)
		{
			PrepareDrawRubber(DC);
//			DrawRubberRect(SelX, SelY, SelX + SelWidth, SelY + SelHeight);
			DrawRubberImageRect(OtherX, OtherY, CurrentX, CurrentY);
			CurrentX = (int32)WindowToImage(x + PanX);
			CurrentY = (int32)WindowToImage(y + PanY);
			if (shift_down != ShiftInverted)
				MakeProportional();
			SetMouseString(CurrentX, CurrentY);
			AbsStretchingRubberArea(shift_down);
//			StretchSelectedRubberArea(shift_down);
			DrawRubberImageRect(OtherX, OtherY, CurrentX, CurrentY);
//			DrawRubberRect(SelX, SelY, SelX + SelWidth, SelY + SelHeight);
			CleanupDrawRubber(DC);
		}
		else
		{
			CurrentX = (int32)WindowToImage(x + PanX);
			CurrentY = (int32)WindowToImage(y + PanY);
			SetMouseString(CurrentX, CurrentY);
			if (x >= SelX - SelectMargin &&
				x < SelX + SelWidth + SelectMargin &&
				y >= SelY - SelectMargin &&
				y < SelY + SelHeight + SelectMargin)
			{
				if (x >= SelX && x < SelX + SelWidth &&
					y >= SelY && y < SelY + SelHeight)
					SetCursor(MoveCursor);
				else if (x < SelX && y < SelY)
					SetCursor(StretchTopLeftCursor);
				else if (x >= SelX + SelWidth && y < SelY)
					SetCursor(StretchTopRightCursor);
				else if (x < SelX && y >= SelY + SelHeight)
					SetCursor(StretchTopRightCursor);
				else if (x >= SelX + SelWidth && y >= SelY + SelHeight)
					SetCursor(StretchTopLeftCursor);
				else if (x >= SelX + SelWidth && y >= SelY + SelHeight)
					SetCursor(StretchTopLeftCursor);
				else if (x < SelX || x >= SelX + SelWidth)
					SetCursor(StretchHorizontalCursor);
				else
					SetCursor(StretchVerticalCursor);
			}
			else
			{
				SetCursor(ArrowCursor);
			}
		}
	}
	else if (NewMapElementStarted)
	{
		switch (MapTool)
		{
			case MapRectangleTool:
			{
				CurrentX = (int32)(WindowToImage(x + PanX));
				CurrentY = (int32)(WindowToImage(y + PanY));
				if (shift_down != ShiftInverted)
					MakeProportional();
				SetMouseString(CurrentX, CurrentY);
				PrepareDrawRubber(DC);
				DrawImageMapElement(DC, &NewElement, true);
				NewElement.Coords[1].x = CurrentX;
				NewElement.Coords[1].y = CurrentY;
				DrawImageMapElement(DC, &NewElement, true);
				CleanupDrawRubber(DC);
				break;
			}
			case MapPointTool:
			{
				CurrentX = (int32)(WindowToImage(x + PanX));
				CurrentY = (int32)(WindowToImage(y + PanY));
				if (shift_down != ShiftInverted)
					MakeProportional();
				SetMouseString(CurrentX, CurrentY);
				PrepareDrawRubber(DC);
				DrawImageMapElement(DC, &NewElement, true);
				NewElement.Coords[0].x = CurrentX;
				NewElement.Coords[0].y = CurrentY;
				DrawImageMapElement(DC, &NewElement, true);
				CleanupDrawRubber(DC);
				break;
			}
			case MapPolygonTool:
			{
				if (NewPolygonStarted)
				{
					CurrentX = (int32)(WindowToImage(x + PanX));
					CurrentY = (int32)(WindowToImage(y + PanY));
					if (shift_down != ShiftInverted)
						MakeProportional();
					SetMouseString(CurrentX, CurrentY);
					PrepareDrawRubber(DC);
					DrawImageMapElement(DC, &NewElement, true);
					NewElement.Coords[NewElement.NumCoords].x = CurrentX;
					NewElement.Coords[NewElement.NumCoords].y = CurrentY;
					DrawImageMapElement(DC, &NewElement, true);
					CleanupDrawRubber(DC);
				}
				else
				{
					SetMouseString((int)(WindowToImage(x + PanX)),
						(int)(WindowToImage(y + PanY)));
				}
				break;
			}
			case MapCircleTool:
			{
				CurrentX = x;
				CurrentY = y;
				if (shift_down != ShiftInverted)
					MakeProportional();
				SetMouseString(CurrentX, CurrentY);
				PrepareDrawRubber(DC);
				DrawImageMapElement(DC, &NewElement, true);
				int x1 = (int)WindowToImage(StartX + PanX);
				int y1 = (int)WindowToImage(StartY + PanY);
				int x2 = (int)WindowToImage(CurrentX + PanX);
				int y2 = (int)WindowToImage(CurrentY + PanY);
				GetCircleRect(x1, y1, x2, y2);
				NewElement.Coords[0].x = x1;
				NewElement.Coords[0].y = y1;
				NewElement.Coords[1].x = x2;
				NewElement.Coords[1].y = y2;
				DrawImageMapElement(DC, &NewElement, true);
				CleanupDrawRubber(DC);
				break;
			}
			default:
				SetMouseString((int)WindowToImage(x + PanX),
					(int)WindowToImage(y + PanY));
				break;
		}
	}
	else if (MovingElement)
	{
		int x1 = (int)WindowToImage(x + PanX);
		int y1 = (int)WindowToImage(y + PanY);
		SetMouseString(x1, y1);
		if (MoveFromSelect)
		{
			if (abs(x1 - StartX) > SelectMargin ||
				abs(y1 - StartY) > SelectMargin)
			{
				MoveFromSelect = false;
				PrepareDrawRubber(DC);
				MoveNewElement(x1 - StartX, y1 - StartY);
				CurrentX = x1;
				CurrentY = y1;
				DrawImageMapElement(DC, &NewElement, true);
				CleanupDrawRubber(DC);
			}
		}
		if (!MoveFromSelect)
		{
			PrepareDrawRubber(DC);
			DrawImageMapElement(DC, &NewElement, true);
			MoveNewElement(x1 - CurrentX, y1 - CurrentY);
			DrawImageMapElement(DC, &NewElement, true);
			CleanupDrawRubber(DC);
			CurrentX = x1;
			CurrentY = y1;
		}
	}
	else if (StretchingElement)
	{
		PrepareDrawRubber(DC);
		DrawImageMapElement(DC, &NewElement, true);
		CurrentX = (int)WindowToImage(x + PanX);
		CurrentY = (int)WindowToImage(y + PanY);
		if ((StretchMode & StretchProportional) ||
				(shift_down != ShiftInverted))
			MakeProportional();
		SetMouseString(CurrentX, CurrentY);
		StretchNewElement(shift_down);
		DrawImageMapElement(DC, &NewElement, true);
		CleanupDrawRubber(DC);
	}
	else if (EditingPolygon)
	{
		PrepareDrawRubber(DC);
		DrawImageMapElement(DC, &NewElement, true);
		CurrentX = (int)WindowToImage(x + PanX);
		CurrentY = (int)WindowToImage(y + PanY);
		SetMouseString(CurrentX, CurrentY);
		NewElement.Coords[EditingPolygonIndex].x = CurrentX;
		NewElement.Coords[EditingPolygonIndex].y = CurrentY;
		DrawImageMapElement(DC, &NewElement, true);
		CleanupDrawRubber(DC);
	}
	else if (ImageMapVisible && ImageMap && MapTool == MapEditTool)
	{
		bool in_selected = false;
		int new_element;
		if (ImageMap->HasSelectedElement() && ImageMap->GetElement(
			ImageMap->GetSelectedElement())->Type != G42ImageMap::Default)
		{
			if (SelStretch)
			{
				if (x >= SelX - SelectMargin &&
					x < SelX + SelWidth + SelectMargin &&
					y >= SelY - SelectMargin &&
					y < SelY + SelHeight + SelectMargin)
				{
					if (x >= SelX && x < SelX + SelWidth &&
						y >= SelY && y < SelY + SelHeight)
						SetCursor(MoveCursor);
					else if (x < SelX && y < SelY)
						SetCursor(StretchTopLeftCursor);
					else if (x >= SelX + SelWidth && y < SelY)
						SetCursor(StretchTopRightCursor);
					else if (x < SelX && y >= SelY + SelHeight)
						SetCursor(StretchTopRightCursor);
					else if (x >= SelX + SelWidth && y >= SelY + SelHeight)
						SetCursor(StretchTopLeftCursor);
					else if (x >= SelX + SelWidth && y >= SelY + SelHeight)
						SetCursor(StretchTopLeftCursor);
					else if (x < SelX || x >= SelX + SelWidth)
						SetCursor(StretchHorizontalCursor);
					else
						SetCursor(StretchVerticalCursor);
					in_selected = true;
					new_element = ImageMap->GetSelectedElement();
				}
			}
			else
			{
				if (x >= SelX && x < SelX + SelWidth &&
					y >= SelY && y < SelY + SelHeight)
				{
					in_selected = true;
					new_element = ImageMap->GetSelectedElement();
					const G42ImageMap::G42MapElement * element =
						ImageMap->GetElement(new_element);
					if (element && element->Type == G42ImageMap::Polygon)
					{
						int box_size = ((SelectMargin + 1) >> 1);
						bool found_point = false;
						for (uint i = 0; i < element->NumCoords; i++)
						{
							int px = (int)(ImageToWindow(
								element->Coords[i].x) - PanX);
							int py = (int)(ImageToWindow(
								element->Coords[i].y) - PanY);
							if (abs(px - x) <= box_size &&
								abs(py - y) <= box_size)
							{
								found_point = true;
								break;
							}
						}
						if (found_point)
							SetCursor(ArrowCursor);
						else
							SetCursor(MoveCursor);
					}
					else if (element && element->Type != G42ImageMap::Default)
					{
						SetCursor(MoveCursor);
					}
				}
			}
		}
		if (!in_selected)
		{
			SetCursor(ArrowCursor);
			int x1 = (int)WindowToImage(x + PanX);
			int y1 = (int)WindowToImage(y + PanY);
			new_element = ImageMap->GetIndexAtPoint(x1, y1);
			SetMouseString(x1, y1);
		}
		if (new_element != CurrentElement)
		{
#if 0
			int old_element = CurrentElement;
			StartRubber();
			PrepareDrawMap(DC);
			if (old_element >= 0)
				DrawImageMapElement(DC, old_element);
			if (new_element >= 0)
				DrawImageMapElement(DC, new_element);
			CurrentElement = new_element;
			if (old_element >= 0)
				DrawImageMapElement(DC, old_element);
			if (new_element >= 0)
				DrawImageMapElement(DC, new_element);
			CleanupDrawMap(DC);
			EndRubber();
#endif
			CurrentElement = new_element;
			Draw();
			if (CurrentElement >= 0)
				SetURLString(ImageMap->GetElement(CurrentElement)->URL);
			else
				SetURLString("");
		}
	}
	else
	{
		SetMouseString((int)WindowToImage(x + PanX),
			(int)WindowToImage(y + PanY));
	}
}
bool
G42SingleImageViewer::MouseDown(int x, int y, int button,
	bool shift_down, bool control_down, uint count)
{
	#ifdef MSWIN
	if (control_down || OldControlDown)
	{
		if (control_down)
		{
			if (OldControlDown)
			{
				int new_x = x;
				int new_y = y;
				CursorXFraction += x - OldCursorX;
				if (CursorXFraction > 0)
				{
					x = OldCursorX + (CursorXFraction >> 2);
					CursorXFraction &= 3;
				}
				else if (CursorXFraction < 0)
				{
					x = OldCursorX - ((3 - CursorXFraction) >> 2);
					CursorXFraction = 3 - ((-CursorXFraction + 3) & 3);
				}
				CursorYFraction += y - OldCursorY;
				if (CursorYFraction > 0)
				{
					y = OldCursorY + (CursorYFraction >> 2);
					CursorYFraction &= 3;
				}
				else if (CursorYFraction < 0)
				{
					y = OldCursorY - ((3 - CursorYFraction) >> 2);
					CursorYFraction = 3 - ((-CursorYFraction + 3) & 3);
				}
				if (x != new_x || y != new_y)
				{
					POINT p;
					p.x = x;
					p.y = y;
					ClientToScreen(Window, &p);
//					GetCursorPos(&p);
//					SettingCursor++;
//					SetCursorPos(p.x + x - new_x, p.y + y - new_y);
//					SettingCursor--;
					SetCursorPos(p.x, p.y);
				}
				OldCursorX = x;
				OldCursorY = y;
			}
			else
			{
				OldControlDown = true;
				OldCursorX = x;
				OldCursorY = y;
				CursorXFraction = 0;
            CursorYFraction = 0;
			}
		}
		else
		{
			OldControlDown = false;
		}
	}
	#endif
	if (TextVisible)
	{
		if (button == 1 || button == 2)
			SetTextStartPos((int)WindowToImage(x + PanX),
				(int)WindowToImage(y + PanY));
		else if (button == 3)
      	AcceptText();
		return true;
	}
	if (button == 2)
		ShiftInverted = true;
	// a double click on a polygon closes the polygon
	if (ImageMapVisible && NewMapElementStarted &&
		!NewPolygonStarted && count > 0)
	{
		button = 3;
	}
	if (button == 1 || button == 2)
	{
		if (TransparentMode)
		{
			CurrentX = (int)WindowToImage(x + PanX);
			CurrentY = (int)WindowToImage(y + PanY);
			SetMouseString(CurrentX, CurrentY);
			ReportingTransColors = true;
			if (CurrentX >= 0 && CurrentX < Image->GetInfo().Width &&
				CurrentY >= 0 && CurrentY < Image->GetInfo().Height)
					TransparentPointClick(CurrentX, CurrentY, false);
			return true;
		}
		
		if (ImageMapVisible)
		{
			switch (MapTool)
			{
				case MapRectangleTool:
				{
					if (!NewMapElementStarted)
					{
						NewMapElementStarted = true;
						StartRubber();
						StartX = (int)WindowToImage(x + PanX);
						StartY = (int)WindowToImage(y + PanY);
						SetMouseString(StartX, StartY);
						NewElement.Type = G42ImageMap::Rectangle;
						NewElement.NumCoords = 2;
						NewElement.Coords[0].x = StartX;
						NewElement.Coords[0].y = StartY;
						NewElement.Coords[1] = NewElement.Coords[0];
						PrepareDrawRubber(DC);
						DrawImageMapElement(DC, &NewElement, true);
						CleanupDrawRubber(DC);
						#ifdef MACOS	// (m.1)
						Rect rect;
						SetRectMac (&rect, NewElement.Coords[0].x, NewElement.Coords[0].y,
							NewElement.Coords[1].x, NewElement.Coords[1].y);
						#endif
					}
					else
					{
						SetMouseString((int)WindowToImage(x + PanX),
							(int)WindowToImage(y + PanY));
					}
					break;
				}
				case MapPointTool:
				{
					if (!NewMapElementStarted)
					{
						NewMapElementStarted = true;
						StartRubber();
						NewElement.Type = G42ImageMap::Point;
						NewElement.NumCoords = 1;
						NewElement.Coords[0].x = (int)WindowToImage(x + PanX);
						NewElement.Coords[0].y = (int)WindowToImage(y + PanY);
						SetMouseString(NewElement.Coords[0].x, NewElement.Coords[0].y);
						PrepareDrawRubber(DC);
						DrawImageMapElement(DC, &NewElement, true);
						CleanupDrawRubber(DC);
					}
					else
					{
						SetMouseString((int)WindowToImage(x + PanX),
							(int)WindowToImage(y + PanY));
					}
					break;
				}
				case MapPolygonTool:
				{
					if (!NewPolygonStarted || !NewMapElementStarted)
					{
						StartX = (int)WindowToImage(x + PanX);
						StartY = (int)WindowToImage(y + PanY);
						SetMouseString(StartX, StartY);
						if (!NewMapElementStarted)
						{
							NewMapElementStarted = true;
							NewPolygonClosed = false;
							StartRubber();
							PrepareDrawRubber(DC);
							NewElement.Type = G42ImageMap::Polygon;
							NewElement.NumCoords = 1;
							NewElement.Coords[0].x = StartX;
							NewElement.Coords[0].y = StartY;
							NewElement.Coords[1] = NewElement.Coords[0];
						}
						else
						{
							PrepareDrawRubber(DC);
							DrawImageMapElement(DC, &NewElement, true);
							NewElement.Coords[NewElement.NumCoords].x =
								StartX;
							NewElement.Coords[NewElement.NumCoords].y =
								StartY;
							StartX = NewElement.Coords[NewElement.NumCoords - 1].x;
							StartY = NewElement.Coords[NewElement.NumCoords - 1].y;
						}
						NewPolygonStarted = true;
						DrawImageMapElement(DC, &NewElement, true);
						CleanupDrawRubber(DC);
					}
					else
					{
						SetMouseString((int)WindowToImage(x + PanX),
							(int)WindowToImage(y + PanY));
					}
					break;
				}
				case MapCircleTool:
				{
					if (!NewMapElementStarted)
					{
						NewMapElementStarted = true;
						StartRubber();
						NewElement.Type = G42ImageMap::Circle;
						NewElement.NumCoords = 2;
						StartX = x;
						StartY = y;
						int x1 = (int)WindowToImage(StartX + PanX);
						int y1 = (int)WindowToImage(StartY + PanY);
						int x2 = (int)WindowToImage(x + PanX);
						int y2 = (int)WindowToImage(y + PanY);
						SetMouseString(x2, y2);
						GetCircleRect(x1, y1, x2, y2);
						NewElement.Coords[0].x = x1;
						NewElement.Coords[0].y = y1;
						NewElement.Coords[1].x = x2;
						NewElement.Coords[1].y = y2;
						PrepareDrawRubber(DC);
						DrawImageMapElement(DC, &NewElement, true);
						CleanupDrawRubber(DC);
					}
					else
					{
						SetMouseString((int)WindowToImage(x + PanX),
							(int)WindowToImage(y + PanY));
					}
					break;
				}
				case MapEditTool:
				{
					if (ImageMap->HasSelectedElement() && ImageMap->GetElement(
						ImageMap->GetSelectedElement())->Type != G42ImageMap::Default)
					{
						#if 0
						if (x >= SelX - SelectMargin &&
							x < SelX + SelWidth + SelectMargin &&
							y >= SelY - SelectMargin &&
							y < SelY + SelHeight + SelectMargin)
						#endif
						if (x >= SelX - GrabMargin &&
							x < SelX + SelWidth + GrabMargin &&
							y >= SelY - GrabMargin &&
							y < SelY + SelHeight + GrabMargin)
						{
							if (x >= SelX + GrabInternalMargin && 
								x < SelX + SelWidth - GrabInternalMargin &&
								y >= SelY + GrabInternalMargin  && 
								y < SelY + SelHeight - GrabInternalMargin)
							{
								if (count)
								{
									EditMapElement(ImageMap->GetSelectedElement());
									SetMouseString((int)WindowToImage(x + PanX),
										(int)WindowToImage(y + PanY));
								}
								else if (ImageMap->GetElement(
									ImageMap->GetSelectedElement())->Type !=
									G42ImageMap::Default)
								{
									if (ImageMap->GetElement(
										ImageMap->GetSelectedElement())->Type ==
										G42ImageMap::Polygon)
									{
										const G42ImageMap::G42MapElement * element =
											ImageMap->GetElement(
												ImageMap->GetSelectedElement());
										int box_size = ((SelectMargin + 1) >> 1);
										for (uint i = 0; i < element->NumCoords; i++)
										{
											int px = (int)(ImageToWindow(
												element->Coords[i].x) - PanX);
											int py = (int)(ImageToWindow(
												element->Coords[i].y) - PanY);
											if (abs(px - x) <= box_size &&
												abs(py - y) <= box_size)
											{
												EditingPolygon = true;
												EditingPolygonIndex = i;
												StartStretchElement();
												NewPolygonClosed = true;
												StartRubber();
												StartX = NewElement.Coords[i].x;
												StartY = NewElement.Coords[i].y;
												CurrentX = (int)WindowToImage(x + PanX);
												CurrentY = (int)WindowToImage(y + PanY);
												SetMouseString(CurrentX, CurrentY);
												NewElement.Coords[i].x = CurrentX;
												NewElement.Coords[i].y = CurrentY;
												PrepareDrawRubber(DC);
												DrawImageMapElement(DC, &NewElement, true);
												CleanupDrawRubber(DC);
												break;
											}
										}
										if (EditingPolygon)
											break;
									}
									MovingElement = true;
									MoveFromSelect = false;
									StartStretchElement();
									StartX = (int)WindowToImage(x + PanX);
									StartY = (int)WindowToImage(y + PanY);
									SetMouseString(StartX, StartY);
									CurrentX = StartX;
									CurrentY = StartY;
									NewPolygonClosed = true;
									StartRubber();
									PrepareDrawRubber(DC);
									DrawImageMapElement(DC, &NewElement, true);
									CleanupDrawRubber(DC);
								}
								break;
							}
							const G42ImageMap::G42MapElement * element =
								ImageMap->GetElement(ImageMap->GetSelectedElement());
							if (element->Type != G42ImageMap::Default &&
								element->Type != G42ImageMap::Point &&
								element->Type != G42ImageMap::Polygon)
							{
								StartStretchElement();
								StartRubber();
								StretchingElement = true;
								NewPolygonClosed = true;
								StretchMode = 0;
								if (element->Type == G42ImageMap::Circle)
									StretchMode |= StretchProportional;
								StretchMinX = element->Coords[0].x;
								StretchMinY = element->Coords[0].y;
								StretchMaxX = StretchMinX;
								StretchMaxY = StretchMinY;
								for (uint i = 1; i < element->NumCoords; i++)
								{
									if (element->Coords[i].x < StretchMinX)
										StretchMinX = element->Coords[i].x;
									if (element->Coords[i].x > StretchMaxX)
										StretchMaxX = element->Coords[i].x;
									if (element->Coords[i].y < StretchMinY)
										StretchMinY = element->Coords[i].y;
									if (element->Coords[i].y > StretchMaxY)
										StretchMaxY = element->Coords[i].y;
								}
								if (x < SelX + GrabInternalMargin)
								{
									StretchMode |= StretchLeft;
									StartX = StretchMinX;
									OtherX = StretchMaxX;
								}
								else if (x >= SelX + SelWidth - GrabInternalMargin)
								{
									StretchMode |= StretchRight | StretchTowardsRight;
									StartX = StretchMaxX;
									OtherX = StretchMinX;
								}
								else if (x >= SelX + (SelWidth >> 1))
								{
									StartX = StretchMaxX;
									OtherX = StretchMinX;
									StretchMode |= StretchTowardsRight;
								}
								else
								{
									StartX = StretchMinX;
									OtherX = StretchMaxX;
								}
								if (y < SelY + GrabInternalMargin)
								{
									StretchMode |= StretchUpper;
									StartY = StretchMinY;
									OtherY = StretchMaxY;
								}
								else if (y >= SelY + SelHeight - GrabInternalMargin)
								{
									StretchMode |= StretchLower | StretchTowardsLower;
									StartY = StretchMaxY;
									OtherY = StretchMinY;
								}
								else if (y >= SelY + (SelHeight >> 1))
								{
									StartY = StretchMaxY;
									OtherY = StretchMinY;
									StretchMode |= StretchTowardsLower;
								}
								else
								{
									StartY = StretchMinY;
									OtherY = StretchMaxY;
								}
								CurrentX = (int)WindowToImage(x + PanX);
								CurrentY = (int)WindowToImage(y + PanY);
								if ((StretchMode & StretchProportional) ||
										(shift_down != ShiftInverted))
									MakeProportional();
								SetMouseString(CurrentX, CurrentY);
								StretchNewElement(shift_down);
								PrepareDrawRubber(DC);
								DrawImageMapElement(DC, &NewElement, true);
								CleanupDrawRubber(DC);
								break;
							}
						}
					}
					int x1 = (int)WindowToImage(x + PanX);
					int y1 = (int)WindowToImage(y + PanY);
					int sel = ImageMap->GetIndexAtPoint(x1, y1);
					if (!count && sel >= 0 &&
							ImageMap->GetElement(sel)->Type == G42ImageMap::Default)
						sel = -1;
					SetMouseString(x1, y1);
					int old_sel = -1;
					if (ImageMap->HasSelectedElement())
					{
						old_sel = ImageMap->GetSelectedElement();
					}
					// select element
					if (sel != old_sel && !count)
					{
						if (sel >= 0)
						{
							ImageMap->SelectElement(sel);
							MapSelChanged();
						}
						else
						{
							ImageMap->ClearSelectedElement();
							MapSelChanged();
						}
						Draw();
#if 0
						StartRubber();
						if (old_sel >= 0)
							DrawImageMapElement(DC, ImageMap->GetSelectedElement());
						if (sel >= 0)
						{
							DrawImageMapElement(DC, sel);
							ImageMap->SelectElement(sel);
							DrawImageMapElement(DC, sel);
								MapSelChanged();
						}
						else
						{
							ImageMap->ClearSelectedElement();
							MapSelChanged();
						}
						if (old_sel >= 0)
							DrawImageMapElement(DC, old_sel);
						EndRubber();
#endif
						if (sel >= 0 && ImageMap->GetElement(sel)->Type !=
							G42ImageMap::Default &&
							x >= SelX && x < SelX + SelWidth &&
							y >= SelY && y < SelY + SelHeight)
						{
							MovingElement = true;
							StartStretchElement();
							MoveFromSelect = true;
							NewPolygonClosed = true;
							StartRubber();
							StartX = (int)WindowToImage(x + PanX);
							StartY = (int)WindowToImage(y + PanY);
							CurrentX = StartX;
							CurrentY = StartY;
						}
					}
					// edit element: note that the element may not be selected
					else if (sel >= 0 && count)
					{
						EditMapElement(sel);
					}
					else if (sel < 0 && count) // add default
					{
						NewElement.Type = G42ImageMap::Default;
						NewElement.NumCoords = 0;
						AddMapElement(NewElement);
					}
					break;
				}
				default:
					SetMouseString((int)WindowToImage(x + PanX),
						(int)WindowToImage(y + PanY));
					break;
			}
		}
		else
		{
			CurrentX = (int)WindowToImage(x + PanX);
			CurrentY = (int)WindowToImage(y + PanY);
			SetMouseString(CurrentX, CurrentY);
			if (AreaSelected)
			{
				if (x >= SelX - SelectMargin &&
					x < SelX + SelWidth + SelectMargin &&
					y >= SelY - SelectMargin &&
					y < SelY + SelHeight + SelectMargin)
				{
					if (x >= SelX && x < SelX + SelWidth &&
						y >= SelY && y < SelY + SelHeight)
					{
						StartX = CurrentX;
						StartY = CurrentY;
						MovingArea = true;
					}
					else
					{
						StretchingArea = true;
						StretchMode = 0;
						if (x < SelX)
						{
							StretchMode |= StretchLeft;
							StartX = SelectedAreaX;
							OtherX = SelectedAreaX + SelectedAreaWidth - 1;
						}
						else if (x >= SelX + SelWidth)
						{
							StretchMode |= StretchRight | StretchTowardsRight;
							StartX = SelectedAreaX + SelectedAreaWidth - 1;
							OtherX = SelectedAreaX;
						}
						else if (x >= SelX + (SelWidth >> 1))
						{
							StartX = SelectedAreaX + SelectedAreaWidth - 1;
							OtherX = SelectedAreaX;
							StretchMode |= StretchTowardsRight;
						}
						else
						{
							StartX = SelectedAreaX;
							OtherX = SelectedAreaX + SelectedAreaWidth - 1;
						}
						if (y < SelY)
						{
							StretchMode |= StretchUpper;
							StartY = SelectedAreaY;
							OtherY = SelectedAreaY + SelectedAreaHeight - 1;
						}
						else if (y >= SelY + SelHeight)
						{
							StretchMode |= StretchLower | StretchTowardsLower;
							StartY = SelectedAreaY + SelectedAreaHeight - 1;
							OtherY = SelectedAreaY;
						}
						else if (y >= SelY + (SelHeight >> 1))
						{
							StartY = SelectedAreaY + SelectedAreaHeight - 1;
							OtherY = SelectedAreaY;
							StretchMode |= StretchTowardsLower;
						}
						else
						{
							StartY = SelectedAreaY;
							OtherY = SelectedAreaY + SelectedAreaHeight - 1;
						}
						if (shift_down != ShiftInverted)
							MakeProportional();
						SetMouseString(CurrentX, CurrentY);
						StartRubber();
						AbsStretchingRubberArea(shift_down);
//						StretchSelectedRubberArea(shift_down);
						PrepareDrawRubber(DC);
						DrawRubberImageRect(OtherX, OtherY, CurrentX, CurrentY);
//						DrawRubberRect(SelX, SelY, SelX + SelWidth, SelY + SelHeight);
						CleanupDrawRubber(DC);
					}
				}
			}
			if (!SelectingArea && !MovingArea && !StretchingArea)
			{
				if (AreaSelected)
				{
					PutAreaDown();
				}
				SelectingArea = true;
				StartRubber();
				CurrentX = WindowToImage(x + PanX);
				CurrentY = WindowToImage(y + PanY);
				StartX = CurrentX;
				StartY = CurrentY;
				SetMouseString(CurrentX, CurrentY);
				PrepareDrawRubber(DC);
				DrawRubberImageRect(StartX, StartY, CurrentX, CurrentY);
				CleanupDrawRubber(DC);
			}
		}
	}
	else if (button == 3)
	{
		if (TransparentMode)
		{
			CurrentX = (int)WindowToImage(x + PanX);
			CurrentY = (int)WindowToImage(y + PanY);
			SetMouseString(CurrentX, CurrentY);
			ReportingTransColors = false;
			return true;
		}
		SetMouseString((int)WindowToImage(x + PanX),
			(int)WindowToImage(y + PanY));
		if (SelectingArea)
		{
			PrepareDrawRubber(DC);
			DrawRubberImageRect(StartX, StartY, CurrentX, CurrentY);
			CleanupDrawRubber(DC);
			SelectingArea = false;
			EndRubber();
		}
		else if (MovingArea)
		{
			int32 old_x = SelectedAreaX;
			int32 old_y = SelectedAreaY;
			SelectedAreaX += StartX - CurrentX;
			SelectedAreaY += StartY - CurrentY;
			int dx = abs(StartX - CurrentX) + 1;
			int dy = abs(StartY - CurrentY) + 1;
			Draw(SelX - SelectMargin - dx, SelY - SelectMargin - dy,
				SelWidth + ((SelectMargin + dx) << 1),
				SelHeight + ((SelectMargin + dy) << 1));
			MovingArea = false;
		}
		else if (StretchingArea)
		{
			PrepareDrawRubber(DC);
			DrawRubberImageRect(OtherX, OtherY, CurrentX, CurrentY);
//			DrawRubberRect(SelX, SelY, SelX + SelWidth, SelY + SelHeight);
#if 0
			SelectedAreaX = min(StartX, OtherX);
			SelectedAreaY = min(StartY, OtherY);
			SelectedAreaWidth = max(StartX, OtherX) - SelectedAreaX + 1;
			SelectedAreaHeight = max(StartY, OtherY) - SelectedAreaY + 1;
			SelX = (int)(ImageToWindow(SelectedAreaX) - PanX);
			SelY = (int)(ImageToWindow(SelectedAreaY) - PanY);
			SelWidth = (int)(ImageToWindow(SelectedAreaX + SelectedAreaWidth) -
				PanX) - SelX + 1;
			SelHeight = (int)(ImageToWindow(SelectedAreaY + SelectedAreaHeight) -
				PanY) - SelY + 1;
#endif
			CleanupDrawRubber(DC);
			EndRubber();
			StretchingArea = false;
		}
		else if (AreaSelected)
		{
			PutAreaDown();
			AreaSelected = false;
		}
		else if (NewMapElementStarted)
		{
			switch (MapTool)
			{
				case MapRectangleTool:
				case MapPointTool:
				case MapCircleTool:
					PrepareDrawRubber(DC);
					DrawImageMapElement(DC, &NewElement, true);
					NewMapElementStarted = false;
					EndRubber();
					break;
				case MapPolygonTool:
					if (NewPolygonStarted)
					{
						PrepareDrawRubber(DC);
						DrawImageMapElement(DC, &NewElement, true);
						NewPolygonStarted = false;
						DrawImageMapElement(DC, &NewElement, true);
						CleanupDrawRubber(DC);
					}
					else
					{
						PrepareDrawRubber(DC);
						DrawImageMapElement(DC, &NewElement, true);
						NewPolygonClosed = true;
						DrawImageMapElement(DC, &NewElement, true);
						bool added = AddMapElement(NewElement);
						if (added)
						{
							NewMapElementStarted = false;
							CleanupDrawRubber(DC);
							EndRubber();
							Draw();
						}
						else
						{
							DrawImageMapElement(DC, &NewElement, true);
							CleanupDrawRubber(DC);
							EndRubber();
							NewMapElementStarted = false;
						}
						NewPolygonClosed = false;
					}
					break;
				default:
					break;
			}
		}
		else if (MovingElement)
		{
			if (!MoveFromSelect)
			{
				PrepareDrawRubber(DC);
				DrawImageMapElement(DC, &NewElement, true);
				CleanupDrawRubber(DC);
			}
			EndRubber();
			MovingElement = false;
			MoveFromSelect = false;
		}
		else if (StretchingElement)
		{
			PrepareDrawRubber(DC);
			DrawImageMapElement(DC, &NewElement, true);
			StretchingElement = false;
			CleanupDrawRubber(DC);
			EndRubber();
			Draw();
		}
		else if (EditingPolygon)
		{
			PrepareDrawRubber(DC);
			DrawImageMapElement(DC, &NewElement, true);
			EditingPolygon = false;
			CleanupDrawRubber(DC);
			EndRubber();
			Draw();
		}
		else if (ImageMapVisible && ImageMap && MapTool == MapEditTool)
		{
			if (ImageMap->HasSelectedElement() && ImageMap->GetElement(
				ImageMap->GetSelectedElement())->Type != G42ImageMap::Default)
			{
				int sel = ImageMap->GetSelectedElement();
				ImageMap->ClearSelectedElement();
				MapSelChanged();
				Draw();
			}
		}
      else
      {
      	// Switch to Pointer Tool
         return false;
      }
	}
	else
	{
		SetMouseString((int)WindowToImage(x + PanX),
			(int)WindowToImage(y + PanY));
	}
   return true;
}
void
G42SingleImageViewer::MouseUp(int x, int y, int button,
	bool shift_down, bool control_down)
{
	#ifdef MSWIN
	if (control_down || OldControlDown)
	{
		if (control_down)
		{
			if (OldControlDown)
			{
//				POINT p;
//				GetCursorPos(&p);
//				ScreenToClient(Window, &p);
//				x = p.x;
//				y = p.y;
				int new_x = x;
				int new_y = y;
				CursorXFraction += x - OldCursorX;
				if (CursorXFraction > 0)
				{
					x = OldCursorX + (CursorXFraction >> 2);
					CursorXFraction &= 3;
				}
				else if (CursorXFraction < 0)
				{
					x = OldCursorX - ((3 - CursorXFraction) >> 2);
					CursorXFraction = 3 - ((-CursorXFraction + 3) & 3);
				}
				CursorYFraction += y - OldCursorY;
				if (CursorYFraction > 0)
				{
					y = OldCursorY + (CursorYFraction >> 2);
					CursorYFraction &= 3;
				}
				else if (CursorYFraction < 0)
				{
					y = OldCursorY - ((3 - CursorYFraction) >> 2);
					CursorYFraction = 3 - ((-CursorYFraction + 3) & 3);
				}
				if (x != new_x || y != new_y)
				{
					POINT p;
					p.x = x;
					p.y = y;
					ClientToScreen(Window, &p);
					SetCursorPos(p.x, p.y);
				}
				OldCursorX = x;
				OldCursorY = y;
			}
			else
			{
				OldControlDown = true;
				OldCursorX = x;
				OldCursorY = y;
				CursorXFraction = 0;
            CursorYFraction = 0;
			}
		}
		else
		{
			OldControlDown = false;
		}
	}
	#endif
	if (button == 2)
		ShiftInverted = false;
	if (TransparentMode)
	{
		CurrentX = (int)WindowToImage(x + PanX);
		CurrentY = (int)WindowToImage(y + PanY);
		SetMouseString(CurrentX, CurrentY);
		if (ReportingTransColors && (button == 1 || button == 2))
		{
			ReportingTransColors = false;
			if (CurrentX >= 0 && CurrentX < Image->GetInfo().Width &&
				CurrentY >= 0 && CurrentY < Image->GetInfo().Height)
					TransparentPointClick(CurrentX, CurrentY, true);
		}
		return;
	}
	if (SelectingArea && (button == 1 || button == 2))
	{
		PrepareDrawRubber(DC);
		DrawRubberImageRect(StartX, StartY, CurrentX, CurrentY);
		CleanupDrawRubber(DC);
		CurrentX = WindowToImage((int32)x + PanX);
		CurrentY = WindowToImage((int32)y + PanY);
		SetMouseString(CurrentX, CurrentY);
		SelectingArea = false;
		if (CurrentX < 0)
			CurrentX = 0;
		if (CurrentX >= Image->GetInfo().Width)
			CurrentX = Image->GetInfo().Width - 1;
		if (CurrentY < 0)
			CurrentY = 0;
		if (CurrentY >= Image->GetInfo().Height)
			CurrentY = Image->GetInfo().Height - 1;
		if (StartX < 0)
			StartX = 0;
		if (StartX >= Image->GetInfo().Width)
			StartX = Image->GetInfo().Width - 1;
		if (StartY < 0)
			StartY = 0;
		if (StartY >= Image->GetInfo().Height)
			StartY = Image->GetInfo().Height - 1;
		SelX = min(StartX, CurrentX);
		SelY = min(StartY, CurrentY);
		SelWidth = max(StartX, CurrentX) - SelX + 1;
		SelHeight = max(StartY, CurrentY) - SelY + 1;
		SelectArea(SelX, SelY, SelX + SelWidth - 1, SelY + SelHeight - 1);
		EndRubber();
		if (AreaSelected)
			Draw(SelX - SelectMargin, SelY - SelectMargin, 
				SelWidth + (SelectMargin << 1), SelHeight + (SelectMargin << 1), 
				true);
	}
	else if (MovingArea && (button == 1 || button == 2))
	{
		int old_x = CurrentX;
		int old_y = CurrentY;
		CurrentX = (int)WindowToImage(x + PanX);
		CurrentY = (int)WindowToImage(y + PanY);
		SetMouseString(CurrentX, CurrentY);
		SelectedAreaX += CurrentX - old_x;
		SelectedAreaY += CurrentY - old_y;
		MovingArea = false;
		Draw(SelX - SelectMargin, SelY - SelectMargin, 
				SelWidth + (SelectMargin << 1), SelHeight + (SelectMargin << 1), 
				true);
	}
	else if (StretchingArea && (button == 1 || button == 2))
	{
		/*  Save these for optimized drawing below  */
		int old_sel_x = SelX;
		int old_sel_y = SelY;
		int old_sel_width = SelWidth;
		int old_sel_height = SelHeight;
		PrepareDrawRubber(DC);
		DrawRubberImageRect(OtherX, OtherY, CurrentX, CurrentY);
//		DrawRubberRect(SelX, SelY, SelX + SelWidth, SelY + SelHeight);
		CleanupDrawRubber(DC);
		CurrentX = (int)WindowToImage(x + PanX);
		CurrentY = (int)WindowToImage(y + PanY);
		if (shift_down != ShiftInverted)
			MakeProportional();
		SetMouseString(CurrentX, CurrentY);
		StretchSelectedRubberArea(shift_down);
		StretchSelectedArea();
		StretchingArea = false;
		EndRubber();
		Draw(min (SelX, old_sel_x) - SelectMargin, 
			min (SelY, old_sel_y) - SelectMargin, 
				max (SelWidth, old_sel_width) + (SelectMargin << 1), 
				max (SelHeight, old_sel_height) + (SelectMargin << 1), 
				true);
	}
	else if (NewMapElementStarted && (button == 1 || button == 2))
	{
		switch (MapTool)
		{
			case MapRectangleTool:
			{
				CurrentX = (int)WindowToImage(x + PanX);
				CurrentY = (int)WindowToImage(y + PanY);
				if (shift_down != ShiftInverted)
					MakeProportional();
				SetMouseString(CurrentX, CurrentY);
				PrepareDrawRubber(DC);
				DrawImageMapElement(DC, &NewElement, true);
				NewElement.Coords[1].x = CurrentX;
				NewElement.Coords[1].y = CurrentY;
				DrawImageMapElement(DC, &NewElement, true);
				bool added = AddMapElement(NewElement);
				if (added)
				{
					NewMapElementStarted = false;
					CleanupDrawRubber(DC);
					EndRubber();
					Draw();
				}
				else
				{
					DrawImageMapElement(DC, &NewElement, true);
					CleanupDrawRubber(DC);
					EndRubber();
					NewMapElementStarted = false;
				}
				break;
			}
			case MapPointTool:
			{
				PrepareDrawRubber(DC);
				DrawImageMapElement(DC, &NewElement, true);
				NewElement.Coords[0].x = (int)WindowToImage(x + PanX);
				NewElement.Coords[0].y = (int)WindowToImage(y + PanY);
				SetMouseString(NewElement.Coords[0].x, NewElement.Coords[0].y);
				DrawImageMapElement(DC, &NewElement, true);
				bool added = AddMapElement(NewElement);
				if (added)
				{
					NewMapElementStarted = false;
					CleanupDrawRubber(DC);
					EndRubber();
					Draw();
				}
				else
				{
					DrawImageMapElement(DC, &NewElement, true);
					CleanupDrawRubber(DC);
					EndRubber();
					NewMapElementStarted = false;
				}
				break;
			}
			case MapPolygonTool:
			{
				if (NewPolygonStarted)
				{
					CurrentX = (int)WindowToImage(x + PanX);
					CurrentY = (int)WindowToImage(y + PanY);
					if (shift_down != ShiftInverted)
						MakeProportional();
					SetMouseString(CurrentX, CurrentY);
					PrepareDrawRubber(DC);
					DrawImageMapElement(DC, &NewElement, true);
					NewElement.Coords[NewElement.NumCoords].x = CurrentX;
					NewElement.Coords[NewElement.NumCoords].y = CurrentY;
					if (NewElement.Coords[NewElement.NumCoords].x !=
							NewElement.Coords[NewElement.NumCoords - 1].x ||
							NewElement.Coords[NewElement.NumCoords].y !=
							NewElement.Coords[NewElement.NumCoords - 1].y)
						NewElement.NumCoords++;
					if (NewElement.NumCoords == 99)
					{
						NewPolygonClosed = true;
						DrawImageMapElement(DC, &NewElement, true);
						bool added = AddMapElement(NewElement);
						if (added)
						{
							NewMapElementStarted = false;
							CleanupDrawRubber(DC);
							EndRubber();
							Draw();
						}
						else
						{
							DrawImageMapElement(DC, &NewElement, true);
							CleanupDrawRubber(DC);
							EndRubber();
							NewMapElementStarted = false;
						}
						NewMapElementStarted = false;
						NewPolygonClosed = false;
						break;
					}
					NewPolygonStarted = false;
					DrawImageMapElement(DC, &NewElement, true);
					CleanupDrawRubber(DC);
				}
				else
				{
					SetMouseString((int)WindowToImage(x + PanX),
						(int)WindowToImage(y + PanY));
				}
				break;
			}
			case MapCircleTool:
			{
				CurrentX = x;
				CurrentY = y;
				if (shift_down != ShiftInverted)
					MakeProportional();
				PrepareDrawRubber(DC);
				DrawImageMapElement(DC, &NewElement, true);
				CleanupDrawRubber(DC);
				int x1 = (int)WindowToImage(StartX + PanX);
				int y1 = (int)WindowToImage(StartY + PanY);
				SetMouseString(x1, y1);
				int x2 = (int)WindowToImage(CurrentX + PanX);
				int y2 = (int)WindowToImage(CurrentY + PanY);
				GetCircleRect(x1, y1, x2, y2);
				NewElement.Coords[0].x = x1;
				NewElement.Coords[0].y = y1;
				NewElement.Coords[1].x = x2;
				NewElement.Coords[1].y = y2;
				DrawImageMapElement(DC, &NewElement, true);
				bool added = AddMapElement(NewElement);
				if (added)
				{
					NewMapElementStarted = false;
					CleanupDrawRubber(DC);
					EndRubber();
					Draw();
				}
				else
				{
					DrawImageMapElement(DC, &NewElement, true);
					CleanupDrawRubber(DC);
					EndRubber();
					NewMapElementStarted = false;
				}
				break;
			}
			default:
				SetMouseString((int)WindowToImage(x + PanX),
					(int)WindowToImage(y + PanY));
				break;
		}
	}
	else if (MovingElement && (button == 1 || button == 2))
	{
		int x1 = (int)WindowToImage(x + PanX);
		int y1 = (int)WindowToImage(y + PanY);
		SetMouseString(x1, y1);
		if (MoveFromSelect)
		{
			if (abs(x1 - StartX) > SelectMargin ||
				abs(y1 - StartY) > SelectMargin)
			{
				MoveFromSelect = false;
				MoveNewElement(x1 - StartX, y1 - StartY); // so it draws correctly
			}
			else
			{
				EndRubber();
				MovingElement = false;
			}
		}
		if (!MoveFromSelect)
		{
			PrepareDrawRubber(DC);
			DrawImageMapElement(DC, &NewElement, true);
			CleanupDrawRubber(DC);
			ImageMap->MoveElement(ImageMap->GetSelectedElement(),
				x1 - StartX, y1 - StartY);
			EndRubber();
			MovingElement = false;
			MoveFromSelect = false; // have to do this before Draw();
			Draw();
		}
		MoveFromSelect = false;
	}
	else if (StretchingElement && (button == 1 || button == 2))
	{
		PrepareDrawRubber(DC);
		DrawImageMapElement(DC, &NewElement, true);
		CleanupDrawRubber(DC);
		EndRubber();
		int old_width = StretchMaxX - StretchMinX + 1;
		int old_height = StretchMaxY - StretchMinY + 1;
		CurrentX = (int)WindowToImage(x + PanX);
		CurrentY = (int)WindowToImage(y + PanY);
		if ((StretchMode & StretchProportional) ||
				(shift_down != ShiftInverted))
			MakeProportional();
		SetMouseString(CurrentX, CurrentY);
		int new_width;
		int dx;
		if ((StretchMode & StretchLeft) ||
			(((StretchMode & StretchProportional) ||
			(shift_down != ShiftInverted)) &&
			!(StretchMode & StretchTowardsRight)))
		{
			new_width = abs(CurrentX - OtherX) + 1;
         if (CurrentX <= OtherX)
				dx = old_width - new_width;
         else
         	dx = old_width;
		}
		else if ((StretchMode & StretchRight) ||
			(((StretchMode & StretchProportional) ||
			(shift_down != ShiftInverted)) &&
			(StretchMode & StretchTowardsRight)))
		{
			new_width = abs(CurrentX - OtherX) + 1;
	      if (CurrentX >= OtherX)
				dx = 0;
      	else
      		dx = -new_width;
		}
		else
		{
			dx = 0;
			new_width = old_width;
		}
		int new_height;
		int dy;
		if ((StretchMode & StretchUpper) ||
			(((StretchMode & StretchProportional) ||
			(shift_down != ShiftInverted)) &&
			!(StretchMode & StretchTowardsLower)))
		{
			new_height = abs(CurrentY - OtherY) + 1;
		  if (CurrentY <= OtherY)
				dy = old_height - new_height;
      	else
      		dy = old_height;
		}
		else if ((StretchMode & StretchLower) ||
			(((StretchMode & StretchProportional) ||
			(shift_down != ShiftInverted)) &&
			(StretchMode & StretchTowardsLower)))
		{
			new_height = abs(CurrentY - OtherY) + 1;
 	     if (CurrentY >= OtherY)
				dy = 0;
      	else
	      	dy = -new_height;
		}
		else
		{
			dy = 0;
			new_height = old_height;
		}
		StretchingElement = false;
		ImageMap->StretchElement(ImageMap->GetSelectedElement(),
			new_width, new_height, dx, dy);
		Draw();
	}
	else if (EditingPolygon && (button == 1 || button == 2))
	{
		PrepareDrawRubber(DC);
		DrawImageMapElement(DC, &NewElement, true);
		CleanupDrawRubber(DC);
		CurrentX = (int)WindowToImage(x + PanX);
		CurrentY = (int)WindowToImage(y + PanY);
		SetMouseString(CurrentX, CurrentY);
		EditingPolygon = false;
		ImageMap->SetPolygonPoint(ImageMap->GetSelectedElement(),
			EditingPolygonIndex, CurrentX, CurrentY);
		EndRubber();
		Draw();
	}
	else
	{
		SetMouseString((int)WindowToImage(x + PanX),
			(int)WindowToImage(y + PanY));
	}
}
void
G42SingleImageViewer::GetCircleRect(int & x1, int & y1, int & x2, int & y2)
{
	int cx = ((x1 + x2) >> 1);
	int cy = ((y1 + y2) >> 1);
	int dx = x2 - x1;
	int dy = y2 - y1;
	int dia = (int)(sqrt((double)dx * (double)dx + (double)dy * (double)dy) + .5);
	int r1 = (dia >> 1);
	int r2 = dia - r1;
	x1 = cx - r1;
	y1 = cy - r1;
	x2 = cx + r2;
	y2 = cy + r2;
}
void
G42SingleImageViewer::SetMapTool(G42MapTool map_tool)
{
	if (NewMapElementStarted)
	{
		DrawImageMapElement(DC, &NewElement, true);
		EndRubber();
		NewMapElementStarted = false;
		NewPolygonStarted = false;
		NewPolygonClosed = false;
	}
	if (MapTool == MapEditTool)
	{
		if (CurrentElement >= 0)
		{
			StartRubber();
			int ce = CurrentElement;
			DrawImageMapElement(DC, ce);
			CurrentElement = -1;
			DrawImageMapElement(DC, ce);
			EndRubber();
		}
	}
	MapTool = map_tool;
	if (MapTool == MapEditTool)
		SetCursor(ArrowCursor);
	else
		SetCursor(CrossCursor);
}
void
G42SingleImageViewer::StartStretchElement(void)
{
	const G42ImageMap::G42MapElement * element = ImageMap->GetElement(
		ImageMap->GetSelectedElement()); 
	NewElement.Type = element->Type;
	NewElement.NumCoords = element->NumCoords;
	for (uint i = 0; i < NewElement.NumCoords; i++)
	{
		NewElement.Coords[i] = element->Coords[i];
	}
}
void
G42SingleImageViewer::StretchNewElement(bool shift_down)
{
	int old_width = StretchMaxX - StretchMinX + 1;
	int old_height = StretchMaxY - StretchMinY + 1;
	int new_width;
	int dx;
	if ((StretchMode & StretchLeft) ||
		(((StretchMode & StretchProportional) ||
		(shift_down != ShiftInverted)) &&
		!(StretchMode & StretchTowardsRight)))
	{
		new_width = abs(CurrentX - OtherX) + 1;
      if (CurrentX <= OtherX)
			dx = old_width - new_width;
      else
			dx = old_width;
	}
	else if ((StretchMode & StretchRight) ||
		(((StretchMode & StretchProportional) ||
		(shift_down != ShiftInverted)) &&
		(StretchMode & StretchTowardsRight)))
	{
		new_width = abs(CurrentX - OtherX) + 1;
      if (CurrentX >= OtherX)
			dx = 0;
      else
      	dx = -new_width;
	}
	else
	{
		dx = 0;
		new_width = old_width;
	}
	int new_height;
	int dy;
	if ((StretchMode & StretchUpper) ||
		(((StretchMode & StretchProportional) ||
		(shift_down != ShiftInverted)) &&
		!(StretchMode & StretchTowardsLower)))
	{
		new_height = abs(CurrentY - OtherY) + 1;
      if (CurrentY <= OtherY)
			dy = old_height - new_height;
      else
      	dy = old_height;
	}
	else if ((StretchMode & StretchLower) ||
		(((StretchMode & StretchProportional) ||
		(shift_down != ShiftInverted)) &&
		(StretchMode & StretchTowardsLower)))
	{
		new_height = abs(CurrentY - OtherY) + 1;
      if (CurrentY >= OtherY)
			dy = 0;
      else
      	dy = -new_height;
	}
	else
	{
		dy = 0;
		new_height = old_height;
	}
	const G42ImageMap::G42MapElement * element =
		ImageMap->GetElement(ImageMap->GetSelectedElement());
	for (uint i = 0; i < NewElement.NumCoords; i++)
	{
		NewElement.Coords[i].x =
			(uint)((int)(((int32)(element->Coords[i].x - StretchMinX) *
			(int32)(new_width - 1)) / (int32)(old_width - 1)) +
			(int)StretchMinX + dx);
		NewElement.Coords[i].y =
			(uint)((int)(((int32)(element->Coords[i].y - StretchMinY) *
			(int32)(new_height - 1)) / (int32)(old_height - 1)) +
			(int)StretchMinY + dy);
	}
}
void
G42SingleImageViewer::MoveNewElement(int dx, int dy)
{
	for (uint i = 0; i < NewElement.NumCoords; i++)
	{
		NewElement.Coords[i].x += dx;
		NewElement.Coords[i].y += dy;
	}
}
void
G42SingleImageViewer::StretchSelectedArea(void)
{
	SelectedImage->ResizeImage(SelectedAreaWidth, SelectedAreaHeight);
	Rebuild();
#if 0	
	if (SelectedStretchImage)
	{
		delete SelectedStretchImage;
		SelectedStretchImage = 0;
	}
	if (SelectedDitherImage)
	{
		delete SelectedDitherImage;
		SelectedDitherImage = 0;
	}
	if (SelectedDitherBaseImage)
	{
		delete SelectedDitherBaseImage;
		SelectedDitherBaseImage = 0;
	}
#endif
}
void
G42SingleImageViewer::StretchSelectedRubberArea(bool shift_down)
{
	AbsStretchingRubberArea(shift_down);
	int new_width;
	int dx;
	if ((StretchMode & StretchLeft) ||
		(((StretchMode & StretchProportional) ||
		(shift_down != ShiftInverted)) &&
		!(StretchMode & StretchTowardsRight)))
	{
		new_width = abs(CurrentX - OtherX) + 1;
		dx = SelectedAreaWidth - new_width;
	}
	else if ((StretchMode & StretchRight) ||
		(((StretchMode & StretchProportional) ||
		(shift_down != ShiftInverted)) &&
		(StretchMode & StretchTowardsRight)))
	{
		new_width = abs(CurrentX - OtherX) + 1;
		dx = 0;
	}
	else
	{
		dx = 0;
		new_width = SelectedAreaWidth;
	}
	int new_height;
	int dy;
	if ((StretchMode & StretchUpper) ||
		(((StretchMode & StretchProportional) ||
		(shift_down != ShiftInverted)) &&
		!(StretchMode & StretchTowardsLower)))
	{
		new_height = abs(CurrentY - OtherY) + 1;
		dy = SelectedAreaHeight - new_height;
	}
	else if ((StretchMode & StretchLower) ||
		(((StretchMode & StretchProportional) ||
		(shift_down != ShiftInverted)) &&
		(StretchMode & StretchTowardsLower)))
	{
		new_height = abs(CurrentY - OtherY) + 1;
		dy = 0;
	}
	else
	{
		dy = 0;
		new_height = SelectedAreaHeight;
	}
	SelectedAreaX += dx;
	SelectedAreaWidth = new_width;
	SelectedAreaY += dy;
	SelectedAreaHeight = new_height;
	SelX = (int)(ImageToWindow(SelectedAreaX) - PanX);
	SelY = (int)(ImageToWindow(SelectedAreaY) - PanY);
	SelWidth = (int)(ImageToWindow(SelectedAreaX + SelectedAreaWidth) -
		PanX) - SelX + 1;
	SelHeight = (int)(ImageToWindow(SelectedAreaY + SelectedAreaHeight) -
		PanY) - SelY + 1;
}
void
G42SingleImageViewer::AbsStretchingRubberArea(bool shift_down)
{
	if ((StretchMode & StretchLeft) ||
		(((StretchMode & StretchProportional) ||
		(shift_down != ShiftInverted)) &&
		!(StretchMode & StretchTowardsRight)))
	{
		if (CurrentX >= OtherX - 1)
			CurrentX = OtherX - 2;
	}
	else if ((StretchMode & StretchRight) ||
		(((StretchMode & StretchProportional) ||
		(shift_down != ShiftInverted)) &&
		(StretchMode & StretchTowardsRight)))
	{
		if (CurrentX <= OtherX + 1)
			CurrentX = OtherX + 2;
	}
	else
	{
		CurrentX = StartX;
	}
	if ((StretchMode & StretchUpper) ||
		(((StretchMode & StretchProportional) ||
		(shift_down != ShiftInverted)) &&
		!(StretchMode & StretchTowardsLower)))
	{
		if (CurrentY >= OtherY - 1)
			CurrentY = OtherY - 2;
	}
	else if ((StretchMode & StretchLower) ||
		(((StretchMode & StretchProportional) ||
		(shift_down != ShiftInverted)) &&
		(StretchMode & StretchTowardsLower)))
	{
		if (CurrentY <= OtherY + 1)
			CurrentY = OtherY + 2;
	}
	else
	{
		CurrentY = StartY;
	}
}
bool
G42SingleImageViewer::AddMapElement(G42ImageMap::G42MapElement &)
{
	return false;
}
void
G42SingleImageViewer::SetStatusString(const char *)
{
}
void
G42SingleImageViewer::SetURLString(const char *)
{
}
void
G42SingleImageViewer::EditMapElement(uint)
{
}
void
G42SingleImageViewer::MapSelChanged(void)
{
}
void
G42SingleImageViewer::AboutToMoveElement(void)
{
	if (CurrentElement >= 0)
	{
		if (DC)
		{
			int index = CurrentElement;
			DrawImageMapElement(DC, index);
			CurrentElement = -1;
			DrawImageMapElement(DC, index);
		}
		else
		{
			StartRubber();
			int index = CurrentElement;
			DrawImageMapElement(DC, index);
			CurrentElement = -1;
			DrawImageMapElement(DC, index);
			StartRubber();
		}
	}
}
// set up and clean up DC
#ifdef MSWIN
void
G42SingleImageViewer::StartRubber(void)
{
	if (DC)
		ReleaseDC(Window, DC);
	DC = GetDC(Window);
//	PrepareImageMapDC(DC);
	SetROP2(DC, R2_NOT);
	SelectObject(DC, GetStockObject(NULL_BRUSH));
	SelectObject(DC, GetStockObject(BLACK_PEN));
}
#endif
#ifdef MSWIN
void
G42SingleImageViewer::EndRubber(void)
{
	if (DC)
		ReleaseDC(Window, DC);
	DC = 0;
}
#endif
#ifdef MACOS
void
G42SingleImageViewer::StartRubber(void)
{
	#ifdef MACOS	// What's going on here???
	Rect rect;
	PenState penState;
	GetPenState (&penState);
	penState.pnMode = srcXor;
	#ifdef SETPENSTATE
	::SetPenState (&penState);
	#endif
	#endif
}
#endif
#ifdef MACOS
void
G42SingleImageViewer::EndRubber(void)
{
	PenNormal ();
}
#endif
void
G42SingleImageViewer::DrawRubberImageRect(int32 x1, int32 y1, int32 x2, int32 y2)
{
	G42ZoomInfo zi(ZoomMode, ZoomInt, ZoomFactor);
	DrawRubberRect(zi.ImageToWindow(x1) - PanX, zi.ImageToWindow(y1) - PanY,
		zi.ImageToWindow(x2) - PanX, zi.ImageToWindow(y2) - PanY);
}
void
G42SingleImageViewer::DrawRubberImageRect(G42DrawLocation location,
	int32 x1, int32 y1, int32 x2, int32 y2)
{
	G42ZoomInfo zi(ZoomMode, ZoomInt, ZoomFactor);
	DrawRubberRect(location,
		zi.ImageToWindow(x1) - PanX, zi.ImageToWindow(y1) - PanY,
		zi.ImageToWindow(x2) - PanX, zi.ImageToWindow(y2) - PanY);
}
#ifdef MSWIN
void
G42SingleImageViewer::DrawRubberRect(G42DrawLocation location,
	int x1, int y1, int x2, int y2)
{
	Rectangle(location, min(x1, x2), min(y1, y2),
		max(x1, x2) + 1, max(y1, y2) + 1);
}
void
G42SingleImageViewer::DrawRubberRect(int x1, int y1, int x2, int y2)
{
	DrawRubberRect(DC, x1, y1, x2, y2);
}
#endif
#ifdef MACOS
void
G42SingleImageViewer::DrawRubberRect(int x1, int y1, int x2, int y2)
{
	Rect rect;
	SetRectMac (&rect, min(x1, x2), min(y1, y2),
		max(x1, x2) + 1, max(y1, y2) + 1);
	FrameRect (&rect);
}
void
G42SingleImageViewer::DrawRubberRect(G42DrawLocation,
	int x1, int y1, int x2, int y2)
{
	Rect rect;
	SetRectMac (&rect, min(x1, x2), min(y1, y2),
		max(x1, x2) + 1, max(y1, y2) + 1);
	FrameRect (&rect);
}
#endif
void
G42SingleImageViewer::DrawIdle (G42DrawLocation location)
{
	if ((ImageMap->HasSelectedElement() && ImageMap->GetElement(
		ImageMap->GetSelectedElement())->Type != G42ImageMap::Default) ||
		AreaSelected)
	{
//	 	PrepareDrawArea(location);	// Howcome this works sans setup? TBD
		DrawSelectedRectangle(location);
//	 	CleanupDrawArea(location);
	}
}
bool 
G42SingleImageViewer::IsAnElementSelected (void)
{
	if (! ImageMap)
		return false;
	if (ImageMap->HasSelectedElement() && 
		ImageMap->GetElement (ImageMap->GetSelectedElement()) -> Type 
			!= G42ImageMap::Default)
		return true;
	return false;
}
/*  Utility functions for Mac  */
#ifdef MACOS
void
G42SingleImageViewer::SetRectMac (Rect *rect, short left, short top, short right, short bottom)
{
	/*  A necessary test to make sure the rectangle gets created right  */
	register short tmp;
	if (top > bottom)
	{
		tmp = top; top = bottom; bottom = tmp;
	}
	if (left > right)
	{
		tmp = left; left = right; right = tmp;
	}
	::SetRect (rect, left, top, right, bottom);
	#if 0
	CheckRect (rect);
	#endif
}
void
G42SingleImageViewer::CheckRect (Rect *rect)
{
	#if 0
	if (rect -> top < 0 || rect -> bottom > WindowHeight)
		SysBeep(0);
	if (rect -> left < 0 || rect -> right > WindowWidth)
		SysBeep(0);
	if (rect -> top > rect -> bottom)
		SysBeep(0);
	if (rect -> left > rect -> right)	
		SysBeep(0);
	#endif
}
#endif
