#include "g42idata.h"
#include "g42iter.h"
#include "g42event.h"
#include "g42image.h"
#include "g42iview.h"
#include <math.h>
void RGBtoHLS(G42ImageData *);
void HLStoRGB(G42ImageData *);
G42Color * RGBtoHLSPalette(G42Color *, int);
G42Color * HLStoRGBPalette(G42Color *, int);
void tone_control_gamma(unsigned char *, int, int);
void tone_control_contrast(unsigned char *, unsigned char *, int, int);
G42ImageData *
ColorAdjust(G42ImageData * data, int high, int mid, int low, int channel,
	G42EventUpdateHandler * event_handler)
{
	unsigned char Map[256];
	int n;
	for (n = 0; n < 256; n++)
		Map[n] = (unsigned char)n;
	unsigned char SatMap[256];
	for (n = 0; n < 256; n++)
		SatMap[n] = (unsigned char)n;
	if (mid != 10)
		tone_control_gamma(Map, mid, channel);
	if (low)
		tone_control_contrast(Map, SatMap, low, channel);
	if (high)
	{
		if (high > 0)
		{
			int32 brightness = 100 - high;
			int n;
			for (n = 0; n < 256; n++)
			{
				int32 v = 255 - Map[n];
				v *= brightness;
				v += 50;
				v /= 100;
				if (v > 255)
					v = 255;
				v = 255 - v;
				Map[n] = (unsigned char)v;
			}
			for (n = 0; n < 256; n++)
				SatMap[n] = n - ((n * (int32)high)/100);
		}
		else if (high < 0)
		{
			int32 brightness = 100 + high;
			int n;
			for (n = 0; n < 256; n++)
			{
				int32 v = Map[n];
				v *= brightness;
				v += 50;
				v /= 100;
				if (v > 255)
					v = 255;
				Map[n] = (unsigned char)v;
			}
			for (n = 0; n < 256; n++)
				SatMap[n] = n + ((n * (int32)high)/100);
		}
	}
	event_handler->SetDenominator((int32)data->GetHeight());
   int32 row_count = 0L;
	if (data->GetNumPalette() == 0)
	{
		if (data->GetDepth() > 8)
		{
      	event_handler->Start();
			switch (channel)
			{
				case 0:
				{
					RGBtoHLS(data);
					G42ImageData24BitIterator image_it(data);
					do
					{
						image_it.Set(image_it.GetBlue(), SatMap[image_it.GetGreen()],
							Map[image_it.GetRed()]);
						if (!image_it.Increment())
						{
							if (!image_it.NextRow())
								break;
                     row_count++;
                     event_handler->Set(row_count);
						}
					}
					while (1);
					HLStoRGB(data);
					break;
				}
				case 1:
				{
					G42ImageData24BitIterator image_it(data);
					do
					{
						image_it.Set(image_it.GetBlue(), image_it.GetGreen(),
							Map[image_it.GetRed()]);
						if (!image_it.Increment())
						{
							if (!image_it.NextRow())
								break;
                     row_count++;
                     event_handler->Set(row_count);
						}
					}
					while (1);
					break;
				}
				case 2:
				{
					G42ImageData24BitIterator image_it(data);
					do
					{
						image_it.Set(image_it.GetBlue(), Map[image_it.GetGreen()],
							image_it.GetRed());
						if (!image_it.Increment())
						{
							if (!image_it.NextRow())
								break;
                     row_count++;
                     event_handler->Set(row_count);
						}
					}
					while (1);
					break;
				}
				case 3:
				{
					G42ImageData24BitIterator image_it(data);
					do
					{
						image_it.Set(Map[image_it.GetBlue()], image_it.GetGreen(),
							image_it.GetRed());
						if (!image_it.Increment())
						{
							if (!image_it.NextRow())
								break;
                     row_count++;
                     event_handler->Set(row_count);
						}
					}
					while (1);
					break;
				}
			}
			event_handler->Stop();
			return data;
		}
		if (data->GetDepth() == 8)
		{
			if (channel != 0)
			{
				G42ImageData * new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), data->GetHeight(), 8, 256);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
				G42Color * new_palette = new G42Color [256];
				for (uint n = 0; n < 256; n++)
					new_palette[n].blue = new_palette[n].green = new_palette[n].red = n;
				new_data->SetPalette(new_palette);
				G42ImageData8BitIterator image_it1(data);
				G42ImageData8BitIterator image_it2(new_data);
				image_it2.Set((unsigned char)image_it1);
				while (1)
				{
					if (!image_it1.Increment())
					{
						if (!image_it1.NextRow())
							break;
						image_it2.NextRow();
						image_it2.Set((unsigned char)image_it1);
						continue;
					}
					image_it2.Increment();
					image_it2.Set((unsigned char)image_it1);
				}
				return ColorAdjust(new_data, high, mid, low, channel, event_handler);
			}
			event_handler->Start();
			G42ImageData8BitIterator image_it(data);
			do
			{
				image_it.Set(Map[(unsigned char)image_it]);
				if (!image_it.Increment())
				{
					if (!image_it.NextRow())
						break;
               row_count++;
            	event_handler->Set(row_count);
				}
			}
			while (1);
         event_handler->Stop();
			return data;
		}
		if (data->GetDepth() == 4)
		{
			if (channel != 0)
			{
				G42ImageData * new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), data->GetHeight(), 4, 16);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
				G42Color * new_palette = new G42Color [16];
				for (uint n = 0; n < 16; n++)
					new_palette[n].blue = new_palette[n].green = new_palette[n].red
						= ((n << 4) + n);
				new_data->SetPalette(new_palette);
				G42ImageData4BitIterator image_it1(data);
				G42ImageData4BitIterator image_it2(new_data);
				do
				{
					image_it2.Set4((unsigned char)image_it1);
					image_it2.Increment();
					if (!image_it1.Increment())
					{
						if (!image_it1.NextRow())
							break;
						image_it2.NextRow();
					}
				} while (1);
				return ColorAdjust(new_data, high, mid, low, channel, event_handler);
			}
			event_handler->Start();
         G42ImageData4BitIterator image_it(data);
         do
         {
            unsigned char val =(unsigned char)image_it;
            val = ((val << 4) + val);
            image_it.Set4((Map[val] >> 4));
            if (!image_it.Increment())
            {
            	if (!image_it.NextRow())
               	break;
               row_count++;
               event_handler->Set(row_count);
            }
         } while (1);
			event_handler->Stop();
			return data;
		}
		if (data->GetDepth() == 1)
		{
			G42ImageData * new_data = new G42ImageData(data->GetData(),
				data->GetWidth(), data->GetHeight(), 1, 2);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42Color * new_palette = new G42Color [16];
			new_palette[0].blue = new_palette[0].green = new_palette[0].red = 0;
			new_palette[1].blue = new_palette[1].green = new_palette[1].red = 255;
			new_data->SetPalette(new_palette);
			G42ImageData1BitIterator image_it1(data);
			G42ImageData1BitIterator image_it2(new_data);
			do
			{
				image_it2.Set(*(image_it1.GetPtr()));
				image_it2.Increment8();
				if (!image_it1.Increment8())
				{
					if (!image_it1.NextRow())
						break;
					image_it2.NextRow();
				}
			} while (1);
			return ColorAdjust(new_data, high, mid, low, channel, event_handler);
		}
	}
	switch (channel)
	{
		case 0:
		{
			G42Color * new_palette = RGBtoHLSPalette(data->GetPalette(),
				data->GetNumPalette());
			for (uint y = 0; y < data->GetNumPalette(); y++)
			{
				new_palette[y].green = SatMap[new_palette[y].green];
				new_palette[y].red = Map[new_palette[y].red];
			}
			G42Color * palette = HLStoRGBPalette(new_palette, data->GetNumPalette());
			delete [] new_palette;
			data->SetPalette(palette);
			delete [] palette;
			break;
		}
		case 1:
		{
			G42Color * new_palette = new G42Color[data->GetNumPalette()];
			G42Color * palette = data->GetPalette();
			for (uint y = 0; y < data->GetNumPalette(); y++)
			{
				new_palette[y].blue = palette[y].blue;
				new_palette[y].green = palette[y].green;
				new_palette[y].red = Map[palette[y].red];
			}
			data->SetPalette(new_palette);
			delete [] new_palette;
			break;
		}
		case 2:
		{
			G42Color * new_palette = new G42Color[data->GetNumPalette()];
			G42Color * palette = data->GetPalette();
			for (uint y = 0; y < data->GetNumPalette(); y++)
			{
				new_palette[y].blue = palette[y].blue;
				new_palette[y].green = Map[palette[y].green];
				new_palette[y].red = palette[y].red;
			}
			data->SetPalette(new_palette);
			delete [] new_palette;
			break;
		}
		case 3:
		{
			G42Color * new_palette = new G42Color[data->GetNumPalette()];
			G42Color * palette = data->GetPalette();
			for (uint y = 0; y < data->GetNumPalette(); y++)
			{
				new_palette[y].blue = Map[palette[y].blue];
				new_palette[y].green = palette[y].green;
				new_palette[y].red = palette[y].red;
			}
			data->SetPalette(new_palette);
			delete [] new_palette;
			break;
		}
	}
   return data;
}
void
tone_control_gamma(unsigned char * Map, int mid, int channel)
{
	double gamma = (10.0 / (double)mid);
	for (int n = 0; n < 256; n++)
	{
		if (Map[n] > 0)
			Map[n] = (unsigned char)(pow((double)((double)(Map[n]) / 255.0),
				gamma) * 255.0 + .5);
	}
}
void
tone_control_contrast(unsigned char * Map, unsigned char * SatMap, int low, int channel)
{
	int32 contrast = low + 100;
	if (low < 0)
	{
		int n;
		for (n = 0; n < 256; n++)
		{
			int32 v = Map[n] - 128;
			v *= contrast;
			if (v > 0)
				v += 50;
			else
				v -= 50;
			v /= 100;
			if (v > 127)
				v = 127;
			if (v < -128)
				v = -128;
			v += 128;
			Map[n] = (unsigned char)v;
		}
		for (n = 0; n < 256; n++)
			SatMap[n] = n + ((n * low)/100);
	}
	else
	{
		if (low == 100)
		{
			int n;
			for (n = 0; n < 128; n++)
				Map[n] = 0;
			for (; n < 256; n++)
				Map[n] = 255;
		}
		else
		{
			int32 cutoff = (low * 128)/100;
			int n;
			for(n = 0; n < cutoff; n++)
				Map[n] = 0;
			for(; n < (255 - cutoff); n++)
				Map[n] = (unsigned char)(((n - cutoff) * 255)/(255-(2*cutoff)));
			for(; n < 256; n++)
				Map[n] = 255;
		}
	}
}
void
G42Image::ColorAdjust(int bright, int gamma, int contrast, int channel)
{
	G42ImageData * new_data = Data;
	if (Data)
   {
   	if (Mask)
      {
      	delete Mask;
      	Mask = 0;
      }
		new_data = ::ColorAdjust(Data, bright, gamma, contrast, channel, GetEventHandler());
	}
	if (new_data && Data != new_data)
	{
		delete Data;
		Data = new_data;
	}
	RebuildInfo();
	for (int i = 0; i < NumViewArray; i++)
	{
		ViewArray[i]->ImageChanged();
	}
	Dirty = true;
}
