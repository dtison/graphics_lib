#include "g42idata.h"
#include "g42iter.h"
#include "g42event.h"
#include "g42image.h"
#include "g42iview.h"
G42ImageData *
Sharpness(G42ImageData * data, int sharp, G42EventUpdateHandler * event_handler)
{
	if (sharp == 0)
		return data;
	int32 sharpness = (sharp * 256) / 50;
	if (!sharpness)
		return data;
	event_handler->SetDenominator((int32)data->GetHeight());
   int32 row_count = 0;
	if (data->GetNumPalette() == 0)
	{
		if (data->GetDepth() > 8)
		{
			int32 row_width = data->GetWidth();
			G42ImageData24BitIterator image_it(data);
			row_width *= image_it.GetCellSize();
			unsigned char * tbuf1 = new unsigned char [row_width];
			unsigned char * sbuf1 = new unsigned char [row_width];
			unsigned char * sbuf2 = new unsigned char [row_width];
			unsigned char * sbuf3 = new unsigned char [row_width];
         unsigned char * sbuf_ptr1 = sbuf1;
         unsigned char * sbuf_ptr2 = sbuf2;
         unsigned char * sbuf_ptr3 = sbuf3;
			do
			{
				memcpy(sbuf_ptr1, image_it.GetPtr(), image_it.GetCellSize());
				sbuf_ptr1 += image_it.GetCellSize();
			} while (image_it.Increment());
         image_it.NextRow();
			do
			{
				memcpy(sbuf_ptr2, image_it.GetPtr(), image_it.GetCellSize());
				sbuf_ptr2 += image_it.GetCellSize();
			} while (image_it.Increment());
         image_it.NextRow();
			do
			{
				memcpy(sbuf_ptr3, image_it.GetPtr(), image_it.GetCellSize());
				sbuf_ptr3 += image_it.GetCellSize();
			} while (image_it.Increment());
         image_it.ResetRow();
         image_it.DecRow();
         sbuf_ptr1 = sbuf1 + image_it.GetCellSize();
         sbuf_ptr2 = sbuf2;
         sbuf_ptr3 = sbuf3 + image_it.GetCellSize();
			memcpy(tbuf1, sbuf_ptr2, image_it.GetCellSize());
			unsigned char * tbuf1_ptr = tbuf1 + image_it.GetCellSize();
         event_handler->Start();
         event_handler->Set(0);
			while (1)
			{
				int32 blue = (int32)*(sbuf_ptr2 + image_it.GetBlueOffset());
				int32 green = (int32)*(sbuf_ptr2 + image_it.GetGreenOffset());
				int32 red = (int32)*(sbuf_ptr2 + image_it.GetRedOffset());
   			sbuf_ptr2 += (2 * image_it.GetCellSize());
            if (sbuf_ptr2 >= (sbuf2 + row_width))
            {
            	row_count++;
               event_handler->Set(row_count);
					sbuf_ptr2 -= image_it.GetCellSize();
               memcpy(tbuf1_ptr, sbuf_ptr2, image_it.GetCellSize());
               tbuf1_ptr = tbuf1;
               do
               {
               	memcpy(image_it.GetPtr(), tbuf1_ptr, image_it.GetCellSize());
                  tbuf1_ptr += image_it.GetCellSize();
               } while (image_it.Increment());
               image_it.NextRow();
               if (!image_it.NextRow())
						break;
               unsigned char * swap = sbuf1;
               sbuf1 = sbuf2;
               sbuf2 = sbuf3;
               sbuf3 = swap;
               sbuf_ptr3 = sbuf3;
               do
               {
               	memcpy(sbuf_ptr3, image_it.GetPtr(), image_it.GetCellSize());
                  sbuf_ptr3 += image_it.GetCellSize();
               } while (image_it.Increment());
               image_it.ResetRow();
               image_it.DecRow();
					sbuf_ptr1 = sbuf1 + image_it.GetCellSize();
               sbuf_ptr2 = sbuf2;
               sbuf_ptr3 = sbuf3 + image_it.GetCellSize();
               tbuf1_ptr = tbuf1;
               memcpy(tbuf1_ptr, sbuf2, image_it.GetCellSize());
               tbuf1_ptr += image_it.GetCellSize();
               continue;
            }
				blue += (int32)*(sbuf_ptr2 + image_it.GetBlueOffset());
				green += (int32)*(sbuf_ptr2 + image_it.GetGreenOffset());
				red += (int32)*(sbuf_ptr2 + image_it.GetRedOffset());
            sbuf_ptr2 -= image_it.GetCellSize();
				blue += (int32)*(sbuf_ptr1 + image_it.GetBlueOffset());
				green += (int32)*(sbuf_ptr1 + image_it.GetGreenOffset());
				red += (int32)*(sbuf_ptr1 + image_it.GetRedOffset());
            sbuf_ptr1 += image_it.GetCellSize();
				blue += (int32)*(sbuf_ptr3 + image_it.GetBlueOffset());
				green += (int32)*(sbuf_ptr3 + image_it.GetGreenOffset());
				red += (int32)*(sbuf_ptr3 + image_it.GetRedOffset());
            sbuf_ptr3 += image_it.GetCellSize();
				blue = (int32)*(sbuf_ptr2 + image_it.GetBlueOffset())
					- ((((blue >> 2) - (int32)*(sbuf_ptr2 + image_it.GetBlueOffset()))
               * sharpness) >> 8);
				if (blue > 255)
					blue = 255;
				if (blue < 0)
					blue = 0;
				green = (int32)*(sbuf_ptr2 + image_it.GetGreenOffset())
					- ((((green >> 2) - (int32)*(sbuf_ptr2 + image_it.GetGreenOffset()))
               * sharpness) >> 8);
				if (green > 255)
					green = 255;
				if (green < 0)
					green = 0;
				red = (int32)*(sbuf_ptr2 + image_it.GetRedOffset())
					- ((((red >> 2) - (int32)*(sbuf_ptr2 + image_it.GetRedOffset()))
               * sharpness) >> 8);
				if (red > 255)
					red = 255;
				if (red < 0)
					red = 0;
				*(tbuf1_ptr + image_it.GetBlueOffset()) = (unsigned char)blue;
				*(tbuf1_ptr + image_it.GetGreenOffset()) = (unsigned char)green;
				*(tbuf1_ptr + image_it.GetRedOffset()) = (unsigned char)red;
				tbuf1_ptr += image_it.GetCellSize();
			}
			delete [] tbuf1;
			delete [] sbuf1;
			delete [] sbuf2;
			delete [] sbuf3;
   		event_handler->Stop();
			return data;
		}
		if (data->GetDepth() == 8)
		{
			int32 row_width = 0L;
			for (uint x = 0; x < data->GetNumTilesAcross(); x++)
			{
				G42ImageTile * tile = data->GetTile(0, x);
				row_width += tile->GetTileWidth();
			}
			unsigned char * tbuf1 = new unsigned char [row_width];
			unsigned char * tbuf2 = new unsigned char [row_width];
			G42ImageData8BitIterator image_it(data);
			unsigned char * tbuf2_ptr = tbuf2;
			do
			{
				memcpy(tbuf2_ptr, image_it.GetPtr(), 1);
				tbuf2_ptr++;
			} while (image_it.Increment());
			image_it.NextRow();
			memcpy(tbuf1, image_it.GetPtr(), 1);
			unsigned char * tbuf1_ptr = tbuf1 + 1;
         event_handler->Start();
         event_handler->Set(0L);
			while (1)
			{
				int32 val = (int32)(unsigned char)image_it;
				image_it.Increment();
				if (!image_it.IncRow())
				{
					image_it.ResetRow();
					image_it.DecRow();
					tbuf2_ptr = tbuf2;
					do
					{
						memcpy(image_it.GetPtr(), tbuf2_ptr, 1);
						tbuf2_ptr++;
					} while (image_it.Increment());
					break;
				}
				val += (int32)(unsigned char)image_it;
				image_it.DecRow();
				image_it.DecRow();
				val += (int32)(unsigned char)image_it;
				image_it.IncRow();
				if (!image_it.Increment())
				{
					memcpy(tbuf1_ptr, image_it.GetPtr(), 1);
					image_it.ResetRow();
					image_it.DecRow();
					tbuf2_ptr = tbuf2;
					do
					{
						memcpy(image_it.GetPtr(), tbuf2_ptr, 1);
						tbuf2_ptr++;
					} while (image_it.Increment());
					image_it.NextRow();
					image_it.NextRow();
               row_count++;
               event_handler->Set(row_count);
					unsigned char * swap = tbuf2;
					tbuf2 = tbuf1;
					tbuf1 = swap;
					memcpy(tbuf1, image_it.GetPtr(), 1);
					tbuf1_ptr = tbuf1 + 1;
					continue;
				}
				val += (int32)(unsigned char)image_it;
				image_it.Decrement();
				val = (int32)(unsigned char)image_it
					- ((((val >> 2) - (int32)(unsigned char)image_it) * sharpness) >> 8);
				if (val > 255)
					val = 255;
				if (val < 0)
					val = 0;
				*tbuf1_ptr = (unsigned char)val;
				tbuf1_ptr++;
			}
			delete [] tbuf1;
			delete [] tbuf2;
         event_handler->Stop();
			return data;
		}
		if (data->GetDepth() == 4)
		{
			G42ImageData * new_data = new G42ImageData(data->GetData(),
				data->GetWidth(), data->GetHeight(), 8, 0);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42LockedID4BitIterator image_it1(data);
			G42ImageData8BitIterator image_it2(new_data);
			do
			{
				int val = (unsigned char)image_it1;
				val = ((val << 4) + val);
				image_it2.Set(val);
				image_it2.Increment();
				if (!image_it1.Increment())
				{
					if (!image_it1.NextRow())
						break;
					image_it2.NextRow();
				}
			} while (1);
			return Sharpness(new_data, sharp, event_handler);
		}
		return data;
	}
	// Palette in Use
	if (data->GetDepth() == 8)
	{
		G42ImageData * new_data = new G42ImageData(data->GetData(),
			data->GetWidth(), data->GetHeight(), 8*TrueColorPixelSize, 0);
		new_data->ReferenceCount++;
		new_data->SetFinished(true);
		new_data->SetValidStartRow(0);
		new_data->SetValidNumRows(new_data->GetHeight());
		G42Color * palette = data->GetPalette();
		G42LockedID8BitIterator image_it1(data);
		G42ImageData24BitIterator image_it2(new_data);
		image_it2.Set(palette[(unsigned char)image_it1].blue,
			palette[(unsigned char)image_it1].green,
			palette[(unsigned char)image_it1].red);
		while (1)
		{
			if (!image_it1.Increment())
			{
				if (!image_it1.NextRow())
					break;
				image_it2.NextRow();
				image_it2.Set(palette[(unsigned char)image_it1].blue,
					palette[(unsigned char)image_it1].green,
					palette[(unsigned char)image_it1].red);
				continue;
			}
			image_it2.Increment();
			image_it2.Set(palette[(unsigned char)image_it1].blue,
				palette[(unsigned char)image_it1].green,
				palette[(unsigned char)image_it1].red);
		}
		return Sharpness(new_data, sharp, event_handler);
	}
	if (data->GetDepth() == 4)
	{
		G42ImageData * new_data = new G42ImageData(data->GetData(),
			data->GetWidth(), data->GetHeight(), 8*TrueColorPixelSize, 0);
		new_data->ReferenceCount++;
		new_data->SetFinished(true);
		new_data->SetValidStartRow(0);
		new_data->SetValidNumRows(new_data->GetHeight());
		G42Color * palette = data->GetPalette();
		G42LockedID4BitIterator image_it1(data);
		G42ImageData24BitIterator image_it2(new_data);
		do
		{
			unsigned char result = (unsigned char)image_it1;
			image_it2.Set(palette[result].blue, palette[result].green,
				palette[result].red);
			image_it2.Increment();
			if (!image_it1.Increment())
			{
				if (!image_it1.NextRow())
					break;
				image_it2.NextRow();
			}
		} while (1);
		return Sharpness(new_data, sharp, event_handler);
	}
	return data;
}
void
G42Image::Sharpness(int sharp)
{
	G42ImageData * new_data = Data;
	if (Data)
   {
   	if (Mask)
      {
      	delete Mask;
      	Mask = 0;
      }
		new_data = ::Sharpness(Data, sharp, GetEventHandler());
   }
	if (new_data && Data != new_data)
	{
		delete Data;
		Data = new_data;
		RebuildInfo();
		for (int i = 0; i < NumViewArray; i++)
		{
			ViewArray[i]->ImageChanged();
		}
	}
	Dirty = true;
}
