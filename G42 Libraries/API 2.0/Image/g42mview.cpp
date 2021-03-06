
#include <g42itype.h>
#include <g42mview.h>
#ifdef MSWIN
G42MultipleImageViewer::G42MultipleImageViewer(HWND window, HWND top_window) :
	Window(window), TopWindow(top_window), DC(0),
	DibHeader(0), Valid(true), DibUsage(DIB_RGB_COLORS), WindowWidth(0),
	WindowHeight(0)
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
}
#endif
#ifdef MACOS
G42MultipleImageViewer::G42MultipleImageViewer (WindowPtr windowPtr, void (*CallBackFn)()) :
	GWorldTile (0), PixMapTile (0), GWindowPtr (0), GPaletteHandle (0),
	RowBytes (0),
	Valid(true), WindowWidth(0),
	WindowHeight(0)
{
	GWindowPtr = windowPtr;
	Rect rect = GWindowPtr -> portRect;
	WindowWidth = rect.right - rect.left;
	WindowHeight = rect.bottom - rect.top;
	CallBack = CallBackFn;
}
#endif
G42MultipleImageViewer::~G42MultipleImageViewer()
{
	Clear();
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
G42MultipleImageViewer::Clear(void)
{
}
void G42MultipleImageViewer::Draw(G42Image * image,
	int pan_x, int pan_y)
{
	Draw(image, pan_x, pan_y, 0, 0, WindowWidth, WindowHeight);
}
void G42MultipleImageViewer::Draw(HDC dc, G42Image * image,
	int pan_x, int pan_y)
{
	Draw(dc, image, pan_x, pan_y, 0, 0, WindowWidth, WindowHeight);
}
void G42MultipleImageViewer::Draw(G42Image * image, int pan_x, int pan_y,
	int draw_x, int draw_y, int draw_width, int draw_height)
{
	HDC dc = 0;
#if defined(MSWIN)
	dc = GetDC(Window);
#endif
	Draw(dc, image, pan_x, pan_y, draw_x, draw_y, draw_width, draw_height);
#if defined(MSWIN)
	ReleaseDC(Window, dc);
#endif
}
void G42MultipleImageViewer::Draw(HDC dc, G42Image * Image, int pan_x,
	int pan_y, int draw_x, int draw_y, int draw_width, int draw_height)
{
	#ifdef MACOS
	if (! PixMapTile)
		InitPixMap ();
	#endif
	if (Image && Image->IsValid() && Image->HasInfo() && Image->IsFinished() &&
		WindowWidth && WindowHeight)
	{
		if (pan_x > draw_x + draw_width || pan_y > draw_y + draw_height ||
			pan_x + Image->GetInfo().Width <= draw_x ||
			pan_y + Image->GetInfo().Height <= draw_y)
			return;
		#ifdef MSWIN
		SetImageInformation(Image);
		SetStretchBltMode(dc, STRETCH_DELETESCANS);
		#endif
		int32 start_x, start_y, end_x, end_y;
		int PanX = -pan_x;
      int PanY = -pan_y;
		if (PanX + draw_x < 0)
			start_x = -(PanX + draw_x) - 1;
		else
			start_x = 0;
		if (PanY + draw_y < 0)
			start_y = -(PanY + draw_y) - 1;
		else
			start_y = 0;
		end_x = Image->GetInfo().Width - PanX;
		if (end_x > draw_width + draw_x)
			end_x = draw_width + draw_x;
		end_y = Image->GetInfo().Height - PanY;
		if (end_y > draw_height + draw_y)
			end_y = draw_height + draw_y;
		// stretch image - MSWin only
		G42Image * image = Image;
		int32 block_x_start = PanX + start_x;
		if (block_x_start > 0 && block_x_start >= (image->GetTileWidth() -
				image->GetFirstTileOffsetX()))
			block_x_start = (block_x_start + image->GetFirstTileOffsetX()) /
				image->GetTileWidth();
		else
			block_x_start = 0;
		int32 block_y_start = PanY + start_y;
		if (block_y_start > 0 && block_y_start >= (image->GetTileHeight() -
				image->GetFirstTileOffsetY()))
			block_y_start = (block_y_start +
				image->GetFirstTileOffsetY()) / image->GetTileHeight();
		else
			block_y_start = 0;
		int32 block_x_end = PanX + end_x;
		if (block_x_end >= (image->GetTileWidth() -
				image->GetFirstTileOffsetX()))
			block_x_end = (block_x_end + image->GetFirstTileOffsetX()) /
				image->GetTileWidth();
		else
			block_x_end = 0;
		int32 block_y_end = PanY + end_y;
		if (block_y_end >= (image->GetTileHeight() -
				image->GetFirstTileOffsetY()))
			block_y_end = (block_y_end + image->GetFirstTileOffsetY()) /
				image->GetTileHeight();
		else
			block_y_end = 0;
		if (block_x_start < 0)
			block_x_start = 0;
		if (block_x_start >= image->GetNumTilesAcross())
			block_x_start = image->GetNumTilesAcross() - 1;
		if (block_x_end < 0)
			block_x_end = 0;
		if (block_x_end >= image->GetNumTilesAcross())
			block_x_end = image->GetNumTilesAcross() - 1;
		if (block_y_start < 0)
			block_y_start = 0;
		if (block_y_start >= image->GetNumTilesDown())
			block_y_start = image->GetNumTilesDown() - 1;
		if (block_y_end < 0)
			block_y_end = 0;
		if (block_y_end >= image->GetNumTilesDown())
			block_y_end = image->GetNumTilesDown() - 1;
		uint index_count = (uint)((block_y_end - block_y_start + 1) *
			(block_x_end - block_x_start + 1));
		uint * index_array = new uint[index_count];
		memset(index_array, 0, sizeof(uint) * index_count);
		uint size = 0;
		for (uint y = (uint)block_y_start; y <= (uint)block_y_end; y++)
		{
			for (int x = (uint)block_x_start; x <= (uint)block_x_end; x++)
			{
				G42ImageTile * tile = image->GetTile(y, x);
				index_array[size++] = tile->GetDataHandle();
			}
		}
		image->PreLoad(index_array, size);
		delete [] index_array;
		for (uint row = (uint)block_y_start; row <= (uint)block_y_end; row++)
		{
			int tile_height = image->GetTileHeight();
			int window_start_y;
			int tile_start_y;
			int window_height;
			if (row)
			{
				window_start_y = row * image->GetTileHeight() -
					image->GetFirstTileOffsetY() - PanY;
				tile_start_y = 0;
				if (window_start_y < 0)
				{
					tile_start_y = -window_start_y;
					window_start_y = 0;
				}
			}
			else
			{
				if (PanY > 0)
				{
					window_start_y = 0;
					tile_start_y = image->GetFirstTileOffsetY() + PanY;
				}
				else
				{
					window_start_y = -PanY;
					tile_start_y = image->GetFirstTileOffsetY();
				}
			}
			if (row < image->GetNumTilesDown() - 1)
			{
				tile_height = image->GetTileHeight() - tile_start_y;
				window_height = (row + 1) * image->GetTileHeight() -
					image->GetFirstTileOffsetY() - PanY - window_start_y;
			}
			else
			{
				tile_height = image->GetLastTileHeight() - tile_start_y;
				window_height = (row) * image->GetTileHeight() +
					image->GetLastTileHeight() -
					image->GetFirstTileOffsetY() - PanY - window_start_y;
			}
			if (window_height + window_start_y > WindowHeight)
			{
				window_height = WindowHeight - window_start_y;
				tile_height = WindowHeight - window_start_y;
			}
			for (uint column = (uint)block_x_start;
				column <= (uint)block_x_end; column++)
			{
				G42LockedImageTile tile(image->GetTile(row, column));
#ifdef MSWIN
				// MSWIN does things upside down
				byte * tile_ptr = *(tile.GetTile()->GetRow(tile_start_y + tile_height - 1));
#else
				byte * tile_ptr = *(tile.GetTile()->GetRow(tile_start_y));
#endif
				int tile_width = image->GetTileWidth();
				int window_start_x;
				int tile_start_x;
				int window_width;
				if (column)
				{
					window_start_x = (column *
						image->GetTileWidth() - image->GetFirstTileOffsetX()) -
						PanX;
					tile_start_x = 0;
					if (window_start_x < 0)
					{
						tile_start_x = -window_start_x;
						window_start_x = 0;
					}
				}
				else
				{
					if (PanX > 0)
					{
						window_start_x = 0;
						tile_start_x = image->GetFirstTileOffsetX() + PanX;
					}
					else
					{
						window_start_x = -PanX;
						tile_start_x = image->GetFirstTileOffsetX();
					}
				}
				if (column < image->GetNumTilesAcross() - 1)
				{
					tile_width = image->GetTileWidth() - tile_start_x;
					window_width = ((column + 1) *
						image->GetTileWidth() -
						image->GetFirstTileOffsetX()) - PanX - window_start_x;
				}
				else
				{
					tile_width = image->GetLastTileWidth() - tile_start_x;
					window_width = ((column) *
						image->GetTileWidth() + image->GetLastTileWidth() -
						image->GetFirstTileOffsetX()) - PanX - window_start_x;
				}
				if (window_width + window_start_x > WindowWidth)
				{
					window_width = WindowWidth - window_start_x;
					tile_width = WindowWidth - window_start_x;
				}
				#ifdef MSWIN
				DibHeader->biWidth = image->GetTileWidth();
				DibHeader->biHeight = tile_height;
				DibHeader->biSizeImage = tile_height *
					(DWORD)tile.GetTile()->GetRowBytes();
				if (tile_width == window_width && tile_height == window_height)
				{
					SetDIBitsToDevice(dc,
						window_start_x, window_start_y,
						tile_width, tile_height,
						tile_start_x, 0,
						0, tile_height, tile_ptr,
							(BITMAPINFO *)DibHeader, DibUsage);
				}
				else
				{
					StretchDIBits(dc,
						window_start_x, window_start_y,
						window_width, window_height,
						tile_start_x, 0,
						tile_width, tile_height,
						tile_ptr, (BITMAPINFO *)DibHeader, DibUsage, SRCCOPY);
				}
				#endif
				#ifdef MACOS
				PixMapPtr _pixMapPtr = *PixMapTile;
				_pixMapPtr -> baseAddr = (Ptr) tile_ptr;
		
				GrafPtr activePort;
				GetPort (&activePort);
				Rect sourceRect, destRect;
				/*  Set up rects */
// 				sourceRect.top 		= tile_start_y;
				sourceRect.top 		= 0;
				sourceRect.left 	= tile_start_x;
	//			sourceRect.bottom 	= sourceRect.top + tile_height;
	//			sourceRect.right 	= sourceRect.left + tile_width;
	
				sourceRect.bottom 	= sourceRect.top + window_height;
				sourceRect.right 	= sourceRect.left + window_width;
				
				int temp_width 	= (int) ((float) window_width / ZoomFactor + 0.5);
				int temp_height = (int) ((float) window_height / ZoomFactor + 0.5);
				sourceRect.bottom 	= sourceRect.top + temp_height;
				sourceRect.right 	= sourceRect.left + temp_width;
				destRect.top 	= window_start_y;
				destRect.left 	= window_start_x;
				destRect.bottom = destRect.top + window_height;
				destRect.right 	= destRect.left + window_width;		
				OffsetRect (&destRect, PanX, PanY);
	//			OffsetRect (&destRect, Margin, Margin);
				
				CopyBits ((BitMapPtr) _pixMapPtr,  &WINBITMAP (activePort), 
					&sourceRect, &destRect, srcCopy + ditherCopy, NULL);
				if (CallBack)
					(*CallBack)();
				#endif
			}
		}
		#ifdef MSWIN
		ReleaseDC(Window, dc);
		#endif
	}
}
#ifdef MACOS
void G42MultipleImageViewer::InitPixMap (void)
{
	/*  Create a GWorld from tiles information in Image  */
	G42ImageTile * tile = Image -> GetTile (0, 0);					
	Rect 	rect;
	rect.top 	= 0;
	rect.left 	= 0;
	rect.bottom = Image -> GetTileHeight();
	rect.right 	= Image -> GetTileWidth();;
	/*  Determine the image depth  */
	
	int BitsPerPixel = Image -> GetInfo().PixelDepth;
	
	RowBytes = tile -> GetRowBytes();
	int Depth = BitsPerPixel;
	
	if (Depth > 8)
		Depth = 32;
	else if (Depth > 4)
		Depth = 8;
			
	/*  Set up the PixMap  */
	PixMapTile = NewPixMap ();
	PixMapPtr pixMapPtr = *PixMapTile;
		
	pixMapPtr -> baseAddr 		= 0;
//	#ifdef __MC68K__
	#if 0
	pixMapPtr -> rowBytes 		= RowBytes;
	#else
	pixMapPtr -> rowBytes 		= (1L << 15) | RowBytes;
	#endif
	pixMapPtr -> bounds 		= rect;
	pixMapPtr -> pmVersion		= 4;
	pixMapPtr -> packType		= 0;
	pixMapPtr -> packSize		= 0;
	pixMapPtr -> hRes			= 72;
	pixMapPtr -> vRes			= 72;		
	pixMapPtr -> pixelType		= RGBDirect;
	pixMapPtr -> pixelSize		= BitsPerPixel;
	pixMapPtr -> cmpCount		= (Depth == 8) ? 1 : 3;		
	pixMapPtr -> cmpSize		= 8;		
	pixMapPtr -> planeBytes		= 0;
	pixMapPtr -> pmTable		= 0;
	pixMapPtr -> pmReserved		= 0;
	
	#ifdef USE_GWORLD
	OSErr err = NewGWorld (&GWorldTile, Depth, &rect, 0, 0, 0);
	if (err)
		return;
	#endif
	/*  Now set up palette stuff  */
	// TODO:  Do not create a PaletteHandle and remove it from being an instance variable
	int numPalette = Image -> GetInfo().NumPalette;
	if (numPalette != 0)
	{
		CTabHandle ctab = GetCTable (8);
		GPaletteHandle = NewPalette ((**ctab).ctSize, nil, pmTolerant, 0);
		ColorSpec *specs;
			
		specs = (**ctab).ctTable;
		(**ctab).ctSize = numPalette - 1;
		const G42Color *colors = Image -> GetInfo().ImagePalette;
		for (int i = 0; i < numPalette; i++)
		{
			specs [i].rgb.red 	= (colors -> red << 8);
			specs [i].rgb.green = (colors -> green << 8);
			specs [i].rgb.blue 	= (colors -> blue << 8);
			specs [i].value = i;
			colors++;
		}
		(**ctab).ctSeed = GetCTSeed();
		CTab2Palette (ctab, GPaletteHandle, pmTolerant, 0);
		pixMapPtr -> pmTable = ctab;	
	}
}
#endif	// MACOS
#ifdef MSWIN
void G42MultipleImageViewer::SetImageInformation(G42Image * Image)
{
	if (Image)
	{
		if (Image->HasInfo())
		{
			DibHeader->biBitCount = Image->GetInfo().PixelDepth;
			if (Image->GetInfo().NumPalette)
			{
				DibHeader->biClrUsed = Image->GetInfo().NumPalette;
				RGBQUAD * dib_pal = (RGBQUAD *)((byte *)DibHeader +
					sizeof (BITMAPINFOHEADER));
				const G42Color * pal = Image->GetInfo().ImagePalette;
				for (int i = 0; i < Image->GetInfo().NumPalette; i++)
				{
					dib_pal[i].rgbRed = pal[i].red;
					dib_pal[i].rgbGreen = pal[i].green;
					dib_pal[i].rgbBlue = pal[i].blue;
					dib_pal[i].rgbReserved = 0;
				}
//				DibUsage = DIB_PAL_COLORS;
			}
			else if (Image->GetInfo().ColorType ==
				G42ImageInfo::ColorTypeGrayscale)
			{
				DibHeader->biClrUsed = (1 << Image->GetInfo().PixelDepth);
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
		}
	}
}
#endif // MSWIN
#ifdef MACOS
#if 0
// david: I don't know if you need this or not
void G42MultipleImageViewer::SetPanXY (int x, int y)
{
	PanX = x;
	PanY = y;
}
void G42MultipleImageViewer::GetPanXY (int & x, int & y)
{
	x = PanX;
	y = PanY;
}
#endif // if 0
#endif // MACOS
void G42MultipleImageViewer::SetWindowSize(int width, int height)
{
	WindowWidth = width;
	WindowHeight = height;
}
#ifdef MSWIN
void G42MultipleImageViewer::WindowSizeChanged(void)
{
	RECT rect;
	::GetClientRect(Window, &rect);
	WindowWidth = rect.right - rect.left;
	WindowHeight = rect.bottom - rect.top;
}
void G42MultipleImageViewer::QueryNewPalette(void)
{
}
void G42MultipleImageViewer::PaletteChanged(void)
{
}
#endif
void
G42MultipleImageViewer::ImageBeingDeleted(G42Image *)
{
}
void
G42MultipleImageViewer::HaveImageEnd(void)
{
}
void
G42MultipleImageViewer::HaveImageInfo(void)
{
}
void 
G42MultipleImageViewer::HaveImageRow(uint32)
{
}
void
G42MultipleImageViewer::ImageChanged(void)
{
}
