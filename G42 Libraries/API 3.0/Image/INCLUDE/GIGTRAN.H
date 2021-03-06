// g42read.h - graphscale ramp reader
#if !defined(GIGTRAN_H)
#define GIGTRAN_H
#include "gpcomp.h"
#include "gmem.h"
class G42TransGrayRamp : public G42PathComponent
{
public:
	G42TransGrayRamp(G42HeapManager & heap,
		int width = 256, int height = 256);
	~G42TransGrayRamp();
	virtual void Process(G42PathObject * object, G42PathComponent * sender);
	virtual void BeginProcessing();
private:
	G42HeapManager & Heap;
	int Width;
	int Height;
};
#endif
