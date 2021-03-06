
#include "gipramp.h"
#include "gidata.h"
#include "gidesc.h"
G42PaletteRamp::G42PaletteRamp(G42HeapManager & heap,
	int width, int height)
:
	Heap(heap), Width(width), Height(height)
{
}
G42PaletteRamp::~G42PaletteRamp()
{
}
void
G42PaletteRamp::Process(G42PathObject *, G42PathComponent *)
{
}
void
G42PaletteRamp::BeginProcessing()
{
	G42PathBegin * begin = new G42PathBegin();
	G42ThrowIfNull(begin);
	Receiver->Process(begin, this);
	
	G42ImageDescriptor temp_desc(Width, Height,
		G42ImageDescriptor::Color8, Heap);
	temp_desc.SetNumPalette(256);
	intf i;
	for (i = 0; i < 256; i++)
	{
		G42Color c((byte)(i >> 1), 0, (byte)(i));
		temp_desc.GetPalette()[i] = c;
	}
	G42ImageDescriptor * desc = new G42ImageDescriptor(temp_desc);
	G42ThrowIfNull(desc);
	Receiver->Process(desc, this);
	for (i = 0; i < temp_desc.GetNumStrips(); i++)
	{
		// protect ourselves from exceptions
		G42Ptr<G42ImageDataColor8> data(
			new G42ImageDataColor8(Heap, i,
				temp_desc.GetRowsInStrip(i),
				temp_desc.GetRowBytes(), temp_desc.GetStripStartInImage(i),
				temp_desc.GetWidth()));
		// new the lock, and protect it from exceptions
		G42Ptr<G42WriteImageData> lock(
			new G42WriteImageData(data.GetReference()));
		G42FillColor8Ramp(data->GetBegin(lock.GetReference()),
			data->GetEnd(lock.GetReference()),
			data->GetStripStartInImage());
		lock.Clear(); // deletes lock class which unlocks the data
		Receiver->Process(data.Release(), this);
	}
	G42PathEnd * end = new G42PathEnd();
	G42ThrowIfNull(end);
	Receiver->Process(end, this);
}
