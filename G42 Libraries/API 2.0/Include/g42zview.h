// G42ZView - Zoomed Image Viewer
#if !defined(G42ZVIEW_H)
#define G42ZVIEW_H
/*
	G42ZoomImageViewer class - displays a single image in a window
	Primary author:
		Guy Eric Schalnat
	Mac author:
		David Ison
*/
#include <g42iview.h>
#include <g42sbar.h>
#include <g42opal.h>
// (m.3)  Deleted some MACOS lines
class G42ZoomImageViewer;
class G42ZoomImageViewer : public G42SimpleImageViewer
{
public:
#ifdef MSWIN
	G42ZoomImageViewer(HWND window, HWND top_window);
#endif
#ifdef MACOS
	G42ZoomImageViewer(GWorldPtr windowPtr, void (*CallBackFn)() = nil);
	void SetPanXY (int x, int y);
	void GetPanXY (int & x, int & y);
#endif
	virtual ~G42ZoomImageViewer(void);
	virtual void ImageBeingDeleted(G42Image * image);
	virtual void Draw(void);
	virtual void Draw(int x, int y, int width, int height, bool expand = false);
	virtual void SetPan(void);
	virtual void Rebuild(void);
	virtual void SetImage(G42Image * image, bool draw = true);
	virtual void SetZoom(double zoom);
	virtual void SetZoomFitAll(void);
	virtual void SetZoomFitWidth(void);
	double GetZoom(void)
		{ return ZoomFactor; }
	virtual void HaveImageInfo(void);
	virtual void HaveImageRow(uint32 row);
	virtual void HaveImageEnd(void);
	virtual void ImageChanged(void);
	virtual void SetWindowSize(int width, int height);
#ifdef MSWIN
	virtual void ProcessHScrollBarCode(uint code, uint pos);
	virtual void ProcessVScrollBarCode(uint code, uint pos);
	virtual void WindowSizeChanged(void);
	virtual bool QueryNewPalette(void); // ges256
	virtual void PaletteChanged(HWND who); // ges256
#endif
	bool NeedIdleAction(void);
	virtual bool IdleAction(void);
	virtual void Clear(void);
	G42Image * GetImage(void)
		{ return Image; }
	// repeat of base class so we don't hide it
	virtual void DrawImage(G42DrawLocation location, G42Image * image,
		int draw_x, int draw_y, int draw_width, int draw_height,
		G42ZoomInfo zoom_info, int32 pan_x = 0, int32 pan_y = 0);
	void TranslateScreenToImage(int32 & x, int32 & y);
protected:
	bool NeedIdle;
	int UpdateTileRow;
	int UpdateTileColumn;
	int UpdateTileStartRow;
	int UpdateTileStartColumn;
	int UpdateTileNumRows;
	int UpdateTileNumColumns;
	G42Image * StretchImage;
	G42Image * DitherImage;
	G42Image * DitherBaseImage;
	G42OptimizedPalette * DitherPalette;
	G42OptimizedPalette * DitherBasePalette;
	bool CheckPartial;
#ifdef MSWIN
	G42DrawLocation DC;
#endif
#ifdef MACOS
	G42DrawLocation DC;
// Stuff to get GWorlds working!!!
	CGrafPtr		OldPrepDrawImgPort;
	GDHandle		OldPrepDrawImgDevice;
	short			PrepareDrawMapCnt;
	short			PrepareDrawRubberCnt;
	short			PrepareDrawImgCnt;
#endif
	int InFit;
	G42ScrollBar * HBar;
	G42ScrollBar * VBar;
	bool HBarVisible;
	bool VBarVisible;
	G42Image * Image;
	int32 PanX, PanY;
	G42ZoomMode ZoomMode;
	int ZoomInt;
	double ZoomFactor;
	uint32 ProgressiveNumRows;
	uint32 ProgressiveStartRow;
	uint32 ProgressiveEndRow;
	bool NeedDither;
	bool NeedStretchDither;
	int32 ImageToWindow(int32 value) const
		{G42ZoomInfo zi(ZoomMode, ZoomInt, ZoomFactor);
		return zi.ImageToWindow(value); }
	int32 WindowToImage(int32 value) const
		{G42ZoomInfo zi(ZoomMode, ZoomInt, ZoomFactor);
		return zi.WindowToImage(value); }
	virtual void PrepareDrawImage(G42DrawLocation location,
		int draw_x, int draw_y, int draw_width, int draw_height);
	virtual void CleanupDrawImage(G42DrawLocation location);
	virtual void Draw(G42DrawLocation location, int x, int y,
		int width, int height, bool expand = false);
	// repeat of base class so we don't hide it
	virtual void DrawImage(G42DrawLocation location, G42Image * image,
		int draw_x, int draw_y, int draw_width, int draw_height,
		G42ZoomInfo zoom_info, int32 pan_x, int32 pan_y,
		G42Image * backup_image, G42ZoomInfo backup_zoom_info);
	// new forms of DrawImage
	virtual void DrawImage(G42DrawLocation location, G42Image * image);
	virtual void DrawImage(G42DrawLocation location, G42Image * image,
		int draw_x, int draw_y, int draw_width, int draw_height);
	virtual void DrawImage(G42DrawLocation location,
   	G42Image * image,
		G42Image * & dither_image, G42Image * & stretch_image,
   	G42Image * & dither_base_image, int32 pan_x, int32 pan_y,
		int draw_x, int draw_y, int draw_width, int draw_height);
	virtual bool ImageIdle(G42Image * & image,
		G42Image * & stretch_image, G42Image * & dither_image,
			G42Image * & dither_base_image, int32 pan_x, int32 pan_y);
	virtual void PreparePalette(void);
	virtual void SetImageInformation(G42Image * image = 0);
	virtual void SetWindowsGrayscalePalette(int num_pal);
};
#endif // G42ZVIEW_H
