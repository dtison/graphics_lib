
#include "gicramp.h"
#include "gidata.h"
#include "gidesc.h"
G42ColorRamp::G42ColorRamp(G42HeapManager & heap,
	int width, int height)
:
	Heap(heap), Width(width), Height(height)
{
}
G42ColorRamp::~G42ColorRamp()
{
}
void
G42ColorRamp::Process(G42PathObject *, G42PathComponent *)
{
}
void
G42ColorRamp::BeginProcessing()
{
	G42PathBegin * begin = new G42PathBegin();
	G42ThrowIfNull(begin);
	Receiver->Process(begin, this);
	G42ImageDescriptor temp_desc(Width, Height,
		G42GetImageType(24, true), Heap);
	G42ImageDescriptor * desc = new G42ImageDescriptor(temp_desc);
	G42ThrowIfNull(desc);
	Receiver->Process(desc, this);
	for (int i = 0; i < temp_desc.GetNumStrips(); i++)
	{
		if (temp_desc.GetImageType() == G42ImageDescriptor::Color24)
		{
			G42Ptr<G42ImageDataColor24> data(new G42ImageDataColor24(Heap, i,
				temp_desc.GetRowsInStrip(i),
				temp_desc.GetRowBytes(),
				temp_desc.GetStripStartInImage(i), temp_desc.GetWidth()));
			G42Ptr<G42WriteImageData> lock(
				new G42WriteImageData(data.GetReference()));
			G42ImageDataColor24::G42Iterator begin = data->GetBegin(lock.GetReference());
			G42ImageDataColor24::G42Iterator end = data->GetEnd(lock.GetReference());
			G42FillColorRamp(data->GetBegin(lock.GetReference()),
				data->GetEnd(lock.GetReference()),
				data->GetStripStartInImage());
			lock.Clear(); // deletes lock class which unlocks the data
			Receiver->Process(data.Release(), this);
		}
		else
		{
			G42Ptr<G42ImageDataColor32> data(new G42ImageDataColor32(Heap, i,
				temp_desc.GetRowsInStrip(i),
				temp_desc.GetRowBytes(),
				temp_desc.GetStripStartInImage(i), temp_desc.GetWidth()));
			G42Ptr<G42WriteImageData> lock(
				new G42WriteImageData(data.GetReference()));
			G42FillColorRamp(data->GetBegin(lock.GetReference()),
				data->GetEnd(lock.GetReference()),
				data->GetStripStartInImage());
			lock.Clear(); // deletes lock class which unlocks the data
			Receiver->Process(data.Release(), this);
		}
	}
	G42PathEnd * end = new G42PathEnd();
	G42ThrowIfNull(end);
	Receiver->Process(end, this);
}
