// gicramp.h - color ramp creator
#if !defined(GICRAMP_H)
#define GICRAMP_H
#include "gpcomp.h"
#include "gmem.h"
class G42ColorRamp : public G42PathComponent
{
public:
	G42ColorRamp(G42HeapManager & heap,
		int width = 256, int height = 256);
	~G42ColorRamp();
	virtual void Process(G42PathObject * object, G42PathComponent * sender);
	virtual void BeginProcessing();
private:
	G42HeapManager & Heap;
	int Width;
	int Height;
};
template <class T>
void G42FillColorRamp(T begin, T end, uint32 r)
{
	for (T i = begin; i != end; i++, r++)
	{
		T::G42Row row(*i);
		int p = 0;
		for (T::G42Row::G42Iterator j = row.GetBegin();
			j != row.GetEnd(); j++, p++)
		{
			(*j) = G42Color((byte)((p) & 0xff), (byte)((r) & 0xff),
				(byte)((r + p) & 0xff));
		}
	}
}
#endif
