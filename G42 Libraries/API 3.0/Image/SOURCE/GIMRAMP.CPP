
#include "gimramp.h"
#include "gidata.h"
#include "gidesc.h"
G42MonoRamp::G42MonoRamp(G42HeapManager & heap,
	int width, int height)
:
	Heap(heap), Width(width), Height(height)
{
}
G42MonoRamp::~G42MonoRamp()
{
}
void
G42MonoRamp::Process(G42PathObject *, G42PathComponent *)
{
}
void
G42MonoRamp::BeginProcessing()
{
	G42PathBegin * begin = new G42PathBegin();
	G42ThrowIfNull(begin);
	Receiver->Process(begin, this);
	
	G42ImageDescriptor temp_desc(Width, Height,
		G42ImageDescriptor::Gray1, Heap);
	G42ImageDescriptor * desc = new G42ImageDescriptor(temp_desc);
	G42ThrowIfNull(desc);
	Receiver->Process(desc, this);
	for (int i = 0; i < temp_desc.GetNumStrips(); i++)
	{
		// protect ourselves from exceptions
		G42Ptr<G42ImageDataGray1> data(
			new G42ImageDataGray1(Heap, i,
				temp_desc.GetRowsInStrip(i),
				temp_desc.GetRowBytes(), temp_desc.GetStripStartInImage(i),
				temp_desc.GetWidth()));
		// new the lock, and protect it from exceptions
		G42Ptr<G42WriteImageData> lock(
			new G42WriteImageData(data.GetReference()));
		G42FillMonoRamp(data->GetBegin(lock.GetReference()),
			data->GetEnd(lock.GetReference()),
			data->GetStripStartInImage());
		lock.Clear(); // deletes lock class which unlocks the data
		Receiver->Process(data.Release(), this);
	}
	G42PathEnd * end = new G42PathEnd();
	G42ThrowIfNull(end);
	Receiver->Process(end, this);
}
