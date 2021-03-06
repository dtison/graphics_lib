// g42zview.cpp - zoomed image viewer
/*
	G42ZoomImageViewer class - displays a single image in a window
	changes:
		ges256 - for displaying in 256 paletted mode
		gessa - stretching selected area code
		gesdtr - adding in an unzoomed dithered image for speed
*/
#include <g42itype.h>
#include <g42zview.h>
#include <g42image.h>
#include "gesdebug.h"
const Margin = 10; // area outside image when zoomed in
#ifdef MACOS		// (m.1)
const MacMargin = Margin;
//const MacMargin = 0;
#include <Quickdraw.h>
#include <ToolUtils.h>
#endif
extern void
ResizeImage(G42ImageData * data, int left, int top, int right, int bottom,
	G42ImageData * new_data, int left2, int top2, int right2, int bottom2);
extern void
FastShrinkImage(G42ImageData *, int, int, int, int, G42ImageData *, int, int, int, int);
void
Dither(G42ImageData * data, G42ImageData * new_data, int left, int top,
	int right, int bottom, G42OptimizedPalette * opt_pal, bool match = false);
extern int
GetResizeDepth(G42ImageData * data, uint32 width, uint32 height);
#ifdef MSWIN
G42ZoomImageViewer::G42ZoomImageViewer(HWND window, HWND top_window) :
	#ifdef MSWIN
	G42SimpleImageViewer(window, top_window),
	DC(0),
	#endif
	#ifdef MACOS	// (m.2) Some changes
	G42SimpleImageViewer(windowPtr, CallBackFn),
	CurrentCursor (&qd.arrow), GWindowPtr (windowPtr), 	CallBack (CallBackFn),
	GWorldTile (0), PixMapTile (0), GWindowPtr (0), GPaletteHandle (0),
	#endif
	DitherBasePalette(0),
	DitherPalette(0), CheckPartial(false), // ges256
	StretchImage(0), DitherImage(0), DitherBaseImage(0),
	Image(0), PanX(-Margin), PanY(-Margin),
	ZoomFactor(1.0), ZoomInt(1), ZoomMode(NotZoomed),
	HBar(0), VBar(0),
	HBarVisible(true), VBarVisible(true), InFit(0),
	NeedIdle(false),
	UpdateTileRow(0),
	UpdateTileColumn(0),
	UpdateTileStartRow(0),
	UpdateTileStartColumn(0),
	UpdateTileNumRows(0),
	UpdateTileNumColumns(0),
	ProgressiveNumRows(0),
	NeedDither(true), NeedStretchDither(true)
{
	HBar = new G42ScrollBar(Window, G42ScrollBar::Horizontal);
	VBar = new G42ScrollBar(Window, G42ScrollBar::Vertical);
//ScreenType = Monochrome;
	if (ScreenType == Palette256)
	{
//		DitherBasePalette = new G42OptimizedPalette(0, 236, false, true);
	}
	else if (ScreenType == Palette16)
	{
		DitherBasePalette = new G42OptimizedPalette(0, 16, false, true);
		DitherPalette = DitherBasePalette;
		SetWindowsPalette(DitherBasePalette->GetPalette(),
			DitherBasePalette->GetNumPalette());
	}
	else if (ScreenType == Monochrome)
	{
		DitherBasePalette = new G42OptimizedPalette(0, 2, false, true);
		DitherPalette = DitherBasePalette;
		SetWindowsPalette(DitherBasePalette->GetPalette(),
			DitherBasePalette->GetNumPalette());
	}
	else
	{
		NeedDither = false;
	}
		
	NeedStretchDither = NeedDither;
}
#endif
#ifdef MACOS
G42ZoomImageViewer::G42ZoomImageViewer (GWorldPtr windowPtr,
	void (*CallBackFn)())
:
	#ifdef MSWIN
	G42SimpleImageViewer(window, top_window),
	DC(0),
	#endif
	#ifdef MACOS	// (m.2) Some changes
	G42SimpleImageViewer(windowPtr, CallBackFn),
	PrepareDrawMapCnt (0),
	PrepareDrawRubberCnt (0),
	PrepareDrawImgCnt (0),
	#endif
	DitherBasePalette(0),
	DitherPalette(0), CheckPartial(false), // ges256
	StretchImage(0), DitherImage(0), DitherBaseImage(0),
	Image(0), PanX(-Margin), PanY(-Margin),
	ZoomFactor(1.0), ZoomInt(1), ZoomMode(NotZoomed),
	HBar(0), VBar(0),
	HBarVisible(true), VBarVisible(true), InFit(0),
	NeedIdle(false),
	UpdateTileRow(0),
	UpdateTileColumn(0),
	UpdateTileStartRow(0),
	UpdateTileStartColumn(0),
	UpdateTileNumRows(0),
	UpdateTileNumColumns(0),
	ProgressiveNumRows(0),
	NeedDither(false), NeedStretchDither(false)
{
}
#endif
G42ZoomImageViewer::~G42ZoomImageViewer()
{
	if (Image)
		Image->RemoveView(this);
	if (HBar)
	{
		delete HBar;
		HBar = 0;
	}
	if (VBar)
	{
		delete VBar;
		VBar = 0;
	}
	if (DitherBasePalette)
	{
		if (DitherPalette == DitherBasePalette)
			DitherPalette = 0;
		delete DitherBasePalette;
		DitherBasePalette = 0;
	}
	Clear(); // deletes selected image, dithered images, and palettes
}
// clear out all images and other information
void
G42ZoomImageViewer::Clear(void)
{
	#ifdef MSWIN
	if (DC)
	{
		ReleaseDC(Window, DC);
		DC = 0;
	}
	#endif
	// start ges256
	if (StretchImage)
	{
		delete StretchImage;
		StretchImage = 0;
	}
	if (DitherImage)
	{
		delete DitherImage;
		DitherImage = 0;
	}
	if (DitherBaseImage)
	{
		delete DitherBaseImage;
		DitherBaseImage = 0;
	}
	if (DitherPalette && DitherPalette != DitherBasePalette)
	{
		delete DitherPalette;
	}
	DitherPalette = DitherBasePalette;
}
void
G42ZoomImageViewer::SetWindowsGrayscalePalette(int num_pal)
{
#ifdef MSWIN
	LOGPALETTE * log_pal = (LOGPALETTE *)new byte [2048]; // just to be safe
	log_pal->palVersion = 0x300;
	log_pal->palNumEntries = (WORD)num_pal;
	for (int i = 0; i < log_pal->palNumEntries; i++)
	{
		log_pal->palPalEntry[i].peRed =
		log_pal->palPalEntry[i].peGreen =
		log_pal->palPalEntry[i].peBlue =
			(byte)(((int32)i * (int32)255) / (int32)(num_pal - 1));
		log_pal->palPalEntry[i].peFlags = 0;
	}
	if (DitherWindowPalette)
		DeleteObject(DitherWindowPalette);
	DitherWindowPalette = CreatePalette(log_pal);
	delete [] (byte *)log_pal;
#endif
}
bool
G42ZoomImageViewer::NeedIdleAction(void)
{
	return (Valid);
}
// start ges256
bool
G42ZoomImageViewer::IdleAction(void)
{
	if (!Valid)
		return false;
	if (!NeedIdle)
		return true;
//	if (ZoomMode != NotZoomed && ZoomFactor > 1.0)
//		return false;
	G42ZoomInfo zoom_info(ZoomMode, ZoomInt, ZoomFactor);
#if 0
	if (!DitherPalette && ScreenType != TrueColor)
	{
		DitherPalette = new G42OptimizedPalette(Image->GetData(), 236);
		const G42Color * dpal = DitherPalette->GetPalette();
		LOGPALETTE * log_pal = (LOGPALETTE *)new byte [2048]; // just to be safe
		log_pal->palVersion = 0x300;
		log_pal->palNumEntries = (WORD)DitherPalette->GetNumPalette();
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
		return true;
	}
#endif
	bool ret = ImageIdle(Image, StretchImage, DitherImage, DitherBaseImage,
		PanX, PanY);
	if (!ret)
		NeedIdle = false;
GESTRACEOUT();
	return true;
}
bool
G42ZoomImageViewer::ImageIdle(G42Image * & image,
	G42Image * & stretch_image, G42Image * & dither_image,
	G42Image * & dither_base_image, int32 pan_x, int32 pan_y)
{
	if (!image || !image->IsFinished())
		return false;
	G42ZoomInfo zoom_info(ZoomMode, ZoomInt, ZoomFactor);
	if (!dither_base_image && NeedDither && ZoomMode != NotZoomed)
	{
GESTRACESTART("BuildBase Start");
		dither_base_image = new G42Image(*image);
GESTRACE("Copy made");
		if (ScreenType == Monochrome)
			dither_base_image->ColorReduce(MethodDither, Gray, 2);
		else if (ScreenType == Palette16)
			dither_base_image->ColorReduce(MethodMatching, WinColor, 16);
		else
			dither_base_image->ColorReduce(DitherPalette, true);
GESTRACE("Reduce completed");
//		if (Image == image)
			Draw();
GESTRACEEND("BuildBase End");
		return true;
	}
	if (!stretch_image && ZoomMode != NotZoomed && ZoomFactor < 1.0)
	{
		G42Image * base_image = image;
		G42ImageInfo * info = new G42ImageInfo(base_image->GetInfo());
		uint32 width = (uint32)zoom_info.ImageToWindow(info->Width);
		uint32 height = (uint32)zoom_info.ImageToWindow(info->Height);
		info->PixelDepth = GetResizeDepth(base_image->GetData(),
			width, height);
		info->Width = width;
		info->Height = height;
		if (info->PixelDepth > 8)
		{
			info->ColorType = G42ImageInfo::ColorTypeTrueColor;
			info->NumPalette = 0;
		}
		stretch_image = new G42Image(image->GetMemoryManager(),
			info, true);
//GES((ges, "Creating StretchImage width, height: %d, %d\n", width, height));
//GES((ges, "Tile width, height, across, down: %d, %d, %d, %d\n\n", StretchImage->GetTileWidth(), StretchImage->GetTileHeight(), StretchImage->GetNumTilesAcross(), StretchImage->GetNumTilesDown()));
		delete info;
		CheckPartial = true;
		return true;
	}
	if (!dither_image && ((NeedDither && ZoomMode == NotZoomed) ||
		(stretch_image && NeedStretchDither)))
	{
		G42Image * base_image = image;
		if (ZoomMode != NotZoomed && stretch_image)
			base_image = stretch_image;
		G42ImageInfo * info = new G42ImageInfo(base_image->GetInfo());
		if (ScreenType == Monochrome)
		{
			info->PixelDepth = 1;
			info->ColorType = G42ImageInfo::ColorTypeGrayscale;
		}
		else
		{
			if (ScreenType == Palette256)
				info->PixelDepth = 8;
			else
				info->PixelDepth = 4;
			info->ColorType = G42ImageInfo::ColorTypePalette;
			info->NumPalette = DitherPalette->GetNumPalette();
			G42Color * pal = new G42Color [info->NumPalette];
			const G42Color * dpal = DitherPalette->GetPalette();
			for (uint i = 0; i < info->NumPalette; i++)
				pal[i] = dpal[i];
			info->ImagePalette.Set(pal); // automatically clears any old palette
		}
		dither_image = new G42Image(image->GetMemoryManager(),
			info, true);
//GES((ges, "Creating DitherImage width, height: %d, %d\n", info->Width, info->Height));
//GES((ges, "Tile width, height, across, down: %d, %d, %d, %d\n\n", DitherImage->GetTileWidth(), DitherImage->GetTileHeight(), DitherImage->GetNumTilesAcross(), DitherImage->GetNumTilesDown()));
		delete info;
		CheckPartial = true;
		return true;
	}
	if (CheckPartial)
	{
		CheckPartial = false;
		if (dither_image)
		{
			uint dither_index_x = 0;
			if (pan_x > 0)
				dither_index_x = dither_image->GetTileColumnIndex((uint32)pan_x);
			uint dither_index_y = 0;
			if (pan_y > 0)
				dither_index_y = dither_image->GetTileRowIndex((uint32)pan_y);
			uint dither_index_width = 0;
			if (pan_x + (int32)WindowWidth >= 0) // no need to clip top part
				dither_index_width = dither_image->GetTileColumnIndex(
					(uint32)pan_x + (uint32)WindowWidth - 1) - dither_index_x + 1;
			uint dither_index_height = 0;
			if (pan_y + (int32)WindowHeight >= 0) // no need to clip top part
				dither_index_height = dither_image->GetTileRowIndex(
					(uint32)pan_y + (uint32)WindowHeight - 1) - dither_index_y + 1;
			if (!dither_index_width || !dither_index_height)
			{
				return false;
			}
			dither_image->SetPartial(dither_index_x, dither_index_y,
				dither_index_width, dither_index_height);
			while (dither_image->NeedPartialUpdate())
			{
				for (uint ty = dither_image->GetPartialUpdateTileY();
					ty < dither_image->GetPartialUpdateTileY() +
					dither_image->GetPartialUpdateTileHeight(); ty++)
				{
					for (uint tx = dither_image->GetPartialUpdateTileX();
						tx < dither_image->GetPartialUpdateTileX() +
						dither_image->GetPartialUpdateTileWidth(); tx++)
					{
						G42ImageTile * tile = dither_image->GetTile(ty, tx);
						if (tile)
							tile->SetNeedUpdate(true);
					}
				}
				dither_image->UpdatePartial();
			}
			UpdateTileRow = dither_index_y;
			UpdateTileColumn = dither_index_x;
			UpdateTileStartRow = dither_index_y;
			UpdateTileStartColumn = dither_index_x;
			UpdateTileNumRows = dither_index_height;
			UpdateTileNumColumns = dither_index_width;
		}
		if (stretch_image)
		{
			uint stretch_index_x = 0;
			uint stretch_index_y = 0;
			uint stretch_index_width = 0;
			uint stretch_index_height = 0;
			if (dither_image)
			{
				stretch_index_x = stretch_image->GetTileColumnIndex(
					dither_image->GetImageStartX(UpdateTileStartColumn));
				stretch_index_y = stretch_image->GetTileRowIndex(
					dither_image->GetImageStartY(UpdateTileStartRow));
				stretch_index_width = stretch_image->GetTileColumnIndex(
					dither_image->GetImageEndX(UpdateTileStartColumn +
					UpdateTileNumColumns - 1)) - stretch_index_x + 1;
				stretch_index_height = stretch_image->GetTileRowIndex(
					dither_image->GetImageEndY(UpdateTileStartRow +
					UpdateTileNumRows - 1)) - stretch_index_y + 1;
			}
			else
			{
				if (pan_x > 0)
					stretch_index_x = stretch_image->GetTileColumnIndex((uint32)pan_x);
				if (pan_y > 0)
					stretch_index_y = stretch_image->GetTileRowIndex((uint32)pan_y);
				if (pan_x + (int32)WindowWidth >= 0) // no need to clip top part
					stretch_index_width = stretch_image->GetTileColumnIndex(
						(uint32)pan_x + (uint32)WindowWidth - 1) -
						stretch_index_x + 1;
				if (pan_y + (int32)WindowHeight >= 0) // no need to clip top part
					stretch_index_height = stretch_image->GetTileRowIndex(
						(uint32)pan_y + (uint32)WindowHeight - 1) -
						stretch_index_y + 1;
			}
			if (!stretch_index_width || !stretch_index_height)
			{
				return false;
			}
			stretch_image->SetPartial(stretch_index_x, stretch_index_y,
				stretch_index_width, stretch_index_height);
         if (stretch_image->GetMask())
			{
				uint mask_index_x = 0;
				uint mask_index_y = 0;
				uint mask_index_width = 0;
				uint mask_index_height = 0;
				if (pan_x > 0)
					mask_index_x = stretch_image->GetMask()->GetTileColumnIndex((uint32)pan_x);
				if (pan_y > 0)
					mask_index_y = stretch_image->GetMask()->GetTileRowIndex((uint32)pan_y);
				if (pan_x + (int32)WindowWidth >= 0) // no need to clip top part
					mask_index_width = stretch_image->GetMask()->GetTileColumnIndex(
						(uint32)pan_x + (uint32)WindowWidth - 1) -
						mask_index_x + 1;
				if (pan_y + (int32)WindowHeight >= 0) // no need to clip top part
					mask_index_height = stretch_image->GetMask()->GetTileRowIndex(
						(uint32)pan_y + (uint32)WindowHeight - 1) -
						mask_index_y + 1;
				stretch_image->GetMask()->SetPartial(mask_index_x, mask_index_y,
					mask_index_width, mask_index_height);
         }
			if (!dither_image)
			{
				UpdateTileRow = stretch_index_y;
				UpdateTileColumn = stretch_index_x;
				UpdateTileStartRow = stretch_index_y;
				UpdateTileStartColumn = stretch_index_x;
				UpdateTileNumRows = stretch_index_height;
				UpdateTileNumColumns = stretch_index_width;
			}
			while (stretch_image->NeedPartialUpdate())
			{
				for (uint ty = stretch_image->GetPartialUpdateTileY();
					ty < stretch_image->GetPartialUpdateTileY() +
					stretch_image->GetPartialUpdateTileHeight(); ty++)
				{
					for (uint tx = stretch_image->GetPartialUpdateTileX();
						tx < stretch_image->GetPartialUpdateTileX() +
						stretch_image->GetPartialUpdateTileWidth(); tx++)
					{
						G42ImageTile * tile = stretch_image->GetTile(ty, tx);
						if (tile)
							tile->SetNeedUpdate(true);
					}
				}
				stretch_image->UpdatePartial();
			}
		}
		return true;
	}
	G42ImageTile * tile;
	G42Image * new_image = dither_image;
	if (!new_image)
		new_image = stretch_image;
	if (!new_image)
	{
		return false;
	}
	while (1)
	{
		tile = new_image->GetTile(UpdateTileRow, UpdateTileColumn);
		if (!tile)
		{
			CheckPartial = true;
			return true;
		}
		if (tile->GetNeedUpdate())
			break;
		UpdateTileColumn++;
		if (UpdateTileColumn - UpdateTileStartColumn >= UpdateTileNumColumns)
		{
			UpdateTileColumn = UpdateTileStartColumn;
			UpdateTileRow++;
			if (UpdateTileRow - UpdateTileStartRow >= UpdateTileNumRows)
			{
GESTRACETOTAL();
				return false;
				// break;
			}
		}
	}
	if (!dither_image && !stretch_image)
		return false;
	int32 dest_x = tile->GetImageOffsetX();
	int32 dest_y = tile->GetImageOffsetY();
	int32 dest_width = tile->GetTileWidth();
	int32 dest_height = tile->GetTileHeight();
	if (ZoomMode == NotZoomed && dither_image)
	{
GESTRACESTART("Dither Start");
		Dither(image->GetData(), dither_image->GetData(),
			dest_x, dest_y,
			dest_x + dest_width - 1, dest_y + dest_height - 1,
			DitherPalette);
GESTRACEEND("Dither End");
GESTRACEADD(GesTraceDither, ges_trace_end - ges_trace);
		tile->SetNeedUpdate(false);
	}
	else if (dither_image && stretch_image)
	{
		uint32 dither_x = tile->GetImageOffsetX();
		uint32 dither_width = (uint32)tile->GetTileWidth();
		uint stretch_index_x = stretch_image->GetTileColumnIndex(dither_x);
		uint stretch_index_width = stretch_image->GetTileColumnIndex(
			dither_x + dither_width - 1) - stretch_index_x + 1;
		uint32 dither_y = tile->GetImageOffsetY();
		uint32 dither_height = (uint32)tile->GetTileHeight();
		uint stretch_index_y = stretch_image->GetTileRowIndex(dither_y);
		uint stretch_index_height = stretch_image->GetTileRowIndex(
			dither_y + dither_height - 1) - stretch_index_y + 1;
		for (uint row = stretch_index_y;
			row < stretch_index_y + stretch_index_height; row++)
		{
			for (uint column = stretch_index_x;
				column < stretch_index_x + stretch_index_width; column++)
			{
				G42ImageTile * stretch_tile = stretch_image->GetTile(row, column);
				if (!stretch_tile)
				{
					CheckPartial = true;
					return true;
				}
				if (stretch_tile->GetNeedUpdate())
				{
					// stretch tile
					uint32 stretch_x = stretch_tile->GetImageOffsetX();
					uint32 stretch_y = stretch_tile->GetImageOffsetY();
					uint32 stretch_width = (uint32)stretch_tile->GetTileWidth();
					uint32 stretch_height = (uint32)stretch_tile->GetTileHeight();
					int32 image_x = zoom_info.WindowToImage(stretch_x);
					int32 image_y = zoom_info.WindowToImage(stretch_y);
					int32 image_width = zoom_info.WindowToImage(
						stretch_x + stretch_width) - image_x;
					int32 image_height = zoom_info.WindowToImage(
						stretch_y + stretch_height) - image_y;
GESTRACESTART("ResizeImage Start");
					ResizeImage(image->GetData(), image_x, image_y,
						image_x + image_width - 1, image_y + image_height - 1,
						stretch_image->GetData(), stretch_x, stretch_y,
						stretch_x + stretch_width - 1,
						stretch_y + stretch_height - 1);
GESTRACEEND("ResizeImage End");
					stretch_tile->SetNeedUpdate(false);
					return true;
				}
			}
		}
GESTRACESTART("Dither Start");
		Dither(stretch_image->GetData(), dither_image->GetData(),
			dest_x, dest_y,
			dest_x + dest_width - 1, dest_y + dest_height - 1,
			DitherPalette);
GESTRACEEND("Dither End");
GESTRACEADD(GesTraceDither, ges_trace_end - ges_trace);
		tile->SetNeedUpdate(false);
	}
	else if (stretch_image)
	{
		uint32 stretch_x = tile->GetImageOffsetX();
		uint32 stretch_y = tile->GetImageOffsetY();
		uint32 stretch_width = (uint32)tile->GetTileWidth();
		uint32 stretch_height = (uint32)tile->GetTileHeight();
		int32 image_x = zoom_info.WindowToImage(stretch_x);
		int32 image_y = zoom_info.WindowToImage(stretch_y);
		int32 image_width = zoom_info.WindowToImage(
			stretch_x + stretch_width) - image_x;
		int32 image_height = zoom_info.WindowToImage(
			stretch_y + stretch_height) - image_y;
GESTRACESTART("ResizeImage Start");
		ResizeImage(image->GetData(), image_x, image_y,
			image_x + image_width - 1, image_y + image_height - 1,
			stretch_image->GetData(), stretch_x, stretch_y,
			stretch_x + stretch_width - 1,
			stretch_y + stretch_height - 1);
GESTRACEEND("ResizeImage End");
		if (image->GetMask())
		{
			FastShrinkImage(image->GetMask(), image_x, image_y,
				image_x + image_width - 1, image_y + image_height - 1,
				stretch_image->GetMask(), stretch_x, stretch_y,
				stretch_x + stretch_width - 1,
				stretch_y + stretch_height - 1);
		}
		tile->SetNeedUpdate(false);
	}
	// tiles should be close to the window, so we can ignore 32 bit wrap
	int draw_x = (int)(dest_x - pan_x);
	int draw_y = (int)(dest_y - pan_y);
	int draw_width = (int)(dest_width);
	int draw_height = (int)(dest_height);
#if 0
	int draw_x = (int)(zoom_info.ImageToWindow(dest_x) - pan_x);
	int draw_y = (int)(zoom_info.ImageToWindow(dest_y) - pan_y);
	int draw_width = (int)(zoom_info.ImageToWindow(dest_width + dest_x) -
		pan_x) - draw_x;
	int draw_height = (int)(zoom_info.ImageToWindow(dest_height + dest_y) -
		pan_y) - draw_y;
#endif
	if (draw_x < 0)
   {
      draw_width += draw_x;
		draw_x = 0;
   }
	if (draw_y < 0)
   {
   	draw_height += draw_y;
		draw_y = 0;
   }
	if (draw_x + draw_width > WindowWidth)
		draw_width = WindowWidth - draw_x;
	if (draw_y + draw_height > WindowHeight)
		draw_height = WindowHeight - draw_y;
	if (draw_x < WindowWidth && draw_y < WindowHeight &&
		draw_width > 0 && draw_height > 0)
	{
//		if (Image == image)
			Draw(draw_x, draw_y, draw_width, draw_height);
	}
	return true;
}
// end ges256
/* 	MSWIN & MACOS version D. Ison  */
void G42ZoomImageViewer::Draw(void)
{
	Draw(WindowX, WindowY, WindowWidth, WindowHeight);
}
void G42ZoomImageViewer::Draw(int draw_x, int draw_y,
	int draw_width, int draw_height, bool expand)
{
	#ifdef MSWIN
	G42DrawLocation location = GetDC(Window);
	#else
	G42DrawLocation location = 0;
	#endif
	Draw(location, draw_x, draw_y, draw_width, draw_height, expand);
	#ifdef MSWIN
	ReleaseDC(Window, location);
	#endif
}
void
G42ZoomImageViewer::PrepareDrawImage(G42DrawLocation location,
	int draw_x, int draw_y, int draw_width, int draw_height)
{
	#ifdef MSWIN
	SetStretchBltMode(location, STRETCH_DELETESCANS);
	if (draw_x || draw_y ||
		draw_width != WindowWidth || draw_height != WindowHeight)
	{
		HRGN r = CreateRectRgn(draw_x, draw_y,
			draw_x + draw_width, draw_y + draw_height);
		SelectClipRgn(location, r);
		DeleteObject(r);
	}
	if (DitherWindowPalette)
	{
		SelectPalette(location, DitherWindowPalette, 0);
		RealizePalette(location);
	}
	#endif
	#ifdef MACOS
	if (location)
	{
		#if 0
		if (PrepareDrawImgCnt != 0)
			SysBeep (0);
		#endif
		::GetGWorld (&OldPrepDrawImgPort, &OldPrepDrawImgDevice);
		::SetGWorld ((GWorldPtr) location, nil);
		::LockPixels (::GetGWorldPixMap ((GWorldPtr) location));
		PrepareDrawImgCnt++;
	}
	#endif
}
void
G42ZoomImageViewer::CleanupDrawImage(G42DrawLocation location)
{
#ifdef MACOS
	if (location)
	{
		::UnlockPixels (::GetGWorldPixMap ((GWorldPtr) location));
		::SetGWorld (OldPrepDrawImgPort, OldPrepDrawImgDevice);
		PrepareDrawImgCnt--;		
	}
#endif
}
void G42ZoomImageViewer::Draw(G42DrawLocation location,
	int draw_x, int draw_y,
	int draw_width, int draw_height, bool expand)
{
	if (Image && Image->IsValid() && Image->HasInfo() &&
		WindowWidth && WindowHeight)
	{
		#ifdef MSWIN
		// fudge partial draws by a pixel
		if (expand)
		{
		int zoom_shift;
		if (ZoomMode == NotZoomed || ZoomFactor < 1.0)
			zoom_shift = 1;
		else
			zoom_shift = (int)(ZoomFactor) + 1;
		if (draw_x >= WindowX + zoom_shift)
		{
			draw_x -= zoom_shift;
			draw_width += zoom_shift;
		}
		else if (draw_x > WindowX)
		{
			draw_width += draw_x - WindowX;
			draw_x = WindowX;
		}
		if (draw_y >= WindowY + zoom_shift)
		{
			draw_y -= zoom_shift;
			draw_height += zoom_shift;
		}
		else if (draw_y > WindowY)
		{
			draw_height += draw_y - WindowY;
			draw_y = WindowY;
		}
		if (draw_x + draw_width < WindowX + WindowWidth - zoom_shift)
			draw_width += zoom_shift;
		else if (draw_x + draw_width < WindowX + WindowWidth)
			draw_width = WindowX + WindowWidth - draw_x;
		if (draw_y + draw_height < WindowY + WindowHeight - zoom_shift)
			draw_height += zoom_shift;
		else if (draw_y + draw_height < WindowY + WindowHeight)
			draw_height = WindowY + WindowHeight - draw_y;
		}
		#endif
		int32 start_x, start_y, end_x, end_y;
		if (PanX + WindowX < 0)
			start_x = -(PanX + WindowX) - 1;
		else
			start_x = -1;
		if (start_x > WindowX + (int32)WindowWidth)
			start_x = WindowX + WindowWidth;
		if (WindowY + PanY < 0)
			start_y = -(WindowY + PanY) - 1;
		else
			start_y = -1;
		if (start_y > WindowY + (int32)WindowHeight)
			start_y = WindowY + WindowHeight;
		end_x = ImageToWindow(Image->GetInfo().Width) - PanX;
		if (end_x > WindowX + (int32)WindowWidth)
			end_x = WindowX + WindowWidth;
		if (end_x < -1)
			end_x = -1;
		end_y = ImageToWindow(Image->GetInfo().Height) - PanY;
		if (end_y > WindowY + (int32)WindowHeight)
			end_y = WindowY + WindowHeight;
		if (end_y < -1)
			end_y = -1;
		#ifdef MSWIN
		// this works because we are drawing on a shadow screen, so the
		// image won't flash
		SelectObject(location, GetStockObject(NULL_PEN));
		SelectObject(location, GetStockObject(LTGRAY_BRUSH));
		Rectangle(location, draw_x, draw_y,
			draw_x + draw_width + 1, draw_y + draw_height + 1);
		SelectObject(location, GetStockObject(BLACK_PEN));
		SelectObject(location, GetStockObject(NULL_BRUSH));
		if (start_x >= 0 && start_y < (int32)WindowHeight && end_y >= 0)
		{
			MoveToEx(location, (int)start_x, (int)start_y, 0);
			LineTo(location, (int)start_x, (int)end_y + 1);
		}
		if (start_y >= 0 && start_x < (int32)WindowWidth && end_x >= 0)
		{
			MoveToEx(location, (int)start_x, (int)start_y, 0);
			LineTo(location, (int)end_x + 1, (int)start_y);
		}
		if (end_x < (int32)WindowWidth && start_y < (int32)WindowHeight &&
			end_y >= 0)
		{
			MoveToEx(location, (int)end_x, (int)start_y, 0);
			LineTo(location, (int)end_x, (int)end_y + 1);
		}
		if (end_y < (int32)WindowHeight && start_x < (int32)WindowWidth &&
			end_x >= 0)
		{
			MoveToEx(location, (int)start_x, (int)end_y, 0);
			LineTo(location, (int)end_x + 1, (int)end_y);
		}
#if 0
		SelectObject(location, GetStockObject(NULL_PEN));
		SelectObject(location, GetStockObject(LTGRAY_BRUSH));
		if (start_x > 0 && start_y < (int32)WindowHeight && end_y >= 0)
		{
			Rectangle(location, 0, (int)start_y, (int)start_x + 1, (int)end_y + 2);
		}
		if (start_y > 0)
		{
			Rectangle(location, 0, 0, (int)WindowWidth + 1, (int)start_y + 1);
		}
		if (end_x < (int32)WindowWidth &&
			start_y < (int32)WindowHeight && end_y >= 0)
		{
			Rectangle(location, (int)end_x + 1, (int)start_y, (int)WindowWidth + 1,
				(int)end_y + 2);
		}
		if (end_y < (int32)WindowHeight)
		{
			Rectangle(location, 0, (int)end_y + 1, (int)WindowWidth + 1,
				(int)WindowHeight + 1);
		}
#endif
		#endif
		PrepareDrawImage(location, draw_x, draw_y, draw_width, draw_height);
		#ifdef MACOS // Draws Border		
		if (MacMargin && (PanX < 0 || PanY < 0))
		{
			uint width = Image -> GetInfo().Width, height = Image -> GetInfo().Height;
			width = ImageToWindow (width);
			height = ImageToWindow (height);
			::PenPat (&qd.gray);			
			::PenState penState;
			::GetPenState (&penState);
			penState.pnSize.h = -PanX;
			penState.pnSize.v = -PanY;
			penState.pnMode = srcCopy;
			::SetPenState (&penState);
			MoveTo (0, 0);
			LineTo (width - PanX, 0);
			LineTo (width - PanX, height - PanY);
			LineTo (0, height - PanY);
			LineTo (0, 0);
			PenNormal();
		}
		#endif
		DrawImage(location, Image, DitherImage, StretchImage, DitherBaseImage,
			PanX, PanY, draw_x, draw_y, draw_width, draw_height);
		CleanupDrawImage(location);
	}
	#ifdef MSWIN
	else if (Window)
	{
		SelectObject(location, GetStockObject(NULL_PEN));
		SelectObject(location, GetStockObject(LTGRAY_BRUSH));
		Rectangle(location, 0, 0, (int)WindowWidth + 1, (int)WindowHeight + 1);
	}
	#endif
}
void
G42ZoomImageViewer::DrawImage(G42DrawLocation location, G42Image * image,
	int draw_x, int draw_y, int draw_width, int draw_height,
	G42ZoomInfo zoom_info, int32 pan_x, int32 pan_y,
	G42Image * backup_image, G42ZoomInfo backup_zoom_info)
{
	G42SimpleImageViewer::DrawImage(
		location, image, draw_x, draw_y, draw_width, draw_height,
		zoom_info, pan_x, pan_y, backup_image, backup_zoom_info);
}
void
G42ZoomImageViewer::DrawImage(G42DrawLocation location, G42Image * image,
	int draw_x, int draw_y, int draw_width, int draw_height,
	G42ZoomInfo zoom_info, int32 pan_x, int32 pan_y)
{
	G42SimpleImageViewer::DrawImage(
		location, image, draw_x, draw_y, draw_width, draw_height,
		zoom_info, pan_x, pan_y);
}
void
G42ZoomImageViewer::DrawImage(G42DrawLocation location, G42Image * image)
{
	DrawImage(location, image, 0, 0, WindowWidth, WindowHeight);
}
// start ges256
void G42ZoomImageViewer::DrawImage(G42DrawLocation location, G42Image * image,
	int draw_x, int draw_y, int draw_width, int draw_height)
{
	DrawImage(location, image, DitherImage, StretchImage, DitherBaseImage,
   	PanX, PanY, draw_x, draw_y, draw_width, draw_height);
}
void G42ZoomImageViewer::DrawImage(G42DrawLocation location, G42Image * image,
	G42Image * & dither_image, G42Image * & stretch_image,
   G42Image * & dither_base_image, int32 pan_x, int32 pan_y,
	int draw_x, int draw_y, int draw_width, int draw_height)
{
	#if 0	// (m.1) DEBUGGING thing
		MoveTo (0, 0);
		LineTo (image -> GetInfo().Width, image -> GetInfo().Height);
		MoveTo (0, image -> GetInfo().Height);
		LineTo (image -> GetInfo().Width, 0);
		return;
	#endif
	G42ZoomInfo zoom_info(ZoomMode, ZoomInt, ZoomFactor);
	if (draw_x < WindowX)
	{
		draw_width -= WindowX - draw_x;
		draw_x = WindowX;
	}
	if (draw_x + draw_width > WindowX + WindowWidth)
		draw_width = WindowX + WindowWidth - draw_x;
	if (draw_y < WindowY)
	{
		draw_height -= WindowY - draw_y;
		draw_y = WindowY;
	}
	if (draw_y + draw_height > WindowY + WindowHeight)
		draw_height = WindowY + WindowHeight - draw_y;
	if (!image || !image->IsValid() || !image->HasInfo() ||
		!image->GetValidNumRows() || !WindowWidth || !WindowHeight)
	{
		#if 0
		#ifdef MSWIN
		SelectObject(location, GetStockObject(LTGRAY_BRUSH));
		Rectangle(location,
			draw_x, draw_y, draw_x + draw_width, draw_y + draw_height);
		#endif
		#endif
		return;
	}
	if (draw_x < -pan_x)
	{
		draw_width -= (-pan_x - draw_x);
		draw_x = -pan_x;
	}
	if (draw_x + draw_width >
		zoom_info.ImageToWindow(image->GetInfo().Width) - pan_x)
	{
		draw_width = zoom_info.ImageToWindow(image->GetInfo().Width) -
			pan_x - draw_x;
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
	if (zoom_info.WindowToImage((int32)draw_y + draw_height + pan_y) >
		(int32)(image->GetValidStartRow() + image->GetValidNumRows()))
	{
		draw_height = (int)(zoom_info.ImageToWindow(
			image->GetValidStartRow() + image->GetValidNumRows()) -
			pan_y - (int32)draw_y);
	}
	if (draw_width <= 0 || draw_height <= 0)
		return;
	if (ScreenType == Monochrome && !dither_base_image &&
		(image->GetInfo().ColorType != G42ImageInfo::ColorTypeGrayscale ||
		image->GetInfo().PixelDepth > 1))
	{
		dither_base_image = new G42Image(*image);
		dither_base_image->ColorReduce(MethodDither, Gray, 2);
	}
	if (dither_image)
	{
		DrawImage(location, dither_image, draw_x, draw_y, draw_width, draw_height,
			G42ZoomInfo(NotZoomed), pan_x, pan_y,
			(NeedDither ? dither_base_image : image), zoom_info);
	}
	else if (!NeedStretchDither && stretch_image &&
		(ZoomMode != NotZoomed && ZoomFactor < 1.0))
	{
		DrawImage(location, stretch_image, draw_x, draw_y, draw_width, draw_height,
			G42ZoomInfo(NotZoomed), pan_x, pan_y, image, zoom_info);
	}
	else if (dither_base_image)
	{
		if (ZoomMode != NotZoomed && ZoomFactor < 1.0 && NeedStretchDither)
			NeedIdle = true;
		if (ZoomMode == NotZoomed && ScreenType != Monochrome && NeedDither)
			NeedIdle = true;
		DrawImage(location, dither_base_image, draw_x, draw_y, draw_width, draw_height,
			zoom_info, pan_x, pan_y, 0, zoom_info);
	}
	else
	{
		if (NeedDither)
		{
			NeedIdle = true;
			if (!DitherPalette)
				DrawImage(location, image, draw_x, draw_y, draw_width, draw_height,
					zoom_info, pan_x, pan_y, 0, zoom_info);
			return;
		}
		if ((ZoomMode != NotZoomed && (NeedStretchDither ||
				ZoomFactor < 1.0)) || NeedDither)
			NeedIdle = true;
		DrawImage(location, image, draw_x, draw_y, draw_width, draw_height,
			zoom_info, pan_x, pan_y, 0, zoom_info);
	}
}
void G42ZoomImageViewer::SetImage(G42Image * image, bool draw)
{
	if (Image)
		Image->RemoveView(this);
	Image = image;
	if (Image)
	{
		Image->AddView(this);
		Clear();
		SetImageInformation(Image);
		SetZoom(1.0);
		PreparePalette();
		#ifdef MSWIN
		if (ScreenType == Palette256)
			QueryNewPalette();
//		Draw();
		#else
		if (draw)
			Draw();
		#endif
	}
}
void G42ZoomImageViewer::ImageChanged(void)
{
	G42SimpleImageViewer::ImageChanged();
	/*  Clear() killed SelectedImage -- Does Clear() need to be here? */
	#ifndef MACOS
	Clear();
	#endif
	SetImageInformation(Image);
	PreparePalette();
	SetPan();
	#ifndef MACOS
	Draw();
	#endif
}
void
G42ZoomImageViewer::SetImageInformation(G42Image * image)
{
#ifdef MSWIN
	if (image)
	{
		if (image->HasInfo())
		{
			DibHeader->biBitCount = (WORD)image->GetInfo().PixelDepth;
			if (ScreenType == Monochrome)
			{
				DibHeader->biClrUsed = 2;
				DibHeader->biBitCount = 1;
				uint16 * dib_pal = (uint16 *)((byte *)DibHeader +
					sizeof (BITMAPINFOHEADER));
				for (int i = 0; i < DibHeader->biClrUsed; i++)
				{
					dib_pal[i] = i;
				}
				DibUsage = DIB_PAL_COLORS;
#if 0
				RGBQUAD * dib_pal = (RGBQUAD *)((byte *)DibHeader +
					sizeof (BITMAPINFOHEADER));
				dib_pal[0].rgbRed = 0;
				dib_pal[0].rgbGreen = 0;
				dib_pal[0].rgbBlue = 0;
				dib_pal[0].rgbReserved = 0;
				dib_pal[1].rgbRed = 0xff;
				dib_pal[1].rgbGreen = 0xff;
				dib_pal[1].rgbBlue = 0xff;
				dib_pal[1].rgbReserved = 0;
				DibUsage = DIB_RGB_COLORS;
#endif
			}
			else if (ScreenType == Palette16)
			{
				if (image->GetInfo().ColorType ==
					G42ImageInfo::ColorTypeGrayscale &&
					image->GetInfo().PixelDepth == 1)
				{
					DibHeader->biClrUsed = 2;
					DibHeader->biBitCount = 1;
					RGBQUAD * dib_pal = (RGBQUAD *)((byte *)DibHeader +
						sizeof (BITMAPINFOHEADER));
					dib_pal[0].rgbRed = 0;
					dib_pal[0].rgbGreen = 0;
					dib_pal[0].rgbBlue = 0;
					dib_pal[0].rgbReserved = 0;
					dib_pal[1].rgbRed = 0xff;
					dib_pal[1].rgbGreen = 0xff;
					dib_pal[1].rgbBlue = 0xff;
					dib_pal[1].rgbReserved = 0;
					DibUsage = DIB_RGB_COLORS;
				}
				else
				{
					DibHeader->biClrUsed = 16;
					DibHeader->biBitCount = 4;
					uint16 * dib_pal = (uint16 *)((byte *)DibHeader +
						sizeof (BITMAPINFOHEADER));
					for (int i = 0; i < DibHeader->biClrUsed; i++)
					{
						dib_pal[i] = i;
					}
					DibUsage = DIB_PAL_COLORS;
				}
			}
			else if (ScreenType == Palette256)
			{
				if (image->GetInfo().ColorType ==
					G42ImageInfo::ColorTypeGrayscale)
				{
					DibHeader->biClrUsed = (1 << image->GetInfo().PixelDepth);
					uint16 * dib_pal = (uint16 *)((byte *)DibHeader +
						sizeof (BITMAPINFOHEADER));
					for (int i = 0; i < DibHeader->biClrUsed; i++)
					{
						dib_pal[i] = (uint16)(((int32)i * (int32)127) /
							(int32)(DibHeader->biClrUsed - 1));
					}
					DibUsage = DIB_PAL_COLORS;
				}
				else if (image->GetInfo().ColorType ==
					G42ImageInfo::ColorTypePalette &&
					image->GetInfo().NumPalette <= 236)
				{
					DibHeader->biClrUsed = image->GetInfo().NumPalette;
					uint16 * dib_pal = (uint16 *)((byte *)DibHeader +
						sizeof (BITMAPINFOHEADER));
					for (int i = 0; i < DibHeader->biClrUsed; i++)
					{
						dib_pal[i] = (uint16)i;
					}
					DibUsage = DIB_PAL_COLORS;
				}
				else if (image->GetInfo().PixelDepth <= 8)
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
					DibUsage = DIB_RGB_COLORS;
				}
				else
				{
					DibUsage = DIB_RGB_COLORS;
					DibHeader->biClrUsed = 0;
				}
			}
			else if (image->GetInfo().NumPalette) // not Palette256
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
#endif
}
void
G42ZoomImageViewer::PreparePalette(void)
{
	if (ScreenType == TrueColor)
		return;
	if (ScreenType == Monochrome)
	{
		if (Image && Image->HasInfo())
		{
			const G42ImageInfo & info = Image->GetInfo();
			if (info.ColorType != G42ImageInfo::ColorTypeGrayscale ||
				info.PixelDepth != 1)
			{
				NeedDither = true;
				NeedStretchDither = true;
				if (!Image->IsFinished()) // if not finished, dither on the fly
				{
					if (DitherBaseImage)
					{
						delete DitherBaseImage;
						DitherBaseImage = 0;
					}
					G42ImageInfo * info = new G42ImageInfo(Image->GetInfo());
					info->PixelDepth = 1;
					info->ColorType = G42ImageInfo::ColorTypeGrayscale;
					DitherBaseImage = new G42Image(Image->GetMemoryManager(),
						info);
					delete info;
				}
			}
			else
			{
				NeedDither = false;
				NeedStretchDither = true;
			}
		}
		return;
	}
	if (ScreenType == Palette16)
	{
		if (Image && Image->HasInfo())
		{
			const G42ImageInfo & info = Image->GetInfo();
			if (info.ColorType != G42ImageInfo::ColorTypeGrayscale ||
				info.PixelDepth != 1)
			{
				NeedDither = true;
				NeedStretchDither = true;
				if (DitherBaseImage)
				{
					delete DitherBaseImage;
					DitherBaseImage = 0;
				}
				if (!Image->IsFinished()) // if finished, we can optimize a palette
				{
				G42ImageInfo * info = new G42ImageInfo(Image->GetInfo());
				info->PixelDepth = 4;
				info->ColorType = G42ImageInfo::ColorTypePalette;
				info->NumPalette = DitherPalette->GetNumPalette();
				G42Color * pal = new G42Color [info->NumPalette];
				const G42Color * dpal = DitherPalette->GetPalette();
				for (uint i = 0; i < info->NumPalette; i++)
					pal[i] = dpal[i];
				info->ImagePalette.Set(pal); // automatically clears any old palette
				DitherBaseImage = new G42Image(Image->GetMemoryManager(),
					info);
				delete info;
				}
			}
			else
			{
				NeedDither = false;
				NeedStretchDither = true;
			}
		}
		return;
	}
	if (Image && Image->HasInfo())
	{
		const G42ImageInfo & info = Image->GetInfo();
		if (info.ColorType == G42ImageInfo::ColorTypeTrueColor ||
			(info.ColorType == G42ImageInfo::ColorTypePalette &&
			info.NumPalette > 236)) // we need to dither these
		{
			NeedDither = true;
			NeedStretchDither = true;
			if (Image->IsFinished()) // if finished, we can optimize a palette
			{
				if (DitherPalette && DitherPalette != DitherBasePalette)
					delete DitherPalette;
				DitherPalette = new G42OptimizedPalette(Image->GetData(), 236,
					false, false, true, // true really should be false,
					0);
				SetWindowsPalette(DitherPalette->GetPalette(),
					DitherPalette->GetNumPalette());
/*
				if (DitherBaseImage)
					*DitherBaseImage = *Image;
				else
					DitherBaseImage = new G42Image(*Image);
				DitherBaseImage->ColorReduce(DitherPalette, true);
*/				
				if (DitherBaseImage)
				{
					delete DitherBaseImage;
					DitherBaseImage = 0;
				}
			}
			else // use base color cube until it is finished
			{
				if (DitherPalette && DitherPalette != DitherBasePalette)
					delete DitherPalette;
				DitherPalette = DitherBasePalette;
				SetWindowsPalette(DitherPalette->GetPalette(),
					DitherPalette->GetNumPalette());
				if (DitherBaseImage)
				{
					delete DitherBaseImage;
					DitherBaseImage = 0;
				}
				G42ImageInfo * info = new G42ImageInfo(Image->GetInfo());
				info->PixelDepth = 8;
				info->ColorType = G42ImageInfo::ColorTypePalette;
				info->NumPalette = DitherPalette->GetNumPalette();
				G42Color * pal = new G42Color [info->NumPalette];
				const G42Color * dpal = DitherPalette->GetPalette();
				for (uint i = 0; i < info->NumPalette; i++)
					pal[i] = dpal[i];
				info->ImagePalette.Set(pal); // automatically clears any old palette
				DitherBaseImage = new G42Image(Image->GetMemoryManager(),
					info);
				delete info;
			}
		}
		else // we can draw these directly, no need for dithering
		{
			NeedDither = false;
			if (info.ColorType == G42ImageInfo::ColorTypePalette)
			{
//				NeedStretchDither = true;
				NeedStretchDither = false;
				SetWindowsPalette((const G42Color *)(info.ImagePalette),
					info.NumPalette);
			}
			else
			{
				NeedStretchDither = false;
				SetWindowsGrayscalePalette(128);
			}
			if (DitherBaseImage)
			{
				delete DitherBaseImage;
				DitherBaseImage = 0;
			}
			if (DitherPalette && DitherPalette != DitherBasePalette)
				delete DitherPalette;
			DitherPalette = DitherBasePalette;
		}
	}
}
void G42ZoomImageViewer::Rebuild(void)
{
	if (Image)
	{
		Image->RebuildInfo();
		// start ges256
		if (DitherImage)
		{
			delete DitherImage;
			DitherImage = 0;
		}
		if (DitherBaseImage)
		{
			delete DitherBaseImage;
			DitherBaseImage = 0;
		}
		if (StretchImage)
		{
			delete StretchImage;
			StretchImage = 0;
		}
		// end ges256
		
		PreparePalette();
		SetImageInformation(Image);
		SetPan();
	}
}
void G42ZoomImageViewer::ImageBeingDeleted(G42Image * image)
{
	if (image == Image)
		Image = 0;
	Clear();
}
void G42ZoomImageViewer::HaveImageEnd(void)
{
	#ifdef MSWIN		// (m.3)
//	SetPan();
	if (DC)
	{
		ReleaseDC(Window, DC);
		DC = 0;
	}
	PreparePalette();
	Draw();
	#endif
}
void G42ZoomImageViewer::HaveImageInfo(void)
{
	SetImageInformation(Image);
	#ifdef MSWIN	// (m.3)
	DC = GetDC(Window);
	if (ScreenType != TrueColor)
	{
		PreparePalette();
		SelectPalette(DC, DitherWindowPalette, false);
		RealizePalette(DC);
	}
	#endif
	SetZoom(1.0);
	Draw();
	ProgressiveNumRows = 0;
}
void G42ZoomImageViewer::HaveImageRow(uint32 row)
{
	if (!ProgressiveNumRows)
	{
		ProgressiveStartRow = ProgressiveEndRow = row;
		ProgressiveNumRows = 1;
		if (row && row != Image->GetInfo().Height - 1)
			return;
	}
	if (row < ProgressiveStartRow)
		ProgressiveStartRow = row;
	if (row > ProgressiveEndRow)
		ProgressiveEndRow = row;
	ProgressiveNumRows++;
	if (row && row != Image->GetInfo().Height - 1 &&
			ProgressiveNumRows < 16)
		return;
// need to draw a row here
//	Draw();
	G42ZoomInfo zoom_info(ZoomMode, ZoomInt, ZoomFactor);
	int32 draw_x = -PanX;
	int32 draw_width = zoom_info.ImageToWindow(
		(int32)Image->GetInfo().Width);
	if (draw_x < 0)
	{
		draw_width += draw_x;
		draw_x = 0;
	}
	if (draw_x + draw_width > WindowWidth)
	{
		draw_width = WindowWidth - draw_x;
	}
	if (draw_width <= 0)
		return;
	int32 draw_y = zoom_info.ImageToWindow(
		(int32)ProgressiveStartRow) - PanY;
	if (draw_y >= WindowHeight)
		return;
	int32 draw_height = zoom_info.ImageToWindow((int32)(
		ProgressiveEndRow + 1)) -
		PanY - draw_y;
	if (draw_height <= 0)
		draw_height = 1;
	if (DitherBaseImage)
	{
		Dither(Image->GetData(), DitherBaseImage->GetData(),
			0, ProgressiveStartRow, Image->GetInfo().Width - 1,
			ProgressiveEndRow,
			DitherPalette, false);
		DrawImage(DC, DitherBaseImage, (int)draw_x, (int)draw_y,
			(int)draw_width, (int)draw_height,
			zoom_info, PanX, PanY, 0, zoom_info);
	}
	else
	{
		DrawImage(DC, Image, (int)draw_x, (int)draw_y,
			(int)draw_width, (int)draw_height,
			zoom_info, PanX, PanY, 0, zoom_info);
	}
	ProgressiveNumRows = 0;
}
void G42ZoomImageViewer::SetZoomFitAll(void)
{
	if (Image && Image->HasInfo() && WindowWidth > Margin * 2 &&
		WindowHeight > Margin * 2)
	{	
#ifdef MSWIN
		if (HBarVisible)
		{
			InFit++;
			HBar->ShowScrollBar(false);
			InFit--;
			HBarVisible = false;
		}
		if (VBarVisible)
		{
			InFit++;
			VBar->ShowScrollBar(false);
			InFit--;
			VBarVisible = false;
		}
#endif
		double zoom_width = (double)(WindowWidth - Margin * 2) /
			(double)Image->GetInfo().Width;
		double zoom_height = (double)(WindowHeight - Margin * 2) /
			(double)Image->GetInfo().Height;
		SetZoom(min(zoom_width, zoom_height));
	}
#ifdef NOMACOS
	if (Image && Image -> HasInfo())
	{
		/*  Step 1:  Grab the screen dimensions  */
		int imageWidth 	 	= Image -> GetInfo().Width;
		int imageHeight 	= Image -> GetInfo().Height;
		Rect screenRect 	= UScreenPort::GetScreenPort() -> portRect;
		if (screenRect.left < 0)
			screenRect.left = 0;
		int screenWidth 	= screenRect.right - screenRect.left;
		int	screenHeight 	= screenRect.bottom - screenRect.top - 25;
		/*  Step 2:  If the image doesn't fit in the screen, we must set a zoom */
		if (imageWidth > screenWidth || imageHeight > screenHeight)
		{
			double zoom_width = (double) (screenWidth - (Margin << 1)) /
				(double) Image -> GetInfo().Width;
			double zoom_height = (double) (screenHeight - (Margin << 1)) /
				(double) Image -> GetInfo().Height;
			SetZoom (min (zoom_width, zoom_height));
		}	
	}
#endif
}
void G42ZoomImageViewer::SetZoomFitWidth(void)
{
	if (Image && Image->HasInfo() && WindowWidth > Margin * 2)
	{
#ifdef MSWIN
		if (HBarVisible)
		{
			InFit++;
			HBar->ShowScrollBar(false);
			InFit--;
			HBarVisible = false;
		}
#endif	
		SetZoom((double)(WindowWidth - Margin * 2) /
			(double)Image->GetInfo().Width);
	}
#ifdef NOMACOS
	if (Image && Image -> HasInfo())
	{
		/*  Step 1:  Grab the screen dimensions  */
		int imageWidth 	 	= Image -> GetInfo().Width;
		Rect screenRect 	= UScreenPort::GetScreenPort() -> portRect;
		if (screenRect.left < 0)
			screenRect.left = 0;
		int screenWidth 	= screenRect.right - screenRect.left;
		/*  Step 2:  If the image doesn't fit in the screen, we must set a zoom */
		if (imageWidth > screenWidth)
		{
			double zoom_width = (double) (screenWidth - (Margin << 1)) / (double) imageWidth;
			SetZoom (zoom_width);
		}	
	}
#endif
}
void G42ZoomImageViewer::SetZoom(double zoom)
{
	if (!WindowWidth || !WindowHeight)
		return;
	uint32 center_x = WindowToImage(((WindowWidth >> 1) + WindowX) + PanX);
	uint32 center_y = WindowToImage(((WindowHeight >> 1) + WindowY) + PanY);
	if ((int)(zoom * (double)WindowWidth + .5) == WindowWidth &&
		(int)(zoom * (double)WindowHeight + .5) == WindowHeight)
	{
		ZoomMode = NotZoomed;
		ZoomFactor = 1.0;
	}
	else if (zoom > 1.0)
	{
		if (zoom > 128.0)
      	zoom = 128.0;
		ZoomInt = (zoom + .5);
		if ((int)(zoom * (double)WindowWidth + .5) == ZoomInt * WindowWidth &&
			(int)(zoom * (double)WindowHeight + .5) == ZoomInt * WindowHeight)
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
		if (((double)Image->GetInfo().Width * zoom) < 4.0)
      	zoom = 4.0 / (double)Image->GetInfo().Width;
      if (((double)Image->GetInfo().Height * zoom) < 4.0)
      	zoom = 4.0 / (double)Image->GetInfo().Height;
		ZoomInt = (1.0 / zoom + .5);
		if (ZoomInt && (int)(zoom * (double)(WindowWidth - 1) + .5) ==
				(WindowWidth - 1) / ZoomInt &&
			(int)(zoom * (double)(WindowHeight - 1) + .5) ==
				(WindowHeight - 1) / ZoomInt)
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
#ifdef MACOS
	WindowX = ImageToWindow(center_x) - (WindowWidth >> 1);
	WindowY = ImageToWindow(center_y) - (WindowHeight >> 1);
#else
	PanX = ImageToWindow(center_x) - (WindowWidth >> 1);
	PanY = ImageToWindow(center_y) - (WindowHeight >> 1);
#endif
	SetPan();
	if (StretchImage)
	{
		delete StretchImage;
		StretchImage = 0;
	}
	if (DitherImage)
	{
		delete DitherImage;
		DitherImage = 0;
	}
}
/* 	MSWIN & MACOS version D. Ison  */
void
G42ZoomImageViewer::SetPan(void)
{
	if (Image && Image->HasInfo() && WindowWidth && WindowHeight)
	{
		int32 image_width = ImageToWindow(Image->GetInfo().Width);
		int32 image_height = ImageToWindow(Image->GetInfo().Height);
		if (image_width <= (uint32)WindowWidth)
		{
			PanX = ((image_width - (int32)WindowWidth) >> 1);
			#ifdef MSWIN
			if (HBarVisible)
			{
				HBar->ShowScrollBar(false);
				HBarVisible = false;
			}
			#endif
			#ifdef MACOS
			WindowX = 0;
			#endif
		}
		else
		{
			#ifdef MACOS
			if (WindowX < 0)
				WindowX = 0;
			if (WindowX + (int32)WindowWidth > image_width + (int32)(Margin << 1))
				WindowX = image_width + (int32)(Margin << 1) - (int32)WindowWidth;
			PanX = -Margin;			
			#else
			if (PanX < (int32)(-Margin))
				PanX = -Margin;
			if (PanX + (int32)WindowWidth > image_width + (int32)Margin)
				PanX = image_width + (int32)Margin - (int32)WindowWidth;
			#endif
			#ifdef MSWIN
			if (!HBarVisible)
			{
				HBar->ShowScrollBar(true);
				HBarVisible = true;
			}
			HBar->SetInitPos(-Margin, image_width + Margin - WindowWidth,
				PanX, 1, WindowWidth >> 1);
			#endif
		}
		if (image_height <= (uint32)WindowHeight)
		{
			PanY = ((image_height - (int32)WindowHeight) >> 1);
			#ifdef MSWIN
			if (VBarVisible)
			{
				VBar->ShowScrollBar(false);
				VBarVisible = false;
			}
			#endif
			#ifdef MACOS
			WindowY = 0;
			#endif			
		}
		else
		{
			#ifdef MACOS
			if (WindowY < 0)
				WindowY = 0;
			if (WindowY + (int32)WindowHeight > image_height + (int32)(Margin << 1))
				WindowY = image_height - (int32)WindowHeight + (int32)(Margin << 1);
			PanY = -Margin;
			#else
			if (PanY < (int32)(-Margin))
				PanY = -Margin;
			if (PanY + (int32)WindowHeight > image_height + (int32)Margin)
				PanY = image_height - (int32)WindowHeight + (int32)Margin;
			#endif
			#ifdef MSWIN
			if (!VBarVisible)
			{
				VBar->ShowScrollBar(true);
				VBarVisible = true;
			}
			VBar->SetInitPos(-Margin, image_height + Margin - WindowHeight,
				PanY, 1, WindowHeight >> 1);
			#endif
		}
	}
	else
	{
		PanX = PanY = -Margin;
		#ifdef MSWIN
		if (HBar && HBarVisible)
		{
			HBar->ShowScrollBar(false);
        	HBarVisible = false;
		}
		if (VBar && VBarVisible)
		{
			VBar->ShowScrollBar(false);
			VBarVisible = false;
		}
		#endif
	}
	if (DitherImage || StretchImage) // ges256
		CheckPartial = true; // ges256
}
#ifdef MACOS
void G42ZoomImageViewer::SetPanXY (int x, int y)
{
// David: I've introduced WindowX and WindowY in g42iview.h and I believe
// you want to use these instead of the Pan variables.
//	PanX = x;
//	PanY = y;
	SetWindowOffset(x, y);
}
void G42ZoomImageViewer::GetPanXY (int & x, int & y)
{
//	x = PanX;
//	y = PanY;
	x = GetWindowX();
	y = GetWindowY();
}
#endif
void G42ZoomImageViewer::SetWindowSize(int width, int height)
{
	G42SimpleImageViewer::SetWindowSize(width, height);
	if (InFit)
		return;
	SetPan();
	#ifndef MACOS
	Draw();
	#endif
}
#ifdef MSWIN
void G42ZoomImageViewer::ProcessHScrollBarCode(uint code, uint pos)
{
	if (HBar && HBarVisible)
	{
		HBar->ProcessScrollMsg(code, pos);
		int dx = PanX - HBar->GetCurrentPos();
		if (dx)
			ScrollWindow(Window, dx, 0, 0, 0);
		PanX = HBar->GetCurrentPos();
		// start ges256
		if (ScreenType != TrueColor || ZoomMode != NotZoomed)
		{
			CheckPartial = true;
			NeedIdle = true;
		}
		// end ges256
//		Draw();
	}
}
void G42ZoomImageViewer::ProcessVScrollBarCode(uint code, uint pos)
{
	if (VBar && VBarVisible)
	{
		VBar->ProcessScrollMsg(code, pos);
		int dy = PanY - VBar->GetCurrentPos();
		if (dy)
			ScrollWindow(Window, 0, dy, 0, 0);
		PanY = VBar->GetCurrentPos();
		// start ges256
		if (ScreenType != TrueColor || ZoomMode != NotZoomed)
		{
			CheckPartial = true;
			NeedIdle = true;
		}
		// end ges256
//		Draw();
	}
}
void G42ZoomImageViewer::WindowSizeChanged(void)
{
	G42SimpleImageViewer::WindowSizeChanged();
	if (InFit)
		return;
	SetPan();
	Draw();
}
bool
G42ZoomImageViewer::QueryNewPalette(void)
{
	bool ret = G42SimpleImageViewer::QueryNewPalette();
   if (ret)
   	Draw();
	return ret;
}
// end ges256
void
G42ZoomImageViewer::PaletteChanged(HWND who)
{
	if (who == Window || who == TopWindow) // don't respond to ourselves
		return;
	G42SimpleImageViewer::PaletteChanged(who);
   Draw();
}
#endif
void 
G42ZoomImageViewer::TranslateScreenToImage(int32 & x, int32 & y)
{
	G42ZoomInfo zi(ZoomMode, ZoomInt, ZoomFactor);
	x = zi.WindowToImage(x + PanX); 
	y = zi.WindowToImage(y + PanY);
	/*  Clipping  */
	if (Image && Image -> HasInfo())
	{	
		if (x < 0) 
			x = 0;  
		if (y < 0) 
			y = 0; 
		int32 clipWidth  = Image -> GetInfo().Width - 1;
		int32 clipHeight = Image -> GetInfo().Height - 1;
		if (x > clipWidth)
			x = clipWidth;
		if (y > clipHeight)
			y = clipHeight;
	}	
}	
