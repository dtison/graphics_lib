// g42iview.cpp - simple image viewer
/*
	G42SimpleImageViewer class - displays an image in a window
	changes:
		ges256 - for displaying in 256 paletted mode
		gessa - stretching selected area code
		gesdtr - adding in an unzoomed dithered image for speed
*/
#include <g42itype.h>
#include <g42iview.h>
#include <g42image.h>
#ifdef MACOS
#include <Quickdraw.h>
#include <ToolUtils.h>
#include <UException.h>	// Temporarily use some PowerPlant exceptions macros
#include "g42macfl.h"
#endif
#ifdef MSWIN
G42SimpleImageViewer::G42SimpleImageViewer(HWND window, HWND top_window) :
	#ifdef MSWIN
	Window(window), TopWindow(top_window),
	DibHeader(0), DibUsage(DIB_RGB_COLORS),
	DitherWindowPalette(0),
	#endif
	#ifdef MACOS	// (m.2) Some changes
	CurrentCursor (&qd.arrow), GWindowPtr (windowPtr), 	CallBack (CallBackFn),
	GWorldTile (0), PixMapTile (0), GPaletteHandle (0),
	#endif
	Valid(true), WindowWidth(0),
	WindowHeight(0),
	WindowX(0), WindowY(0),
	ScreenType(TrueColor)
{
	DibHeader = (BITMAPINFOHEADER *)new char [sizeof (BITMAPINFOHEADER) +
		256 * sizeof (RGBQUAD)];
	if (!DibHeader)
	{
		Valid = false;
		return;
	}
	memset(DibHeader, 0, sizeof (BITMAPINFOHEADER) +
		256 * sizeof (RGBQUAD));
	DibHeader->biPlanes = 1;
	DibHeader->biSize = sizeof (BITMAPINFOHEADER);
	DibHeader->biCompression = BI_RGB;
	WindowSizeChanged();
	HDC dc = GetDC(0);
	int bits_per_plane = GetDeviceCaps(dc, BITSPIXEL);
	int planes = GetDeviceCaps(dc, PLANES);
	ReleaseDC(0, dc);
	int depth = bits_per_plane * planes;
	if (depth <= 1)
		ScreenType = Monochrome;
	else if (depth <= 4)
		ScreenType = Palette16;
	else if (depth <= 8)
		ScreenType = Palette256;
	else
		ScreenType = TrueColor;
}
#endif
#ifdef MACOS
G42SimpleImageViewer::G42SimpleImageViewer (GWorldPtr windowPtr, void (*CallBackFn)()) :
	#ifdef MSWIN
	Window(window), TopWindow(top_window),
	DibHeader(0), DibUsage(DIB_RGB_COLORS),
	DitherWindowPalette(0),
	#endif
	#ifdef MACOS	// (m.2) Some changes
	GWindowPtr (windowPtr), CallBack (CallBackFn),
	GWorldTile (0), PixMapTile (0), GPaletteHandle (0), 
	RowBytes (0), ChangedForDraw (true),
	#endif
	Valid(true), WindowWidth(0),
	WindowHeight(0),
	WindowX(0), WindowY(0),
	ScreenType(TrueColor)
{
	if (GWindowPtr)
	{
		Rect rect = GWindowPtr -> portRect;
		WindowWidth = rect.right - rect.left;
		WindowHeight = rect.bottom - rect.top;
	}
	#if 0
	/*  Set up the screen type and stuff  */
	if (GetScreenDepth() < 8)
		ScreenType = Monochrome;	
	#endif
}
#endif
G42SimpleImageViewer::~G42SimpleImageViewer()
{
	// start ges256
	#ifdef MSWIN
	if (DibHeader)
	{
		delete [] (char *)DibHeader;
		DibHeader = 0;
	}
	if (DitherWindowPalette)
	{
		DeleteObject(DitherWindowPalette);
		DitherWindowPalette = 0;
	}
	#endif
	// we don't own ImageMap, so we won't delete it
	#ifdef MACOS
	if (PixMapTile)
	{
		DisposePixMap (PixMapTile);
		PixMapTile = 0;
	}
	if (GPaletteHandle)
	{
		DisposePalette (GPaletteHandle);
		GPaletteHandle = 0;
	}
	#endif
}
void
G42SimpleImageViewer::SetWindowsPalette(const G42Color * dpal, int num_pal)
{
#ifdef MSWIN
	LOGPALETTE * log_pal = (LOGPALETTE *)new byte [2048]; // just to be safe
	log_pal->palVersion = 0x300;
	log_pal->palNumEntries = (WORD)num_pal;
	for (int i = 0; i < log_pal->palNumEntries; i++)
	{
		log_pal->palPalEntry[i].peRed = dpal[i].red;
		log_pal->palPalEntry[i].peGreen = dpal[i].green;
		log_pal->palPalEntry[i].peBlue = dpal[i].blue;
		log_pal->palPalEntry[i].peFlags = 0;
	}
	if (DitherWindowPalette)
		DeleteObject(DitherWindowPalette);
	DitherWindowPalette = CreatePalette(log_pal);
	delete [] (byte *)log_pal;
#endif
}
void
G42SimpleImageViewer::DrawImage(G42DrawLocation location, G42Image * image,
	int draw_x, int draw_y, int draw_width, int draw_height,
	G42ZoomInfo zoom_info, int32 pan_x, int32 pan_y)
{
	if (!image || !image->HasInfo())
		return;
	// don't draw off the window
	if (draw_x < WindowX)
	{
		draw_width -= (WindowX - draw_x);
		draw_x = WindowX;
	}
	// don't draw off the image
	if ((int32)draw_x + pan_x < 0)
	{
		int new_draw_x = (int)(-pan_x);
		draw_width -= new_draw_x - draw_x;
		draw_x = new_draw_x;
	}
	if (draw_x + draw_width > WindowX + WindowWidth)
	{
		draw_width = WindowX + WindowWidth - draw_x;
	}
	if (zoom_info.WindowToImage((int32)draw_x + draw_width + pan_x) >
		(int32)image->GetInfo().Width)
	{
		draw_width = (int)(zoom_info.ImageToWindow(
			image->GetInfo().Width) - pan_x - (int32)draw_x);
	}
	if (draw_y < WindowY)
	{
		draw_height -= WindowY - draw_y;
		draw_y = WindowY;
	}
	if ((int32)draw_y <
		zoom_info.ImageToWindow((int32)image->GetValidStartRow()) - pan_y)
	{
		int32 new_draw_y = (zoom_info.ImageToWindow(
			(int32)image->GetValidStartRow()) - pan_y);
		if (new_draw_y >= (int32)(draw_y + draw_height))
			return;
		draw_height -= (int)new_draw_y - draw_y;
		draw_y = (int)new_draw_y;
	}
	if (draw_y + draw_height > WindowY + WindowHeight)
	{
		draw_height = WindowY + WindowHeight - draw_y;
	}
	if (zoom_info.WindowToImage((int32)draw_y + draw_height + pan_y) >
		(int32)(image->GetValidStartRow() + image->GetValidNumRows()))
	{
		draw_height = (int)(zoom_info.ImageToWindow(
			image->GetValidStartRow() + image->GetValidNumRows()) -
			pan_y - (int32)draw_y);
	}
	if (draw_width <= 0 || draw_height <= 0)
		return;
	DrawImage(location, image, draw_x, draw_y, draw_width, draw_height,
		zoom_info, pan_x, pan_y, 0, zoom_info);
}
// note, we assume that the draw coordinates are within the
// image and location.  Thus, all clipping must be done outside this
// function.  On the other hand, it means we don't waste time
// reclipping what we already know to be valid.
// we also assume the image is valid.
void
G42SimpleImageViewer::DrawImage(G42DrawLocation location, G42Image * image,
	int draw_x, int draw_y, int draw_width, int draw_height,
	G42ZoomInfo zoom_info, int32 pan_x, int32 pan_y,
	G42Image * backup_image, G42ZoomInfo backup_zoom_info)
{
	#ifdef MACOS	// (m.3)
	if (! SameAsPrevImage (image))	// Optimized with image-changed detection
	{
		if (PixMapTile)
		{
			DisposePixMap (PixMapTile);
			PixMapTile = nil;
		}
		PrevImageInfo = image -> GetInfo();
	}
	if (! PixMapTile)
		InitPixMap (image);	
	#endif
	#ifdef MSWIN
	if (DitherWindowPalette)
	{
		SelectPalette(location, DitherWindowPalette, false);
		RealizePalette(location);
	}
	#endif
	uint tile_index_x = image->GetTileColumnIndex(
		zoom_info.WindowToImage(draw_x + pan_x));
	uint tile_index_width = image->GetTileColumnIndex(
		zoom_info.WindowToImage(draw_x + draw_width + pan_x) - 1) -
		tile_index_x + 1;
	uint tile_index_y = image->GetTileRowIndex(
		zoom_info.WindowToImage(draw_y + pan_y));
	uint tile_index_height = image->GetTileRowIndex(
		zoom_info.WindowToImage(draw_y + draw_height + pan_y) - 1) -
		tile_index_y + 1;
	for (uint row = tile_index_y; row < tile_index_y + tile_index_height;
		row++)
	{
		int tile_y = (int)image->GetTileStartY(row);
		int tile_height = (int)(image->GetTileEndY(row) - tile_y + 1);
		int loc_y = (int)(zoom_info.ImageToWindow(image->GetImageStartY(row)) -
			pan_y);
		int loc_height = (int)(zoom_info.ImageToWindow(
			image->GetImageEndY(row) + 1) - pan_y) - loc_y;
		int top_margin = 0;
		int bottom_margin = 0;
		// make sure we are drawing whole blocks.  No need to do this on mac,
		// as it has good enough clipping to handle it
		#ifndef MACOS
		if (loc_y < draw_y)
		{
			loc_height -= (draw_y - loc_y);
			loc_y = draw_y;
			int new_tile_y = (int)(zoom_info.WindowToImage(
				(int32)loc_y + pan_y) - (int32)image->GetImageStartY(row));
			tile_height -= new_tile_y;
			tile_y += new_tile_y;
			do
			{
				loc_y = (int)(zoom_info.ImageToWindow((int32)tile_y -
					(int32)image->GetTileStartY(row) +
					(int32)image->GetImageStartY(row)) - pan_y);
				if (loc_y >= draw_y)
					break;
				tile_y++;
				tile_height--;
			} while (1);
			loc_height -= (loc_y - draw_y);
			top_margin += loc_y - draw_y;
		}
		if (tile_height <= 0 || loc_height <= 0)
			continue; // should never happen, but better safe then sorry
		if (loc_y + loc_height > draw_y + draw_height)
		{
			loc_height = draw_y + draw_height - loc_y;
			tile_height = (int)(zoom_info.WindowToImage(
				(int32)(loc_y + loc_height) + pan_y) -
				(int32)image->GetImageStartY(row)) -
				(tile_y - (int)image->GetTileStartY(row));
			int old_loc_height = loc_height;
			do
			{
				loc_height = (int)(zoom_info.ImageToWindow((int32)tile_y +
					(int32)tile_height -
					(int32)image->GetTileStartY(row) +
					(int32)image->GetImageStartY(row)) - pan_y) - loc_y;
				if (loc_y + loc_height <= draw_y + draw_height)
					break;
				tile_height--;
			} while (1);
			bottom_margin += old_loc_height - loc_height;
		}
		#endif	
		if (tile_height <= 0 || loc_height <= 0)
			continue; // should never happen, but better safe then sorry
		for (uint column = tile_index_x;
			column < tile_index_x + tile_index_width;
			column++)
		{
			int tile_x = (int)image->GetTileStartX(column);
			int tile_width = (int)(image->GetTileEndX(column) - tile_x + 1);
			int loc_x = (int)(zoom_info.ImageToWindow(
				image->GetImageStartX(column)) - pan_x);
			int loc_width = (int)(zoom_info.ImageToWindow(
				image->GetImageEndX(column) + 1) - pan_x) - loc_x;
			int left_margin = 0;
			int right_margin = 0;
			#ifndef MACOS
			if (loc_x < draw_x)
			{
				loc_width -= (draw_x - loc_x);
				loc_x = draw_x;
				int new_tile_x = (int)(zoom_info.WindowToImage(
					(int32)loc_x + pan_x) - (int32)image->GetImageStartX(column));
				tile_width -= (new_tile_x);
				tile_x += new_tile_x;
				do
				{
					loc_x = (int)(zoom_info.ImageToWindow((int32)tile_x -
						(int32)image->GetTileStartX(column) +
						(int32)image->GetImageStartX(column)) - pan_x);
					if (loc_x >= draw_x)
						break;
					tile_x++;
					tile_width--;
				} while (1);
				loc_width -= (loc_x - draw_x);
				left_margin += loc_x - draw_x;
			}
			if (tile_width <= 0 || loc_width <= 0)
				continue; // should never happen, but better safe then sorry
			if (loc_x + loc_width > draw_x + draw_width)
			{
				loc_width = draw_x + draw_width - loc_x;
				tile_width = (int)(zoom_info.WindowToImage(
					(int32)(loc_x + loc_width) + pan_x) -
					(int32)image->GetImageStartX(column)) -
					(tile_x - (int)image->GetTileStartX(column));
				int old_loc_width = loc_width;
				do
				{
					loc_width = (int)(zoom_info.ImageToWindow((int32)tile_x +
						(int32)tile_width -
						(int32)image->GetTileStartX(column) +
						(int32)image->GetImageStartX(column)) - pan_x) - loc_x;
					if (loc_x + loc_width <= draw_x + draw_width)
						break;
					tile_width--;
				} while (1);
				right_margin += old_loc_width - loc_width;
			}
			#endif
			if (tile_width <= 0 || loc_width <= 0)
				continue; // should never happen, but better safe then sorry
			G42ImageTile * tile = image->GetTile(row, column);
			if (!tile || tile->GetNeedUpdate())
			{
				if (backup_image)
				{
					DrawImage(location, backup_image, loc_x, loc_y, loc_width, loc_height,
						backup_zoom_info, pan_x, pan_y, 0, backup_zoom_info);
				}
				continue;
			}
			SetImageInformation(image);
			DrawImageTile(location, loc_x, loc_y, loc_width, loc_height,
				tile, tile_x, tile_y, tile_width, tile_height, image->GetData(),
				image->GetMask(), pan_x, pan_y, image);
			#ifdef MSWIN
			if (top_margin || bottom_margin || left_margin || right_margin)
			{
				HBRUSH sbrush = CreateSolidBrush(RGB(192, 192, 192));
				HBRUSH old_brush = (HBRUSH)SelectObject(location, sbrush);
				HPEN old_pen = (HPEN)SelectObject(location, GetStockObject(NULL_PEN));
				if (top_margin && !draw_y)
				{
					Rectangle(location, loc_x - left_margin, loc_y - top_margin,
						loc_x + loc_width + 1 + right_margin, loc_y + 1);
				}
				if (bottom_margin && (int32)draw_y + (int32)draw_height ==
					(int32)WindowHeight)
				{
					Rectangle(location, loc_x - left_margin, loc_y + loc_height,
						loc_x + loc_width + 1 + right_margin,
						loc_y + loc_height + bottom_margin + 1);
				}
				if (left_margin && !draw_x)
				{
					Rectangle(location, loc_x - left_margin, loc_y,
						loc_x + 1, loc_y + loc_height + 1);
				}
				if (right_margin && (int32)draw_x + (int32)draw_width ==
					(int32)WindowWidth)
				{
					Rectangle(location, loc_x + loc_width,
						loc_y, loc_x + loc_width + right_margin + 1,
						loc_y + loc_height + 1);
				}
				SelectObject(location, old_brush);
				SelectObject(location, old_pen);
				DeleteObject(sbrush);
			}
			#endif
		}
	}
}
void
G42SimpleImageViewer::DrawImageTile(G42DrawLocation location,
	int loc_x, int loc_y, int loc_width, int loc_height,
	G42ImageTile * itile, int tile_x, int tile_y,
	int tile_width, int tile_height, G42ImageData * image,
	G42ImageData * mask, int32 pan_x, int32 pan_y, G42Image *theImage)
{
	G42LockedImageTile tile(itile);
	#ifdef MSWIN
	byte * tile_ptr = *(tile.GetTile()->GetRow(
		tile_y + tile_height - 1));
	if (!tile_ptr)
		return;
	DibHeader->biWidth = (int)tile.GetTile()->GetWidth();
	DibHeader->biHeight = tile_height;
	DibHeader->biSizeImage = (DWORD)tile_height *
		(DWORD)tile.GetTile()->GetRowBytes();
	if (mask)
	{
		int32 image_x = (int32)tile_x +
			(int32)tile.GetTile()->GetImageOffsetX();
		int32 image_y = (int32)tile_y +
			(int32)tile.GetTile()->GetImageOffsetY();
		int32 image_width = (int32)tile_width;
		int32 image_height = (int32)tile_height;
		HBITMAP bmp1 = CreateCompatibleBitmap(location, loc_width,
			loc_height);
		HBITMAP bmp2 = CreateCompatibleBitmap(location, loc_width,
			loc_height);
		HDC mdc = CreateCompatibleDC(location);
		if (DitherWindowPalette)
		{
			SelectPalette(mdc, DitherWindowPalette, false);
			RealizePalette(mdc);
		}
		SetStretchBltMode(mdc, STRETCH_DELETESCANS);
		SetBkMode(mdc, TRANSPARENT);
		HBITMAP old_bmp = (HBITMAP)SelectObject(mdc, bmp1);
		StretchDIBits(mdc,
			0, 0,
			loc_width, loc_height,
			tile_x, 0,
			tile_width, tile_height,
			tile_ptr, (BITMAPINFO *)DibHeader, DibUsage, SRCCOPY);
		DrawMask(mdc, mask, 0, 0, loc_width, loc_height,
			image_x, image_y, image_width, image_height);
		SelectObject(mdc, bmp2);
		HBRUSH sbrush = CreateSolidBrush(RGB(192, 192, 192));
		HBRUSH old_brush = (HBRUSH)SelectObject(mdc, sbrush);
		SelectObject(mdc, GetStockObject(NULL_PEN));
		Rectangle(mdc, 0, 0, loc_width + 1, loc_height + 1);
		SelectObject(mdc, old_brush);
		DeleteObject(sbrush);
#if defined(WIN32)
		::SetBrushOrgEx(mdc,
			7 - (int)((pan_x + (int32)loc_x) & 7),
			7 -(int)((pan_y + (int32)loc_y) & 7), 0);
#else
		::SetBrushOrg(mdc, (int)((pan_x + (int32)loc_x) & 7),
			(int)((pan_y + (int32)loc_y) & 7));
#endif
		HBRUSH pbrush = CreateHatchBrush(HS_BDIAGONAL,
			RGB(128, 128, 128));
		old_brush = (HBRUSH)SelectObject(mdc, pbrush);
		Rectangle(mdc, 0, 0, loc_width + 1, loc_height + 1);
		SelectObject(mdc, old_brush);
		DeleteObject(pbrush);
		DrawMask(mdc, mask, 0, 0, loc_width, loc_height,
			image_x, image_y, image_width, image_height, true);
		HDC mdc2 = CreateCompatibleDC(location);
		SetStretchBltMode(mdc2, STRETCH_DELETESCANS);
		if (DitherWindowPalette)
		{
			SelectPalette(mdc2, DitherWindowPalette, false);
			RealizePalette(mdc2);
		}
		HBITMAP old_bmp2 = (HBITMAP)SelectObject(mdc2, bmp1);
		BitBlt(mdc, 0, 0, loc_width, loc_height, mdc2,
			0, 0, SRCPAINT);
		BitBlt(location, loc_x, loc_y,
			loc_width, loc_height,
			mdc, 0, 0, SRCCOPY);
		SelectObject(mdc2, old_bmp2);
		DeleteDC(mdc2);
		DeleteObject(bmp2);
		SelectObject(mdc, old_bmp);
		DeleteDC(mdc);
		DeleteObject(bmp1);
	}
	else
	{
		if (tile_width == loc_width && tile_height == loc_height)
		{
			SetDIBitsToDevice(location,
				loc_x, loc_y,
				tile_width, tile_height,
				tile_x, 0,
				0, tile_height, tile_ptr,
				(BITMAPINFO *)DibHeader, DibUsage);
		}
		else
		{
			StretchDIBits(location,
				loc_x, loc_y,
				loc_width, loc_height,
				tile_x, 0,
				tile_width, tile_height,
				tile_ptr, (BITMAPINFO *)DibHeader, DibUsage, SRCCOPY);
		}
	}
	#endif
	#ifdef MACOS
	(**PixMapTile).baseAddr = (Ptr) (byte *) *(tile.GetTile()->GetRow (0));
	Rect sourceRect, destRect;
	/*  Set up rects */
	::SetRect (&sourceRect, tile_x, tile_y, tile_x + tile_width, tile_y + tile_height);
	::SetRect (&destRect, loc_x, loc_y, loc_x + loc_width, loc_y + loc_height);
	PixMapHandle sourcePixmap = PixMapTile;
	PixMapHandle destPixmap = ::GetGWorldPixMap (location);	
	// destPixmap == 0 means we're not printing
	if (destPixmap)
		if (mask)	
		{
			short offsetX = ((pan_x + (int32)loc_x) & 7); // Make patterns match across tiles
			short offsetY = ((pan_y + (int32)loc_y) & 7);
			DrawTileMask (location, image, tile_x, tile_y, tile_width, tile_height, 
				offsetX, offsetY, tile, mask, sourceRect, destRect);	
		}
		else
			DrawTileNormal (location, image, sourceRect, destRect);
	else
			DrawTilePrint (location, image, sourceRect, destRect, tile, theImage);
				
	#endif		// MACOS
}
#ifdef MACOS
void
G42SimpleImageViewer::DrawTileNormal (G42DrawLocation location, 
	G42ImageData *image, Rect &sourceRect, 
	Rect &destRect)
{
	/*  Now put the source image into the draw location  */
	short transferMode = 	//	(If grayscale / monochrome image, need to invert)
		(image -> GetDepth() <= 8 && image -> GetNumPalette() == 0) ? notSrcCopy : srcCopy;
	short sourceDepth 	= (**PixMapTile).cmpCount * (**PixMapTile).cmpSize;
	short destDepth 	= sourceDepth;
	PixMapHandle destPixmap = ::GetGWorldPixMap (location);
	if (destPixmap)	// Normal onscreen or gworld drawing
	{
		destDepth = (**destPixmap).cmpCount * (**destPixmap).cmpSize;
		#if 0
		if (sourceDepth > 8 && destDepth <= 8)
		#else
		if (sourceDepth > destDepth)
		#endif
			transferMode |= ditherCopy;
		
	}
	else			// No pixmap: for printing
		transferMode |= ditherCopy;
	::LockPixels (PixMapTile);
	if (destPixmap)
		::LockPixels (location -> portPixMap);
	::CopyBits ((BitMapPtr) *PixMapTile, &WINBITMAP (location),
		&sourceRect, &destRect, transferMode, NULL);
	if (destPixmap)
		::UnlockPixels (location -> portPixMap);
	::UnlockPixels (PixMapTile);
}
void
G42SimpleImageViewer::DrawTilePrint (G42DrawLocation location, 
	G42ImageData *image, Rect &sourceRect, Rect &destRect,
	G42LockedImageTile &tile, G42Image *theImage)
{
	
	int32 image_x = (int32)sourceRect.left +
		(int32)tile.GetTile()->GetImageOffsetX();
	int32 image_y = (int32) sourceRect.top +
		(int32)tile.GetTile()->GetImageOffsetY();
	int tile_width = sourceRect.right - sourceRect.left;
	int tile_height = sourceRect.bottom - sourceRect.top;
	G42Image *tileImage = new G42Image (*theImage, image_x, image_y, tile_width, tile_height);
	#if 0
	if (! tileImage -> IsValid())
		SysBeep (0);
	#endif
	PicHandle pict = G42MacFile::PICTFromG42Image (tileImage, this, true);
	GWorldPtr oldPort;
	GDHandle oldDevice;
	::GetGWorld  (&oldPort, &oldDevice);
	::SetGWorld ((GWorldPtr) location, nil);	
	::DrawPicture (pict, &destRect);
	::SetGWorld ((GWorldPtr) oldPort, oldDevice);	
	::KillPicture (pict);
	delete tileImage;
	#if 0
	/*  Now put the source image into the draw location  */
	short transferMode = 	//	(If grayscale image, need to invert)
		(image -> GetDepth() <= 8 && image -> GetNumPalette() == 0) ? notSrcCopy : srcCopy;
	short sourceDepth 	= (**PixMapTile).cmpCount * (**PixMapTile).cmpSize;
	short destDepth 	= sourceDepth;
	PixMapHandle destPixmap = ::GetGWorldPixMap (location);
	transferMode += ditherCopy;
		
	::LockPixels (PixMapTile);
	if (destPixmap)
		::LockPixels (location -> portPixMap);
	::CopyBits ((BitMapPtr) *PixMapTile, &WINBITMAP (location),
		&sourceRect, &destRect, transferMode, NULL);
	if (destPixmap)
		::UnlockPixels (location -> portPixMap);
	::UnlockPixels (PixMapTile);
	#endif
}
void
G42SimpleImageViewer::DrawTileMask (G42DrawLocation location, 
	G42ImageData *image, int tile_x, int tile_y, 
	int tile_width, int tile_height, short offsetX, short offsetY,
	G42LockedImageTile &tile, G42ImageData *mask, 
	Rect &sourceRect, Rect &destRect)
{
	Rect stretchedRect1;
	::SetRect (&stretchedRect1, 0, 0, destRect.right - destRect.left, destRect.bottom - destRect.top);
	
	/*  Step 1:  Create the (source) mask bitmap */
	int32 maskX1 = (int32) tile_x -
		(int32) tile.GetTile() -> GetTileOffsetX() +
		(int32) tile.GetTile() -> GetImageOffsetX();
	int32 maskY1 = (int32) tile_y -
		(int32) tile.GetTile() -> GetTileOffsetY() +
		(int32) tile.GetTile() -> GetImageOffsetY();
	int32 maskWidth1  	= (int32) tile_width;
	int32 maskHeight1 	= (int32) tile_height;
	int rowBytes1 		= Width2Bytes (maskWidth1, 1);
	int maskBytes1   	= rowBytes1 * maskHeight1;
	char *maskData1 	= new char [maskBytes1];
	ThrowIfNil_ (maskData1);
	BitMap maskBitmap1;
	maskBitmap1.baseAddr 	= maskData1;
	maskBitmap1.rowBytes 	= rowBytes1;
	maskBitmap1.bounds		= sourceRect;
	FillMask ((byte *) maskData1, mask, rowBytes1, maskX1, maskY1, maskWidth1, 
		maskHeight1, true);
		
	/*  Now stretch mask bitmap  */
	int maskWidth2  = destRect.right - destRect.left;
	int maskHeight2 = destRect.bottom - destRect.top;
	int rowBytes2 	= Width2Bytes (maskWidth2, 1);
	int maskBytes2  = rowBytes2 * maskHeight2;
	char *maskData2 = new char [maskBytes2];
//	memset (maskData2, 0xFF, maskBytes2);
	ThrowIfNil_ (maskData2);
	BitMap maskBitmap2;
	maskBitmap2.baseAddr 	= maskData2;
	maskBitmap2.rowBytes 	= rowBytes2;
	maskBitmap2.bounds		= stretchedRect1;
	::CopyBits (&maskBitmap1, &maskBitmap2, &sourceRect, &stretchedRect1, nil, nil);
	/*  All done with mask1  */
	delete [] maskData1;
#if 0
::CopyBits (&maskBitmap2, &WINBITMAP (location),
	&stretchedRect1, &destRect, srcCopy, NULL);
#endif
	/*  Step 3:  Create a GWorld for stretched tile (at location depth)  */
	PixMapHandle pixmap = GetGWorldPixMap (location);
	short depth = (**pixmap).cmpSize * (**pixmap).cmpCount;
	switch (depth)
	{
		case 15:
			depth = 16;
		break;
		case 24:
			depth = 32;
		break;
	}
	/*  Step 4:  Create a GWorld for stretched / patterned tile  */
	Rect stretchedRect2 = stretchedRect1;
	::OffsetRect (&stretchedRect2, offsetX, offsetY);
	G42DrawLocation maskLocation;
	QDErr err = ::NewGWorld (&maskLocation, depth, &stretchedRect2, nil, nil, useTempMem);
	ThrowIfError_ (err);
	/*  Step x:  Erase background of GWorld  */
	G42DrawLocation oldLocation;
	GDHandle oldDevice;
	#ifdef USE_ERASERECT
	::GetGWorld (&oldLocation, &oldDevice);
	::SetGWorld (maskLocation, nil);	
	::EraseRect (&stretchedRect2);
	::SetGWorld (oldLocation, oldDevice);
	#endif
	short transferMode = 	//	(If grayscale / monochrome image, need to invert)
		(image -> GetDepth() <= 8 && image -> GetNumPalette() == 0) ? notSrcCopy : srcCopy;
	/*  Step x: Copy the source pixmap into that GWorld  */
	::LockPixels (PixMapTile);
	::LockPixels (maskLocation -> portPixMap);
	::CopyBits ((BitMapPtr) *PixMapTile, &WINBITMAP (maskLocation),
		&sourceRect, &stretchedRect2, transferMode + ditherCopy, NULL);
#if 0
::CopyBits (&WINBITMAP (maskLocation), &WINBITMAP (location),
	&stretchedRect2, &destRect, srcCopy, NULL);
#endif
	::UnlockPixels (maskLocation -> portPixMap);
	::UnlockPixels (PixMapTile);
	/*  Step 5:  Focus location and draw the rectangle as a pattern  */
	::GetGWorld (&oldLocation, &oldDevice);
	::SetGWorld (location, nil);
	Pattern pattern;
	::GetIndPattern (&pattern, sysPatListID, 28); 
	Rect patternRect = destRect;
	::FillRect (&patternRect, &pattern);
	::SetGWorld (oldLocation, oldDevice);
	/*  Step 6:  Copymask stretched tile using stretched mask bitmap  */
	::LockPixels (PixMapTile);
	::LockPixels (::GetGWorldPixMap (maskLocation));		
	::CopyMask (&WINBITMAP (maskLocation), &maskBitmap2, &WINBITMAP (location),
	 	&stretchedRect2, &stretchedRect1, &destRect);
	::UnlockPixels (::GetGWorldPixMap (maskLocation));
	::UnlockPixels (PixMapTile);
	/*  All done with mask2 and maskLocation  */
	delete [] maskData2;
	::DisposeGWorld (maskLocation);
}
int
G42SimpleImageViewer::Width2Bytes (int width, short bitsPerPixel)
{
	int rowBytes = 0;
	switch (bitsPerPixel)
	{
		case 1:
			rowBytes = ((width + 7) >> 3);
			if (rowBytes & 3) 	// 32 bit boundaries
				rowBytes += (4 - (rowBytes & 3));		
		break;
	}
	return rowBytes;
}
#endif
void 
G42SimpleImageViewer::FillMask(byte * mask_data, G42ImageData * mask,
	int32 row_bytes,
	int32 mask_x, int32 mask_y, int32 mask_width, int32 mask_height,
	bool invert)
{
	/*  Temporary things  */
	int loc_x = 0;
	int loc_y = 0; 
	int loc_width  = mask_width;
	int loc_height = mask_height;
	int image_x = mask_x;
	int image_y = mask_y;
	int image_width = mask_width;
	int image_height = mask_height;
	G42ImageData * image = mask;
		int first_tile_y = image->GetTileRowIndex(image_y);
		int last_tile_y = image->GetTileRowIndex(image_y + image_height);
		int first_tile_x = image->GetTileColumnIndex(image_x);
		int last_tile_x = image->GetTileColumnIndex(image_x + image_width);
		int current_loc_x = loc_x;
		int current_loc_y = loc_y;
		int current_loc_width = loc_width;
		int current_loc_height = loc_height;
		int32 current_image_x = image_x;
		int32 current_image_y = image_y;
		int32 current_image_width = image_width;
		int32 current_image_height = image_height;
		for (int row = first_tile_y; row <= last_tile_y; row++)
		{
			current_loc_x = loc_x;
			current_loc_width = loc_width;
			current_image_x = image_x;
			current_image_width = image_width;
			
			int source_height;
			int dest_height;
			for (int column = first_tile_x; column <= last_tile_x; column++)
			{
				G42ImageTile * itile = image->GetTile(row, column);
				if (!itile)
					continue; // this should not happen either
				G42LockedImageTile tile(itile);
				int source_y = current_image_y - tile.GetTile()->GetImageOffsetY()
					+ tile.GetTile()->GetTileOffsetY();
				int dest_y = current_loc_y;
				source_height = current_image_height;
				dest_height = current_loc_height;
				if ((source_y + source_height - tile.GetTile()->GetTileOffsetY()) > tile.GetTile()->GetTileHeight())
				{
					source_height = tile.GetTile()->GetTileHeight()
						+ tile.GetTile()->GetTileOffsetY() - source_y;
					dest_height = (int)(((int32)source_height * (int32)loc_height) / (int32)image_height);
				}
				int source_x = current_image_x - tile.GetTile()->GetImageOffsetX()
					+ tile.GetTile()->GetTileOffsetX();
				int dest_x = current_loc_x;
				int source_width = current_image_width;
				int dest_width = current_loc_width;
				if ((source_x + source_width - tile.GetTile()->GetTileOffsetX()) > tile.GetTile()->GetTileWidth())
				{
					source_width = tile.GetTile()->GetTileWidth()
						+ tile.GetTile()->GetTileOffsetX() - source_x;
					dest_width = (int)(((int32)source_width * (int32)loc_width) / (int32)image_width);
				}
				#if 0
#ifdef MSWIN
				// MSWIN does things upside down
				byte * tile_ptr = *(tile.GetTile()->GetRow(
					source_y + source_height - 1));
#else
				byte * tile_ptr = *(tile.GetTile()->GetRow(source_y));
#endif
				dib_header->bmiHeader.biWidth = (int)image->GetTileWidth();
				dib_header->bmiHeader.biHeight = source_height;
				dib_header->bmiHeader.biSizeImage = (DWORD)source_height *
					(DWORD)tile.GetTile()->GetRowBytes();
				StretchDIBits(mdc,
					dest_x, dest_y,
					dest_width, dest_height,
					source_x, 0,
					source_width, source_height,
					tile_ptr, dib_header, DIB_RGB_COLORS, SRCAND);
				#endif
				byte * mask_ptr = mask_data + row_bytes * dest_y +
					(dest_x >> 3);
				int start_mm = (1 << (7 - (source_x & 7)));
				for (int y = 0; y < dest_height; y++)
				{
					if (start_mm == 0x80)
					{
						memcpy(mask_ptr, (byte *)(*(tile.GetTile()->GetRow(
							source_y + y))) + (source_x >> 3),
							((source_width + 7) >> 3));
					}
					else
					{
						byte * dp2 = mask_ptr;
						int mm = start_mm;
						int dm = 0x80;
						byte * mp2 = (byte *)(*(tile.GetTile()->GetRow(
							source_y + y))) + (source_x >> 3);
						for (int x = 0; x < source_width; x++)
						{
							if (mm & (*mp2))
								*dp2 |= dm;
							else
								*dp2 &= ~dm;
							if (mm == 1)
							{
								mm = 0x80;
								mp2++;
							}
							else
							{
								mm >>= 1;
							}
							if (dm == 1)
							{
								dm = 0x80;
								dp2++;
							}
							else
							{
								dm >>= 1;
							}
						}
					}
					//memset(mask_ptr, ((0xaaaa >> (y & 7)) & 0xff), 
					//	(tile_width >> 3));
					if (invert)
					{
						for (int i = 0; i < ((source_width + 7) >> 3); i++)
							mask_ptr[i] = ~mask_ptr[i];
					}
					mask_ptr += row_bytes;
				}
				current_image_x += source_width;
				current_image_width -= source_width;
				current_loc_x += dest_width;
				current_loc_width -= dest_width;
			}
			current_image_y += source_height;
			current_image_height -= source_height;
			current_loc_y += dest_height;
			current_loc_height -= dest_height;
		}
//		delete [] (byte *)dib_header;
}
#ifdef MACOS
void
G42SimpleImageViewer::InitPixMap (G42Image * image)
{
	/*  Create a GWorld from tiles information in Image  */
	G42ImageTile * tile = image -> GetTile (0, 0);					
	#ifdef NOTYET
	G42LockedImageTile tile (image->GetTile(0, 0));
	#endif
	Rect 	rect;
	::SetRect (&rect, 0, 0, image -> GetTileWidth(), image -> GetTileHeight());
	/*  Determine the image depth  */
	int BitsPerPixel = image -> GetInfo().PixelDepth;
	RowBytes = tile -> GetRowBytes();
	int Depth = BitsPerPixel;
	if (Depth > 8)
		Depth = 32;
	else 
		if (Depth > 4)
			Depth = 8;
			
	/*  Set up the PixMap  */
	PixMapTile = NewPixMap ();
	PixMapPtr pixMapPtr = *PixMapTile;
		
	pixMapPtr -> baseAddr 		= 0;
	pixMapPtr -> rowBytes 		= (1L << 15) | RowBytes;
	pixMapPtr -> bounds 		= rect;
	pixMapPtr -> pmVersion		= 0;	// 4?
	pixMapPtr -> packType		= 0;
	pixMapPtr -> packSize		= 0;
	pixMapPtr -> hRes			= 72;
	pixMapPtr -> vRes			= 72;
	/*  Portions complements of Karl.. */
	if (Depth <= 8) 
	{
		pixMapPtr -> pixelType		= 0;
		pixMapPtr -> pixelSize		= Depth;
		pixMapPtr -> cmpCount		= 1;		
		pixMapPtr -> cmpSize		= Depth;
	}	
	else 
	{
		pixMapPtr -> pixelType		= RGBDirect;
		pixMapPtr -> pixelSize		= Depth;
		pixMapPtr -> cmpCount		= 3;		
		pixMapPtr -> cmpSize		= 8;		
	}			
	pixMapPtr -> planeBytes		= 0;
	pixMapPtr -> pmReserved		= 0;
	/*  Kill the palette in the PixMap */
	DisposeCTable((**PixMapTile).pmTable);	
	pixMapPtr -> pmTable		= 0;
	/*  Use the original palette stuff  */
	int numPalette = image -> GetInfo().NumPalette;
	if (numPalette != 0)
	{
		CTabHandle ctab = GetCTable (8);
		ColorSpec *specs;
			
		specs = (**ctab).ctTable;
		(**ctab).ctSize = numPalette - 1;
		const G42Color *colors = image -> GetInfo().ImagePalette;
		for (int i = 0; i < numPalette; i++)
		{
			specs [i].rgb.red 	= (colors -> red << 8);
			specs [i].rgb.green = (colors -> green << 8);
			specs [i].rgb.blue 	= (colors -> blue << 8);
			specs [i].value = i;
			colors++;
		}
		(**ctab).ctSeed = GetCTSeed();
		pixMapPtr -> pmTable = ctab;	
	}	
}
#endif	// MACOS
void 
G42SimpleImageViewer::ImageChanged(void)
{
	#ifdef MACOS
	ChangedForDraw = true;
	#endif
}
#ifdef MSWIN
void 
G42SimpleImageViewer::SetImageInformation(G42Image * image)
{
	if (image)
	{
		if (image->HasInfo())
		{
			DibHeader->biBitCount = (WORD)image->GetInfo().PixelDepth;
			if (image->GetInfo().ColorType ==
				G42ImageInfo::ColorTypePalette)
			{
				DibHeader->biClrUsed = image->GetInfo().NumPalette;
				RGBQUAD * dib_pal = (RGBQUAD *)((byte *)DibHeader +
					sizeof (BITMAPINFOHEADER));
				const G42Color * pal = image->GetInfo().ImagePalette;
				for (int i = 0; i < image->GetInfo().NumPalette; i++)
				{
					dib_pal[i].rgbRed = pal[i].red;
					dib_pal[i].rgbGreen = pal[i].green;
					dib_pal[i].rgbBlue = pal[i].blue;
					dib_pal[i].rgbReserved = 0;
				}
//				DibUsage = DIB_PAL_COLORS;
			}
			else if (image->GetInfo().ColorType ==
				G42ImageInfo::ColorTypeGrayscale)
			{
				DibHeader->biClrUsed = (1 << image->GetInfo().PixelDepth);
				RGBQUAD * dib_pal = (RGBQUAD *)((byte *)DibHeader +
					sizeof (BITMAPINFOHEADER));
				for (int i = 0; i < DibHeader->biClrUsed; i++)
				{
					dib_pal[i].rgbRed = dib_pal[i].rgbGreen =
						dib_pal[i].rgbBlue = (byte)(((int32)i * (int32)255) /
						(int32)(DibHeader->biClrUsed - 1));
					dib_pal[i].rgbReserved = 0;
				}
			}
			else
			{
				DibHeader->biClrUsed = 0;
			}
		}
	}
}
#endif
#ifdef MACOS
void 
G42SimpleImageViewer::SetImageInformation(G42Image *)
{
}
#endif
void G42SimpleImageViewer::ImageBeingDeleted(G42Image *)
{
}
void G42SimpleImageViewer::HaveImageEnd(void)
{
}
void G42SimpleImageViewer::HaveImageInfo(void)
{
}
void G42SimpleImageViewer::HaveImageRow(uint32)
{
}
void G42SimpleImageViewer::SetWindowSize(int width, int height)
{
	WindowWidth = width;
	WindowHeight = height;
}
void G42SimpleImageViewer::SetWindowOffset(int32 x, int32 y)
{
	WindowX = x;
	WindowY = y;
}
#ifdef MSWIN
void G42SimpleImageViewer::WindowSizeChanged(void)
{
	RECT rect;
	::GetClientRect(Window, &rect);
	WindowWidth = rect.right - rect.left;
	WindowHeight = rect.bottom - rect.top;
}
// start ges256
bool
G42SimpleImageViewer::QueryNewPalette(void)
{
	if (!DitherWindowPalette)
		return false;
	HDC dc = GetDC(TopWindow);
	SelectPalette(dc, DitherWindowPalette, false);
	RealizePalette(dc);
	ReleaseDC(TopWindow, dc);
	return true;
}
// end ges256
void
G42SimpleImageViewer::PaletteChanged(HWND who)
{
	if (who == Window || who == TopWindow) // don't respond to ourselves
		return;
	if (DitherWindowPalette)
	{
		HDC dc = GetDC(TopWindow);
		SelectPalette(dc, DitherWindowPalette, false);
		RealizePalette(dc);
		ReleaseDC(TopWindow, dc);
	}
}
#endif
/*  Future TODO Here:  Create a class similar to G42ImageInfo, but smaller
    so we don't eat up any more memory than necessary  D. Ison */
#ifdef MACOS
bool G42SimpleImageViewer::SameAsPrevImage (G42Image *image)
{
	bool ret_val = true;
	if (ChangedForDraw)
	{
		ret_val = false;
		ChangedForDraw = false;
	}
	else
	if (image -> GetInfo().ColorType != PrevImageInfo.ColorType)		
		ret_val = false;
	else
		if (image -> GetInfo().Width != PrevImageInfo.Width)		
			ret_val = false;
		else
			if (image -> GetInfo().Height != PrevImageInfo.Height)		
				ret_val = false;
			else
				if (image -> GetInfo().PixelDepth != PrevImageInfo.PixelDepth)		
					ret_val = false;
				else
					if (image -> GetInfo().NumPalette != PrevImageInfo.NumPalette)		
						ret_val = false;				
					
	return (ret_val);
}
#endif
void
G42ZoomInfo::Set(double zoom, uint32 width, uint32 height)
{
	if (!width)
		width = 100000;
	if (!height)
		height = 100000;
		
	if ((int32)(zoom * (double)width + .5) == width ||
		(int32)(zoom * (double)height + .5) == height)
	{
		ZoomMode = NotZoomed;
		ZoomFactor = 1.0;
	}
	else if (zoom > 1.0)
	{
#ifdef MSWIN
		if (zoom > 128.0)
			zoom = 128.0;
#endif			
		ZoomInt = (zoom + .5);
		if (((int32)ZoomInt * width) == (int32)(zoom * (double)width + .5) &&
			((int32)ZoomInt * height) == (int32)(zoom * (double)height + .5))
		{
			ZoomFactor = ZoomInt;
			int s = 1;
			while ((1 << s) < ZoomInt)
				s++;
			if ((1 << s) == ZoomInt)
			{
				ZoomMode = ZoomShiftNormal;
				ZoomInt = s;
			}
			else
			{
				ZoomMode = ZoomIntNormal;
			}
		}
		else
		{
			ZoomMode = Zoomed;
			ZoomFactor = zoom;
		}
	}
	else
	{
		ZoomInt = (1.0 / zoom + .5);
//		if (zoom == 1.0 / (double)ZoomInt)
		if ((width / (int32)ZoomInt) == (int32)(zoom * (double)width + .5) &&
			(height / (int32)ZoomInt) == (int32)(zoom * (double)height + .5))
		{
			ZoomFactor = 1.0 / (double)ZoomInt;
			int s = 1;
			while ((1 << s) < ZoomInt)
				s++;
			if ((1 << s) == ZoomInt)
			{
				ZoomMode = ZoomShiftInverted;
				ZoomInt = s;
			}
			else
			{
				ZoomMode = ZoomIntInverted;
			}
		}
		else
		{
			ZoomMode = Zoomed;
			ZoomFactor = zoom;
		}
	}
}
void
G42ZoomInfo::Set(G42ZoomMode zoom_mode,
	int zoom_int, double zoom_factor)
{
	ZoomMode = zoom_mode;
	ZoomInt = zoom_int;
	ZoomFactor = zoom_factor;
}
G42ZoomInfo::G42ZoomInfo(G42ZoomMode zoom_mode,	int zoom_int,
	double zoom_factor)
{
	Set(zoom_mode, zoom_int, zoom_factor);
}
G42ZoomInfo::G42ZoomInfo(double zoom_factor, uint32 width, uint32 height)
{
	Set(zoom_factor, width, height);
}
int32
G42ZoomInfo::WindowToImage(int32 value) const
{
	int32 ret;
	bool negitive;
	if (value >= 0)
	{
		negitive = false;
	}
	else
	{
		negitive = true;
		value = -value;
	}
	switch (ZoomMode)
	{
		case NotZoomed:
			ret = value;
			break;
		case ZoomShiftNormal:
			ret = (value >> ZoomInt);
			break;
		case ZoomShiftInverted:
			ret = (value << ZoomInt);
			break;
		case ZoomIntNormal:
			ret = (value / ZoomInt);
			break;
		case ZoomIntInverted:
			ret = (value * ZoomInt);
			break;
		default:
			ret = (int32)((double)value / ZoomFactor);
			break;
	}
	if (negitive)
		ret = -ret;
	return ret;
}
int32
G42ZoomInfo::ImageToWindow(int32 value) const
{
	int32 ret;
	bool negitive;
	if (value >= 0)
	{
		negitive = false;
	}
	else
	{
		negitive = true;
		value = -value;
	}
	switch (ZoomMode)
	{
		case NotZoomed:
			ret = value;
			break;
		case ZoomShiftNormal:
			ret = (value << ZoomInt);
			break;
		case ZoomShiftInverted:
			ret = (value >> ZoomInt);
			break;
		case ZoomIntNormal:
			ret = (value * ZoomInt);
			break;
		case ZoomIntInverted:
			ret = (value / ZoomInt);
			break;
		default:
			ret = (int32)((double)value * ZoomFactor);
			break;
	}
	if (negitive)
		ret = -ret;
	return ret;
}
#ifdef MACOS
//  This finds the depth of the deepest screen if there are more than one.
short
G42SimpleImageViewer::GetScreenDepth (void)
{
	GDHandle device = ::GetDeviceList ();
	short tempDepth, depth;
	depth = 0;
	while (device)
	{
		tempDepth = (**(**device).gdPMap).pixelSize;
		if (tempDepth > depth)
			depth = tempDepth;
		device = ::GetNextDevice (device);	
	}
	return (depth);		
}
#endif	// MACOS
#ifdef MACOS
short
G42SimpleImageViewer::PixmapDepth (short depth)
{
	short retValue = depth;
	switch (depth)
	{
		case 15:
			retValue = 16;
		break;
		case 24:
			retValue = 32;
		break;
	}
	return retValue;
}
#endif // MACOS
void
G42SimpleImageViewer::DrawMask(G42DrawLocation mdc, G42ImageData * image,
	int loc_x, int loc_y, int loc_width, int loc_height,
	int32 image_x, int32 image_y, int32 image_width, int32 image_height,
	bool invert)
{
#ifdef MSWIN
	if (image)
	{
		BITMAPINFO * dib_header = (BITMAPINFO *)new byte [
			sizeof (BITMAPINFOHEADER) + 256 * sizeof (RGBQUAD)];
		int row_bytes = (image_width >> 3);
		row_bytes = ((row_bytes + 3) & ~3);
		dib_header->bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
		dib_header->bmiHeader.biWidth = image_width;
		dib_header->bmiHeader.biHeight = image_height;
		dib_header->bmiHeader.biPlanes = 1;
		dib_header->bmiHeader.biBitCount = 1;
		dib_header->bmiHeader.biCompression = BI_RGB;
		dib_header->bmiHeader.biSizeImage =
			(int32)image_height * (int32)row_bytes;
		dib_header->bmiHeader.biXPelsPerMeter = 0;
		dib_header->bmiHeader.biYPelsPerMeter = 0;
		dib_header->bmiHeader.biClrUsed = 2;
		dib_header->bmiHeader.biClrImportant = 2;
		if (invert)
		{
			dib_header->bmiColors[0].rgbRed = 0;
			dib_header->bmiColors[0].rgbGreen = 0;
			dib_header->bmiColors[0].rgbBlue = 0;
			dib_header->bmiColors[0].rgbReserved = 0;
			dib_header->bmiColors[1].rgbRed = 0xff;
			dib_header->bmiColors[1].rgbGreen = 0xff;
			dib_header->bmiColors[1].rgbBlue = 0xff;
			dib_header->bmiColors[1].rgbReserved = 0;
		}
		else
		{
			dib_header->bmiColors[0].rgbRed = 0xff;
			dib_header->bmiColors[0].rgbGreen = 0xff;
			dib_header->bmiColors[0].rgbBlue = 0xff;
			dib_header->bmiColors[0].rgbReserved = 0;
			dib_header->bmiColors[1].rgbRed = 0;
			dib_header->bmiColors[1].rgbGreen = 0;
			dib_header->bmiColors[1].rgbBlue = 0;
			dib_header->bmiColors[1].rgbReserved = 0;
		}
		int32 block_x_start = image_x;
		if (block_x_start > 0 && block_x_start >=
			(int32)image->GetTileWidth() - (int32)image->GetFirstTileOffsetX())
		{
			block_x_start = ((int32)block_x_start +
				(int32)image->GetFirstTileOffsetX()) /
				(int32)image->GetTileWidth();
		}
		else
		{
			block_x_start = 0;
		}
		int32 block_y_start = image_y;
		if (block_y_start > 0 && block_y_start >=
			(int32)image->GetTileHeight() -
			(int32)image->GetFirstTileOffsetY())
		{
			block_y_start = ((int32)block_y_start +
				(int32)image->GetFirstTileOffsetY()) /
				(int32)image->GetTileHeight();
		}
		else
		{
			block_y_start = 0;
		}
		int32 block_x_end = image_x + image_width - 1;
		if (block_x_end > 0 && block_x_end >=
			(int32)image->GetTileWidth() - (int32)image->GetFirstTileOffsetX())
		{
			block_x_end = ((int32)block_x_end +
				(int32)image->GetFirstTileOffsetX()) /
				(int32)image->GetTileWidth();
		}
		else
		{
			block_x_end = 0;
		}
		int32 block_y_end = image_y + image_height - 1;
		if (block_y_end > 0 && block_y_end >=
			(int32)image->GetTileHeight() -
			(int32)image->GetFirstTileOffsetY())
		{
			block_y_end = ((int32)block_y_end +
				(int32)image->GetFirstTileOffsetY()) /
				(int32)image->GetTileHeight();
		}
		else
		{
			block_y_end = 0;
		}
		if (block_x_start < 0)
			block_x_start = 0;
		if (block_x_start >= (int32)image->GetNumTilesAcross())
			block_x_start = (int32)image->GetNumTilesAcross() - 1;
		if (block_x_end < 0)
			block_x_end = 0;
		if (block_x_end >= (int32)image->GetNumTilesAcross())
			block_x_end = (int32)image->GetNumTilesAcross() - 1;
		if (block_y_start < 0)
			block_y_start = 0;
		if (block_y_start >= (int32)image->GetNumTilesDown())
			block_y_start = (int32)image->GetNumTilesDown() - 1;
		if (block_y_end < 0)
			block_y_end = 0;
		if (block_y_end >= (int32)image->GetNumTilesDown())
			block_y_end = (int32)image->GetNumTilesDown() - 1;
#if 0
		int32 index_count = (block_y_end - block_y_start + 1) *
			(block_x_end - block_x_start + 1);
		if (index_count > 0 && index_count < 16000L)
			// if not, preloading won't work anyway (as if the memory manager would)
		{
			uint * index_array = new uint[(size_t)index_count];
			memset(index_array, 0, sizeof(uint) * (uint)index_count);
			uint size = 0;
			for (int y = (int)block_y_start; y <= (int)block_y_end; y++)
			{
				for (int x = (int)block_x_start; x <= (int)block_x_end; x++)
				{
					G42ImageTile * tile = image->GetTile(y, x);
					if (tile)
						index_array[size++] = tile->GetDataHandle();
					else
						index_array[size++] = 0;
				}
			}
			image->PreLoad(index_array, size);
			delete [] index_array;
		}
#endif
		for (int row = (int)block_y_start; row <= (int)block_y_end; row++)
		{
			int tile_height;
			int tile_start_y;
			int window_start_y;
			int window_height;
			if (row)
			{
				if ((int32)image_y > (int32)image->GetTileHeight() * (int32)row -
					(int32)image->GetFirstTileOffsetY())
				{
					tile_start_y = (int)((int32)image_y -
						(int32)(image->GetTileHeight() * (int32)row -
						(int32)image->GetFirstTileOffsetY()));
					window_start_y = (int)loc_y;
				}
				else
				{
					tile_start_y = 0;
					window_start_y = (int)(((((int32)tile_start_y +
						(int32)image->GetTileHeight() *
						(int32)row - (int32)image->GetFirstTileOffsetY() -
						(int32)image_y) * (int32)loc_height) /
						(int32)image_height) + (int32)loc_y);
				}
			}
			else
			{
				tile_start_y = (int)((int32)image->GetFirstTileOffsetY() +
					(int32)image_y);
				window_start_y = (int)(((((int32)tile_start_y - (int32)image_y) *
					(int32)loc_height) / (int32)image_height) + (int32)loc_y);
			}
			if (!row)
			{
				if (image->GetNumTilesDown() == 1)
				{
					tile_height = (int)((int32)image->GetLastTileHeight() -
						((int32)tile_start_y - (int32)image->GetFirstTileOffsetY()));
				}
				else
				{
					tile_height = (int)((int32)image->GetTileHeight() -
						(int32)tile_start_y);
				}
			}
			else if (row < (int)image->GetNumTilesDown() - 1)
			{
				tile_height = (int)((int32)image->GetTileHeight() -
					(int32)tile_start_y);
			}
			else
			{
				tile_height = (int)((int32)image->GetLastTileHeight() -
					(int32)tile_start_y);
			}
			if (row)
			{
				window_height = (int)(((((int32)tile_start_y + (int32)tile_height +
					(int32)image->GetTileHeight() * (int32)(row) -
					(int32)image->GetFirstTileOffsetY() - (int32)image_y) *
					(int32)loc_height) / (int32)image_height) + (int32)loc_y -
					(int32)window_start_y);
			}
			else
			{
				window_height = (int)(((((int32)tile_start_y + (int32)tile_height -
					(int32)image_y) *
					(int32)loc_height) / (int32)image_height) + (int32)loc_y -
					(int32)window_start_y);
			}
			if ((int32)window_start_y + (int32)window_height >
				(int32)loc_y + (int32)loc_height)
			{
				window_height = (int)((int32)loc_y + (int32)loc_height -
					(int32)window_start_y);
				tile_height = (int)(((((int32)window_start_y +
					(int32)window_height - (int32)loc_y) *
					(int32)image_height) / (int32)loc_height) +
					(int32)image_y - (int32)tile_start_y);
			}
			if ((int32)tile_start_y + (int32)tile_height >
				(int32)image->GetTileHeight())
			{
				tile_height = image->GetTileHeight() - tile_start_y;
				window_height = (int)(((((int32)tile_start_y +
					(int32)tile_height - (int32)image_y) *
					(int32)loc_height) / (int32)image_height) +
					(int32)loc_y - (int32)window_start_y);
			}
			if (window_height <= 0 || tile_height <= 0)
				continue;
			for (int column = (int)block_x_start;
				column <= (int)block_x_end; column++)
			{
				G42ImageTile * itile = image->GetTile(row, column);
				if (!itile)
					continue; // this should not happen either
				G42LockedImageTile tile(itile);
				int tile_width;
				int tile_start_x;
				int window_start_x;
				int window_width;
				if (column)
				{
					if ((int32)image_x > (int32)image->GetTileWidth() *
						(int32)column -
						(int32)image->GetFirstTileOffsetX())
					{
						tile_start_x = (int)((int32)image_x -
							(int32)(image->GetTileWidth() * (int32)column -
							(int32)image->GetFirstTileOffsetX()));
						window_start_x = (int)loc_x;
					}
					else
					{
						tile_start_x = 0;
						window_start_x = (int)(((((int32)tile_start_x +
							(int32)image->GetTileWidth() *
							(int32)column - (int32)image->GetFirstTileOffsetX() -
							(int32)image_x) * (int32)loc_width) /
							(int32)image_width) + (int32)loc_x);
					}
				}
				else
				{
					tile_start_x = (int)((int32)image->GetFirstTileOffsetX() +
						(int32)image_x);
					window_start_x = (int)(((((int32)tile_start_x - (int32)image_x) *
						(int32)loc_width) / (int32)image_width) + (int32)loc_x);
				}
				if (!column)
				{
					if (image->GetNumTilesAcross() == 1)
					{
						tile_width = (int)((int32)image->GetLastTileWidth() -
							((int32)tile_start_x -
							(int32)image->GetFirstTileOffsetX()));
					}
					else
					{
						tile_width = (int)((int32)image->GetTileWidth() -
							(int32)tile_start_x);
					}
				}
				else if (column < (int)image->GetNumTilesAcross() - 1)
				{
					tile_width = (int)((int32)image->GetTileWidth() -
						(int32)tile_start_x);
				}
				else
				{
					tile_width = (int)((int32)image->GetLastTileWidth() -
						(int32)tile_start_x);
				}
				if (column)
				{
					window_width = (int)(((((int32)tile_start_x +
						(int32)tile_width +
						(int32)image->GetTileWidth() * (int32)(column) -
						(int32)image->GetFirstTileOffsetX() - (int32)image_x) *
						(int32)loc_width) / (int32)image_width) + (int32)loc_x -
						(int32)window_start_x);
				}
				else
				{
					window_width = (int)(((((int32)tile_start_x +
						(int32)tile_width - (int32)image_x) *
						(int32)loc_width) / (int32)image_width) + (int32)loc_x -
						(int32)window_start_x);
				}
				if ((int32)window_start_x + (int32)window_width >
					(int32)loc_x + (int32)loc_width)
				{
					window_width = (int)((int32)loc_x + (int32)loc_width -
						(int32)window_start_x);
// Not sure about this
					tile_width = (int)(((((int32)window_start_x +
						(int32)window_width - (int32)loc_x) *
						(int32)image_width) / (int32)loc_width) +
						(int32)image_x - (int32)tile_start_x -
                  (int32)tile.GetTile()->GetImageOffsetX());
				}
				if ((int32)tile_start_x + (int32)tile_width >
					(int32)image->GetTileWidth())
				{
					tile_width = image->GetTileWidth() - tile_start_x;
					window_width = (int)(((((int32)tile_start_x +
						(int32)tile_width - (int32)image_x) *
						(int32)loc_width) / (int32)image_width) +
						(int32)loc_x - (int32)window_start_x);
				}
#if 0
				int tile_width = (int)image->GetTileWidth();
				int window_start_x;
				int tile_start_x;
				int window_width;
				if (column)
				{
					tile_start_x = 0;
				}
				else
				{
					tile_start_x = (int)((int32)image->GetFirstTileOffsetX() +
						image_x;
				}
				if (!column)
				{
					if (image->GetNumTilesAcross() == 1)
					{
						tile_width = (int)((int32)image->GetLastTileWidth() -
							((int32)tile_start_x - (int32)image->GetFirstTileOffsetX()));
					}
					else
					{
						tile_width = (int)((int32)image->GetTileWidth() -
							(int32)tile_start_x);
					}
				}
				else if (column < (int)image->GetNumTilesAcross() - 1)
				{
					tile_width = (int)image->GetTileWidth() - tile_start_x;
				}
				else
				{
					tile_width = (int)image->GetLastTileWidth() - tile_start_x;
				}
#endif
				if (window_width <= 0 || tile_width <= 0)
					continue;
#ifdef MSWIN
				// MSWIN does things upside down
				byte * tile_ptr = *(tile.GetTile()->GetRow(
					tile_start_y + tile_height - 1));
#else
				byte * tile_ptr = *(tile.GetTile()->GetRow(tile_start_y));
#endif
				#ifdef MSWIN
				dib_header->bmiHeader.biWidth = (int)image->GetTileWidth();
				dib_header->bmiHeader.biHeight = tile_height;
				dib_header->bmiHeader.biSizeImage = (DWORD)tile_height *
					(DWORD)tile.GetTile()->GetRowBytes();
				StretchDIBits(mdc,
					window_start_x, window_start_y,
					window_width, window_height,
					tile_start_x, 0,
					tile_width, tile_height,
					tile_ptr, dib_header, DIB_RGB_COLORS, SRCAND);
				#endif
				#if 0	// This is obsolete.  See DrawTile()
				#ifdef MACOS
				PixMapPtr pixMapPtr = *PixMapTile;
				pixMapPtr -> baseAddr = (Ptr) tile_ptr;
				GrafPtr activePort;
				GetPort (&activePort);
				Rect sourceRect, destRect;
				/*  Set up rects */
				sourceRect.top 		= 0;	// 0 because of Windoze disp. drvr bugs
				sourceRect.left 	= tile_start_x;
				sourceRect.bottom 	= sourceRect.top + window_height;
				sourceRect.right 	= sourceRect.left + window_width;
				int temp_width 	= (int) ((float) window_width / ZoomFactor + 0.5);
				int temp_height = (int) ((float) window_height / ZoomFactor + 0.5);
				sourceRect.bottom 	= sourceRect.top + temp_height;
				sourceRect.right 	= sourceRect.left + temp_width;
				destRect.top 		= window_start_y;
				destRect.left 		= window_start_x;
				destRect.bottom 	= destRect.top + window_height;
				destRect.right 		= destRect.left + window_width;
				#define USEOFFSETRECT
				#ifdef USEOFFSETRECT
				OffsetRect (&destRect, PanX, PanY);
				#else
				SetOrigin (1, 1);	// For trying to get maps to draw right...
				#endif
				CopyBits ((BitMapPtr) pixMapPtr,  &WINBITMAP (activePort),
					&sourceRect, &destRect, srcCopy + ditherCopy, NULL);
				if (CallBack)
					(*CallBack)();
				#endif
				#endif
			}
		}
	}
#endif
}
