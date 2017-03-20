#include "g42idata.h"
#include "g42iter.h"
#include "g42event.h"
#include "g42image.h"
#include "g42iview.h"
#define DEBUG
#include "CommMac.h"
void
SwapRB(G42ImageData * data, G42EventUpdateHandler * event_handler)
{
	if (data->GetNumPalette() == 0)
	{
		if (data->GetDepth() <= 8)
			return;
		uint index_array [64];
		uint count = 0;
      uint y;
		for (y = 0; y < data->GetNumTilesDown(); y++)
		{
			for (uint x = 0; x < data->GetNumTilesAcross(); x++)
			{
				G42ImageTile * tile = data->GetTile(y, x);
				index_array[count++] = tile->GetDataHandle();
			}
		}
      data->GetData()->PreLoad(index_array, count);
      event_handler->SetDenominator((int32)data->GetHeight());
      event_handler->Start();
      event_handler->Set(0L);
      int32 row_count = 0L;
		G42ImageData24BitIterator image_it(data);
		do
		{
			image_it.Set(image_it.GetRed(), image_it.GetGreen(), image_it.GetBlue());
			if (!image_it.Increment())
			{
				if (!image_it.NextRow())
					break;
				row_count++;
            event_handler->Set(row_count);
			}
		} while (1);
      event_handler->Stop();
	}
	else
	{
		G42Color * palette = data->GetPalette();
		G42Color * new_palette = new G42Color [data->GetNumPalette()];
		for (int n = 0; n < data->GetNumPalette(); n++)
		{
			new_palette[n].red = palette[n].blue;
			new_palette[n].green = palette[n].green;
			new_palette[n].blue = palette[n].red;
		}
		data->SetPalette(new_palette);
      delete [] new_palette;
   }
}
void
G42Image::SwapRB()
{
	if (Data)
   {
   	if (Mask)
      {
      	delete Mask;
         Mask = 0;
      }
		::SwapRB(Data, GetEventHandler());
		for (int i = 0; i < NumViewArray; i++)
		{
			ViewArray[i]->ImageChanged();
		}
   }
	Dirty = true;
}
