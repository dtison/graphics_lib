// gicarea.h - G42AreaSingleController
#if !defined(GICASING_H)
#define GICASING_H
#include "gicarea.h"
class G42AreaSingleController : public G42AreaController
{
public:
	G42AreaSingleController(G42Notifier * notifier = 0);
	~G42AreaSingleController(void);
	virtual G42Image * GetImage(void);
	virtual G42Display * GetDisplay(void);
	virtual void Draw(G42Rect & rect, G42DrawLocation location = 0,
		bool image_only = false);
	virtual void Rebuild(G42Rect & rect, G42DrawLocation location = 0,
		bool image_only = false);
	virtual void SetImageScaleTranslate(void);
	virtual void SetArea(void);
	virtual void SetViewport(const G42Rect & viewport);
	virtual void SetSurfaceController(G42SurfaceController * surface);
	virtual void ResetImageFilters(void);
	virtual void	BeginProcessing(void); // output data as a head of a path
	virtual void	ClearPath(bool path_excepted = false);  // user calls this to clear a path that
	virtual void	Process(G42PathObject * object, G42PathComponent * sender);
												// receive a path object from Sender
	virtual void Play(void);
	virtual void Pause(void);
	virtual void Stop(void);
	virtual void Forward(void);
	virtual void Backward(void);
	virtual void Rewind(void);
	virtual void GoToEnd(void);
	virtual bool IsPaused(void);
	virtual bool IsPlaying(void);	
	virtual bool GetDrawAnimated(void);
	virtual void SetDrawAnimated(bool state = true);
	
protected:
	G42Ptr<G42ImageController> ImageController;
	G42Ptr<G42ImageStreamDuplicator> Duplicator;
	G42Image * PathImage;
	bool ClearModified;
	bool BlockDrawImage;
	bool SavedDrawAnimated;
	bool NeedImageInfoChangedMessage;
	virtual void SetupPath(void);
};
#endif // GICASING_H
