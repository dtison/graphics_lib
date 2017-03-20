// g42disp.h
#if !defined(G42DISP_H)
#define G42DISP_H
#include "gitype.h"
#include "gidata.h"
#include "gidesc.h"
#include "gpcomp.h"
#include "gtimer.h"
#include "giadesc.h"
#include "giaframe.h"
class G42ScreenPalette;
class G42SurfaceController;
class G42Display : public G42PathComponent, public G42TimerClass
{
	friend class G42ScreenPalette;
public:
	G42Display(G42Surface surface = 0);
	virtual ~G42Display();
	void Backward(void);
	void Clear();
	void Draw(const G42Rect & rect, G42DrawLocation location = 0);
	void Draw(G42DrawLocation location = 0) { Draw(Viewport, location); }
	void Forward(void);
	uint32 GetActiveFrame(void) const
		{ return ActiveFrame; }
	bool GetAnimatedDraw(void)
		{ return AnimatedDraw; }
	bool GetAutoDraw(void)
		{ return AutoDraw; }
	bool GetAutoPlay(void)
		{ return AutoPlay; }
	uint32 GetNumFrames(void) const
		{ return (*NumFrames); }
	const G42ScaleTranslate & GetScaleTranslate(void) { return ScaleTranslate; }
	G42Surface GetSurface(void) const { return Surface; }
	const G42Rect & GetViewport(void) { return Viewport; }
	void GoToEnd(void)
		{ if ((*NumFrames) > 0) SetActiveFrame((*NumFrames) - 1); }
	bool IsPaused(void) const
		{ return Paused; }
	bool IsPlaying(void) const
		{ return Playing; }
	void Pause(void);
	void Play(void);
	void Rewind(void)
		{ SetActiveFrame(0); }
	void SetActiveFrame(uint32 index);
	void SetAutoDraw(bool state = true)
		{ AutoDraw = state; }
	void SetAutoPlay(bool state = true);
	void SetAnimatedDraw(bool state = true);
	void SetScaleTranslate(const G42ScaleTranslate & trans);
	void SetScreenPalette(G42ScreenPalette * palette); // sets up auto-dithering
	void SetSurface(G42Surface surface) { Surface = surface; }
	void SetSurfaceController(G42SurfaceController * surface);
	void SetViewport(const G42Rect & viewport);
	void Stop(void);
	// really should be protected (I think)
	void Process(G42PathObject * object, G42PathComponent * sender);
	virtual void ClearPath(bool path_excepted = false);
	virtual void BeginProcessing (void);
	#ifdef MACOS	// TODO:  Fix this
	static int16 MacGetScreenDepth (void);
	#endif
private:
	uint32 											ActiveFrame;
	G42AnimatedImageDescriptor *				AnimatedDescriptor;
	bool 												AnimatedDraw;
	bool 												AutoDraw;
	bool 												AutoPlay;
	G42Vector <G42ImageData **> *				Data;
	G42Vector <G42ImageDescriptor *> *		Descriptor;
	G42Vector <G42ImageData **>	 			DitherData;
	G42Vector <G42ImageDescriptor *>			DitherDescriptor;
	G42Vector <G42AnimatedImageFrame *>		DitherFrame;
	G42MultipleImageDescriptor *	 			DitherMultiDescriptor;
	uint32	 										DitherNumFrames;
	bool												DitherPath; // for ScreenPalette
	G42ScaleTranslate 							DitherScaleTranslate;
	G42Vector <G42AnimatedImageFrame *> *	Frame;
	uint32	 										Height;
	bool 												InvalidateFill;
	G42Rect 											InvalidateRect;
	G42DrawLocation	 							Location;
	G42MultipleImageDescriptor ** 			MultiDescriptor;
	bool 												NeedInvalidate;
	uint32 *	 										NumFrames;
	G42Vector <G42ImageData **>	 			OriginalData;
	G42Vector <G42ImageDescriptor *>			OriginalDescriptor;
	G42Vector <G42AnimatedImageFrame *>		OriginalFrame;
	G42MultipleImageDescriptor *	 			OriginalMultiDescriptor;
	uint32	 										OriginalNumFrames;
	G42ScaleTranslate 							OriginalScaleTranslate;
	bool 												Paused;
	bool 												Playing;
	bool 												Processing;
	G42ScaleTranslate 							ScaleTranslate;
	G42ScreenPalette *							ScreenPalette;
	bool												SingleFrameOnly;
	G42Surface 										Surface;
	G42SurfaceController * 						SurfaceController;
	uintf 											TimerID;
	bool 												TimerRunning;
	bool												UseDitheredImage;
	G42Rect 											Viewport;
	uint32 											Width;
	#ifdef MSWIN
	BITMAPINFOHEADER * 							DibHeader;
	BITMAPINFOHEADER * 							MaskHeader;
	RGBQUAD * 										MaskPalette;
	int 												WindowsDibUsage;
	#endif
	#ifdef MACOS
	RgnHandle										ClipRegion;
	PixMapHandle 									PixmapHandle;
	#endif
	void ClearDithered(void);
	void DrawData(G42ImageData * data, const G42Rect & rect,
		G42DrawLocation location = 0);
	void DrawData(G42ImageData * data, G42DrawLocation location = 0)
		{ DrawData(data, Viewport, location); }
	void DrawFrame(uint32 frame, const G42Rect & rect,
		G42DrawLocation location = 0);
	void DrawFrame(uint32 frame, G42DrawLocation location = 0)
		{ DrawFrame(frame, Viewport, location); }
	void HandleReplace(void);
	void HandleTimer(void);
	bool IsTrueColor(void); // for ScreenPalette
	intf GetNumGrays(void); // returns number of grays if grayscale
	intf GetNumPalette(void); // returns number of palettes if paletted
	const G42Color * GetPalette(void); // returns palette if paletted image
	void NextFrame(void);
	void Redither(void); // for ScreenPalette
	void SetDelay(void);
	void SetDitherPath(bool dither_path = true); // for ScreenPalette
	void SetupPixmap (void);
	void StartAnimatedTimer(uint32 msec);
	void StopAnimatedTimer(void);
	virtual void TimerStopped(uintf id);
};
#endif
