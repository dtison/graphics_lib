// g42iview - base class for image viewers
#if !defined(G42IVIEW_H)
#define G42IVIEW_H
#include <g42itype.h>
#ifdef MACOS
#include <g42iinfo.h>
#endif
class G42Image;
class G42ImageData;
class G42ImageTile;
#ifdef MACOS
	typedef CGrafPtr G42DrawLocation;
#endif
#ifdef MSWIN
	typedef HDC G42DrawLocation;
#endif
#ifdef XWIN
	typedef Drawable G42DrawLocation;
#endif
class G42ImageViewer
{
public:
	virtual void ImageBeingDeleted(G42Image * image) = 0;
	virtual void HaveImageEnd(void) = 0;
	virtual void HaveImageInfo(void) = 0;
	virtual void HaveImageRow(uint32 row) = 0;
	virtual void ImageChanged(void) = 0;
};
enum G42ZoomMode {NotZoomed = 0,
	ZoomShiftNormal = 1, ZoomShiftInverted = 2,
	ZoomIntNormal = 3, ZoomIntInverted = 4, Zoomed = 5};
class G42ZoomInfo
{
	public:
		G42ZoomInfo(G42ZoomMode zoom_mode = NotZoomed,
			int zoom_int = 1, double zoom_factor = 1.0);
		G42ZoomInfo(double zoom_factor, uint32 image_width = 0,
			uint32 image_height = 0);
		void Set(G42ZoomMode zoom_mode = NotZoomed, int zoom_int = 1,
			double zoom_factor = 1.0);
		void Set(double zoom_factor, uint32 image_width = 0,
			uint32 image_height = 0);
		int32 WindowToImage(int32 value) const;
		int32 ImageToWindow(int32 value) const;
		G42ZoomMode GetZoomMode(void) const
			{ return ZoomMode; }
		int GetZoomInt(void) const
			{ return ZoomInt; }
		double GetZoomFactor(void) const
			{ return ZoomFactor; }
	private:
		G42ZoomMode ZoomMode;
		int ZoomInt;
		double ZoomFactor;
};
#ifdef MACOS
class G42LockedImageTile;
#endif
class G42SimpleImageViewer : public G42ImageViewer
{
public:
#ifdef MSWIN
	G42SimpleImageViewer(HWND window, HWND top_window);
#endif
#ifdef MACOS
	G42SimpleImageViewer(GWorldPtr windowPtr, void (*CallBackFn)() = nil);
#endif
	virtual ~G42SimpleImageViewer(void);
	uint GetWindowWidth(void)
		{ return WindowWidth; }
	uint GetWindowHeight(void)
		{ return WindowHeight; }
	int32 GetWindowX(void)
		{ return WindowX; }
	int32 GetWindowY(void)
		{ return WindowY; }
	virtual void SetWindowSize(int width, int height);
	virtual void SetWindowOffset(int32 x, int32 y);
	virtual void ImageBeingDeleted(G42Image * image);
	virtual void HaveImageInfo(void);
	virtual void HaveImageRow(uint32 row);
	virtual void HaveImageEnd(void);
	virtual void ImageChanged(void);
	virtual void DrawImage(G42DrawLocation location, G42Image * image,
		int draw_x, int draw_y, int draw_width, int draw_height,
		G42ZoomInfo zoom_info, int32 pan_x = 0, int32 pan_y = 0);
	void DrawImageAt(G42DrawLocation location, G42Image * image, int origin_x, 
		int origin_y)
	{
		G42ZoomInfo zoom_info;
		DrawImage (location, image, 0, 0, WindowWidth, WindowHeight,
			zoom_info, -origin_x, -origin_y);
	}
	void DrawImageAt(G42DrawLocation location, G42Image * image, int origin_x, 
		int origin_y, G42ZoomInfo zoom_info)
	{
		DrawImage (location, image, 0, 0, WindowWidth, WindowHeight,
			zoom_info, -origin_x, -origin_y);
	}
	virtual void SetWindowsPalette(const G42Color * dpal, int num_pal);
#ifdef MSWIN
	virtual void WindowSizeChanged(void);
	virtual bool QueryNewPalette(void); // ges256
	virtual void PaletteChanged(HWND who); // ges256
#endif
	enum G42ScreenType {TrueColor, Palette256, Palette16, Monochrome};
	G42ScreenType GetScreenType(void) const
		{ return ScreenType; }
protected:
	G42ScreenType ScreenType;
#ifdef MSWIN
	HWND Window;
	HWND TopWindow;
	BITMAPINFOHEADER * DibHeader;
	int DibUsage;
	HPALETTE DitherWindowPalette; // ges256
#endif
#ifdef MACOS	// Things for basic image drawing
	GWorldPtr 		GWorldTile;
	PixMapHandle	PixMapTile;
	GWorldPtr		GWindowPtr;
	PaletteHandle 	GPaletteHandle;
	int				RowBytes;
	void			(*CallBack)();
	void			InitPixMap (G42Image * image);
	G42ImageInfo	PrevImageInfo;
	bool 			SameAsPrevImage (G42Image *image);
	short			GetScreenDepth (void);
	short			PixmapDepth (short depth);
	Boolean			ChangedForDraw;
#endif
	uint WindowWidth;
	uint WindowHeight;
	int32 WindowX;
	int32 WindowY;
	bool Valid;
	virtual void SetImageInformation(G42Image * image = 0);
	// virtual mostly so it doesn't get hidden by new forms
	virtual void DrawImage(G42DrawLocation location, G42Image * image,
		int draw_x, int draw_y, int draw_width, int draw_height,
		G42ZoomInfo zoom_info, int32 pan_x, int32 pan_y,
		G42Image * backup_image, G42ZoomInfo backup_zoom_info);
	void DrawMask(G42DrawLocation mdc, G42ImageData * image,
		int dc_x, int dc_y, int dc_width, int dc_height,
		int32 image_x, int32 image_y, int32 image_width, int32 image_height,
		bool invert = false);
	void FillMask(byte * mask_data, G42ImageData * mask,
		int32 row_bytes,
		int32 image_x, int32 image_y, int32 image_width, int32 image_height,
		bool invert = false);
	void DrawImageTile(G42DrawLocation location,
		int loc_x, int loc_y, int loc_width, int loc_height,
		G42ImageTile * tile, int tile_x, int tile_y,
		int tile_width, int tile_height, G42ImageData * image,
		G42ImageData * mask = 0, int32 pan_x = 0, int32 pan_y = 0, 
		G42Image *theImage = 0);
	#ifdef MACOS
	void DrawTileNormal (G42DrawLocation location, G42ImageData *image,
		Rect &sourceRect, Rect &destRect);	
	void DrawTilePrint (G42DrawLocation location, G42ImageData *image,
		Rect &sourceRect, Rect &destRect, G42LockedImageTile &tile,
		G42Image *theImage);	
	void DrawTileMask (G42DrawLocation location, 
		G42ImageData *image, int tile_x, int tile_y, int tile_width, int tile_height,
		short offsetX, short offsetY, G42LockedImageTile &tile, G42ImageData *mask, 
		Rect &sourceRect, Rect &destRect);	
	int	Width2Bytes (int width, short bitsPerPixel);
	#endif
};
#endif // G42IVIEW_H
