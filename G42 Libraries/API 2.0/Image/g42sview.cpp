// g42sview.cpp - single image viewer
/*
	G42SingleImageViewer class - displays a single image in a window
	changes:
		ges256 - for displaying in 256 paletted mode
		gessa - stretching selected area code
		gesdtr - adding in an unzoomed dithered image for speed
*/
#include <g42sview.h>
#include <g42image.h>
#include <gesdebug.h>
const Margin = 10; // area outside image when zoomed in
#ifdef MSWIN
G42SingleImageViewer::G42SingleImageViewer(HWND window, HWND top_window) :
	#ifdef MSWIN
	G42ZoomImageViewer(window, top_window),
	DC(0),
	TextStyle(0), WindowsCursor(0),
	TransCursor(0), Shadow(0), ShadowDC(0),
	#endif
	#ifdef MACOS	// (m.2) Some changes
	G42ZoomImageViewer(),
	CurrentCursor (&qd.arrow), GWindowPtr (windowPtr), 	CallBack (CallBackFn),
	GWorldTile (0), PixMapTile (0), GWindowPtr (0), GPaletteHandle (0),
	#endif
	SelectedStretchImage(0), SelectedDitherImage(0), SelectedDitherBaseImage(0),
	ImageMap(0), ImageMapVisible(false),
	SelectedAreaX(0),
	SelectedAreaY(0),
	SelectedAreaWidth(0),
	SelectedAreaHeight(0),
	MovingArea(false),
	StretchingArea(false),
	AreaSelected(false),
	SelectingArea(false),
	SelectedImage(0),
	NeedMainIdle(false), NeedSelectedIdle(false), CheckMainPartial(false),
	CheckSelectedPartial(false),
	TextVisible(false), TextStarted(false), NumTextArray(0), ErasingText(0),
	EditingPolygon(false),
	MapTool(MapEditTool), NewMapElementStarted(false),
	NewPolygonStarted(false), NewPolygonClosed(false), MovingElement(false),
	CurrentElement(-1), StretchingElement(false), ShiftInverted(false),
	OldControlDown(false), SettingCursor(0), TransparentMode(false),
	ReportingTransColors(false)
{
GESTRACE("Library View Constructor");
	WindowsCursor = LoadCursor(0, IDC_ARROW);
}
#endif
#ifdef MACOS
G42SingleImageViewer::G42SingleImageViewer (GWorldPtr windowPtr, short xOrigin,
	short yOrigin) :
	#ifdef MSWIN
	G42ZoomImageViewer(window, top_window),
	DC(0),
	TextStyle(0), WindowsCursor(0),
	TransCursor(0),
	#endif
	#ifdef MACOS	// (m.2) Some changes
	G42ZoomImageViewer(windowPtr, 0),
	CurrentCursor (&qd.arrow),
	ActivePort ((GWorldPtr) windowPtr),
	ShadowPort (0),
	XOrigin (xOrigin),
	YOrigin (yOrigin),
	OldPrepDrawMapPort (0),
	OldPrepDrawRubberPort (0),
	OldPrepDrawMapDevice (0),
	OldPrepDrawRubberDevice (0),
	UseShadow (true),
	#endif
	SelectedStretchImage(0), SelectedDitherImage(0), SelectedDitherBaseImage(0),
	ImageMap(0), ImageMapVisible(false),
	SelectedAreaX(0),
	SelectedAreaY(0),
	SelectedAreaWidth(0),
	SelectedAreaHeight(0),
	MovingArea(false),
	StretchingArea(false),
	AreaSelected(false),
	SelectingArea(false),
	NeedMainIdle(false), NeedSelectedIdle(false), CheckMainPartial(false),
	CheckSelectedPartial(false),
	SelectedImage(0),
	TextVisible(false), TextStarted(false), NumTextArray(0), ErasingText(0),
	EditingPolygon(false),
	MapTool(MapEditTool), NewMapElementStarted(false),
	NewPolygonStarted(false), NewPolygonClosed(false), MovingElement(false),
	CurrentElement(-1), StretchingElement(false), ShiftInverted(false),
	OldControlDown(false), SettingCursor(0), TransparentMode(false),
	ReportingTransColors(false)
{
}
#endif
GESTRACEBASE
G42SingleImageViewer::~G42SingleImageViewer()
{
GESTRACEOUT();
	Clear(); // deletes selected image, dithered images, and palettes
	#ifdef MSWIN
	if (ShadowDC)
	{
		DeleteDC(ShadowDC);
		ShadowDC = 0;
	}
	if (Shadow)
	{
		DeleteObject(Shadow);
		Shadow = 0;
	}
	#endif
	#ifdef MACOS
	if (ShadowPort)
	{
		DisposeGWorld (ShadowPort);
		ShadowPort = nil;
	}
	#endif
}
// clear out all images and other information
void
G42SingleImageViewer::Clear(void)
{
	if (SelectedImage)
	{
		SelectedImageChanged(0);
		delete SelectedImage;
		SelectedImage = 0;
	}
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
}
// start ges256
bool
G42SingleImageViewer::IdleAction(void)
{
	if (!Valid)
		return false;
	if (NeedIdle)
	{
		NeedSelectedIdle = true;
		NeedMainIdle = true;
		NeedIdle = false;
	}
	if (!NeedMainIdle && !NeedSelectedIdle)
		return true;
	if (CheckPartial)
	{
		CheckSelectedPartial = true;
		CheckMainPartial = true;
		CheckPartial = false;
	}
	if (ScreenType != TrueColor && NeedDither && !DitherPalette)
		return true;
GESTRACESTART("Idle Action Start");
GESTRACESTORE(OldDraw, GesTraceDraw);
GESTRACESTORE(OldDither, GesTraceDither);
	G42ZoomInfo zoom_info(ZoomMode, ZoomInt, ZoomFactor);
	bool ret = false;
	if (SelectedImage && NeedSelectedIdle)
	{
		int32 pan_x = PanX - ImageToWindow(SelectedAreaX);
		int32 pan_y = PanY - ImageToWindow(SelectedAreaY);
		CheckPartial = CheckSelectedPartial;
		ret = ImageIdle(SelectedImage, SelectedStretchImage, SelectedDitherImage,
			SelectedDitherBaseImage, pan_x, pan_y);
		CheckSelectedPartial = CheckPartial;
		CheckPartial = false;
		if (!ret)
		{
			NeedSelectedIdle = false;
		}
	}
	if (!ret)
	{
		CheckPartial = CheckMainPartial;
		ret = ImageIdle(Image, StretchImage, DitherImage, DitherBaseImage,
			PanX, PanY);
		CheckMainPartial = CheckPartial;
		CheckPartial = false;
		if (!ret)
			NeedMainIdle = false;
	}
GESTRACEEND("Idle Action End");
GESTRACEADD(GesTraceOther, ges_trace_end - ges_trace - (GesTraceDraw - OldDraw) - (GesTraceDither - OldDither));
	return true;
}
void G42SingleImageViewer::Draw(void)
{
	G42ZoomImageViewer::Draw();
}
void G42SingleImageViewer::PutShadow(int draw_x, int draw_y,
	int draw_width, int draw_height)
{
#ifdef MSWIN
	HDC dc = GetDC(Window);
	BitBlt(dc, draw_x, draw_y, draw_width, draw_height,
		ShadowDC, draw_x, draw_y, SRCCOPY);
	ReleaseDC(Window, dc);
	if (ShadowDC)
	{
		DeleteDC(ShadowDC);
		ShadowDC = 0;
	}
	if (Shadow)
	{
		DeleteObject(Shadow);
		Shadow = 0;
	}
#endif
#ifdef MACOS
	if (ShadowPort)
	{
		Rect sourceRect, destRect;
		::SetRect (&destRect, draw_x, draw_y, (draw_x + draw_width), (draw_y + draw_height));
		sourceRect = destRect;
		short transferMode = srcCopy;
		short sourceDepth 	= (**ShadowPort -> portPixMap).cmpCount * (**ShadowPort -> portPixMap).cmpSize;
		short destDepth 	= (**ActivePort -> portPixMap).cmpCount * (**ActivePort -> portPixMap).cmpSize;
		if (sourceDepth > 8 && destDepth <= 8)
			transferMode += ditherCopy;
		::LockPixels (::GetGWorldPixMap (ShadowPort));
		::LockPixels (ActivePort -> portPixMap);
		::CopyBits((BitMapPtr) &ShadowPort->portPixMap,
				(BitMapPtr) &ActivePort -> portPixMap,
				&sourceRect, &destRect, transferMode, nil);
		::UnlockPixels (ActivePort -> portPixMap);
		::UnlockPixels (::GetGWorldPixMap (ShadowPort));
	}
#endif
}
void G42SingleImageViewer::PrepareShadow(int, int, int, int)
{
#ifdef MSWIN
	HDC dc = GetDC(Window);
	if (!Shadow)
	{
		if (!WindowWidth || !WindowHeight)
		{
			ReleaseDC(Window, dc);
			return;
		}
		Shadow = CreateCompatibleBitmap(dc, WindowWidth, WindowHeight);
	}
	ShadowDC = CreateCompatibleDC(dc);
	ReleaseDC(Window, dc);
	SelectObject(ShadowDC, Shadow);
#endif
#ifdef MACOS
	CGrafPtr	savePort;
	GDHandle	saveDevice;
	Rect 		rect;
//	#define NO_SHADOW
	#ifdef NO_SHADOW
	return;
	#endif
	if (! UseShadow)
		return;
		
	if (! ShadowPort)
	{
		/*  Create one and set it up like the current window  */
		if (! WindowWidth || ! WindowHeight)
			return;
		::SetRect (&rect, WindowX, WindowY, WindowX + WindowWidth, WindowY + WindowHeight);
		::GetGWorld (&savePort, &saveDevice);
		short depth = GetScreenDepth();
		QDErr err = ::NewGWorld (&ShadowPort, depth, &rect, nil, nil, useTempMem);
		if (! err)
		{
			/*  Make sure there's at least some free _after_ gworld is created  */
			long bytes = ::MaxBlock();
			if (bytes < (256 * 1024))
			{
				DisposeGWorld (ShadowPort);
				ShadowPort = nil;
			}
			else
			{
				::SetGWorld (ShadowPort, nil);
				::SetOrigin (rect.left, rect.top);
				::SetGWorld (savePort, saveDevice);
			}
		}
		else
			ShadowPort = nil;
	}
	else
	{
		/*  Reconfigure the shadow so it mimics the window  */
		::GetGWorld (&savePort, &saveDevice);
		::SetGWorld (ShadowPort, nil);
		::SetOrigin (WindowX, WindowY);		// (Somehow this works for scrolling)
		::SetGWorld (savePort, saveDevice);
	}
#endif
}
#if 0
// PALETTE MUCK
#define ScreenDepth(gdh)	((**((**gdh).gdPMap)).pixelSize)
void SetGrayPalette(short depth, CWindowPtr w);
#define COLORS	16
#define WIDTH	256
#define HEIGHT	256
#define MAGNIFY	1.0
#define USE_GRAY	1
unsigned char *Graph;	/* contents of file */
#define EXACT	0		/* used with pmTolerant, only exact matches */
// END PALETTE MUCK
void SetGrayPalette(short depth, CWindowPtr w)
{
	short colors = 1 << depth;
	CTabHandle ctab = GetCTable(depth);
	PaletteHandle pal = NewPalette((**ctab).ctSize, nil, pmTolerant, EXACT);
	ColorSpec *specs;
	short i;
	specs = (**ctab).ctTable;
	for (i = 0; i < colors; ++i) {
			specs[i].rgb.red = specs[i].rgb.green = specs[i].rgb.blue
					= i * 65535 / (COLORS - 1);
			specs[i].value = i;
	}
	/* this alerts the color mgr that the table changed */
	(**ctab).ctSeed = GetCTSeed();
	CTab2Palette(ctab, pal, pmTolerant, EXACT);
	SetPalette((WindowPtr) w, pal, TRUE);
	ActivatePalette((WindowPtr) w);
}
#endif
void
G42SingleImageViewer::PrepareDrawImage(G42DrawLocation location,
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
	inherited::PrepareDrawImage (location, draw_x, draw_y, draw_width, draw_height);
	#if 0
	G42Image *image = Image;
	int numPalette = image -> GetInfo().NumPalette;
	if (numPalette != 0)
	{
		CTabHandle ctab = GetCTable (8);
		GPaletteHandle = ::NewPalette ((**ctab).ctSize, nil, pmTolerant, 0);
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
		::CTab2Palette (ctab, GPaletteHandle, pmTolerant, 0);
		::SetPalette ((GrafPtr) -1, GPaletteHandle, true);
		::ActivatePalette ((GrafPtr) ActivePort);
	}
	#endif
	#if 0
	::SetOrigin (XOrigin, YOrigin);
	#endif
	#endif
}
void
G42SingleImageViewer::CleanupDrawImage(G42DrawLocation location)
{
	#ifdef MACOS
	inherited::CleanupDrawImage (location);
	#endif
}
void G42SingleImageViewer::Draw(int draw_x, int draw_y,
	int draw_width, int draw_height, bool expand)
{
GESTRACESTART("Draw Start");
	if (expand)
	{
		// fudge partial draws by a pixel
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
	PrepareShadow(draw_x, draw_y, draw_width, draw_height);
	#ifdef MSWIN
	G42DrawLocation location = ShadowDC;
//	G42DrawLocation location = GetDC(Window);
	#endif
	#ifdef MACOS
	G42DrawLocation location = (ShadowPort ? ShadowPort : ActivePort);
	DC = ActivePort;
	#endif
	Draw(location, draw_x, draw_y, draw_width, draw_height, expand);
	PutShadow(draw_x, draw_y, draw_width, draw_height);
	#ifdef MSWIN
//	ReleaseDC(Window, dc);
	#endif
GESTRACEEND("Draw End");
GESTRACEADD(GesTraceDraw, ges_trace_end - ges_trace);
if (Image && Image->IsFinished() && !NeedDither)
{
GESTRACETOTAL();
GESTRACEOUT();
}
}
void G42SingleImageViewer::Draw(G42DrawLocation location,
	int draw_x, int draw_y,
	int draw_width, int draw_height, bool expand)
{
	#ifdef MACOS
	/*  For drawing the selected area, we want to ensure calling InitPixmap() in iview.cpp  */
	if (AreaSelected)
		ChangedForDraw = true;
	#endif
	G42ZoomImageViewer::Draw(location, draw_x, draw_y, draw_width, draw_height,
		expand);
	if (AreaSelected)
	{
		PrepareDrawArea(location);
		DrawArea(location);
		CleanupDrawArea(location);
		if (StretchingArea)
		{
			PrepareDrawRubber(location);
			DrawRubberImageRect(location, OtherX, OtherY, CurrentX, CurrentY);
			CleanupDrawRubber(location);
		}
	}
	if (SelectingArea)
	{
		PrepareDrawRubber(location);
		DrawRubberImageRect(location, StartX, StartY, CurrentX, CurrentY);
		CleanupDrawRubber(location);
	}
	if (ImageMap)
	{
		PrepareDrawMap(location);
		DrawMap(location);
		CleanupDrawMap(location);
	}
	if (TextVisible)
	{
		PrepareDrawText(location);
		DrawText(location);
		CleanupDrawText(location);
	}
}
#ifdef MSWIN
void
G42SingleImageViewer::PrintImage(HDC dc,
	int32 x, int32 y, int32 width, int32 height)
{
	if (!Image || !Image->IsFinished() || !Image->HasInfo())
		return;
	SetImageInformation(Image);
	double zoom = (double)width / (double)(Image->GetInfo().Width);
	double zoom_h = (double)height / (double)(Image->GetInfo().Height);
	int32 print_x = x;
	int32 print_y = y;
	if (zoom > zoom_h)
	{
		zoom = zoom_h;
		int32 print_width = (int32)(zoom * (double)Image->GetInfo().Width + .5);
		print_x += ((width - print_width) >> 1);
	}
	else
	{
		int32 print_height = (int32)(zoom * (double)Image->GetInfo().Height + .5);
		print_y += ((height - print_height) >> 1);
	}
	for (uint row = 0; row < Image->GetNumTilesDown(); row++)
	{
		for (uint column = 0; column < Image->GetNumTilesAcross(); column++)
		{
			G42ImageTile * itile = Image->GetTile(row, column);
			if (!itile)
				continue;
			int tile_width = itile->GetTileWidth();
			int tile_height = itile->GetTileHeight();
			int tile_start_x = itile->GetTileOffsetX();
			int tile_start_y = itile->GetTileOffsetY();
			int32 print_tile_start_x = (int32)(zoom *
				(double)itile->GetImageOffsetX() + .5) + print_x;
			int32 print_tile_width = (int32)(zoom *
				(double)(itile->GetImageOffsetX() + itile->GetTileWidth()) + .5) +
				print_x - print_tile_start_x;
			int32 print_tile_start_y = (int32)(zoom *
				(double)itile->GetImageOffsetY() + .5) + print_y;
			int32 print_tile_height = (int32)(zoom *
				(double)(itile->GetImageOffsetY() + itile->GetTileHeight()) + .5) +
				print_y - print_tile_start_y;
			G42LockedImageTile tile(itile);
			byte * tile_ptr = *(tile.GetTile()->GetRow(
				tile_start_y + tile_height - 1));
			DibHeader->biWidth = (int)Image->GetTileWidth();
			DibHeader->biHeight = tile_height;
			DibHeader->biSizeImage = (DWORD)tile_height *
				(DWORD)tile.GetTile()->GetRowBytes();
			if (tile_width == print_tile_width &&
				tile_height == print_tile_height)
			{
				SetDIBitsToDevice(dc,
					print_tile_start_x, print_tile_start_y,
					tile_width, tile_height,
					tile_start_x, 0,
					0, tile_height, tile_ptr,
						(BITMAPINFO *)DibHeader, DibUsage);
			}
			else
			{
				StretchDIBits(dc,
					print_tile_start_x, print_tile_start_y,
					print_tile_width, print_tile_height,
					tile_start_x, 0,
					tile_width, tile_height,
					tile_ptr, (BITMAPINFO *)DibHeader, DibUsage, SRCCOPY);
			}
		}
	}
}
#endif // MSWIN
void
G42SingleImageViewer::PreparePalette(void)
{
	#ifdef MSWIN		// (m.3)
	if (!SelectedImage)
	{
		G42ZoomImageViewer::PreparePalette();
		return;
	}
	if (ScreenType != Palette256)
		return;
	if (Image && Image->HasInfo() && Image->IsFinished())
	{
		if (DitherPalette != DitherBasePalette)
		{
			delete DitherPalette;
		}
		DitherPalette = new G42OptimizedPalette(Image->GetData(), 236,
			false, false, true, // true really should be false,
			(SelectedImage ? SelectedImage->GetData() : 0));
		SetWindowsPalette(DitherPalette->GetPalette(),
			DitherPalette->GetNumPalette());
	}
	#endif
}
void G42SingleImageViewer::Rebuild(void)
{
	if (SelectedImage)
	{
		SelectedImage->RebuildInfo();
		SelectedAreaWidth = SelectedImage->GetInfo().Width;
		SelectedAreaHeight = SelectedImage->GetInfo().Height;
		// start ges256
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
		if (SelectedStretchImage)
		{
			delete SelectedStretchImage;
			SelectedStretchImage = 0;
		}
		// end ges256
	}
	G42ZoomImageViewer::Rebuild();
}
void G42SingleImageViewer::SetZoom(double zoom)
{
	G42ZoomImageViewer::SetZoom(zoom);
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
	/*  Kill the shadow buffer  */
	#ifdef MACOS
	if (ShadowPort)
	{
		DisposeGWorld (ShadowPort);
		ShadowPort = 0;
	}	
	#endif
}
void
G42SingleImageViewer::PutAreaDown(void)
{
	if (AreaSelected && SelectedImage)
	{
		SelectedImageChanged(0);
		Image->CombineImage(SelectedImage, SelectedAreaX, SelectedAreaY);
		AreaSelected = false;
		delete SelectedImage;
		SelectedImage = 0;
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
		if (SelectedStretchImage)
		{
			delete SelectedStretchImage;
			SelectedStretchImage = 0;
		}
		SetCursor(ArrowCursor);
		Image->RebuildInfo();
		// start ges256
		if (StretchImage)
		{
			delete StretchImage;
			StretchImage = 0;
			NeedIdle = true;
		}
		// draw before removing dithers so we don't erase the whole thing
		// under 8 bit.  The idles should take care of putting the image
		// back on the screen if the draw doesn't do it for us.
		#if 0
		Draw();
		#else
		Draw (SelX - SelectMargin, SelY - SelectMargin, 
				SelWidth + (SelectMargin << 1), SelHeight + (SelectMargin << 1), 
				true);
		#endif
	
		if (DitherImage)
		{
			delete DitherImage;
			DitherImage = 0;
			NeedIdle = true;
		}
		if (DitherBaseImage)
		{
			delete DitherBaseImage;
			DitherBaseImage = 0;
			NeedIdle = true;
		}
		// end ges256
	}
}
// create a selected area - these are in image coordinates
void
G42SingleImageViewer::SelectArea(int32 x1, int32 y1, int32 x2, int32 y2)
{
	if (!Image || !Image->HasInfo())
		return;
	SelectedAreaX = min(x1, x2);
	SelectedAreaY = min(y1, y2);
	SelectedAreaWidth = max(x1, x2) - SelectedAreaX + 1;
	SelectedAreaHeight = max(y1, y2) - SelectedAreaY + 1;
	if (SelectedAreaX < 0)
	{
		SelectedAreaWidth += SelectedAreaX;
		SelectedAreaX = 0;
	}
	if (SelectedAreaY < 0)
	{
		SelectedAreaHeight += SelectedAreaY;
		SelectedAreaY = 0;
	}
	if (SelectedAreaX + SelectedAreaWidth > Image->GetInfo().Width)
		SelectedAreaWidth = Image->GetInfo().Width - SelectedAreaX;
	if (SelectedAreaY + SelectedAreaHeight > Image->GetInfo().Height)
		SelectedAreaHeight = Image->GetInfo().Height - SelectedAreaY;
	if (SelectedAreaWidth <= 1 || SelectedAreaHeight <= 1)
		return;
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
	if (SelectedStretchImage)
	{
		delete SelectedStretchImage;
		SelectedStretchImage = 0;
	}
	if (SelectedImage)
		delete SelectedImage;
	SelectedImage = new G42Image(*Image,
		(uint32)SelectedAreaX, (uint32)SelectedAreaY,
		(uint32)SelectedAreaWidth, (uint32)SelectedAreaHeight);
	SelectedImageChanged(SelectedImage);
	AreaSelected = true;
	
//		if (erase)	
		{
	  		/* Make a white image & put it where the selection was/is  */
			G42Image blank_image (SelectedImage -> GetMemoryManager());	
			G42Color color (255, 255, 255);
		
			G42ImageInfo info;
			info.Width = SelectedImage -> GetInfo().Width;
			info.Height = SelectedImage -> GetInfo().Height;
			info.PixelDepth = 4;
			info.ColorType = G42ImageInfo::ColorTypePalette;
			info.NumPalette = 1;
			G42Color * pal = new G42Color [info.NumPalette];
			pal[0] = color;
			info.ImagePalette.Set(pal);
			info.ImageType = Unknown;
			blank_image.SetInfo(info);
			blank_image.FloodFill(color, color, color, 0, 0, 0);
			if (Image -> GetMask())
			{
				blank_image.SetTransparency (G42Color (0, 0, 0));
			}
			
			Image -> CombineImage(&blank_image, SelectedAreaX, SelectedAreaY);
		}
}
void
G42SingleImageViewer::SelectedImageChanged(G42Image *)
{
}
void
G42SingleImageViewer::DeleteSelectedImage(bool erase)
{
	if (AreaSelected && SelectedImage)
	{
		#if 0
		if (erase)	
		{
	  		/* Make a white image & put it where the selection was/is  */
			G42Image blank_image (SelectedImage -> GetMemoryManager());	
			G42Color color (255, 255, 255);
		
			G42ImageInfo info;
			info.Width = SelectedImage -> GetInfo().Width;
		   info.Height = SelectedImage -> GetInfo().Height;
		   info.PixelDepth = 4;
		   info.ColorType = G42ImageInfo::ColorTypePalette;
		   info.NumPalette = 1;
		   G42Color * pal = new G42Color [info.NumPalette];
		   pal[0] = color;
		   info.ImagePalette.Set(pal);
		   info.ImageType = Unknown;
		   blank_image.SetInfo(info);
		   blank_image.FloodFill(color, color, color, 0, 0, 0);
			Image -> CombineImage(&blank_image, SelectedAreaX, SelectedAreaY);
		}
		#endif
		
		AreaSelected = false;
		SelectedImageChanged(0);
		delete SelectedImage;
		SelectedImage = 0;
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
		if (SelectedStretchImage)
		{
			delete SelectedStretchImage;
			SelectedStretchImage = 0;
		}
		SetCursor(ArrowCursor);
		#ifndef MACOS
		Draw();
		#endif
	}
}
void
G42SingleImageViewer::AddSelectedImage(G42Image * image, bool reset_location)
{
	if (!image || !image->HasInfo())
	{
		DeleteSelectedImage();
		return;
	}
	if (reset_location)
	{
		G42ZoomInfo zi(ZoomMode, ZoomInt, ZoomFactor);
		SelectedAreaX = zi.WindowToImage(max(WindowX, PanX) + Margin) + 1;
		SelectedAreaY = zi.WindowToImage(max(WindowY, PanY) + Margin) + 1;
	}
	if (AreaSelected && SelectedImage)
	{
		*SelectedImage = *image;
	}
	else
	{
		AreaSelected = true;
		if (SelectedImage)
		{
			*SelectedImage = *image;
		}
		else
		{
			SelectedImage = new G42Image(*image);
		}
		SelectedImageChanged(SelectedImage);
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
	if (SelectedStretchImage)
	{
		delete SelectedStretchImage;
		SelectedStretchImage = 0;
	}
	SelectedAreaWidth = SelectedImage->GetInfo().Width;
	SelectedAreaHeight = SelectedImage->GetInfo().Height;
	Draw();
}
void
G42SingleImageViewer::CreateSelectedImage(int32 x, int32 y,
	int32 width, int32 height)
{
	if (!Image || !Image->HasInfo())
	{
		DeleteSelectedImage();
		return;
	}
	if (AreaSelected && SelectedImage)
	{
		PutAreaDown();
	}
	SelectArea(x, y, x + width - 1, y + height - 1);
	Draw();
}
void
G42SingleImageViewer::SetTransparentMode(bool mode)
{
	TransparentMode = mode;
	if (mode)
		SetCursor(TransparencyCursor);
	else
		SetCursor(ArrowCursor);
}
bool
G42SingleImageViewer::GetTransparentMode(void)
{
	return TransparentMode;
}
// base function stub
void
G42SingleImageViewer::TransparentPointClick(uint32, uint32, bool)
{
}
#ifdef MSWIN
void
G42SingleImageViewer::SetCursor()
{
	if (WindowsCursor)
		::SetCursor(WindowsCursor);
}
void
G42SingleImageViewer::WindowSizeChanged(void)
{
	G42ZoomImageViewer::WindowSizeChanged();
	if (Shadow)
	{
		DeleteObject(Shadow);
		Shadow = 0;
	}
}
#endif
void 
G42SingleImageViewer::SetWindowSize(int width, int height)
{
	G42ZoomImageViewer::SetWindowSize(width, height);
	#ifdef MACOS
	if (ShadowPort)
	{
		DisposeGWorld (ShadowPort);
		ShadowPort = 0;
	}	
	#endif
}
void
G42SingleImageViewer::SetCursor(G42CursorType type)
{
#ifdef MSWIN
	switch (type)
	{
		case ArrowCursor:
			WindowsCursor = LoadCursor(0, IDC_ARROW);
			break;
		case CrossCursor:
			WindowsCursor = LoadCursor(0, IDC_CROSS);
			break;
		case TextCursor:
			WindowsCursor = LoadCursor(0, IDC_IBEAM);
			break;
		case MoveCursor:
			WindowsCursor = LoadCursor(0, IDC_SIZE);
#if defined(WIN32)
         if (!WindowsCursor)
         	WindowsCursor = LoadCursor(0, IDC_SIZEALL);
#endif
			break;
		case StretchHorizontalCursor:
			WindowsCursor = LoadCursor(0, IDC_SIZEWE);
			break;
		case StretchVerticalCursor:
			WindowsCursor = LoadCursor(0, IDC_SIZENS);
			break;
		case StretchTopLeftCursor:
			WindowsCursor = LoadCursor(0, IDC_SIZENWSE);
			break;
		case StretchTopRightCursor:
			WindowsCursor = LoadCursor(0, IDC_SIZENESW);
			break;
		case TransparencyCursor:
			if (TransCursor)
				WindowsCursor = TransCursor;
			else
				WindowsCursor = LoadCursor(0, IDC_ARROW);
			break;
	}
#endif
#ifdef MACOS
	switch (type)
	{
		case ArrowCursor:
	//		::SetCursor (&qd.arrow);
			CurrentCursor = &qd.arrow;
			break;
		case CrossCursor:
	//		::SetCursor (*GetCursor(crossCursor));
			CurrentCursor =  *GetCursor (crossCursor);
			break;
		case TextCursor:					
	//		::SetCursor (*GetCursor(iBeamCursor));
			CurrentCursor =  *GetCursor (iBeamCursor);
			break;
		case MoveCursor:
	//		::SetCursor (*GetCursor(plusCursor));
			CurrentCursor =  *GetCursor (plusCursor);
			break;
	}
#endif		// MACOS
}
/*  ---(Added DrawImageOnly() for Mac clipboard and printing [7-96]) */
/*  Just draws the whole image  									 */
void
G42SingleImageViewer::DrawImageOnly (G42DrawLocation location, G42Image *image)
{
	int width 	= image -> GetInfo().Width;
	int height 	= image -> GetInfo().Height;
	DrawImageOnly (location, image, 0, 0, width, height, false);
}
/*  Optionally scales coordinates of drawing to fit into specified size (ie, for printing) */
void
G42SingleImageViewer::DrawImageOnly (G42DrawLocation location, G42Image *image, 
	int x, int y, int width, int height, bool use_zoom)
{
	int print_x = x;
	int print_y = y;
	int print_width  = width;
	int print_height = height;
	double zoom = 1.0;
	if (use_zoom)
	{
		if (width * image->GetInfo().Height < height * image->GetInfo().Width)
		{
			zoom = (double)width / (double)image->GetInfo().Width;
			print_height = (int)(zoom * (double)(image->GetInfo().Height) + .5);
			print_y += ((height - print_height) >> 1);
		}
		else
		{
			zoom = (double)height / (double)image->GetInfo().Height;
			print_width = (int)(zoom * (double)(image->GetInfo().Width) + .5);
			print_x += ((width - print_width) >> 1);
		}
	}
	G42ZoomImageViewer::PrepareDrawImage (location, 
		print_x, print_y, print_width, print_height);
	G42ZoomInfo zinfo(zoom); 
	G42SimpleImageViewer::DrawImage (location, image, print_x, print_y, 
		print_width, print_height,
		zinfo, -print_x, -print_y, 0, G42ZoomInfo());	
	G42ZoomImageViewer::CleanupDrawImage (location);
}
#if 0
/*  Experimenting with trying to fix the mapping draw bug on Mac  */
void
G42SingleImageViewer::DrawMapOnly (G42DrawLocation location)
{
	if (ImageMap)
	{
		PrepareDrawMap(location);
		DrawMap(location);
		CleanupDrawMap(location);
	}
}
#endif
#ifdef MACOS
void
G42SingleImageViewer::SetUseShadow (Boolean useShadow) 
{
	UseShadow = useShadow;
	if (! UseShadow)
	{
		if (ShadowPort)
		{
			DisposeGWorld (ShadowPort);
			ShadowPort = nil;
		}
	}
}
#endif
