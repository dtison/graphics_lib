#include "g42idata.h"
#include "g42iter.h"
#include "g42image.h"
#include "g42iview.h"
G42ImageData *
ColorMask(G42ImageData * data, byte * mask, int width, int height, G42Color color,
	int x, int y)
{
// this really should go here, but since this function is reentrient, it
// can't be clipped until you are ready to put the image down
#if 0
	byte * mp = mask;
	int m = 0x80;
	int mrb = ((width + 7) >> 3);
	if (y < 0)
	{
		mp += mrb * (-y);
		height += y;
		y = 0;
	}
	if (y + height > data->GetHeight())
	{
		height = data->GetHeight() - y;
	}
	if (x < 0)
	{
		mp += ((-x) >> 3);
		m = (1 << (7 - ((-x) & 7)));
		width += x;
		x = 0;
	}
	if (x + width > data->GetWidth())
	{
		width = data->GetWidth() - x;
	}
	if (width <= 0 || height <= 0)
		return data;
#endif
	switch (data->GetDepth())
	{
		case 32:
		case 24:
		{
			byte * mp = mask;
			int m = 0x80;
			int mrb = ((width + 7) >> 3);
			if (y < 0)
			{
				mp += mrb * (-y);
				height += y;
				y = 0;
			}
			if (y + height > data->GetHeight())
			{
				height = data->GetHeight() - y;
			}
			if (x < 0)
			{
				mp += ((-x) >> 3);
				m = (1 << (7 - ((-x) & 7)));
				width += x;
				x = 0;
			}
			if (x + width > data->GetWidth())
			{
				width = data->GetWidth() - x;
			}
			if (width <= 0 || height <= 0)
				return data;
			G42ImageData24BitIterator image_it(data, false);
			image_it.SetArea(x, y, x + width - 1, y + height - 1);
			byte * ptr = mp;
			int ptr_mask = m;
			do
			{
				if ((*ptr) & ptr_mask)
					image_it.Set(color.blue, color.green, color.red);
            if (ptr_mask > 0x01)
            	ptr_mask >>= 1;
				else
            {
            	ptr_mask = 0x80;
               ptr++;
				}
            if (!image_it.Increment())
            {
            	if (!image_it.NextRow())
						break;
					mp += mrb;
					ptr = mp;
					ptr_mask = m;
				}
         } while (1);
         return data;
      }
      case 8:
      {
      	if (data->GetNumPalette() != 0)
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
            do
            {
					unsigned char val = (unsigned char)image_it1;
					image_it2.Set(palette[val].blue, palette[val].green,
               	palette[val].red);
					image_it2.Increment();
               if (!image_it1.Increment())
               {
               	if (!image_it1.NextRow())
                  	break;
                  image_it2.NextRow();
               }
            } while (1);
            return ColorMask(new_data, mask, width, height, color, x, y);
         }
         else
			{
	      	G42ImageData * new_data = new G42ImageData(data->GetData(),
   	      	data->GetWidth(), data->GetHeight(), 8*TrueColorPixelSize, 0);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
	       	G42LockedID8BitIterator image_it1(data);
	       	G42ImageData24BitIterator image_it2(new_data);
            do
				{
					unsigned char val = (unsigned char)image_it1;
					image_it2.Set(val, val, val);
               image_it2.Increment();
               if (!image_it1.Increment())
               {
               	if (!image_it1.NextRow())
                  	break;
                  image_it2.NextRow();
               }
            } while (1);
				return ColorMask(new_data, mask, width, height, color, x, y);
         }
      }
      case 4:
      {
      	if (data->GetNumPalette() != 0)
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
					unsigned char val = (unsigned char)image_it1;
					image_it2.Set(palette[val].blue, palette[val].green,
               	palette[val].red);
               image_it2.Increment();
					if (!image_it1.Increment())
               {
               	if (!image_it1.NextRow())
                  	break;
                  image_it2.NextRow();
               }
            } while (1);
            return ColorMask(new_data, mask, width, height, color, x, y);
         }
         else
         {
				G42ImageData * new_data = new G42ImageData(data->GetData(),
   	      	data->GetWidth(), data->GetHeight(), 8*TrueColorPixelSize, 0);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
	       	G42LockedID4BitIterator image_it1(data);
	       	G42ImageData24BitIterator image_it2(new_data);
            do
            {
					unsigned char val = (unsigned char)image_it1;
               val = ((val << 4) + val);
					image_it2.Set(val, val, val);
               image_it2.Increment();
               if (!image_it1.Increment())
               {
               	if (!image_it1.NextRow())
                  	break;
                  image_it2.NextRow();
               }
            } while (1);
				return ColorMask(new_data, mask, width, height, color, x, y);
         }
      }
      case 1:
		{
      	if (data->GetNumPalette() != 0)
         {
	      	G42ImageData * new_data = new G42ImageData(data->GetData(),
   	      	data->GetWidth(), data->GetHeight(), 8*TrueColorPixelSize, 0);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
            G42Color * palette = data->GetPalette();
	       	G42LockedID1BitIterator image_it1(data);
	       	G42ImageData24BitIterator image_it2(new_data);
            do
            {
					if ((unsigned char)image_it1)
						image_it2.Set(palette[1].blue, palette[1].green,
   	            	palette[1].red);
               else
						image_it2.Set(palette[0].blue, palette[0].green,
   	            	palette[0].red);
               image_it2.Increment();
               if (!image_it1.Increment())
               {
               	if (!image_it1.NextRow())
                  	break;
                  image_it2.NextRow();
               }
            } while (1);
            return ColorMask(new_data, mask, width, height, color, x, y);
			}
         else
         {
	      	G42ImageData * new_data = new G42ImageData(data->GetData(),
   	      	data->GetWidth(), data->GetHeight(), 8*TrueColorPixelSize, 0);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
	       	G42LockedID1BitIterator image_it1(data);
	       	G42ImageData24BitIterator image_it2(new_data);
            do
            {
					unsigned char val = (unsigned char)image_it1;
					image_it2.Set(val, val, val);
               image_it2.Increment();
               if (!image_it1.Increment())
               {
               	if (!image_it1.NextRow())
                  	break;
                  image_it2.NextRow();
               }
				} while (1);
            return ColorMask(new_data, mask, width, height, color, x, y);
         }
      }
   }
	return data;
}
G42ImageData *
ColorMask8(G42ImageData * data, byte * mask, int width, int height, G42Color color,
	int x, int y)
{
	switch (data->GetDepth())
	{
		case 32:
		case 24:
		{
			byte * mp = mask;
			int mrb = width;
			if (y < 0)
			{
				mp += mrb * (-y);
				height += y;
				y = 0;
			}
			if (y + height > data->GetHeight())
			{
				height = data->GetHeight() - y;
			}
			if (x < 0)
			{
				mp += -x;
				width += x;
				x = 0;
			}
			if (x + width > data->GetWidth())
			{
				width = data->GetWidth() - x;
			}
			if (width <= 0 || height <= 0)
				return data;
				
			G42ImageData24BitIterator image_it(data, false);
			image_it.SetArea(x, y, x + width - 1, y + height - 1);
			byte * ptr = mp;
			do
			{
				int32 blue = image_it.GetBlue32();
				int32 green = image_it.GetGreen32();
				int32 red = image_it.GetRed32();
            blue = (((int32)(*ptr) * ((int32)color.blue - blue))/255L) + blue;
            if (blue < 0L)
            	blue = 0L;
            if (blue > 255L)
            	blue = 255L;
            green = (((int32)(*ptr) * ((int32)color.green - green))/255L) + green;
            if (green < 0L)
            	green = 0L;
            if (green > 255L)
            	green = 255L;
            red = (((int32)(*ptr) * ((int32)color.red - red))/255L) + red;
            if (red < 0L)
            	red = 0L;
            if (red > 255L)
            	red = 255L;
         	image_it.Set(blue, green, red);
            ptr++;
            if (!image_it.Increment())
            {
            	if (!image_it.NextRow())
						break;
					mp += mrb;
					ptr = mp;
            }
         } while (1);
         return data;
      }
      case 8:
      {
      	if (data->GetNumPalette() != 0)
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
            do
            {
					unsigned char val = (unsigned char)image_it1;
					image_it2.Set(palette[val].blue, palette[val].green,
               	palette[val].red);
               image_it2.Increment();
               if (!image_it1.Increment())
               {
               	if (!image_it1.NextRow())
                  	break;
                  image_it2.NextRow();
               }
            } while (1);
            return ColorMask8(new_data, mask, width, height, color, x, y);
         }
         else
         {
	      	G42ImageData * new_data = new G42ImageData(data->GetData(),
   	      	data->GetWidth(), data->GetHeight(), 8*TrueColorPixelSize, 0);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
	       	G42LockedID8BitIterator image_it1(data);
	       	G42ImageData24BitIterator image_it2(new_data);
            do
            {
					unsigned char val = (unsigned char)image_it1;
					image_it2.Set(val, val, val);
               image_it2.Increment();
               if (!image_it1.Increment())
               {
               	if (!image_it1.NextRow())
                  	break;
                  image_it2.NextRow();
               }
            } while (1);
            return ColorMask8(new_data, mask, width, height, color, x, y);
         }
      }
      case 4:
      {
      	if (data->GetNumPalette() != 0)
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
					unsigned char val = (unsigned char)image_it1;
					image_it2.Set(palette[val].blue, palette[val].green,
               	palette[val].red);
               image_it2.Increment();
               if (!image_it1.Increment())
               {
               	if (!image_it1.NextRow())
                  	break;
                  image_it2.NextRow();
               }
            } while (1);
            return ColorMask8(new_data, mask, width, height, color, x, y);
         }
         else
         {
	      	G42ImageData * new_data = new G42ImageData(data->GetData(),
   	      	data->GetWidth(), data->GetHeight(), 8*TrueColorPixelSize, 0);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
	       	G42LockedID4BitIterator image_it1(data);
	       	G42ImageData24BitIterator image_it2(new_data);
            do
            {
					unsigned char val = (unsigned char)image_it1;
               val = ((val << 4) + val);
					image_it2.Set(val, val, val);
               image_it2.Increment();
               if (!image_it1.Increment())
               {
               	if (!image_it1.NextRow())
                  	break;
                  image_it2.NextRow();
               }
            } while (1);
            return ColorMask8(new_data, mask, width, height, color, x, y);
         }
      }
      case 1:
		{
      	if (data->GetNumPalette() != 0)
         {
	      	G42ImageData * new_data = new G42ImageData(data->GetData(),
   	      	data->GetWidth(), data->GetHeight(), 8*TrueColorPixelSize, 0);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
            G42Color * palette = data->GetPalette();
	       	G42LockedID1BitIterator image_it1(data);
	       	G42ImageData24BitIterator image_it2(new_data);
            do
            {
					if ((unsigned char)image_it1)
						image_it2.Set(palette[1].blue, palette[1].green,
   	            	palette[1].red);
               else
						image_it2.Set(palette[0].blue, palette[0].green,
   	            	palette[0].red);
               image_it2.Increment();
               if (!image_it1.Increment())
               {
               	if (!image_it1.NextRow())
                  	break;
                  image_it2.NextRow();
               }
            } while (1);
            return ColorMask8(new_data, mask, width, height, color, x, y);
         }
         else
         {
	      	G42ImageData * new_data = new G42ImageData(data->GetData(),
   	      	data->GetWidth(), data->GetHeight(), 8*TrueColorPixelSize, 0);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
	       	G42LockedID1BitIterator image_it1(data);
	       	G42ImageData24BitIterator image_it2(new_data);
            do
            {
					unsigned char val = (unsigned char)image_it1;
					image_it2.Set(val, val, val);
               image_it2.Increment();
               if (!image_it1.Increment())
               {
               	if (!image_it1.NextRow())
                  	break;
                  image_it2.NextRow();
               }
            } while (1);
            return ColorMask8(new_data, mask, width, height, color, x, y);
         }
      }
   }
	return data;
}
G42ImageData *
TransColorMask(G42ImageData * data, byte * mask, int width, int height,
	int x, int y)
{
	switch (data->GetDepth())
   {
      case 1:
		{
       	G42ImageData1BitIterator image_it(data, false);
         image_it.SetArea(x, y, x + width - 1, y + height - 1);
			byte * ptr = mask;
         unsigned char ptr_mask = 0x80;
         do
         {
				if (*ptr & ptr_mask)
	         	image_it.Set8(0);
            if (ptr_mask > 0x01)
            	ptr_mask >>= 1;
            else
            {
            	ptr_mask = 0x80;
               ptr++;
				}
            if (!image_it.Increment())
            {
            	if (!image_it.NextRow())
               	break;
            }
         } while (1);
         return data;
      }
   }
	return data;
}
void
G42Image::DrawColorMask(byte * mask, int width, int height, G42Color color,
		int x, int y)
{
	G42ImageData * new_data = Data;
	G42ImageData * new_mask = Mask;
	if (Data)
   {
		new_data = ::ColorMask(Data, mask, width, height, color, x, y);
      if (Mask)
			new_mask = ::TransColorMask(Mask, mask, width, height, x, y);
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
void
G42Image::DrawColorMask8(byte * mask, int width, int height, G42Color color,
		int x, int y)
{
	G42ImageData * new_data = Data;
	G42ImageData * new_mask = Mask;
	if (Data)
   {
		new_data = ::ColorMask8(Data, mask, width, height, color, x, y);
      if (Mask)
			new_mask = ::TransColorMask(Mask, mask, width, height, x, y);
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
