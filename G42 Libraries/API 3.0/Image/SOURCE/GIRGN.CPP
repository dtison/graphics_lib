#include "girgn.h"
#include "giimage.h"
#ifdef MACOS
#include <Quickdraw.h>
#endif
#define MAX_ANTIALIAS_QUALITY
//#define MED_ANTIALIAS_QUALITY
//#define MIN_ANTIALIAS_QUALITY
#ifdef MAX_ANTIALIAS_QUALITY
const int8 G42RegionBits = 3;
const int8 G42CountBits  = 2;
#elif defined (MED_ANTIALIAS_QUALITY)
const int8 G42RegionBits = 2;
const int8 G42CountBits  = 4;
#elif defined (MIN_ANTIALIAS_QUALITY)
const int8 G42RegionBits = 1;
const int8 G42CountBits  = 6;
#endif
/*  Stuff for SetHeap()  Hack  */
G42HeapManager * G42Region::Heap;
/***  G42RegionInfo Class  ***/
G42RegionInfo::G42RegionInfo (G42Vector <G42Point> & points, int16 num_points,
	bool expand)
: 
	#ifdef MACOS
	GWorld (0),
	#endif
	NumPoints (num_points), RegionBitsHeight (0), RegionBitsRowBytes (0),
	RegionBitsWidth (0)
 {
	RegionBounds.SetNull();
	RegionRect.SetNull();
	int16 i;    // Watcom pacifier
	if (expand)
	{
		for (int16 i = 0; i < NumPoints; i++)
		{
			points[i].X <<= G42RegionBits;
			points[i].Y <<= G42RegionBits;
		}
	}
	/*  Find region bounds  */
	G42Point max_point(points[0]);
	for (i = 1; i < NumPoints; i++)
	{
		if (max_point.X < points[i].X)
			max_point.X = points[i].X;
		if (max_point.Y < points[i].Y)
			max_point.Y = points[i].Y;
	}
	RegionBounds = G42Rect(0, 0, max_point.X, max_point.Y);
	RegionBitsWidth  = max_point.X + 1;
	RegionBitsHeight = max_point.Y + 1;
	#ifdef MSWIN
	/* Setup RegionBits stuff  */
	HBITMAP bitmap;
	HBITMAP old_bitmap;
	HDC dc;
	WinSetupRegionBits (bitmap, old_bitmap, dc);
	/*  Draw polygon  */
 	//  (save off points for windows structures)
	G42APtr <POINT> win_points (new POINT [NumPoints]);
	win_points[0] = points[0];
	for (i = 1; i < NumPoints; i++)
	{
		win_points[i] = points[i];
	}
	// include any pixel that is within the outermost loop
	SetPolyFillMode(dc, WINDING);
	::Polygon(dc, win_points, NumPoints);
	/*  Cleanup stuff  */
	WinCleanupRegionBits (bitmap, old_bitmap, dc);
	#endif
	#ifdef MACOS
	CGrafPtr port;
	GDHandle gdh;
	MacSetupRegionBits (port, gdh);
	/*  Draw polygon into GWorld  */
	PolyHandle poly_handle = ::OpenPoly();
	::MoveTo (points [0].X, points [0].Y);
	for (int16 i = 1; i < NumPoints; i++)
	{
		::LineTo (points [i].X, points [i].Y);
	}
	::ClosePoly();
	::PaintPoly (poly_handle);
	::KillPoly (poly_handle);
	/*  Restore port  */
	::SetGWorld (port, gdh);
	#endif
}
/*  Make an rectangle or ellipse region */
G42RegionInfo::G42RegionInfo (G42Rect rect, G42RegionInfo::G42RegionType type,
	bool expand)
:
	#ifdef MACOS
	GWorld (0),
	#endif
	NumPoints (4), RegionBitsHeight (0), RegionBitsRowBytes (0),
	RegionBitsWidth (0)
{
	RegionBounds.SetNull();
	RegionRect.SetNull();
	if (expand)
	{
		rect.Top <<= G42RegionBits;
		rect.Left <<= G42RegionBits;
		rect.Bottom <<= G42RegionBits;
		rect.Right <<= G42RegionBits;
	}
	RegionBounds = rect;
	RegionBitsWidth = rect.Right + 1;
	RegionBitsHeight = rect.Bottom + 1;
	#ifdef MSWIN
	HBITMAP bitmap;
	HBITMAP old_bitmap;
	HDC dc;
	WinSetupRegionBits (bitmap, old_bitmap, dc);
	if (type == G42RegionInfo::Rectangle) {
		::Rectangle(dc, rect.Left, rect.Top, rect.Right, rect.Bottom);
	}
	else {
		::Ellipse(dc, rect.Left, rect.Top, rect.Right, rect.Bottom);
	}
	WinCleanupRegionBits (bitmap, old_bitmap, dc);
	#endif
	#ifdef MACOS
	/*  Setup port  */
	CGrafPtr port;
	GDHandle gdh;
	MacSetupRegionBits (port, gdh);
	if (type == G42RegionInfo::Rectangle) {
		::PaintRect (&((Rect) RegionBounds));
	}
	else {
		::PaintOval (&((Rect) RegionBounds));
	}
	/*  Restore port  */
	::SetGWorld (port, gdh);
	#endif
}
/*  Make a copy */
G42RegionInfo::G42RegionInfo (const G42RegionInfo & info)
:
	#ifdef MACOS
	GWorld (0),
	#endif
	NumPoints (0), RegionBitsHeight (0), RegionBitsRowBytes (0),
	RegionBitsWidth (0)
{
	/*  Take care of bits first  */
	int32 bits_size 	= info.RegionBitsRowBytes * info.RegionBitsHeight;
	RegionBits = new byte [bits_size];
	G42ThrowIfNull (RegionBits);		
	/*  The rest of the data  */
	NumPoints 			= info.NumPoints;
	memcpy (RegionBits, info.RegionBits, bits_size); 
	RegionBitsHeight 	= info.RegionBitsHeight;
	RegionBitsRowBytes = info.RegionBitsRowBytes;
	RegionBitsWidth 	= info.RegionBitsWidth;
	RegionBounds 		= info.RegionBounds;
	RegionRect 			= info.RegionRect;
}
G42RegionInfo::~G42RegionInfo (void)
{
	#ifdef MSWIN
	#endif
	#ifdef MACOS
	if (GWorld)
	{
		::DisposeGWorld (GWorld);
		GWorld = 0;
	}
	#endif
}
#ifdef MACOS
void 
G42RegionInfo::MacSetupRegionBits (CGrafPtr & port, GDHandle & gdh)
{
	/*  Make offscreen GWorld  */
	QDErr err = ::NewGWorld (&GWorld, 1, &((Rect) RegionBounds), 0, 0, useTempMem);
	if (err != noErr) {
		throw (G42MemoryException (0));
	}
	PixMapHandle pixmap_handle = ::GetGWorldPixMap (GWorld);
	RegionBitsRowBytes = (**pixmap_handle).rowBytes & 0x7FFF;
	RegionBits = (byte *) ::GetPixBaseAddr (pixmap_handle);
	/*  Save current port  */
	::GetGWorld (&port, &gdh);
	::SetGWorld (GWorld, 0);
	/*  Draw Background  */
	Rect rect = RegionBounds;
	::ForeColor (whiteColor);
	::PaintRect (&rect);
	::ForeColor (blackColor);
}
#endif
#ifdef MSWIN
void
G42RegionInfo::WinCleanupRegionBits (HBITMAP & bitmap, HBITMAP & old_bitmap,
	HDC & dc)
{
	// create a dib header with extra room (2K is cheap) for BoundsChecker
	BITMAPINFOHEADER * info = (BITMAPINFOHEADER *)new byte
		[2048 + sizeof (BITMAPINFOHEADER)];
	// set up dib
	info->biSize = sizeof (BITMAPINFOHEADER);
	info->biWidth = RegionBitsWidth;
	info->biHeight = RegionBitsHeight;
	info->biPlanes = 1;
	info->biBitCount = 1;
	info->biCompression = BI_RGB;
	info->biSizeImage = RegionBitsHeight * RegionBitsRowBytes;
	info->biXPelsPerMeter = 0;
	info->biYPelsPerMeter = 0;
	info->biClrUsed = 2;
	info->biClrImportant = 2;
	// set first pixel to white (forground), second to black (background)
	DWORD * pal = (DWORD *)((byte *)info + sizeof (BITMAPINFOHEADER));
	pal[0] = RGB(255, 255, 255);
	pal[1] = RGB(0, 0, 0);
	// unselect bitmap
	SelectObject(dc, old_bitmap);
	// copy the shape into the mask we allocated
	GetDIBits(dc, bitmap, 0, RegionBitsHeight, RegionBits, (BITMAPINFO *)info,
		DIB_RGB_COLORS);
	// cleanup
	DeleteDC(dc);
	delete [] (byte *)info;
	DeleteObject(bitmap);
}
#endif
#ifdef MSWIN
void
G42RegionInfo::WinSetupRegionBits (HBITMAP & bitmap, HBITMAP & old_bitmap,
	HDC & dc)
{
	RegionBitsRowBytes = ((((RegionBitsWidth + 7) >> 3) + 3) & ~3);
	// allocate the new bit mask and init it to all ones (on)
	RegionBits = new byte [RegionBitsHeight * RegionBitsRowBytes];
	memset(RegionBits, 0xff, RegionBitsHeight * RegionBitsRowBytes);
	// create a mono bitmap
	bitmap = CreateBitmap(RegionBitsWidth, RegionBitsHeight, 1, 1, 0);
	// get a dc compatible with the screen (HWND = 0)
	HDC sdc = GetDC(0);
	dc = CreateCompatibleDC(sdc);
	ReleaseDC(0, sdc);
	// select old bitmap
	old_bitmap = (HBITMAP)SelectObject(dc, bitmap);
	// set foreground color to white (= 1), background to black (=0)
	SetTextColor(dc, RGB(255, 255, 255));
	SetBkColor(dc, RGB(0, 0, 0));
	// fill the area with black (background = 0)
	SelectObject(dc, GetStockObject(BLACK_PEN));
	SelectObject(dc, GetStockObject(BLACK_BRUSH));
	::Rectangle(dc, 0, 0, RegionBitsWidth, RegionBitsHeight);
	// draw the shape in white (foreground = 1)
	SelectObject(dc, GetStockObject(WHITE_PEN));
	SelectObject(dc, GetStockObject(WHITE_BRUSH));
}
#endif
/*** G42Region Class  ***/
G42Region::G42Region (G42Vector <G42Point> & points, int16 num_points) :
	HaveAntiMask (false), NumPoints (num_points), RegionType (G42Region::Polygon)
{
	RegionRect.SetNull();
	/*  Check parameters  */
	if (num_points < 0) {
		throw G42InvalidParameterException (0);
	}
	/*  Store points */
	for (int16 i = 0; i < NumPoints; i++)
	{
		Points [i] = points [i];
	}
	RegionInfo = new G42RegionInfo (points, num_points);
	G42ThrowIfNull (RegionInfo);
}
G42Region::G42Region (G42Rect rect, G42Region::G42RegionType type) : 
	HaveAntiMask (false), NumPoints (4), RegionRect (rect), RegionType (type)
{
	/*  Store points  */
	Points [0] = rect.GetTopLeft();
	Points [1] = rect.GetTopRight();
	Points [2] = rect.GetBottomLeft();
	Points [3] = rect.GetBottomRight();
	RegionInfo = new G42RegionInfo (rect, (G42RegionInfo::G42RegionType) type);
	G42ThrowIfNull (RegionInfo);
}
#ifdef NOTYET
G42Region::G42Region (G42Image * mask_image) : 
	HaveMask (false), HaveAntiMask(false),
	NumPoints (0), RegionHandle (0), RegionType (G42Region::Polygon)
{
	if (mask_image == 0) {
		throw (G42InvalidParameterException (0));
	}
	/*  Not using RegionRect  */
	RegionRect.SetNull();
#if 0
	/*  Save mask image and update RegionBounds  */
	MaskImage = mask_image;
	RegionBounds.Set (0, 0, MaskImage -> GetInfo().GetWidth() - 1,
		MaskImage -> GetInfo().GetHeight() - 1);
#endif
}
#endif
/*  Make a copy */
G42Region::G42Region (const G42Region & info)
{
	/*  Handle Data and Descriptor  */
	if (info.AntiMaskDescriptor)
	{
		AntiMaskDescriptor = new G42ImageDescriptor (*info.AntiMaskDescriptor);
		G42ThrowIfNull (AntiMaskDescriptor);
		for (int32 i = 0; i < AntiMaskDescriptor -> GetNumStrips(); i++)
		{
			if (info.AntiMaskData [i]) {
				AntiMaskData [i] = info.AntiMaskData [i] -> MakeCopy();			
			}
		}
	}
	/*  Skip Cached Image TODO:  Fix this -- copy read locked G42ReadCachedImage */
	
	/*  More data in alpha order  */
	HaveAntiMask = info.HaveAntiMask;
	NumPoints 		= info.NumPoints;
	/*  Copy points  */
	for (int16 i = 0; i < NumPoints; i++)
	{
		Points [i] = info.Points [i];
	}
	/*  Duplicate Region Info  */
	RegionInfo = new G42RegionInfo (*info.RegionInfo);
	G42ThrowIfNull (RegionInfo);
	/*  Rest of data  */
	RegionRect = info.RegionRect;
	RegionType = info.RegionType;
}
void
G42Region::AntiMaskFromRegionInfo (G42RegionInfo & info)
{
	/*  Allocate Mask Descriptor and Data if needed */
	int32 width  = (info.RegionBounds.Right - info.RegionBounds.Left + 1) >> G42RegionBits;
	int32 height = (info.RegionBounds.Bottom - info.RegionBounds.Top + 1) >> G42RegionBits;
	AntiMaskDescriptor = new G42ImageDescriptor (width, height,
		G42ImageDescriptor::Gray8, *Heap);
	/*  Allocate strips  */
	int32 i;
	for (i = 0; i < AntiMaskDescriptor -> GetNumStrips(); i++)
	{
		AntiMaskData [i] = new G42ImageDataGray8 (*Heap, i,
			AntiMaskDescriptor -> GetRowsInStrip (i),
			AntiMaskDescriptor -> GetRowBytes(),
			AntiMaskDescriptor -> GetStripStartInImage (i),
			AntiMaskDescriptor -> GetWidth());
	}
	/*  Go thru points in bounding rectangle and test if point is in region  */
	G42WriteCachedImage <DataGray8, DataGray8Iterator> masked_image 
		 (AntiMaskDescriptor, AntiMaskData);
	for (i = 0; i < height; i++)
	{
		DataGray8Iterator mask_ptr = masked_image [i];
		for (int32 j = 0; j < width; j++)
		{
			uintf count = 0;
			G42Point point (j, i);
			for (uintf y = (point.Y << G42RegionBits);
				y < ((point.Y + 1) << G42RegionBits); y++)
			{
				for (uintf x = (point.X << G42RegionBits);
					x < ((point.X + 1) << G42RegionBits); x++)
				{
					if (info.PointInRegion (G42Point (x, y)))
					{
						count++;
					}
				}
			}
			count <<= G42CountBits;
			if (count >= 128)
				count--;
			*mask_ptr++ = (byte) count;
		}
	}
}
byte
G42Region::PointInRegionAntialias (const G42Point & point)
{
	/*  Make sure we have antialiased region info  */
	if (! HaveAntiMask)
	{
		G42Ptr <G42RegionInfo> info;
		switch (RegionType)
		{
			case Rectangle:
			case Ellipse:
				info = new G42RegionInfo (RegionRect, 
					(G42RegionInfo::G42RegionType) RegionType, true);
			break;
			case Polygon:
				info = new G42RegionInfo (Points, NumPoints, true);
			break;
		}
		G42ThrowIfNull (info);
		/*  Need to make Grayscale antialiased mask  */
		AntiMaskFromRegionInfo (*info);
		HaveAntiMask = true;
	}
	if (! AntiMaskedImage) {
		AntiMaskedImage = new G42ReadCachedImage <DataGray8, DataGray8ReadIterator>
			(AntiMaskDescriptor, AntiMaskData);
	}
	/*  Test point in mask image  */
	DataGray8ReadIterator mask_ptr = (*AntiMaskedImage) [point.Y];
	return *mask_ptr [point.X];
}
//123456789012345678901234567890123456789012345678901234567890123456789012345678
