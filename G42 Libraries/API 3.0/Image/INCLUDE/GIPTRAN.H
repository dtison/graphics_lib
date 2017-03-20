// g42read.h - graphscale ramp reader
#if !defined(GIPTRAN_H)
#define GIPTRAN_H
#include "gpcomp.h"
#include "gmem.h"
class G42TransPaletteRamp : public G42PathComponent
{
public:
	G42TransPaletteRamp(G42HeapManager & heap,
		int width = 256, int height = 256);
	~G42TransPaletteRamp();
	virtual void Process(G42PathObject * object, G42PathComponent * sender);
	virtual void BeginProcessing();
private:
	G42HeapManager & Heap;
	int Width;
	int Height;
};
#endif
