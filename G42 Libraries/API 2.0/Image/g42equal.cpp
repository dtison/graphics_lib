#include "g42idata.h"
#include "g42iter.h"
#include "g42event.h"
#include "g42image.h"
#include "g42iview.h"
void RGBtoHLS(G42ImageData *);
void HLStoRGB(G42ImageData *);
G42Color * RGBtoHLSPalette(G42Color *, int);
G42Color * HLStoRGBPalette(G42Color *, int);
void
Equalize(G42ImageData * data, G42EventUpdateHandler * event_handler)
{
	event_handler->SetDenominator((int32)data->GetHeight());
   int32 row_count = 0L;
	if (data->GetNumPalette() != 0)
	{
		if (data->GetDepth() == 1)
			return;
		if (data->GetDepth() == 4)
		{
			long * histogram = new long [256];
			memset(histogram, 0, 256 * sizeof(long));
			G42Color * new_palette = RGBtoHLSPalette(data->GetPalette(),
				data->GetNumPalette());
			int32 npix = 0L;
			G42LockedID4BitIterator image_it(data);
         event_handler->Start();
         event_handler->Set(0L);
			do
			{
				histogram[new_palette[(unsigned char)image_it].red]++;
				npix++;
				if (!image_it.Increment())
				{
					if (!image_it.NextRow())
						break;
               row_count++;
               event_handler->Set(row_count);
				}
			} while (1);
			float * transform = new float [256];
			transform[0] = (float)histogram[0]/npix;
			uint n;
			for (n = 1; n < 256; n++)
				transform[n] = transform[n-1] + (float)histogram[n]/npix;
			for (n = 0; n < 256; n++)
				histogram[n] = (transform[n] + .001961) * 255;
			for (n = 0; n < data->GetNumPalette(); n++)
				new_palette[n].red = histogram[new_palette[n].red];
			G42Color * palette = HLStoRGBPalette(new_palette, data->GetNumPalette());
			delete [] new_palette;
			data->SetPalette(palette);
			delete [] palette;
			delete [] histogram;
			delete [] transform;
         event_handler->Stop();
			return;
		}
		if (data->GetDepth() == 8)
		{
			long * histogram = new long [256];
			memset(histogram, 0, 256 * sizeof(long));
			G42Color * new_palette = RGBtoHLSPalette(data->GetPalette(),
				data->GetNumPalette());
			int32 npix = 0L;
			uint y;
         event_handler->SetDenominator((int32)data->GetNumTiles());
         int32 tile_count = 0L;
         event_handler->Start();
         event_handler->Set(0L);
			for (y = 0; y < data->GetNumTilesDown(); y++)
			{
				for (uint x = 0; x < data->GetNumTilesAcross(); x++)
				{
					G42LockedImageTile tile(data->GetTile(y, x));
					for (uint z = 0; z < tile.GetTile()->GetTileHeight(); z++)
					{
						G42ImageRow * row = tile.GetTile()->GetRow(z);
						unsigned char * ptr = *row;
						for (int n = 0; n < tile.GetTile()->GetTileWidth(); n++, ptr++)
						{
							histogram[new_palette[*ptr].red]++;
							npix++;
						}
					}
               tile_count++;
               event_handler->Set(tile_count);
				}
			}
			float * transform = new float [256];
			transform[0] = (float)histogram[0]/npix;
			uint n;
			for (n = 1; n < 256; n++)
				transform[n] = transform[n-1] + (float)histogram[n]/npix;
			for (n = 0; n < 256; n++)
				histogram[n] = (transform[n] + .001961) * 255;
			for (n = 0; n < data->GetNumPalette(); n++)
				new_palette[n].red = histogram[new_palette[n].red];
			G42Color * palette = HLStoRGBPalette(new_palette, data->GetNumPalette());
			delete [] new_palette;
			data->SetPalette(palette);
			delete [] palette;
			delete [] histogram;
			delete [] transform;
         event_handler->Stop();
			return;
		}
	}
	if (data->GetDepth() == 1)
		return;
	if (data->GetDepth() >= 8)
	{
		long * histogram = new long [256];
		memset(histogram, 0, 256 * sizeof(long));
		RGBtoHLS(data);
		int32 npix = 0L;
		G42ImageData24BitIterator image_it(data);
		histogram[image_it.GetRed()]++;
		npix++;
      event_handler->Start();
      event_handler->Set(0L);
		while (1)
		{
			if (!image_it.Increment())
			{
				if (!image_it.NextRow())
					break;
            row_count++;
            event_handler->Set(row_count>>1);
			}
			histogram[image_it.GetRed()]++;
			npix++;
		}
		float * transform = new float [256];
		transform[0] = (float)histogram[0]/npix;
		uint n;
		for(n = 1; n < 256; n++)
			transform[n] = transform[n-1] + (float)histogram[n]/npix;
		for(n = 0; n < 256; n++)
			histogram[n] = (transform[n] + .001961) * 255;
		image_it.Reset();
		image_it.Set(image_it.GetBlue(), image_it.GetGreen(),
			histogram[image_it.GetRed()]);
      row_count = 0L;
		while (1)
		{
			if (!image_it.Increment())
			{
				if (!image_it.NextRow())
					break;
            row_count++;
            event_handler->Set(((int32)data->GetHeight() + row_count) >> 1);
			}
			image_it.Set(image_it.GetBlue(), image_it.GetGreen(),
				histogram[image_it.GetRed()]);
		}
		HLStoRGB(data);
		delete [] histogram;
		delete [] transform;
      event_handler->Stop();
		return;
	}
	if (data->GetDepth() == 8)
	{
		long * histogram = new long [256];
		memset(histogram, 0, 256 * sizeof(long));
		G42ImageData8BitIterator image_it(data);
		int32 npix = 0L;
      event_handler->Start();
      event_handler->Set(0L);
      do
      {
			histogram[(unsigned char)image_it]++;
         npix++;
         if (!image_it.Increment())
         {
         	if (!image_it.NextRow())
            	break;
            row_count++;
            event_handler->Set(row_count >> 1);
         }
      } while (1);
		float * transform = new float [256];
		transform[0] = (float)histogram[0]/npix;
		uint n;
		for(n = 1; n < 256; n++)
			transform[n] = transform[n-1] + (float)histogram[n]/npix;
		for(n = 0; n < 256; n++)
			histogram[n] = (transform[n] + .001961) * 255;
		image_it.Reset();
      row_count = 0L;
      do
      {
      	image_it.Set(histogram[(unsigned char)image_it]);
         if (!image_it.Increment())
         {
         	if (!image_it.NextRow())
            	break;
            row_count++;
            event_handler->Set(((int32)data->GetHeight() + row_count) >> 1);
         }
      } while (1);
		delete [] histogram;
		delete [] transform;
		event_handler->Stop();
		return;
	}
	if (data->GetDepth() == 4)
	{
		long * histogram = new long [16];
		memset(histogram, 0, 16 * sizeof(long));
		int32 npix = 0L;
		uint y;
      event_handler->SetDenominator((int32)data->GetNumTiles());
      event_handler->Start();
      event_handler->Set(0L);
      int32 tile_count = 0L;
		for (y = 0; y < data->GetNumTilesDown(); y++)
		{
			for (uint x = 0; x < data->GetNumTilesAcross(); x++)
			{
				G42LockedImageTile tile(data->GetTile(y, x));
				for (uint z = 0; z < tile.GetTile()->GetTileHeight(); z++)
				{
					G42ImageRow * row = tile.GetTile()->GetRow(z);
					unsigned char * ptr = *row;
					for (int n = 0; n < (tile.GetTile()->GetTileWidth()>>1); n++, ptr++)
					{
						histogram[((*ptr)>>4)]++;
						npix++;
						histogram[((*ptr) & 0x0f)]++;
						npix++;
					}
					if (tile.GetTile()->GetTileWidth() & 0x01)
					{
						ptr++;
						histogram[((*ptr)>>4)]++;
						npix++;
					}
				}
         	tile_count++;
            event_handler->Set(tile_count >> 1);
			}
		}
		float * transform = new float [16];
		transform[0] = (float)histogram[0]/npix;
		uint n;
		for(n = 1; n < 16; n++)
			transform[n] = transform[n-1] + (float)histogram[n]/npix;
		for(n = 0; n < 16; n++)
			histogram[n] = (transform[n] + .031375) * 15;
      tile_count = 0L;
		for (y = 0; y < data->GetNumTilesDown(); y++)
		{
			for (uint x = 0; x < data->GetNumTilesAcross(); x++)
			{
				G42ModifiableImageTile tile(data->GetTile(y, x));
				for (uint z = 0; z < tile.GetTile()->GetTileHeight(); z++)
				{
					G42ImageRow * row = tile.GetTile()->GetRow(z);
					unsigned char * ptr = *row;
					for (int n = 0; n < (tile.GetTile()->GetTileWidth()>>1); n++, ptr++)
					{
						unsigned char result = histogram[((*ptr)>>4)];
						result <<= 4;
						result += histogram[((*ptr) & 0x0f)];
						*ptr = result;
					}
					if (tile.GetTile()->GetTileWidth() & 0x01)
					{
						unsigned char result = histogram[((*ptr)>>4)];
						result <<= 4;
						*ptr = result;
					}
				}
				tile.MarkModified();
            tile_count++;
            event_handler->Set(((int32)data->GetNumTiles() + tile_count) >> 1);
			}
		}
		delete [] histogram;
		delete [] transform;
		event_handler->Stop();
		return;
   }
}
void
G42Image::Equalize()
{
	if (Data)
   {
   	if (Mask)
      {
      	delete Mask;
         Mask = 0;
      }
		::Equalize(Data, GetEventHandler());
		for (int i = 0; i < NumViewArray; i++)
		{
			ViewArray[i]->ImageChanged();
		}
   }
	Dirty = true;
}
