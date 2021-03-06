// g42disp.cpp
#include "gidisp.h"
#include "gicsur.h"
//#include "gesdebug.h"
G42Display::G42Display(G42Surface surface)
:
#ifdef MSWIN
	DibHeader(0), MaskHeader(0), MaskPalette(0), WindowsDibUsage(DIB_RGB_COLORS),
	TimerID(0),
#endif
#ifdef MACOS
	PixmapHandle(0),
#endif
	MultiDescriptor(0), NumFrames(0), Width(0), Height(0), ActiveFrame(0),
	Surface(surface), AutoDraw(true), AnimatedDraw(true),
	AnimatedDescriptor(0), Processing(false), TimerRunning(false),
	SurfaceController(0), NeedInvalidate(false), InvalidateFill(false),
	Playing(false), Paused(false), AutoPlay(true), SingleFrameOnly(false)
{
	#ifdef MACOS
	ClipRegion = ::NewRgn();
	G42ThrowIfNull (ClipRegion);
	#endif
}
G42Display::~G42Display()
{
	Clear();
#ifdef MSWIN
	if (TimerID)
	{
		ReleaseTimerID(TimerID);
		TimerID = 0;
	}
#endif
}
void
G42Display::Clear()
{
	if (TimerRunning)
	{
		StopAnimatedTimer();
	}
	if (MultiDescriptor)
	{
		delete MultiDescriptor;
		MultiDescriptor = 0;
	}
	AnimatedDescriptor = 0; // just a dynamic_cast of MultiDescriptor
	for (uint32 j = 0; j < NumFrames; j++)
	{
		if (Data[j])
		{
			for (int i = 0; i < Descriptor[j]->GetNumStrips(); i++)
			{
				if ((Data[j])[i])
					delete (Data[j])[i];
			}
			delete [] Data[j];
			Data[j] = 0;
		}
		if (Descriptor[j])
		{
			delete Descriptor[j];
			Descriptor[j] = 0;
		}
		if (Frame[j])
		{
			delete Frame[j];
			Frame[j] = 0;
		}
	}
	Descriptor.Clear();
	Data.Clear();
	Frame.Clear();
	NumFrames = 0;
	ActiveFrame = 0;
	// should these keep their old values?
	Playing = false;
	Paused = false;
	#ifdef MSWIN
	if (DibHeader)
	{
		delete [] (char *)DibHeader;
		DibHeader = 0;
	}
	if (MaskHeader)
	{
		delete [] (char *)MaskHeader;
		MaskHeader = 0;
	}
	
	MaskPalette = 0;
	#endif
	#ifdef MACOS
	if (PixmapHandle)
	{
		::DisposePixMap (PixmapHandle);
		PixmapHandle = 0;
	}
	if (ClipRegion)
	{
		::DisposeRgn (ClipRegion);
		ClipRegion = 0;
	}
	#endif
}
void
G42Display::Draw(const G42Rect & rect, G42DrawLocation location)
{
	if (!NumFrames)
	{
		return;
	}
	if (AnimatedDraw && !SingleFrameOnly)
	{
		for (uint32 frame = 0; frame <= ActiveFrame; frame++)
		{
			if (frame == ActiveFrame ||
				(!Frame[frame] || Frame[frame]->GetReplace() !=
				G42AnimatedImageFrame::Background))
			{
				DrawFrame(frame, rect, location);
			}
		}
	}
	else
	{
		DrawFrame(ActiveFrame, rect, location);
	}
}
void
G42Display::DrawFrame(uint32 frame, const G42Rect & rect,
	G42DrawLocation location)
{
	if (!NumFrames || frame >= NumFrames ||
		!Descriptor[frame] || !Data[frame])
	{
		return;
	}
	G42Rect area(rect);
	area &= Viewport;
	if (area.IsNull() || area.Top > area.Bottom || area.Left > area.Right)
	{
		return;
	}
	G42Rect image_rect(0, 0, Descriptor[frame]->GetWidth() - 1,
		Descriptor[frame]->GetHeight() - 1);
	ScaleTranslate.Convert(image_rect);
	area &= image_rect;
	image_rect = area;
	ScaleTranslate.ConvertBack(image_rect);
	if (area.IsNull() || area.Top > area.Bottom ||
		area.Left > area.Right)
	{
		return;
	}
	image_rect.Bottom++;
	if (image_rect.Top < 0)
	{
		image_rect.Top = 0;
	}
	if (image_rect.Bottom < 0)
	{
		image_rect.Bottom = 0;
	}
	if (image_rect.Top >= Descriptor[frame]->GetHeight())
	{
		image_rect.Top = Descriptor[frame]->GetHeight() - 1;
	}
	if (image_rect.Bottom >= Descriptor[frame]->GetHeight())
	{
		image_rect.Bottom = Descriptor[frame]->GetHeight() - 1;
	}
	if (image_rect.Bottom < image_rect.Top)
	{
		image_rect.Bottom = image_rect.Top;
	}
	uint32 start = Descriptor[frame]->GetStripNum(image_rect.Top);
	uint32 end = Descriptor[frame]->GetStripNum(image_rect.Bottom);
	if (InvalidateFill || (start <= end && frame == 0 && NumFrames > 1 &&
		!Descriptor[0]->IsTransparent() && MultiDescriptor &&
		MultiDescriptor->HasBackgroundColor()))
	{
#ifdef MSWIN
		G42DrawLocationVerifier dc(location, Surface,
			(WindowPalette ? WindowPalette->Pal : 0));
		HBRUSH b = CreateSolidBrush(RGB(MultiDescriptor->GetBackgroundColor().Red,
			MultiDescriptor->GetBackgroundColor().Green,
			MultiDescriptor->GetBackgroundColor().Blue));
		HBRUSH old_b = (HBRUSH)SelectObject(dc, b);
		HPEN old_p = (HPEN)SelectObject(dc, GetStockObject(NULL_PEN));
		Rectangle(dc, area.Left - Viewport.Left, area.Top - Viewport.Top,
			area.Right - Viewport.Left + 1, area.Bottom - Viewport.Top + 1);
		SelectObject(dc, old_p);
		SelectObject(dc, old_b);
		InvalidateFill = false;
#endif
	}
	for (uint32 i = start; i <= end; i++)
	{
		if ((Data[frame])[i])
		{
			DrawData((Data[frame])[i], Descriptor[ActiveFrame], area, location);
		}
	}
}
#ifdef MSWIN
void
G42Display::DrawData(G42ImageData * data, const G42Rect & clip,
	G42DrawLocation location)
{
	G42Point offset(0, 0);
	if (Frame[ActiveFrame])
	{
		offset = Frame[ActiveFrame]->GetOffset();
	}
	ScaleTranslate.Convert(offset);
	G42ScaleTranslate scale_translate(
		ScaleTranslate.GetScaleX(), ScaleTranslate.GetScaleY(), offset);
	G42DrawLocationVerifier dc(location, Surface,
		(WindowPalette ? WindowPalette->Pal : 0));
	SetStretchBltMode(dc, STRETCH_DELETESCANS);
	G42Rect source_rect(0, data->GetStripStartInImage(),
		Descriptor[ActiveFrame]->GetWidth() - 1,
		data->GetStripStartInImage() + data->GetNumRows() - 1);
	G42Rect dest_rect(source_rect);
	scale_translate.Convert(dest_rect);
	G42Rect clipped_dest(dest_rect);
	clipped_dest &= clip;
	if (clipped_dest.IsEmpty())
		return;
	G42Rect clipped_source(clipped_dest);
	scale_translate.ConvertBack(clipped_source);
	bool need_reclip = true;
	G42Rect reclipped_dest;
	while (need_reclip)
	{
		if (data->IsTransparent() || Descriptor[ActiveFrame]->GetDepth() == 1)
		{
			if (clipped_source.Left & 7)
			{
				clipped_source.Left &= ~7;
			}
			if ((clipped_source.Right & 7) != 7)
			{
				clipped_source.Right = ((clipped_source.Right + 8) & ~7) - 1;
			}
			if (clipped_source.Right >= Descriptor[ActiveFrame]->GetWidth())
			{
				clipped_source.Right = Descriptor[ActiveFrame]->GetWidth() - 1;
			}
		}
		reclipped_dest = clipped_source;
		scale_translate.Convert(reclipped_dest);
		need_reclip = false;
		if (reclipped_dest.Left > clipped_dest.Left)
		{
			if (clipped_source.Left > source_rect.Left)
			{
				need_reclip = true;
				clipped_source.Left--;
			}
			else
			{
				reclipped_dest.Left = clipped_dest.Left;
			}
		}
		if (reclipped_dest.Top > clipped_dest.Top)
		{
			if (clipped_source.Top > source_rect.Top)
			{
				need_reclip = true;
				clipped_source.Top--;
			}
			else
			{
				reclipped_dest.Top = clipped_dest.Top;
			}
		}
		if (reclipped_dest.Right < clipped_dest.Right)
		{
			if (clipped_source.Right < source_rect.Right)
			{
				need_reclip = true;
				clipped_source.Right++;
			}
			else
			{
				reclipped_dest.Right = clipped_dest.Right;
			}
		}
		if (reclipped_dest.Bottom < clipped_dest.Bottom)
		{
			if (clipped_source.Bottom < source_rect.Bottom)
			{
				need_reclip = true;
				clipped_source.Bottom++;
			}
			else
			{
				reclipped_dest.Bottom = clipped_dest.Bottom;
			}
		}
	}
//GES((ges, "clip: %d, %d, %d, %d\n", clip.Left, clip.Top, clip.GetWidth(), clip.GetHeight()));
//GES((ges, "source_rect: %d, %d, %d, %d\n", source_rect.Left, source_rect.Top, source_rect.GetWidth(), source_rect.GetHeight()));
//GES((ges, "dest_rect: %d, %d, %d, %d\n", dest_rect.Left, dest_rect.Top, dest_rect.GetWidth(), dest_rect.GetHeight()));
//GES((ges, "clipped_dest: %d, %d, %d, %d\n", clipped_dest.Left, clipped_dest.Top, clipped_dest.GetWidth(), clipped_dest.GetHeight()));
//GES((ges, "clipped_source: %d, %d, %d, %d\n", clipped_source.Left, clipped_source.Top, clipped_source.GetWidth(), clipped_source.GetHeight()));
//GES((ges, "reclipped_dest: %d, %d, %d, %d\n\n", reclipped_dest.Left, reclipped_dest.Top, reclipped_dest.GetWidth(), reclipped_dest.GetHeight()));
	G42Rect strip_rect(clipped_source);
	strip_rect.Top -= data->GetStripStartInImage();
	strip_rect.Bottom -= data->GetStripStartInImage();
	if (strip_rect.Left < 0)
	{
		strip_rect.Left = 0;
	}
	
	if (strip_rect.Top < 0)
	{
		strip_rect.Top = 0;
	}
	// if only one row, no need for clipping
	if (strip_rect.GetHeight() <= 0)
	{
		strip_rect.Bottom = strip_rect.Top;
		reclipped_dest.Top = clipped_dest.Top;
		reclipped_dest.Bottom = clipped_dest.Bottom;
	}
	if (strip_rect.GetWidth() <= 0)
	{
		strip_rect.Right = strip_rect.Left;
		reclipped_dest.Left = clipped_dest.Left;
		reclipped_dest.Right = clipped_dest.Right;
	}
	G42ReadImageData rdata(*data);
	DibHeader->biHeight = data->GetNumRows();
	if (data->IsTransparent())
	{
#if 1
		int32 loc_width = reclipped_dest.GetWidth();
		int32 loc_height = reclipped_dest.GetHeight();
		HBITMAP bmp1 = CreateCompatibleBitmap(dc, loc_width,
			loc_height);
		HBITMAP bmp2 = CreateCompatibleBitmap(dc, loc_width,
			loc_height);
		HDC mhdc = CreateCompatibleDC(dc);
		G42DrawLocationVerifier mdc(mhdc, Surface,
			(WindowPalette ? WindowPalette->Pal : 0), true);
		SetStretchBltMode(mdc, STRETCH_DELETESCANS);
		SetBkMode(mdc, TRANSPARENT);
		HBITMAP old_bmp = (HBITMAP)SelectObject(mdc, bmp1);
		SelectObject(mdc, GetStockObject(BLACK_BRUSH));
		SelectObject(mdc, GetStockObject(NULL_PEN));
		Rectangle(mdc, 0, 0, loc_width, loc_height);
		DibHeader->biHeight = strip_rect.GetHeight();
		MaskHeader->biHeight = strip_rect.GetHeight();
		StretchDIBits(mdc,
			0, 0,
			loc_width, loc_height,
			strip_rect.Left, 0, strip_rect.GetWidth(), strip_rect.GetHeight(),
			rdata.GetRow(strip_rect.Bottom),
			(BITMAPINFO *)DibHeader, WindowsDibUsage, SRCCOPY);
		MaskPalette[0].rgbRed = MaskPalette[0].rgbGreen =
			MaskPalette[0].rgbBlue = 0;
		MaskPalette[1].rgbRed = MaskPalette[1].rgbGreen =
			MaskPalette[1].rgbBlue = 0xff;
		StretchDIBits(mdc,
			0, 0,
			loc_width, loc_height,
			strip_rect.Left, 0, strip_rect.GetWidth(), strip_rect.GetHeight(),
			rdata.GetRow(strip_rect.Bottom, true),
			(BITMAPINFO *)MaskHeader, DIB_RGB_COLORS, SRCAND);
		SelectObject(mdc, bmp2);
		SelectObject(mdc, GetStockObject(WHITE_BRUSH));
		Rectangle(mdc, 0, 0, loc_width + 1, loc_height + 1);
		MaskPalette[0].rgbRed = MaskPalette[0].rgbGreen =
			MaskPalette[0].rgbBlue = 0xff;
		MaskPalette[1].rgbRed = MaskPalette[1].rgbGreen =
			MaskPalette[1].rgbBlue = 0;
		StretchDIBits(mdc,
			0, 0,
			loc_width, loc_height,
			strip_rect.Left, 0, strip_rect.GetWidth(), strip_rect.GetHeight(),
			rdata.GetRow(strip_rect.Bottom, true),
			(BITMAPINFO *)MaskHeader, DIB_RGB_COLORS, SRCAND);
		BitBlt(dc,
			clipped_dest.Left - Viewport.Left,
			clipped_dest.Top - Viewport.Top,
			clipped_dest.GetWidth(), clipped_dest.GetHeight(),
			mdc,
			clipped_dest.Left - reclipped_dest.Left,
			clipped_dest.Top - reclipped_dest.Top,
			SRCAND);
		SelectObject(mdc, bmp1);
#if 1
		BitBlt(dc,
			clipped_dest.Left - Viewport.Left,
			clipped_dest.Top - Viewport.Top,
			clipped_dest.GetWidth(), clipped_dest.GetHeight(),
			mdc,
			clipped_dest.Left - reclipped_dest.Left,
			clipped_dest.Top - reclipped_dest.Top,
			SRCPAINT);
#endif			
		mdc.Clear();
		SelectObject(mhdc, old_bmp);
		DeleteDC(mhdc);
		DeleteObject(bmp1);
		DeleteObject(bmp2);
#endif		
#if 0 // old way that required BitBlt'ing from screen
		int32 loc_width = reclipped_dest.GetWidth();
		int32 loc_height = reclipped_dest.GetHeight();
		HBITMAP bmp1 = CreateCompatibleBitmap(dc, loc_width,
			loc_height);
		HBITMAP bmp2 = CreateCompatibleBitmap(dc, loc_width,
			loc_height);
		HDC mhdc = CreateCompatibleDC(dc);
		G42DrawLocationVerifier mdc(mhdc, Surface,
			(WindowPalette ? WindowPalette->Pal : 0), true);
		SetStretchBltMode(mdc, STRETCH_DELETESCANS);
		SetBkMode(mdc, TRANSPARENT);
		HBITMAP old_bmp = (HBITMAP)SelectObject(mdc, bmp1);
		DibHeader->biHeight = strip_rect.GetHeight();
		StretchDIBits(mdc,
			0, 0,
			loc_width, loc_height,
			strip_rect.Left, 0, strip_rect.GetWidth(), strip_rect.GetHeight(),
			rdata.GetRow(strip_rect.Bottom),
			(BITMAPINFO *)DibHeader, WindowsDibUsage, SRCCOPY);
		MaskPalette[0].rgbRed = MaskPalette[0].rgbGreen =
			MaskPalette[0].rgbBlue = 0;
		MaskPalette[1].rgbRed = MaskPalette[1].rgbGreen =
			MaskPalette[1].rgbBlue = 0xff;
		StretchDIBits(mdc,
			0, 0,
			loc_width, loc_height,
			strip_rect.Left, 0, strip_rect.GetWidth(), strip_rect.GetHeight(),
			rdata.GetRow(strip_rect.Bottom, true),
			(BITMAPINFO *)MaskHeader, DIB_RGB_COLORS, SRCAND);
		SelectObject(mdc, bmp2);
		BitBlt(mdc,
			clipped_dest.Left - reclipped_dest.Left,
			clipped_dest.Top - reclipped_dest.Top,
			clipped_dest.GetWidth(), clipped_dest.GetHeight(),
			dc,
			clipped_dest.Left - Viewport.Left,
			clipped_dest.Top - Viewport.Top,
			SRCCOPY);
		MaskPalette[0].rgbRed = MaskPalette[0].rgbGreen =
			MaskPalette[0].rgbBlue = 0xff;
		MaskPalette[1].rgbRed = MaskPalette[1].rgbGreen =
			MaskPalette[1].rgbBlue = 0;
		StretchDIBits(mdc,
			0, 0,
			loc_width, loc_height,
			strip_rect.Left, 0, strip_rect.GetWidth(), strip_rect.GetHeight(),
			rdata.GetRow(strip_rect.Bottom, true),
			(BITMAPINFO *)MaskHeader, DIB_RGB_COLORS, SRCAND);
		HDC mhdc2 = CreateCompatibleDC(dc);
		G42DrawLocationVerifier mdc2(mhdc2, Surface,
			(WindowPalette ? WindowPalette->Pal : 0), true);
		SetStretchBltMode(mdc2, STRETCH_DELETESCANS);
		HBITMAP old_bmp2 = (HBITMAP)SelectObject(mdc2, bmp1);
		BitBlt(mdc, 0, 0, loc_width, loc_height, mdc2,
			0, 0, SRCPAINT);
		BitBlt(dc,
			clipped_dest.Left - Viewport.Left,
			clipped_dest.Top - Viewport.Top,
			clipped_dest.GetWidth(), clipped_dest.GetHeight(),
			mdc,
			clipped_dest.Left - reclipped_dest.Left,
			clipped_dest.Top - reclipped_dest.Top,
			SRCCOPY);
		mdc2.Clear();
		SelectObject(mhdc2, old_bmp2);
		DeleteDC(mhdc2);
		mdc.Clear();
		SelectObject(mhdc, old_bmp);
		DeleteDC(mhdc);
		DeleteObject(bmp1);
		DeleteObject(bmp2);
#endif		
	}
	else
	{
		int32 loc_width = reclipped_dest.GetWidth();
		int32 loc_height = reclipped_dest.GetHeight();
		HBITMAP bmp1 = CreateCompatibleBitmap(dc, loc_width,
			loc_height);
		HDC mhdc = CreateCompatibleDC(dc);
		G42DrawLocationVerifier mdc(mhdc, Surface,
			(WindowPalette ? WindowPalette->Pal : 0), true);
		SetStretchBltMode(mdc, STRETCH_DELETESCANS);
		SetBkMode(mdc, TRANSPARENT);
		HBITMAP old_bmp = (HBITMAP)SelectObject(mdc, bmp1);
		DibHeader->biHeight = strip_rect.GetHeight();
		StretchDIBits(mdc,
			0, 0,
			loc_width, loc_height,
			strip_rect.Left, 0, strip_rect.GetWidth(), strip_rect.GetHeight(),
			rdata.GetRow(strip_rect.Bottom),
			(BITMAPINFO *)DibHeader, WindowsDibUsage, SRCCOPY);
		BitBlt(dc,
			clipped_dest.Left - Viewport.Left,
			clipped_dest.Top - Viewport.Top,
			clipped_dest.GetWidth(), clipped_dest.GetHeight(),
			mdc,
			clipped_dest.Left - reclipped_dest.Left,
			clipped_dest.Top - reclipped_dest.Top,
			SRCCOPY);
		mdc.Clear();
		SelectObject(mhdc, old_bmp);
		DeleteDC(mhdc);
		DeleteObject(bmp1);
	}
}
#endif
#ifdef MACOS
void
G42Display::DrawData (G42ImageData * data, G42ImageDescriptor * descriptor, 
	const G42Rect &clip, G42DrawLocation)
{
	G42ReadImageData rdata(*data);	// Read lock
	
	G42Rect clip_rect = clip;
	clip_rect.Right++;
	clip_rect.Bottom++;
	/*  Save current clipping region & setup clipping rect for this draw  */
	::GetClip (ClipRegion);
	Rect rectm = clip_rect;
	::ClipRect (&rectm);
	/*  Set up strip Pixmap  */
	::SetRect (&rectm, 0, 0, descriptor -> GetWidth(),
		descriptor -> GetRowsInStrip (data -> GetStripNum()));
	(**PixmapHandle).bounds   = rectm;
	(**PixmapHandle).baseAddr = (char *) rdata.GetRow (0);
	Rect sourceRect = rectm;
	int rowNum =  data -> GetStripStartInImage();
	G42Rect rectg (0, rowNum, descriptor -> GetWidth(), 
		rowNum + descriptor -> GetRowsInStrip(data -> GetStripNum())); 
	ScaleTranslate.Convert (rectg);
	Rect destRect = rectg;
	/*  If grayscale / monochrome image, need to invert)  */
// TODO:
	short transferMode = (! descriptor -> IsColor()) ? notSrcCopy : srcCopy;
	int16 sourceDepth = (**PixmapHandle).cmpCount * (**PixmapHandle).cmpSize;
	GWorldPtr screenPort;
	::GetPort ((GrafPtr *) &screenPort);
	int16 destDepth = sourceDepth;
	PixMapHandle destPixmap = ::GetGWorldPixMap (screenPort);
#if 1
	if (destPixmap)	// Normal onscreen or gworld drawing
	{
		destDepth = (**destPixmap).cmpCount * (**destPixmap).cmpSize;
		if (sourceDepth > destDepth)
			transferMode |= ditherCopy;
	}
	else			// No pixmap: for printing (?)
		transferMode |= ditherCopy;
#endif
	/*  Get Display port  */
	CGrafPtr port;
	::GetPort ((GrafPtr *) &port);
	::LockPixels (PixmapHandle);
	if (data -> IsTransparent())
	{
		BitMap trans_bitmap;
		trans_bitmap.baseAddr = (char *) rdata.GetRow (0, true);	
		trans_bitmap.rowBytes 	= data -> GetTransRowBytes();
		trans_bitmap.bounds		= rectm;  // (**destPixmap).bounds;  why not this
		::CopyMask ((BitMapPtr) *PixmapHandle, &trans_bitmap, &WINBITMAP (port),
			&sourceRect, &sourceRect, &destRect);	
	}
	else
	{
		/*  Put the strip onto the active port  */
		::LockPixels (port -> portPixMap);
		::CopyBits ((BitMapPtr) *PixmapHandle, &WINBITMAP (port),
			&sourceRect, &destRect, transferMode, NULL);
		::UnlockPixels (port -> portPixMap);
	}
	::UnlockPixels (PixmapHandle);	
	/*  Restore clipping  */
	::SetClip (ClipRegion);
}
#endif // MACOS
#ifdef MACOS
void 
G42Display::MacDrawData (G42ImageData * data, G42ImageDescriptor * descriptor, 
	const G42Rect &clip)
{
	Descriptor [0] = descriptor;
	NumFrames = 1;
	if (! PixmapHandle) {
		SetupPixmap (true);
	}
	DrawData (data, descriptor, clip, 0);
	NumFrames = 0;
}	
#endif
#ifdef MACOS
//  This finds the depth of the deepest screen if there are more than one.
int16
G42Display::MacGetScreenDepth (void)
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
void
G42Display::Process(G42PathObject * object, G42PathComponent *)
{
	if (typeid(*object) == typeid(G42PathBegin))
	{
		Clear();
		delete object;
		Processing = true;
	}
	else if (dynamic_cast<G42MultipleImageDescriptor *>(object))
	{
		G42MultipleImageDescriptor * desc =
			dynamic_cast<G42MultipleImageDescriptor *>(object);
		if (MultiDescriptor)
			delete MultiDescriptor;
		MultiDescriptor = desc;
		if (dynamic_cast<G42AnimatedImageDescriptor *>(desc))
		{
			AnimatedDescriptor = dynamic_cast<G42AnimatedImageDescriptor *>(desc);
		}
	}
	else if (typeid(*object) == typeid(G42ImageDescriptor))
	{
		G42ImageDescriptor * desc = dynamic_cast<G42ImageDescriptor *>(object);
		NumFrames++;
		Descriptor[NumFrames - 1] = desc;
		Frame[NumFrames - 1] = 0;
		Data[NumFrames - 1] = new G42ImageData *
			[Descriptor[NumFrames - 1]->GetNumStrips()];
		memset(Data[NumFrames - 1], 0,
			Descriptor[NumFrames - 1]->GetNumStrips() *
			sizeof (G42ImageData *));
		if (AutoPlay && !Playing && NumFrames == 2 && !Paused && AnimatedDraw)
		{
			Playing = true;
		}
		if (Playing && !Paused && !TimerRunning && NumFrames > 1 &&
			ActiveFrame == NumFrames - 2)
		{
			NextFrame();
		}
		SetupPixmap();
	}
	else if (dynamic_cast<G42AnimatedImageFrame *>(object))
	{
		G42AnimatedImageFrame * frame =
			dynamic_cast<G42AnimatedImageFrame *>(object);
		if (Frame[NumFrames - 1])
			delete Frame[NumFrames - 1];
		Frame[NumFrames - 1] = frame;
		if (Playing && !Paused && AnimatedDraw &&
			ActiveFrame == NumFrames - 1 && !TimerRunning)
		{
			SetDelay();
		}
	}
	else if (dynamic_cast<G42ImageData *>(object))
	{
		G42ImageData * data = dynamic_cast<G42ImageData *>(object);
		if ((Data[NumFrames - 1])[data->GetStripNum()])
			delete (Data[NumFrames - 1])[data->GetStripNum()];
		(Data[NumFrames - 1])[data->GetStripNum()] = data;
		if (AutoDraw && ActiveFrame == NumFrames - 1)
		{
#ifdef NOTYET
			DrawData((Data[NumFrames - 1])[data->GetStripNum()]);
#endif
		}
	}
	else if (typeid(*object) == typeid(G42PathEnd))
	{
		Sender = 0;
		delete object;
		if (Playing && !Paused && !TimerRunning && NumFrames > 1)
		{
			HandleTimer();
		}
		Processing = false;
	}
	else
	{
		delete object;
	}
}
void
G42Display::ClearPath(bool path_excepted)
{
	Processing = false;
	G42PathComponent::ClearPath(path_excepted);
}
void
G42Display::HandleReplace(void)
{
	NeedInvalidate = false;
	InvalidateFill = false;
	if (ActiveFrame >= NumFrames || NumFrames < 2 || !Frame[ActiveFrame])
	{
		return;
	}
	if (Frame[ActiveFrame]->GetReplace() == G42AnimatedImageFrame::Background)
	{
		NeedInvalidate = true;
		G42Point offset = Frame[ActiveFrame]->GetOffset();
		G42Rect rect(offset.X, offset.Y,
			offset.X + Descriptor[ActiveFrame]->GetWidth() - 1,
			offset.Y + Descriptor[ActiveFrame]->GetHeight() - 1);
		ScaleTranslate.Convert(rect);
		InvalidateRect = rect;
		if (!Descriptor[0]->IsTransparent() && MultiDescriptor &&
			MultiDescriptor->HasBackgroundColor())
		{
			InvalidateFill = true;
		}
	}
}
void
G42Display::NextFrame(void)
{
	if (ActiveFrame >= NumFrames - 1)
	{
		if (!Processing && AnimatedDescriptor &&
			AnimatedDescriptor->GetRepeat())
		{
			HandleReplace();
			ActiveFrame = 0;
			SetupPixmap();
		}
	}
	else if (ActiveFrame < NumFrames - 1)
	{
		HandleReplace();
		ActiveFrame++;
		SetupPixmap();
	}
	if (ActiveFrame == 0 && NumFrames > 1 &&
		!Descriptor[0]->IsTransparent() && MultiDescriptor &&
		MultiDescriptor->HasBackgroundColor())
	{
		InvalidateFill = true;
		InvalidateRect = G42Rect(0, 0, AnimatedDescriptor->GetWidth() - 1,
			AnimatedDescriptor->GetHeight() - 1);
		ScaleTranslate.Convert(InvalidateRect);
	}
	G42Point offset(0, 0);
	if (Frame[ActiveFrame])
		offset = Frame[ActiveFrame]->GetOffset();
	G42Rect frame_rect(offset.X, offset.Y,
		offset.X + Descriptor[ActiveFrame]->GetWidth() - 1,
		offset.Y + Descriptor[ActiveFrame]->GetHeight() - 1);
	ScaleTranslate.Convert(frame_rect);
	if (NeedInvalidate && !Processing)
	{
		if (SurfaceController)
		{
			InvalidateRect |= frame_rect;
			SingleFrameOnly = true;
			SurfaceController->DrawInternal(InvalidateRect);
			SingleFrameOnly = false;
		}
		NeedInvalidate = false;
	}
	else if (!Processing)
	{
		SingleFrameOnly = true;
		SurfaceController->DrawInternal(frame_rect);
		SingleFrameOnly = false;
	}
}
void
G42Display::StartAnimatedTimer(uint32 msec)
{
	#if defined(MSWIN)
	if (!TimerID)
	{
		TimerID = GetTimerID(Surface);
	}
	StartTimer(TimerID, msec);
	TimerRunning = true;
	#endif
	#ifdef MACOS
	if (msec) {}
	#endif
}
void
G42Display::StopAnimatedTimer(void)
{
	#ifdef MSWIN
	StopTimer(TimerID);
	TimerRunning = false;
	#endif
}
void
G42Display::TimerStopped(uintf /* id */)
{
#ifdef MSWIN
	StopAnimatedTimer();
	HandleTimer();
#endif
}
void
G42Display::HandleTimer(void)
{
	if (AnimatedDraw && Playing)
	{
		NextFrame();
		SetDelay();
	}
}
void
G42Display::SetDelay(void)
{
	if (AnimatedDraw && Playing && !Paused && ActiveFrame < NumFrames)
	{
		if (ActiveFrame == NumFrames - 1 &&
			(!AnimatedDescriptor || !AnimatedDescriptor->GetRepeat()))
		{
			return;
		}
		uint32 delay = 1;
		if (Frame[ActiveFrame])
		{
			delay = Frame[ActiveFrame]->GetDelay();
		}
		if (delay < 1)
		{
			delay = 1;
		}
		StartAnimatedTimer(delay);
	}
}
void
G42Display::SetAnimatedDraw(bool state)
{
	if (state != AnimatedDraw)
	{
		AnimatedDraw = state;
		if (AnimatedDraw)
		{
			if (!Playing && AutoPlay && NumFrames > 1)
			{
				Playing = true;
				Paused = false;
				HandleTimer();
			}
		}
		else
		{
			if (Playing && !Paused)
			{
				ActiveFrame = 0;
				SetupPixmap();
			}
			Playing = false;
			Paused = false;
			Draw();
		}
	}
}
void
G42Display::Backward(void)
{
	if (ActiveFrame > 0)
	{
		SetActiveFrame(ActiveFrame - 1);
	}
}
void
G42Display::Forward(void)
{
	if (Playing)
	{
		StopAnimatedTimer();
		NextFrame();
		if (!Paused)
		{
			SetDelay();
		}
	}
	else if (ActiveFrame < NumFrames - 1)
	{
		SetActiveFrame(ActiveFrame + 1);
	}
}
void
G42Display::Play(void)
{
	if (Playing && Paused)
	{
		Pause(); // toggling pause will run it again
	}
	else if (!Playing && AnimatedDraw)
	{
		if (ActiveFrame >= NumFrames - 1)
		{
			SetActiveFrame(0);
		}
		Playing = true;
		Paused = false;
		SetDelay();
	}
}
void
G42Display::Pause(void)
{
	if (Playing)
	{
		Paused = !Paused;
		if (Paused)
		{
			StopAnimatedTimer(); // kill timer
		}
		else
		{
			HandleTimer(); // just like the timer stopped
		}
	}
}
void
G42Display::Stop(void)
{
	if (Playing)
	{
		Playing = false;
		Paused = false;
		StopAnimatedTimer();
		SetActiveFrame(0);
	}
}
void
G42Display::SetActiveFrame(uint32 index)
{
	if (index >= NumFrames)
	{
		return;
	}
	
	G42Rect area;
	if (AnimatedDraw && AnimatedDescriptor)
	{
		area = G42Rect(0, 0, AnimatedDescriptor->GetWidth(),
			AnimatedDescriptor->GetHeight());
	}
	else if (Descriptor[index])
	{
		area = G42Rect(0, 0, Descriptor[index]->GetWidth(),
			Descriptor[index]->GetHeight());
	}
	else
	{
		return;
	}
	ScaleTranslate.Convert(area);
	if (Playing)
	{
		StopAnimatedTimer();
		if (index == ActiveFrame + 1)
		{
			NextFrame();
		}
		else
		{
			ActiveFrame = index;
			SurfaceController->DrawInternal(area);
//			Draw(); // should draw from 0 to active frame
		}
		SetDelay();
	}
	else if (AnimatedDraw)
	{
		ActiveFrame = index;
		SurfaceController->DrawInternal(area);
//		Draw(); // should draw from 0 to active frame
	}
	else
	{
		ActiveFrame = index;
		SurfaceController->DrawInternal(area);
//		Draw(); // should only draw current frame
	}
}
void
G42Display::SetAutoPlay(bool state)
{
	AutoPlay = state;
	if (AutoPlay && NumFrames > 1 && !Playing && AnimatedDraw)
	{
		Play();
	}
}
void
G42Display::SetupPixmap (bool force_setup)
{
	if (! force_setup)
	{
		if (!NumFrames || ActiveFrame >= NumFrames || !Descriptor[ActiveFrame])
		{
			return;
		}
	}
	#ifdef MSWIN
	if (!DibHeader)
		DibHeader = (BITMAPINFOHEADER *)(new char [
			sizeof (BITMAPINFOHEADER) + 256 * sizeof (RGBQUAD)]);
	memset(DibHeader, 0, sizeof (BITMAPINFOHEADER));
	DibHeader->biSize = sizeof (BITMAPINFOHEADER);
	DibHeader->biWidth = Descriptor[ActiveFrame]->GetWidth();
	DibHeader->biHeight = Descriptor[ActiveFrame]->GetRowsPerStrip();
	DibHeader->biPlanes = 1;
	DibHeader->biCompression = BI_RGB;
	if (WindowPalette && WindowPalette->Pal)
	{
		DibHeader->biBitCount = Descriptor[ActiveFrame]->GetDepth();
		if (Descriptor[ActiveFrame]->IsPaletted())
		{
			WindowsDibUsage = DIB_PAL_COLORS;
			DibHeader->biClrUsed = WindowPalette->NumPal;
			uint16 * dib_pal = (uint16 *)(((byte *)DibHeader) +
				sizeof (BITMAPINFOHEADER));
			for (intf i = 0; i < DibHeader->biClrUsed; i++)
			{
				dib_pal[i] = WindowPalette->ColorMap[i];
			}
		}
		else if (!Descriptor[ActiveFrame]->IsColor())
		{
			WindowsDibUsage = DIB_PAL_COLORS;
			intf num_grays = (1 << Descriptor[ActiveFrame]->GetDepth());
			DibHeader->biClrUsed = num_grays;
			uint16 * dib_pal = (uint16 *)(((byte *)DibHeader) +
				sizeof (BITMAPINFOHEADER));
			if (num_grays == 256)
			{
				for (intf i = 0; i < DibHeader->biClrUsed; i++)
				{
					dib_pal[i] = WindowPalette->GrayMap[i];
				}
			}
			else // monochrome is only other option here
			{
				dib_pal[0] = WindowPalette->GrayMap[0];
				dib_pal[1] = WindowPalette->GrayMap[255];
			}
		}
		else
		{
			WindowsDibUsage = DIB_RGB_COLORS;
			DibHeader->biClrUsed = 0;
		}
	}
	else // true/high color screen
	{
		DibHeader->biBitCount = Descriptor[ActiveFrame]->GetDepth();
		WindowsDibUsage = DIB_RGB_COLORS;
		if (Descriptor[ActiveFrame]->IsPaletted())
		{
			DibHeader->biClrUsed = Descriptor[ActiveFrame]->GetNumPalette();
			const G42Color * pal = Descriptor[ActiveFrame]->GetPalette();
			RGBQUAD * dib_pal = (RGBQUAD *)(((byte *)DibHeader) +
				sizeof (BITMAPINFOHEADER));
			for (intf i = 0; i < DibHeader->biClrUsed; i++)
			{
				dib_pal[i].rgbRed = pal[i].Red;
				dib_pal[i].rgbGreen = pal[i].Green;
				dib_pal[i].rgbBlue = pal[i].Blue;
			}
		}
		else if (!Descriptor[ActiveFrame]->IsColor())
		{
			intf num_grays = (1 << Descriptor[ActiveFrame]->GetDepth());
			DibHeader->biClrUsed = num_grays;
			RGBQUAD * dib_pal = (RGBQUAD *)(((byte *)DibHeader) +
				sizeof (BITMAPINFOHEADER));
			if (num_grays == 256)
			{
				for (intf i = 0; i < DibHeader->biClrUsed; i++)
				{
					dib_pal[i].rgbRed = dib_pal[i].rgbGreen =
						dib_pal[i].rgbBlue = (byte)i;
				}
			}
			else if (num_grays == 2)
			{
				dib_pal[0].rgbRed = dib_pal[0].rgbGreen =
					dib_pal[0].rgbBlue = (byte)0;
				dib_pal[1].rgbRed = dib_pal[1].rgbGreen =
					dib_pal[1].rgbBlue = (byte)255;
			}
			else
			{
				for (intf i = 0; i < DibHeader->biClrUsed; i++)
				{
					byte g = (byte)((i * (int32)255) / (num_grays - 1));
					dib_pal[i].rgbRed = dib_pal[i].rgbGreen =
						dib_pal[i].rgbBlue = g;
				}
			}
		}
	}
	if (Descriptor[ActiveFrame]->IsTransparent())
	{
		if (!MaskHeader)
		{
			MaskHeader = (BITMAPINFOHEADER *)(new char [
				sizeof (BITMAPINFOHEADER) + 256 * sizeof (RGBQUAD)]);
			MaskPalette = (RGBQUAD *)((byte *)(MaskHeader) +
				sizeof (BITMAPINFOHEADER));
		}
		memset(MaskHeader, 0, sizeof (BITMAPINFOHEADER));
		MaskHeader->biSize = sizeof (BITMAPINFOHEADER);
		MaskHeader->biWidth = Descriptor[ActiveFrame]->GetWidth();
		MaskHeader->biHeight = Descriptor[ActiveFrame]->GetRowsPerStrip();
		MaskHeader->biPlanes = 1;
		MaskHeader->biBitCount = 1;
		MaskHeader->biCompression = BI_RGB;
	}
	#endif
	#ifdef MACOS
	uint8 depth 	= Descriptor[ActiveFrame] -> GetDepth();
	if (depth > 8)
		depth = 32;
	else
		if (depth > 4)
			depth = 8;
	if (! PixmapHandle)
		PixmapHandle = ::NewPixMap ();
	Rect rect;
	::SetRect (&rect, 0, 0, Descriptor[ActiveFrame] -> GetWidth(),
		Descriptor[ActiveFrame] -> GetRowsPerStrip());
	/*  Set up the PixMap  */
	(**PixmapHandle).baseAddr 		= 0;
	(**PixmapHandle).rowBytes 		= (1L << 15) | Descriptor[ActiveFrame] -> GetRowBytes();
	(**PixmapHandle).bounds 		= rect;
	(**PixmapHandle).pmVersion		= 0;
	(**PixmapHandle).packType		= 0;
	(**PixmapHandle).packSize		= 0;
	(**PixmapHandle).hRes			= 72;
	(**PixmapHandle).vRes			= 72;
	/*  Portions complements of Karl at SoftQuad  */
	if (depth <= 8)
	{
		(**PixmapHandle).pixelType		= 0;
		(**PixmapHandle).pixelSize		= depth;
		(**PixmapHandle).cmpCount		= 1;
		(**PixmapHandle).cmpSize		= depth;
	}
	else
	{
		(**PixmapHandle).pixelType		= RGBDirect;
		(**PixmapHandle).pixelSize		= depth;
		(**PixmapHandle).cmpCount		= 3;
		(**PixmapHandle).cmpSize		= 8;
	}
	(**PixmapHandle).planeBytes		= 0;
	(**PixmapHandle).pmReserved		= 0;
	/*  Kill the palette in the PixMap */
	DisposeCTable ((**PixmapHandle).pmTable);
	(**PixmapHandle).pmTable		= 0;
	/*  Use the original palette stuff  */
	int numPalette = Descriptor[ActiveFrame] -> GetNumPalette();
	if (numPalette != 0)
	{
		CTabHandle ctab = GetCTable (8);
		ColorSpec *specs;
		specs = (**ctab).ctTable;
		(**ctab).ctSize = numPalette - 1;
		const G42Color *colors = Descriptor[ActiveFrame] -> GetPalette();
		for (int i = 0; i < numPalette; i++)
		{
			specs [i].rgb.red 	= (colors -> Red << 8);
			specs [i].rgb.green = (colors -> Green << 8);
			specs [i].rgb.blue 	= (colors -> Blue << 8);
			specs [i].value = i;
			colors++;
		}
		(**ctab).ctSeed = GetCTSeed();
		(**PixmapHandle).pmTable = ctab;
	}
	#endif
}
#if 0
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
	pixMapPtr -> bounds 			= rect;
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
#endif
