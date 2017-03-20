#include "g42idata.h"
#include "g42iter.h"
#include "g42image.h"
#include "g42iview.h"
G42Color
GetColorAtPoint(G42ImageData * data, uint32 x, uint32 y)
{
	switch(data->GetDepth())
	{
		case 32:
    	case 24:
		{
			G42LockedID24BitIterator image_it1(data);
			int n = 0;
			while (n < y)
			{
				image_it1.NextRow();
				n++;
			}
			n = 0;
			while (n < x)
			{
				image_it1.Increment();
				n++;
			}
			return G42Color(image_it1.GetRed(), image_it1.GetGreen(),
				image_it1.GetBlue());
			break;
		}
		case 8:
		{
			G42LockedID8BitIterator image_it1(data);
			int n = 0;
			while (n < y)
			{
				image_it1.NextRow();
				n++;
			}
			n = 0;
			while (n < x)
			{
				image_it1.Increment();
				n++;
			}
			byte val_ref = (byte)image_it1;
			if (data->GetNumPalette())
				return data->GetPalette()[val_ref];
			else
				return G42Color(val_ref, val_ref, val_ref);
			break;
		}
		case 4:
		{
			G42LockedID4BitIterator image_it1(data);
			int n = 0;
			while (n < y)
			{
				image_it1.NextRow();
				n++;
			}
			n = 0;
			while (n < x)
			{
				image_it1.Increment();
				n++;
			}
			byte val_ref = (byte)image_it1;
			if (data->GetNumPalette())
			{
				return data->GetPalette()[val_ref];
			}
			else
			{
				val_ref |= (byte)(val_ref << 4);
				return G42Color(val_ref, val_ref, val_ref);
			}
			break;
		}
		case 1:
		{
			G42LockedID1BitIterator image_it1(data);
			int n = 0;
			while (n < y)
			{
				image_it1.NextRow();
				n++;
			}
			n = 0;
			while (n < x)
			{
				image_it1.Increment();
				n++;
			}
			byte val_ref = (byte)image_it1 / 255;
			if (data->GetNumPalette())
			{
				return data->GetPalette()[val_ref];
			}
			else
			{
				if (val_ref)
					val_ref = 0xff;
				return G42Color(val_ref, val_ref, val_ref);
			}
			break;
		}
	}
	return G42Color(0, 0, 0);
}
void
CreateTransMask(G42ImageData * data, G42ImageData * mask, G42Color color)
{
	switch(data->GetDepth())
	{
		case 32:
		case 24:
		{
			G42LockedID24BitIterator image_it1(data);
			G42ImageData1BitIterator image_it2(mask);
         do
         {
				if ((image_it1.GetRed() == color.red)
					&& (image_it1.GetGreen() == color.green)
					&& (image_it1.GetBlue() == color.blue))
					image_it2.Set8(1);
				else
					image_it2.Set8(0);
				image_it2.Increment();
				if (!image_it1.Increment())
				{
					if (!image_it1.NextRow())
						break;
					image_it2.NextRow();
				}
			} while (1);
			break;
		}
		case 8:
		{
         G42LockedID8BitIterator image_it1(data);
			G42ImageData1BitIterator image_it2(mask);
			byte num_val_ref = 0;
			byte val_ref[256];
			if (data->GetNumPalette())
			{
				int i = data->GetNumPalette() - 1;
				for (; i >= 0; i--)
				{
					if (data->GetPalette()[i].red == color.red &&
						data->GetPalette()[i].green == color.green &&
						data->GetPalette()[i].blue == color.blue)
					{
						val_ref[num_val_ref++] = i;
					}
				}
				if (!num_val_ref)
					return;
			}
			else
			{
				if (color.red != color.green || color.red != color.blue)
					return;
				val_ref[num_val_ref++] = color.red;
			}
			do
			{
				byte p = (unsigned char)image_it1;
				bool set = false;
				for (int i = 0; i < num_val_ref; i++)
				{
					if (p == val_ref[i])
					{
						set = true;
						image_it2.Set8(1);
						break;
					}
				}
				if (!set)
					image_it2.Set8(0);
				image_it2.Increment();
				if (!image_it1.Increment())
				{
					if (!image_it1.NextRow())
						break;
					image_it2.NextRow();
				}
			} while (1);
			break;
		}
      case 4:
      {
         G42LockedID4BitIterator image_it1(data);
         G42ImageData1BitIterator image_it2(mask);
			byte num_val_ref = 0;
			byte val_ref[256];
			if (data->GetNumPalette())
			{
				int i = data->GetNumPalette() - 1;
				for (; i >= 0; i--)
				{
					if (data->GetPalette()[i].red == color.red &&
						data->GetPalette()[i].green == color.green &&
						data->GetPalette()[i].blue == color.blue)
					{
						val_ref[num_val_ref++] = i;
					}
				}
				if (!num_val_ref)
					return;
			}
			else
			{
				// make sure it's gray
				if (color.red != color.green || color.red != color.blue)
					return;
				int ref = (color.red >> 4);
				// verify the gray is one of the 16 gray palette
				if ((ref | (ref << 4)) != color.red)
					return;
				val_ref[num_val_ref++] = ref;
			}
			do
			{
				byte p = (unsigned char)image_it1;
				bool set = false;
				for (int i = 0; i < num_val_ref; i++)
				{
					if (p == val_ref[i])
					{
						set = true;
						image_it2.Set8(1);
						break;
					}
				}
				if (!set)
					image_it2.Set8(0);
				image_it2.Increment();
            if (!image_it1.Increment())
         	{
            	if (!image_it1.NextRow())
               	break;
               image_it2.NextRow();
            }
         } while (1);
			break;
      }
      case 1:
      {
         G42LockedID1BitIterator image_it1(data);
         G42ImageData1BitIterator image_it2(mask);
			byte val_ref = 0;
			if (data->GetNumPalette())
			{
				int i = 0;
				for (; i < data->GetNumPalette(); i++)
				{
					if (data->GetPalette()[i].red == color.red &&
						data->GetPalette()[i].green == color.green &&
						data->GetPalette()[i].blue == color.blue)
					{
						val_ref = i;
						break;
					}
				}
				if (i >= data->GetNumPalette())
					return;
				val_ref *= 255;
			}
			else
			{
				// make sure it's gray
				if (color.red != color.green || color.red != color.blue)
					return;
				if (color.red == 0xff)
					val_ref = 0xff;
				else if (!color.red)
					val_ref = 0;
				else
					return;
			}
         do
         {
				if ((unsigned char)image_it1 == val_ref)
               image_it2.Set8(1);
				else
            	image_it2.Set8(0);
				image_it2.Increment();
            if (!image_it1.Increment())
         	{
            	if (!image_it1.NextRow())
               	break;
               image_it2.NextRow();
            }
         } while (1);
			break;
      }
   }
}
#if 0
// original
void
CreateTransMask(G42ImageData * data, G42ImageData * mask, int32 x, int32 y)
{
	switch(data->GetDepth())
   {
		case 32:
    	case 24:
      {
         G42LockedID24BitIterator image_it1(data);
         G42ImageData1BitIterator image_it2(mask);
 			int n = 0;
         while (n < y)
         {
         	image_it1.NextRow();
            n++;
         }
         n = 0;
         while (n < x)
         {
         	image_it1.Increment();
            n++;
         }
         unsigned char red_ref = image_it1.GetRed();
         unsigned char green_ref = image_it1.GetGreen();
         unsigned char blue_ref = image_it1.GetBlue();
         image_it1.Reset();
         do
         {
				if ((image_it1.GetRed() == red_ref)
            	&& (image_it1.GetGreen() == green_ref)
            	&& (image_it1.GetBlue() == blue_ref))
               image_it2.Set8(1);
            else
            	image_it2.Set8(0);
				image_it2.Increment();
            if (!image_it1.Increment())
         	{
            	if (!image_it1.NextRow())
               	break;
               image_it2.NextRow();
            }
         } while (1);
			break;
      }
      case 8:
      {
         G42LockedID8BitIterator image_it1(data);
         G42ImageData1BitIterator image_it2(mask);
 			int n = 0;
         while (n < y)
         {
         	image_it1.NextRow();
            n++;
         }
         n = 0;
         while (n < x)
         {
         	image_it1.Increment();
            n++;
         }
         unsigned char val_ref = (unsigned char)image_it1;
         image_it1.Reset();
         do
         {
				if ((unsigned char)image_it1 == val_ref)
               image_it2.Set8(1);
            else
            	image_it2.Set8(0);
				image_it2.Increment();
            if (!image_it1.Increment())
         	{
            	if (!image_it1.NextRow())
               	break;
               image_it2.NextRow();
            }
         } while (1);
			break;
      }
      case 4:
      {
         G42LockedID4BitIterator image_it1(data);
         G42ImageData1BitIterator image_it2(mask);
 			int n = 0;
         while (n < y)
         {
         	image_it1.NextRow();
            n++;
         }
         n = 0;
         while (n < x)
         {
         	image_it1.Increment();
            n++;
         }
         unsigned char val_ref = (unsigned char)image_it1;
         image_it1.Reset();
         do
         {
				if ((unsigned char)image_it1 == val_ref)
               image_it2.Set8(1);
            else
            	image_it2.Set8(0);
				image_it2.Increment();
            if (!image_it1.Increment())
         	{
            	if (!image_it1.NextRow())
               	break;
               image_it2.NextRow();
            }
         } while (1);
			break;
      }
      case 1:
      {
         G42LockedID1BitIterator image_it1(data);
         G42ImageData1BitIterator image_it2(mask);
 			int n = 0;
         while (n < y)
         {
         	image_it1.NextRow();
            n++;
         }
         n = 0;
         while (n < x)
         {
         	image_it1.Increment();
            n++;
         }
         unsigned char val_ref = (unsigned char)image_it1;
         image_it1.Reset();
         do
         {
				if ((unsigned char)image_it1 == val_ref)
               image_it2.Set8(1);
            else
            	image_it2.Set8(0);
				image_it2.Increment();
            if (!image_it1.Increment())
         	{
            	if (!image_it1.NextRow())
               	break;
               image_it2.NextRow();
            }
         } while (1);
			break;
      }
   }
}
#endif // original
void
G42Image::SetTransparency(G42Color color)
{
	if (!Mask)
	{
		Mask = new G42ImageData(DataCore, Info->Width, Info->Height,
			1, 0);
		if (!Mask)
		{
			Valid = false;
			return;
		}
		Mask->SetFinished(true);
		Mask->SetValidStartRow(0);
		Mask->SetValidNumRows(Info->Height);
	}
	if (Mask)
	{
		::CreateTransMask(Data, Mask, color);
		if (Info)
		{
			Info->IsTransparent = true;
			Info->TransparentColor = color;
		}
	}
	Dirty = true;
}
void
G42Image::ClearTransparency(void)
{
	if (Mask)
	{
		delete Mask;
		Mask = 0;
		if (Info)
			Info->IsTransparent = false;
		Dirty = true;
	}
}
G42Color
G42Image::GetColorAtPoint(uint32 x, uint32 y)
{
	return ::GetColorAtPoint(Data, x, y);
}
