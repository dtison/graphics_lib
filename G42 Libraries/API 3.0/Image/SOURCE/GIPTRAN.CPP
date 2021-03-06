
#include "giptran.h"
#include "gidata.h"
#include "gidesc.h"
G42TransPaletteRamp::G42TransPaletteRamp(G42HeapManager & heap,
	int width, int height)
:
	Heap(heap), Width(width), Height(height)
{
}
G42TransPaletteRamp::~G42TransPaletteRamp()
{
}
void
G42TransPaletteRamp::Process(G42PathObject *, G42PathComponent *)
{
}
void
G42TransPaletteRamp::BeginProcessing()
{
	G42PathBegin * begin = new G42PathBegin();
	G42ThrowIfNull(begin);
	Receiver->Process(begin, this);
	
	G42ImageDescriptor temp_desc(Width, Height,
		G42ImageDescriptor::Color8, Heap, true);
	temp_desc.SetTransparency();
	temp_desc.SetNumPalette(256);
	int i;
	for (i = 0; i < 256; i++)
	{
		G42Color c((byte)i, (byte)(255 - i), (byte)(i >> 1));
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
				temp_desc.GetWidth(), temp_desc.GetTransRowBytes()));
		// new the lock, and protect it from exceptions
		G42Ptr<G42WriteImageData> lock(
			new G42WriteImageData(data.GetReference()));
		for (int row_index = 0; row_index < temp_desc.GetRowsInStrip(i);
			row_index++)
		{
			byte * row = lock->GetRow(row_index);
			memset(row, i * temp_desc.GetRowsPerStrip() + row_index,
				data->GetRowBytes());
			byte * mask = lock->GetRow(row_index, true);
			memset(mask, i * temp_desc.GetRowsPerStrip() + row_index,
				data->GetTransRowBytes());
		}
		lock.Clear(); // deletes lock class which unlocks the data
#if 0 // old way of locking
		{ // scope limiter for wdata to lock and unlock it
			G42WriteImageData wdata(data.GetReference());
			for (int row_index = 0; row_index < temp_desc.GetRowsInStrip(i);
				row_index++)
			{
				byte * row = wdata.GetRow(row_index);
				memset(row, i * temp_desc.GetRowsPerStrip() + row_index,
					data->GetRowBytes());
			}
		} // end wdata scope limiter, which unlocks the data
#endif
		Receiver->Process(data.Release(), this);
	}
	G42PathEnd * end = new G42PathEnd();
	G42ThrowIfNull(end);
	Receiver->Process(end, this);
}
