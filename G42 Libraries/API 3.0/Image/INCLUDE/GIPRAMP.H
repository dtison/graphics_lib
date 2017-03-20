// g42read.h - graphscale ramp reader
#if !defined(GIPRAMP_H)
#define GIPRAMP_H
#include "gpcomp.h"
#include "gmem.h"
class G42PaletteRamp : public G42PathComponent
{
public:
	G42PaletteRamp(G42HeapManager & heap,
		int width = 256, int height = 256);
	~G42PaletteRamp();
	virtual void Process(G42PathObject * object, G42PathComponent * sender);
	virtual void BeginProcessing();
private:
	G42HeapManager & Heap;
	int Width;
	int Height;
};
template <class Iterator>
void G42FillColor8Ramp(Iterator begin, Iterator end, uint32 r)
{
	for (Iterator i = begin; i != end; i++, r++)
	{
		Iterator::G42Row row(*i);
		uint32 p = 0;
		for (Iterator::G42Row::G42Iterator j = row.GetBegin();
			j != row.GetEnd(); j++, p++)
		{
			*j = (byte)((r + p) & 0xff);
		}
	}
}
#endif
