#include "g42idata.h"
#include "g42iter.h"
#include "G42event.h"
#include "g42image.h"
#include "g42iview.h"
void RGBtoHLS(G42ImageData *);
void HLStoRGB(G42ImageData *);
G42Color * RGBtoHLSPalette(G42Color *, int);
G42Color * HLStoRGBPalette(G42Color *, int);
G42ImageData *
Emboss(G42ImageData * data, int angle, bool do_color, G42EventUpdateHandler * event_handler)
{
	static int table[19][3] = {{512, 0, 0}, {486, 0, 26},
							{451, 1, 60}, {408, 4, 100},
							{357, 10, 145}, {301, 19, 192},
							{241, 30, 241}, {180, 45, 287},
							{128, 64, 320}, {88, 88, 336},
							{64, 128, 320}, {45, 180, 287},
							{30, 241, 241}, {19, 301, 192},
							{10, 357, 145}, {4, 408, 100},
							{1, 451, 60}, {0, 486, 26},
							{0, 512, 0}};
	int Angle = angle;
	if (Angle > 90)
		Angle = 90;
	if (Angle < 0)
		Angle = 0;
	Angle = (Angle + 3) / 5;
	event_handler->SetDenominator((int32)data->GetHeight());
   int32 row_count = 0L;
	if (!do_color)
	{
		if (data->GetNumPalette() == 0)
		{
			if (data->GetDepth() > 8)
			{
				G42ImageData * new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), data->GetHeight(), 8, 0);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
				G42LockedID24BitIterator image_it1(data);
				G42ImageData8BitIterator image_it2(new_data);
				int32 tval2 = ((image_it1.GetBlue32() * (int32)58)
					+ (image_it1.GetGreen32() * (int32)301)
					+ (image_it1.GetRed32() * (int32)153)) >> 9;
				image_it1.IncRow();
				int32 tval4 = ((image_it1.GetBlue32() * (int32)58)
					+ (image_it1.GetGreen32() * (int32)301)
					+ (image_it1.GetRed32() * (int32)153)) >> 9;
				image_it1.DecRow();
            event_handler->Start();
            event_handler->Set(0L);
            do
            {
            	int32 tval = tval2;
               int32 tval3 = tval4;
               if (!image_it1.Increment())
               {
               	image_it2.Set(0x80);
                  image_it1.NextRow();
                  image_it2.NextRow();
						tval2 = ((image_it1.GetBlue32() * (int32)58)
							+ (image_it1.GetGreen32() * (int32)301)
							+ (image_it1.GetRed32() * (int32)153)) >> 9;
         			if (!image_it1.IncRow())
                  {
                  	while (1)
                     {
                     	image_it2.Set(0x80);
                        if (!image_it2.Increment())
                        	break;
                     }
                     break;
                  }
						tval4 = ((image_it1.GetBlue32() * (int32)58)
							+ (image_it1.GetGreen32() * (int32)301)
							+ (image_it1.GetRed32() * (int32)153)) >> 9;
						image_it1.DecRow();
                  row_count++;
                  event_handler->Set(row_count);
                  continue;
               }
					tval2 = ((image_it1.GetBlue32() * (int32)58)
						+ (image_it1.GetGreen32() * (int32)301)
						+ (image_it1.GetRed32() * (int32)153)) >> 9;
					image_it1.IncRow();
					tval4 = ((image_it1.GetBlue32() * (int32)58)
						+ (image_it1.GetGreen32() * (int32)301)
						+ (image_it1.GetRed32() * (int32)153)) >> 9;
					image_it1.DecRow();
					int grad = 0x80 + (tval - (((int32)(table[Angle][0])
						* tval2 + (int32)(table[Angle][1]) * tval3 +
						(int32)(table[Angle][2]) * tval4) >> 9));
					if (grad > 0xff)
						grad = 0xff;
					if (grad < 0)
						grad = 0;
               image_it2.Set(grad);
               image_it2.Increment();
            } while (1);
            event_handler->Stop();
				return new_data;
			}
			if (data->GetDepth() == 8)
			{
				G42ImageData8BitIterator image_it(data);
				int32 tval2 = (int32)(unsigned char)image_it;
				image_it.IncRow();
				int32 tval4 = (int32)(unsigned char)image_it;
				image_it.DecRow();
         	event_handler->Start();
            event_handler->Set(0L);
				while (1)
				{
					int32 tval = tval2;
					if (!image_it.Increment())
					{
						image_it.Set(0x80);
						if (!image_it.NextRow())
							break;
						tval2 = (int32)(unsigned char)image_it;
						if (!image_it.IncRow())
						{
							image_it.Set(0x80);
							while (image_it.Increment())
								image_it.Set(0x80);
							break;
						}
						tval4 = (int32)(unsigned char)image_it;
						image_it.DecRow();
                  row_count++;
                  event_handler->Set(row_count);
						continue;
					}
					tval2 = (int32)(unsigned char)image_it;
					int32 tval3 = tval4;
					if (!image_it.IncRow())
					{
						break;
					}
					tval4 = (int32)(unsigned char)image_it;
					image_it.DecRow();
					int grad = 0x80 + (tval - (((int32)(table[Angle][0])
						* tval2 + (int32)(table[Angle][1]) * tval3 +
						(int32)(table[Angle][2]) * tval4) >> 9));
					if (grad > 0xff)
						grad = 0xff;
					if (grad < 0)
						grad = 0;
					image_it.Decrement();
					image_it.Set(grad);
					image_it.Increment();
				}
				event_handler->Stop();
            return data;
			}
			if (data->GetDepth() == 4)
			{
				G42ImageData * new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), data->GetHeight(), 8, 256);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
				G42Color * new_palette = new G42Color [256];
				for (int n = 0; n < 256; n++)
					new_palette[n].blue = new_palette[n].green = new_palette[n].red = n;
				new_data->SetPalette(new_palette);
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
	         return Emboss(new_data, angle, false, event_handler);
			}
			if (data->GetDepth() == 1)
			{
				G42ImageData * new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), data->GetHeight(), 8, 0);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
				G42LockedID1BitIterator image_it1(data);
				G42ImageData8BitIterator image_it2(new_data);
				do
				{
					image_it2.Set((unsigned char)image_it1);
					image_it2.Increment();
					if (!image_it1.Increment())
					{
						if (!image_it1.NextRow())
							break;
						image_it2.NextRow();
					}
				} while (1);
				return Emboss(new_data, angle, false, event_handler);
			}
		}
		else	// Palette in use
		{
			if (data->GetDepth() == 8)
			{
				G42ImageData * new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), data->GetHeight(), 8, 0);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
				G42Color * palette = data->GetPalette();
				G42LockedID8BitIterator image_it1(data);
				G42ImageData8BitIterator image_it2(new_data);
				do
				{
					int32 red = (int32)(palette[(unsigned char)image_it1].red);
					int32 green = (int32)(palette[(unsigned char)image_it1].green);
					int32 blue = (int32)(palette[(unsigned char)image_it1].blue);
					int val = (int)((((int32)red * (int32)306) +
						((int32)green * (int32)601) +
						((int32)blue * (int32)117) + (int32)512) >> 10);
					image_it2.Set(val);
					image_it2.Increment();
					if (!image_it1.Increment())
					{
						if (!image_it1.NextRow())
							break;
						image_it2.NextRow();
					}
				} while (1);
				return Emboss(new_data, angle, false, event_handler);
			}
			if (data->GetDepth() == 4)
			{
				G42ImageData * new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), data->GetHeight(), 8, 0);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
				G42Color * palette = data->GetPalette();
				G42LockedID4BitIterator image_it1(data);
				G42ImageData8BitIterator image_it2(new_data);
				do
				{
					unsigned char result = (unsigned char)image_it1;
					int tval = (int)((((int32)palette[result].blue * (int32)117)
						+ ((int32)palette[result].green * (int32)601)
						+ ((int32)palette[result].red * (int32)306)
						+ (int32)512) >> 10);
					image_it2.Set(tval);
					image_it2.Increment();
					if (!image_it1.Increment())
					{
						if (!image_it1.NextRow())
							break;
						image_it2.NextRow();
					}
				} while (1);
				return Emboss(new_data, angle, false, event_handler);
			}
			if (data->GetDepth() == 1)
			{
				G42ImageData * new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), data->GetHeight(), 8, 0);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
				G42Color * palette = data->GetPalette();
				G42LockedID1BitIterator image_it1(data);
				G42ImageData8BitIterator image_it2(new_data);
				do
				{
					unsigned char result = 0;
					if ((unsigned char)image_it1)
						result = 1;
					int tval = (int)((((int32)palette[result].blue * (int32)117)
						+ ((int32)palette[result].green * (int32)601)
						+ ((int32)palette[result].red * (int32)306)
						+ (int32)512) >> 10);
					image_it2.Set(tval);
					image_it2.Increment();
					if (!image_it1.Increment())
					{
						if (!image_it1.NextRow())
							break;
						image_it2.NextRow();
					}
				} while (1);
				return Emboss(new_data, angle, false, event_handler);
         }
		}
	}
#if 0
   else
   {
   	switch (data->GetDepth())
      {
      	case 24:
         {
				RGBtoHLS(data);
				G42ImageData24BitIterator image_it(data);
				int32 tval2 = image_it.GetRed();
				image_it.IncRow();
				int32 tval4 = image_it.GetRed();
				image_it.DecRow();
            do
            {
            	int32 tval = tval2;
               int32 tval3 = tval4;
               if (!image_it.Increment())
               {
               	image_it.Set(image_it.GetBlue(), 0, 0xff);
                  image_it.NextRow();
						tval2 = image_it.GetRed();
         			if (!image_it.IncRow())
                  {
                  	while (1)
                     {
                     	image_it.Set(image_it.GetBlue(), 0,
                        	0xff);
                        if (!image_it.Increment())
                        	break;
                     }
                     break;
                  }
						tval4 = image_it.GetRed();
						image_it.DecRow();
                  continue;
               }
					tval2 = image_it.GetRed();
					image_it.IncRow();
					tval4 = image_it.GetRed();
					image_it.DecRow();
					int grad = (tval - (((int32)(table[Angle][0])
						* tval2 + (int32)(table[Angle][1]) * tval3 +
						(int32)(table[Angle][2]) * tval4) >> 9));
               if (grad < 0)
               	grad = -grad;
               grad *= 3;
					if (grad > 0xff)
						grad = 0xff;
					grad = 255 - grad;
               image_it.Set(image_it.GetBlue(), image_it.GetGreen(), grad);
            } while (1);
				HLStoRGB(data);
				return data;
         }
         case 8:
         {
         	return data;
         }
         case 4:
         {
         	return data;
         }
         case 1:
         {
         	return data;
         }
      }
   }
#endif
  	return data;
}
void
G42Image::Emboss(int angle)
{
	G42ImageData * new_data = Data;
	if (Data)
   {
   	if (Mask)
      {
      	delete Mask;
         Mask = 0;
      }
		new_data = ::Emboss(Data, angle, false, GetEventHandler());
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
