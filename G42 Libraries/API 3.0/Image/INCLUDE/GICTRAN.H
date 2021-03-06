// gicramp.h - color ramp creator
#if !defined(GICTRAN_H)
#define GICTRAN_H
#include "gpcomp.h"
#include "gmem.h"
class G42TransColorRamp : public G42PathComponent
{
public:
	G42TransColorRamp(G42HeapManager & heap,
		int width = 256, int height = 256);
	~G42TransColorRamp();
	virtual void Process(G42PathObject * object, G42PathComponent * sender);
	virtual void BeginProcessing();
private:
	G42HeapManager & Heap;
	int Width;
	int Height;
};
#endif
