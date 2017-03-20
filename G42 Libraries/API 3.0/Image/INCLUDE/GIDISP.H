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
class G42SurfaceController;
class G42Display : public G42PathComponent, public G42TimerClass
{
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
		{ return NumFrames; }
	const G42ScaleTranslate & GetScaleTranslate(void) { return ScaleTranslate; }
	G42Surface GetSurface(void) const { return Surface; }
	const G42Rect & GetViewport(void) { return Viewport; }
	void GoToEnd(void)
		{ if (NumFrames > 0) SetActiveFrame(NumFrames - 1); }
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
	void SetScaleTranslate(const G42ScaleTranslate & trans)
		{ ScaleTranslate = trans; }
	void SetSurface(G42Surface surface) { Surface = surface; }
	void SetSurfaceController(G42SurfaceController * surface)
		{ SurfaceController = surface; }
	void SetViewport(const G42Rect & viewport)
		{ Viewport = viewport; }
	void Stop(void);
	// really should be protected (I think)
	void Process(G42PathObject * object, G42PathComponent * sender);
	virtual void ClearPath(bool path_excepted = false);
	#ifdef MSWIN
	void SetPaletteInfo(G42PaletteInfo & info)
		{ WindowPalette = &info; }
	#endif
	#ifdef MACOS
//	void SetPaletteInfo (G42PaletteInfo & info)
//		{if (info.NumPal) {}}
	void MacDrawData (G42ImageData * data, G42ImageDescriptor * descriptor, 
		const G42Rect &clip);
	static int16 MacGetScreenDepth (void);
	#endif
private:
	uint32 										ActiveFrame;
	G42AnimatedImageDescriptor *			AnimatedDescriptor;
	bool 											AnimatedDraw;
	bool 											AutoDraw;
	bool 											AutoPlay;
	G42Vector<G42ImageData **> 			Data;
	G42Vector<G42ImageDescriptor *>		Descriptor;
	G42Vector<G42AnimatedImageFrame *>	Frame;
	uint32 										Height;
	bool 											InvalidateFill;
	G42Rect 										InvalidateRect;
	G42DrawLocation 							Location;
	G42MultipleImageDescriptor * 			MultiDescriptor;
	bool 											NeedInvalidate;
	uint32 										NumFrames;
	bool 											Paused;
	bool 											Playing;
	bool 											Processing;
	G42ScaleTranslate 						ScaleTranslate;
	bool											SingleFrameOnly;
	G42Surface 									Surface;
	G42SurfaceController * 					SurfaceController;
	uintf 										TimerID;
	bool 											TimerRunning;
	G42Rect 										Viewport;
	uint32 										Width;
	#ifdef MSWIN
	BITMAPINFOHEADER * 						DibHeader;
	BITMAPINFOHEADER * 						MaskHeader;
	RGBQUAD * 									MaskPalette;
	int 											WindowsDibUsage;
	#endif
	#ifdef MACOS
	RgnHandle									ClipRegion;
	PixMapHandle 								PixmapHandle;
	#endif
	void DrawData(G42ImageData * data, G42ImageDescriptor * descriptor, 
		const G42Rect & rect, G42DrawLocation location = 0);
#ifdef NOTYET
	void DrawData(G42ImageData * data, G42DrawLocation location = 0)
		{ DrawData(data, Viewport, location); }
#endif
	void DrawFrame(uint32 frame, const G42Rect & rect,
		G42DrawLocation location = 0);
	void DrawFrame(uint32 frame, G42DrawLocation location = 0)
		{ DrawFrame(frame, Viewport, location); }
	void HandleReplace(void);
	void HandleTimer(void);
	void NextFrame(void);
	void SetDelay(void);
	void SetupPixmap (bool force_setup = false);
	void StartAnimatedTimer(uint32 msec);
	void StopAnimatedTimer(void);
	virtual void TimerStopped(uintf id);
};
#endif
