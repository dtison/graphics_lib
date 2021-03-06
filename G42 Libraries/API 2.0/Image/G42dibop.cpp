#include "g42idata.h"
#include "g42iter.h"
void
FillDIBFromImage(byte G42HUGE * dib, G42ImageData * data)
{
   byte G42HUGE * ptr = dib;
	switch (data->GetDepth())
   {
		case 32:
   	case 24:
      {
      	G42LockedID24BitIterator image_it(data);
         image_it.SetLastRow();
         do
         {
				*ptr = image_it.GetBlue();
            ptr++;
				*ptr = image_it.GetGreen();
            ptr++;
				*ptr = image_it.GetRed();
            ptr++;
            if (!image_it.Increment())
            {
            	image_it.ResetRow();
            	if (!image_it.DecRow())
               	break;
            	if (((ptr - dib) % 4) != 0)
               	ptr += (4 - ((ptr - dib) % 4));
            }
	      } while (1);
         break;
      }
      case 8:
      {
      	G42LockedID8BitIterator image_it(data);
         image_it.SetLastRow();
        	do
         {
            *ptr = (unsigned char)image_it;
            ptr++;
	         if (!image_it.Increment())
            {
            	image_it.ResetRow();
     	      	if (!image_it.DecRow())
        	      	break;
           		if (((ptr - dib) % 4) != 0)
              		ptr += (4 - ((ptr - dib) % 4));
	         }
         } while (1);
         break;
      }
      case 4:
      {
      	G42LockedID4BitIterator image_it(data);
         image_it.SetLastRow();
        	do
         {
            *ptr = *(image_it.GetPtr());
            ptr++;
            image_it.Increment();
	         if (!image_it.Increment())
            {
            	image_it.ResetRow();
     	      	if (!image_it.DecRow())
        	      	break;
           		if (((ptr - dib) % 4) != 0)
              		ptr += (4 - ((ptr - dib) % 4));
	         }
         } while (1);
         break;
      }
      case 1:
      {
      	G42LockedID1BitIterator image_it(data);
         image_it.SetLastRow();
        	do
         {
            *ptr = *(image_it.GetPtr());
            ptr++;
            if (!image_it.Increment8())
  	         {
            	image_it.ResetRow();
     	      	if (!image_it.DecRow())
        	      	break;
           		if (((ptr - dib) % 4) != 0)
              		ptr += (4 - ((ptr - dib) % 4));
            }
         } while (1);
         break;
      }
   }
   return ;
}
void
FillImageFromDIB(byte G42HUGE * dib, G42ImageData * data)
{
	byte G42HUGE * ptr = dib;
   switch (data->GetDepth())
   {
   	case 32:
   	case 24:
      {
      	G42ImageData24BitIterator image_it(data);
         image_it.SetLastRow();
         do
         {
         	unsigned char blue = *ptr;
            ptr++;
         	unsigned char green = *ptr;
            ptr++;
         	unsigned char red = *ptr;
            ptr++;
            image_it.Set(blue, green, red);
            if (!image_it.Increment())
            {
            	image_it.ResetRow();
            	if (!image_it.DecRow())
               	break;
            	if (((ptr - dib) % 4) != 0)
               	ptr += (4 - ((ptr - dib) % 4));
            }
	      } while (1);
         break;
      }
      case 8:
      {
      	G42ImageData8BitIterator image_it(data);
         image_it.SetLastRow();
         do
         {
            image_it.Set(*ptr);
            ptr++;
            if (!image_it.Increment())
            {
            	image_it.ResetRow();
            	if (!image_it.DecRow())
               	break;
            	if (((ptr - dib) % 4) != 0)
               	ptr += (4 - ((ptr - dib) % 4));
            }
	      } while (1);
         break;
      }
      case 4:
      {
      	G42ImageData4BitIterator image_it(data);
         image_it.SetLastRow();
         do
         {
            image_it.Set(*ptr);
            ptr++;
            image_it.Increment();
            if (!image_it.Increment())
            {
            	image_it.ResetRow();
            	if (!image_it.DecRow())
               	break;
            	if (((ptr - dib) % 4) != 0)
               	ptr += (4 - ((ptr - dib) % 4));
            }
	      } while (1);
         break;
      }
      case 1:
      {
      	G42ImageData1BitIterator image_it(data);
         image_it.SetLastRow();
         do
         {
            image_it.Set(*ptr);
            ptr++;
            if (!image_it.Increment8())
            {
            	image_it.ResetRow();
            	if (!image_it.DecRow())
               	break;
            	if (((ptr - dib) % 4) != 0)
               	ptr += (4 - ((ptr - dib) % 4));
            }
	      } while (1);
         break;
      }
   }
   return;
}
