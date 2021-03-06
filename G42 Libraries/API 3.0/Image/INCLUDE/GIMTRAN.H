// g42read.h - graphscale ramp reader
#if !defined(GIMTRAN_H)
#define GIMTRAN_H
#include "gpcomp.h"
#include "gmem.h"
class G42TranMonoRamp : public G42PathComponent
{
public:
	G42TranMonoRamp(G42HeapManager & heap,
		int width = 256, int height = 256);
	~G42TranMonoRamp();
	virtual void Process(G42PathObject * object, G42PathComponent * sender);
	virtual void BeginProcessing();
private:
	G42HeapManager & Heap;
	int Width;
	int Height;
};
#endif
