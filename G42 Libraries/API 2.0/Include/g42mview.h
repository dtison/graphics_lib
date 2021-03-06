// G42MView - Multiple Image Viewer
#if !defined(G42MVIEW_H)
#define G42MVIEW_H
#include <g42iview.h>
#include <g42image.h>
#ifdef MACOS
typedef GWorldPtr * HDC;
#endif
class G42MultipleImageViewer : public G42ImageViewer
{
public:
#ifdef MSWIN
	G42MultipleImageViewer(HWND window, HWND top_window);
#endif
#ifdef MACOS
	G42MultipleImageViewer(WindowPtr windowPtr, void (*CallBackFn)() = nil);
#endif
	~G42MultipleImageViewer(void);
	void Draw(G42Image * image, int x, int y);
	void Draw(G42Image * image, int x, int y,
		int window_x, int window_y, int window_width, int window_height);
	void SetWindowSize(int width, int height);
#ifdef MSWIN
	void Draw(HDC dc, G42Image * image, int x, int y);
	void Draw(HDC dc, G42Image * image, int x, int y,
		int window_x, int window_y, int window_width, int window_height);
	void WindowSizeChanged(void);
	void QueryNewPalette(void);
	void PaletteChanged(void);
#endif
	virtual void ImageBeingDeleted(G42Image * image);
	virtual void HaveImageEnd(void);
	virtual void HaveImageInfo(void);
	virtual void HaveImageRow(uint32 row);
	virtual void ImageChanged(void);
private:
#ifdef MSWIN
	HWND Window;
	HWND TopWindow;
	HDC DC;
	BITMAPINFOHEADER * DibHeader;
	int DibUsage;
	void SetImageInformation(G42Image * image);
#endif
#ifdef MACOS
	GWorldPtr 		GWorldTile;
	PixMapHandle	PixMapTile;
	WindowPtr		GWindowPtr;
	PaletteHandle 	GPaletteHandle;
	int				RowBytes;
	void			(*CallBack)();
	void			InitPixMap (void);
	HDC DC;
	void SetPen2Wide (PenState & oldPenState);
#endif
	uint WindowWidth;
	uint WindowHeight;
	bool Valid;
	void Clear(void);
};
#endif // G42MVIEW_H
