#include "g42idata.h"
#include "g42event.h"
#include "g42image.h"
#include "g42iview.h"
void
Invert(G42ImageData * data, G42EventUpdateHandler * event_handler)
{
	if (data->GetNumPalette() == 0)
	{
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
		event_handler->SetDenominator((int32)data->GetNumTiles());
      int32 tile_count = 0L;
      event_handler->Start();
      event_handler->Set(0L);
		for (y = 0; y < data->GetNumTilesDown(); y++)
		{
			for (uint x = 0; x < data->GetNumTilesAcross(); x++)
			{
				G42ModifiableImageTile tile(data->GetTile(y, x));
				for (uint z = 0; z < tile.GetTile()->GetTileHeight(); z++)
				{
					G42ImageRow * row = tile.GetTile()->GetRow(z + tile.GetTile()->GetTileOffsetY());
					unsigned char * ptr = *row;
					for (uint n = 0; n < tile.GetTile()->GetRowBytes(); n++, ptr++)
						*ptr = ~(*ptr);
				}
				tile.GetTile()->MarkModified();
				tile_count++;
            event_handler->Set(tile_count);
			}
		}
      event_handler->Stop();
	}
	else
	{
		G42Color * palette = data->GetPalette();
		G42Color * new_palette = new G42Color [data->GetNumPalette()];
		for (int n = 0; n < data->GetNumPalette(); n++)
		{
      	new_palette[n].red = ~(palette[n].red);
			new_palette[n].green = ~(palette[n].green);
			new_palette[n].blue = ~(palette[n].blue);
		}
		data->SetPalette(new_palette);
		delete [] new_palette;
	}
}
void
G42Image::Invert()
{
	if (Data)
	{
		if (Mask)
		{
			delete Mask;
			Mask = 0;
		}
		::Invert(Data, GetEventHandler());
 		for (int i = 0; i < NumViewArray; i++)
		{
			ViewArray[i]->ImageChanged();
		}
   	}
	Dirty = true;
}
