
#include "gimtran.h"
#include "gidata.h"
#include "gidesc.h"
G42TranMonoRamp::G42TranMonoRamp(G42HeapManager & heap,
	int width, int height)
:
	Heap(heap), Width(width), Height(height)
{
}
G42TranMonoRamp::~G42TranMonoRamp()
{
}
void
G42TranMonoRamp::Process(G42PathObject *, G42PathComponent *)
{
}
void
G42TranMonoRamp::BeginProcessing()
{
	G42PathBegin * begin = new G42PathBegin();
	G42ThrowIfNull(begin);
	Receiver->Process(begin, this);
	
	G42ImageDescriptor temp_desc(Width, Height,
		G42ImageDescriptor::Gray1, Heap, true);
	temp_desc.SetTransparency();
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
