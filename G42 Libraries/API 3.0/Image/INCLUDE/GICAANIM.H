// gicaanim.h - G42AreaAnimatedController
#if !defined(GICAANIM_H)
#define GICAANIM_H
#include "gicambas.h"
class G42AreaAnimatedController : public G42MultiAreaBaseController
{
public:
	G42AreaAnimatedController(G42Notifier * notifier = 0);
	~G42AreaAnimatedController(void);
	// functions from G42PathComponent
	virtual void	Process(G42PathObject * object, G42PathComponent * sender);
	virtual void	BeginProcessing(void);
	virtual bool GetDrawAnimated(void);
	virtual void SetDrawAnimated(bool state = true);
	virtual void SetThumbnailMode(bool thumbnail_mode = true);
	virtual void Play(void);
	void SetUseSelectedInPath(bool use_selected = true);
	bool GetUseSelectedInPath(void) const
		{ return UseSelectedInPath; }
protected:
	G42Image * FrameImage;
	uint32 CurrentFrameIndex;
	bool UseSelectedInPath;
	void FinishFrame(void);
};
#endif // GICAANIM_H
