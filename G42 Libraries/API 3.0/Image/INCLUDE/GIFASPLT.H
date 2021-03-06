// gifasplt.h - animated image frame splitter
#if !defined(GIFASPLT_H)
#define GIFASPLT_H
#include "gifilter.h"
class G42ImageAnimatedSplitter : public G42ImageFilter
{
public:
	// pass in the frame or frames you want split out
	enum G42LinkOrder { FirstLinkGetsSplitFrames, FirstLinkGetsNonSplitFrames };
	G42ImageAnimatedSplitter(uint32 frame,
		G42LinkOrder order = FirstLinkGetsNonSplitFrames);
	G42ImageAnimatedSplitter(uint32 * frames, uint32 num_frames,
		G42LinkOrder order = FirstLinkGetsNonSplitFrames);
	G42ImageAnimatedSplitter(const G42Vector <uint32> & frames,
		uint32 num_frames, G42LinkOrder order = FirstLinkGetsNonSplitFrames);
	virtual ~G42ImageAnimatedSplitter(void);
	// defaults non-split frames first, split frames second, if either is
	// not linked, the frames get thrown out
	void MakeNextLinkGetSplitFrames(void)
		{ NextReceiverGetsSplitFrames = true; }
	void MakeNextLinkGetNonSplitFrames(void)
		{ NextReceiverGetsSplitFrames = false; }
	virtual G42ImageFilter * MakeCopy(void) const;
	virtual void ClearPath(bool path_excepted = false);
	virtual void HandleBegin(G42PathBegin * begin, G42PathComponent * sender);
	virtual void HandleDescriptor(G42ImageDescriptor * descriptor,
		G42PathComponent * sender);
	virtual void HandleData(G42ImageData * data, G42PathComponent * sender);
	virtual void HandleEnd(G42PathEnd * end, G42PathComponent * sender);
	virtual void HandleUnknown(G42PathObject * object,
		G42PathComponent * sender);
	virtual void ReceiverDestructing(G42PathComponent * receiver); // throw()
	virtual void ReplaceReceiver(G42PathComponent * old_receiver,
		G42PathComponent * new_receiver);
			// throw(G42InvalidParameterException) (and check derived class)
	virtual void SetReceiver(G42PathComponent * receiver);
			// throw(G42InvalidParameterException, G42PathMultipleReceiverException)
private:
	uint32					CurrentFrame;
	bool						FirstFrame;
	G42Vector <uint32>	Frames;
	bool						NextReceiverGetsSplitFrames; // defaults false
	uint32					NumFrames;
	bool						SplitFrame;
	G42PathComponent *	SplitReceiver;
};
#endif // GIFASPLT_H
