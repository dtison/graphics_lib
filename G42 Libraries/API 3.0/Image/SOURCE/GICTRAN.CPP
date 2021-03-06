
#include "gictran.h"
#include "gidata.h"
#include "gidesc.h"
G42TransColorRamp::G42TransColorRamp(G42HeapManager & heap,
	int width, int height)
:
	Heap(heap), Width(width), Height(height)
{
}
G42TransColorRamp::~G42TransColorRamp()
{
}
void
G42TransColorRamp::Process(G42PathObject *, G42PathComponent *)
{
}
void
G42TransColorRamp::BeginProcessing()
{
	G42PathBegin * begin = new G42PathBegin();
	G42ThrowIfNull(begin);
	Receiver->Process(begin, this);
	
	G42ImageDescriptor temp_desc(Width, Height,
		G42GetImageType(24, true), Heap, true);
	temp_desc.SetTransparency();
	G42ImageDescriptor * desc = new G42ImageDescriptor(temp_desc);
	G42ThrowIfNull(desc);
	Receiver->Process(desc, this);
	for (int i = 0; i < temp_desc.GetNumStrips(); i++)
	{
		// protect ourselves from exceptions
		G42Ptr<G42ImageDataColor24> data(
			new G42ImageDataColor24(Heap, i,
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
			for (uint32 p = 0; p < Width; p++)
			{
				if (temp_desc.GetDepth() == 32)
					*row++ = 0;
				*row++ = (byte)((i * temp_desc.GetRowsPerStrip() + row_index + p) & 0xff);
				*row++ = (byte)((i * temp_desc.GetRowsPerStrip() + row_index) & 0xff);
				*row++ = (byte)((p) & 0xff);
			}
			byte * mask = lock->GetRow(row_index, true);
			memset(mask, i * temp_desc.GetRowsPerStrip() + row_index,
				data->GetTransRowBytes());
		}
		lock.Clear(); // deletes lock class which unlocks the data
		Receiver->Process(data.Release(), this);
	}
	G42PathEnd * end = new G42PathEnd();
	G42ThrowIfNull(end);
	Receiver->Process(end, this);
}
