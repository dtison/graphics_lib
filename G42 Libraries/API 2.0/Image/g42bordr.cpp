#include "g42idata.h"
#include "g42iter.h"
#include "G42event.h"
#include "g42image.h"
#include "g42iview.h"
G42ImageData * ButtonizeNoColor(G42ImageData *, int, bool, G42EventUpdateHandler *);
G42ImageData * Buttonize(G42ImageData *, G42Color, G42Color, int ,bool, bool,
	G42EventUpdateHandler *);
G42ImageData *
AddBorder(G42ImageData * data, G42Color border_color, int size,
	G42EventUpdateHandler * event_handler)
{
	event_handler->SetDenominator((int32)(data->GetHeight() + (size * 2)));
	int32 row_count = 0L;
   event_handler->Start();
   event_handler->Set(0L);
	switch (data->GetDepth())
   {
		case 32:
   	case 24:
      {
      	G42ImageData * new_data = new G42ImageData(data->GetData(),
         	data->GetWidth() + (size * 2), data->GetHeight() + (size * 2),
            8*TrueColorPixelSize, 0);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42LockedID24BitIterator image_it1(data);
			G42ImageData24BitIterator image_it2(new_data);
         for (int n = 0; n < size; n++)
         {
         	do
            {
            	image_it2.Set(border_color.blue, border_color.green,
               	border_color.red);
            } while (image_it2.Increment());
            image_it2.NextRow();
            row_count++;
            event_handler->Set(row_count);
         }
			for (int n = 0; n < size; n++)
         {
           	image_it2.Set(border_color.blue, border_color.green,
              	border_color.red);
           	image_it2.Increment();
         }
         do
         {
            image_it2.Set(image_it1.GetBlue(), image_it1.GetGreen(),
            	image_it1.GetRed());
            image_it2.Increment();
            if (!image_it1.Increment())
            {
					for (int n = 0; n < size; n++)
   	         {
      	      	image_it2.Set(border_color.blue, border_color.green,
         	      	border_color.red);
            		image_it2.Increment();
	            }
               if (!image_it1.NextRow())
               	break;
               image_it2.NextRow();
               row_count++;
               event_handler->Set(row_count);
					for (int n = 0; n < size; n++)
   		      {
      		     	image_it2.Set(border_color.blue, border_color.green,
         		     	border_color.red);
           			image_it2.Increment();
		         }
            }
         } while (1);
         image_it2.NextRow();
         row_count++;
         event_handler->Set(row_count);
         for (int n = 0; n < size; n++)
         {
         	do
            {
            	image_it2.Set(border_color.blue, border_color.green,
               	border_color.red);
            } while (image_it2.Increment());
            image_it2.NextRow();
            row_count++;
            event_handler->Set(row_count);
         }
         event_handler->Stop();
		   return new_data;
      }
      case 8:
      {
      	G42ImageData * new_data = new G42ImageData(data->GetData(),
         	data->GetWidth() + (size * 2), data->GetHeight() + (size * 2),
            8*TrueColorPixelSize, 0);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42LockedID8BitIterator image_it1(data);
			G42ImageData24BitIterator image_it2(new_data);
			if (data->GetNumPalette() != 0)
         {
         	G42Color * palette = data->GetPalette();
            
	         for (int n = 0; n < size; n++)
   	      {
      	   	do
         	   {
            		image_it2.Set(border_color.blue, border_color.green,
               		border_color.red);
	            } while (image_it2.Increment());
   	         image_it2.NextRow();
               row_count++;
					event_handler->Set(row_count);
      	   }
				for (int n = 0; n < size; n++)
   	      {
      	     	image_it2.Set(border_color.blue, border_color.green,
         	     	border_color.red);
           		image_it2.Increment();
	         }
   	      do
      	   {
            	unsigned char val = (unsigned char)image_it1;
         	   image_it2.Set(palette[val].blue, palette[val].green,
            		palette[val].red);
	            image_it2.Increment();
   	         if (!image_it1.Increment())
      	      {
						for (int n = 0; n < size; n++)
   	      	   {
      	      		image_it2.Set(border_color.blue, border_color.green,
         	      		border_color.red);
	            		image_it2.Increment();
		            }
      	         if (!image_it1.NextRow())
         	      	break;
            	   image_it2.NextRow();
                  row_count++;
                  event_handler->Set(row_count);
						for (int n = 0; n < size; n++)
	   		      {
   	   		     	image_it2.Set(border_color.blue, border_color.green,
      	   		     	border_color.red);
         	  			image_it2.Increment();
		      	   }
	            }
   	      } while (1);
	         image_it2.NextRow();
            row_count++;
            event_handler->Set(row_count);
   	      for (int n = 0; n < size; n++)
      	   {
         		do
            	{
	            	image_it2.Set(border_color.blue, border_color.green,
   	            	border_color.red);
      	      } while (image_it2.Increment());
         	   image_it2.NextRow();
               row_count++;
               event_handler->Set(row_count);
	         }
	         event_handler->Stop();
   	      return new_data;
         }
         else
         {
	         for (int n = 0; n < size; n++)
   	      {
      	   	do
         	   {
            		image_it2.Set(border_color.blue, border_color.green,
               		border_color.red);
	            } while (image_it2.Increment());
   	         image_it2.NextRow();
               row_count++;
               event_handler->Set(row_count);
      	   }
				for (int n = 0; n < size; n++)
   	      {
      	     	image_it2.Set(border_color.blue, border_color.green,
         	     	border_color.red);
           		image_it2.Increment();
	         }
   	      do
      	   {
            	unsigned char val = (unsigned char)image_it1;
         	   image_it2.Set(val, val, val);
	            image_it2.Increment();
   	         if (!image_it1.Increment())
      	      {
						for (int n = 0; n < size; n++)
   	      	   {
      	      		image_it2.Set(border_color.blue, border_color.green,
         	      		border_color.red);
	            		image_it2.Increment();
		            }
      	         if (!image_it1.NextRow())
         	      	break;
            	   image_it2.NextRow();
                  row_count++;
                  event_handler->Set(row_count);
						for (int n = 0; n < size; n++)
	   		      {
   	   		     	image_it2.Set(border_color.blue, border_color.green,
      	   		     	border_color.red);
         	  			image_it2.Increment();
		      	   }
	            }
   	      } while (1);
	         image_it2.NextRow();
            row_count++;
            event_handler->Set(row_count);
   	      for (int n = 0; n < size; n++)
      	   {
         		do
            	{
	            	image_it2.Set(border_color.blue, border_color.green,
   	            	border_color.red);
      	      } while (image_it2.Increment());
         	   image_it2.NextRow();
               row_count++;
               event_handler->Set(row_count);
	         }
	         event_handler->Stop();
   	      return new_data;
         }
      }
      case 4:
      {
      	if (data->GetNumPalette() != 0)
         {
	       	G42ImageData * new_data = new G42ImageData(data->GetData(),
   	      	data->GetWidth() + (size * 2), data->GetHeight() + (size * 2),
      	      8, data->GetNumPalette() + 1);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
				G42Color * palette = data->GetPalette();
            G42Color * new_palette = new G42Color[new_data->GetNumPalette()];
            for (int n = 0; n < new_data->GetNumPalette() - 1; n++)
            {
            	new_palette[n].red = palette[n].red;
            	new_palette[n].green = palette[n].green;
            	new_palette[n].blue = palette[n].blue;
            }
            unsigned char border_index = new_data->GetNumPalette() - 1;
           	new_palette[border_index].red = border_color.red;
           	new_palette[border_index].green = border_color.green;
           	new_palette[border_index].blue = border_color.blue;
            new_data->SetPalette(new_palette);
            delete [] new_palette;
				G42LockedID4BitIterator image_it1(data);
				G42ImageData8BitIterator image_it2(new_data);
	         for (int n = 0; n < size; n++)
   	      {
      	   	do
         	   {
            		image_it2.Set(border_index);
	            } while (image_it2.Increment());
   	         image_it2.NextRow();
               row_count++;
               event_handler->Set(row_count);
      	   }
				for (int n = 0; n < size; n++)
   	      {
      	     	image_it2.Set(border_index);
           		image_it2.Increment();
	        	}
   	      do
      	   {
         	   image_it2.Set((unsigned char)image_it1);
	            image_it2.Increment();
   	         if (!image_it1.Increment())
      	      {
						for (int n = 0; n < size; n++)
   	      	   {
      	      		image_it2.Set(border_index);
	            		image_it2.Increment();
		            }
      	         if (!image_it1.NextRow())
         	      	break;
            	   image_it2.NextRow();
                  row_count++;
                  event_handler->Set(row_count);
						for (int n = 0; n < size; n++)
	   		      {
   	   		     	image_it2.Set(border_index);
         	  			image_it2.Increment();
		      	   }
	            }
   	      } while (1);
	         image_it2.NextRow();
            row_count++;
            event_handler->Set(row_count);;
   	      for (int n = 0; n < size; n++)
      	   {
         		do
            	{
	            	image_it2.Set(border_index);
      	      } while (image_it2.Increment());
         	   image_it2.NextRow();
               row_count++;
               event_handler->Set(row_count);
	         }
	         event_handler->Stop();
   	      return new_data;
         }
         else
         {
	       	G42ImageData * new_data = new G42ImageData(data->GetData(),
   	      	data->GetWidth() + (size * 2), data->GetHeight() + (size * 2),
      	      8, 17);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
            G42Color * new_palette = new G42Color[new_data->GetNumPalette()];
            for (unsigned char n = 0; n < new_data->GetNumPalette() - 1; n++)
            {
            	new_palette[n].red = ((n << 4) + n);
            	new_palette[n].green = ((n << 4) + n);
            	new_palette[n].blue = ((n << 4) + n);
            }
            unsigned char border_index = new_data->GetNumPalette() - 1;
           	new_palette[border_index].red = border_color.red;
           	new_palette[border_index].green = border_color.green;
           	new_palette[border_index].blue = border_color.blue;
            new_data->SetPalette(new_palette);
            delete [] new_palette;
				G42LockedID4BitIterator image_it1(data);
				G42ImageData8BitIterator image_it2(new_data);
	         for (int n = 0; n < size; n++)
   	      {
      	   	do
         	   {
            		image_it2.Set(border_index);
	            } while (image_it2.Increment());
   	         image_it2.NextRow();
               row_count++;
               event_handler->Set(row_count);
      	   }
				for (int n = 0; n < size; n++)
   	      {
      	     	image_it2.Set(border_index);
           		image_it2.Increment();
	         }
   	      do
      	   {
         	   image_it2.Set((unsigned char)image_it1);
	            image_it2.Increment();
   	         if (!image_it1.Increment())
      	      {
						for (int n = 0; n < size; n++)
   	      	   {
      	      		image_it2.Set(border_index);
	            		image_it2.Increment();
		            }
      	         if (!image_it1.NextRow())
         	      	break;
            	   image_it2.NextRow();
	               row_count++;
   	            event_handler->Set(row_count);
						for (int n = 0; n < size; n++)
	   		      {
   	   		     	image_it2.Set(border_index);
         	  			image_it2.Increment();
		      	   }
	            }
   	      } while (1);
	         image_it2.NextRow();
            row_count++;
            event_handler->Set(row_count);
   	      for (int n = 0; n < size; n++)
      	   {
         		do
            	{
	            	image_it2.Set(border_index);
      	      } while (image_it2.Increment());
         	   image_it2.NextRow();
               row_count++;
               event_handler->Set(row_count);
	         }
	         event_handler->Stop();
   	      return new_data;
         }
      }
      case 1:
      {
      	if (data->GetNumPalette() != 0)
         {
	       	G42ImageData * new_data = new G42ImageData(data->GetData(),
   	      	data->GetWidth() + (size * 2), data->GetHeight() + (size * 2),
      	      4, data->GetNumPalette() + 1);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
				G42Color * palette = data->GetPalette();
            G42Color * new_palette = new G42Color[new_data->GetNumPalette()];
            for (int n = 0; n < new_data->GetNumPalette() - 1; n++)
            {
            	new_palette[n].red = palette[n].red;
            	new_palette[n].green = palette[n].green;
            	new_palette[n].blue = palette[n].blue;
            }
            unsigned char border_index = new_data->GetNumPalette() - 1;
           	new_palette[border_index].red = border_color.red;
           	new_palette[border_index].green = border_color.green;
           	new_palette[border_index].blue = border_color.blue;
            new_data->SetPalette(new_palette);
            delete [] new_palette;
				G42LockedID1BitIterator image_it1(data);
				G42ImageData4BitIterator image_it2(new_data);
	         for (int n = 0; n < size; n++)
   	      {
      	   	do
         	   {
            		image_it2.Set4(border_index);
	            } while (image_it2.Increment());
   	         image_it2.NextRow();
               row_count++;
               event_handler->Set(row_count);
      	   }
				for (int n = 0; n < size; n++)
   	      {
      	     	image_it2.Set4(border_index);
           		image_it2.Increment();
	        	}
     	      do
      	   {
               if ((unsigned char)image_it1)
	         	   image_it2.Set4(1);
               else
               	image_it2.Set4(0);
	            image_it2.Increment();
   	         if (!image_it1.Increment())
      	      {
						for (int n = 0; n < size; n++)
   	      	   {
      	      		image_it2.Set4(border_index);
	            		image_it2.Increment();
		            }
      	         if (!image_it1.NextRow())
         	      	break;
            	   image_it2.NextRow();
	               row_count++;
   	            event_handler->Set(row_count);
						for (int n = 0; n < size; n++)
	   		      {
   	   		     	image_it2.Set4(border_index);
         	  			image_it2.Increment();
		      	   }
	            }
   	      } while (1);
	         image_it2.NextRow();
            row_count++;
            event_handler->Set(row_count);
   	      for (int n = 0; n < size; n++)
      	   {
         		do
            	{
	            	image_it2.Set4(border_index);
      	      } while (image_it2.Increment());
         	   image_it2.NextRow();
               row_count++;
               event_handler->Set(row_count);
	         }
	         event_handler->Stop();
   	      return new_data;
         }
         else
         {
	       	G42ImageData * new_data = new G42ImageData(data->GetData(),
   	      	data->GetWidth() + (size * 2), data->GetHeight() + (size * 2),
      	      4, 3);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
            G42Color * new_palette = new G42Color[new_data->GetNumPalette()];
            new_palette[0].red = 0;
            new_palette[0].green = 0;
            new_palette[0].blue = 0;
            new_palette[1].red = 0xff;
            new_palette[1].green = 0xff;
            new_palette[1].blue = 0xff;
            unsigned char border_index = new_data->GetNumPalette() - 1;
           	new_palette[border_index].red = border_color.red;
           	new_palette[border_index].green = border_color.green;
           	new_palette[border_index].blue = border_color.blue;
            new_data->SetPalette(new_palette);
            delete [] new_palette;
				G42LockedID1BitIterator image_it1(data);
				G42ImageData4BitIterator image_it2(new_data);
	         for (int n = 0; n < size; n++)
   	      {
      	   	do
         	   {
            		image_it2.Set4(border_index);
	            } while (image_it2.Increment());
   	         image_it2.NextRow();
               row_count++;
               event_handler->Set(row_count);
      	   }
				for (int n = 0; n < size; n++)
   	      {
      	     	image_it2.Set4(border_index);
           		image_it2.Increment();
	         }
   	      do
      	   {
               if ((unsigned char)image_it1)
	         	   image_it2.Set4(1);
               else
               	image_it2.Set4(0);
	            image_it2.Increment();
   	         if (!image_it1.Increment())
      	      {
						for (int n = 0; n < size; n++)
   	      	   {
      	      		image_it2.Set4(border_index);
	            		image_it2.Increment();
		            }
      	         if (!image_it1.NextRow())
         	      	break;
            	   image_it2.NextRow();
 	               row_count++;
   	            event_handler->Set(row_count);
						for (int n = 0; n < size; n++)
	   		      {
   	   		     	image_it2.Set4(border_index);
         	  			image_it2.Increment();
		      	   }
	            }
   	      } while (1);
	         image_it2.NextRow();
            row_count++;
            event_handler->Set(row_count);
   	      for (int n = 0; n < size; n++)
      	   {
         		do
            	{
	            	image_it2.Set4(border_index);
      	      } while (image_it2.Increment());
         	   image_it2.NextRow();
               row_count++;
               event_handler->Set(row_count);
	         }
	         event_handler->Stop();
   	      return new_data;
         }
      }
   }
   return data;
}
G42ImageData *
Buttonize(G42ImageData * data, G42Color top_color, G42Color bottom_color,
	int size, bool invert, bool has_color, G42EventUpdateHandler * event_handler)
{
	if (!has_color)
   	return ButtonizeNoColor(data, size, invert, event_handler);
	if (invert)
   {
    	G42Color swap(top_color.red, top_color.green, top_color.blue);
      top_color.red = bottom_color.red;
      top_color.green = bottom_color.green;
      top_color.blue = bottom_color.blue;
      bottom_color.red = swap.red;
      bottom_color.green = swap.green;
      bottom_color.blue = swap.blue;
   }
	event_handler->SetDenominator((int32)(data->GetHeight() + (size * 2)));
	int32 row_count = 0L;
   event_handler->Start();
   event_handler->Set(0L);
	switch (data->GetDepth())
   {
   	case 32:
    	case 24:
      {
      	G42ImageData * new_data = new G42ImageData(data->GetData(),
         	data->GetWidth() + (size * 2), data->GetHeight() + (size * 2),
            8*TrueColorPixelSize, 0);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42LockedID24BitIterator image_it1(data);
			G42ImageData24BitIterator image_it2(new_data);
         for (int n = 0; n < size; n++)
         {
         	do
            {
            	image_it2.Set(top_color.blue, top_color.green,
               	top_color.red);
            } while (image_it2.Increment());
            image_it2.NextRow();
            row_count++;
            event_handler->Set(row_count);
         }
         image_it2.Reset();
         for (int n = 0; n < (size - 1); n++)
         {
         	image_it2.Set(0, 0, 0);
            image_it2.Increment();
            image_it2.IncRow();
         }
      	do
         {
         	image_it2.Set(0, 0, 0);
         } while (image_it2.Increment());
         image_it2.Reset();
         for (int n = 0; n < size; n++)
         {
	         image_it2.SetRowEnd();
         	for (int i = 0; i < n; i++)
            {
					image_it2.Set(bottom_color.blue, bottom_color.green,
               	bottom_color.red);
               image_it2.Decrement();
				}
	        	image_it2.Set(0, 0, 0);
        	   image_it2.IncRow();
         }
         image_it2.ResetRow();
			for (int n = 0; n < (size - 1); n++)
         {
           	image_it2.Set(top_color.blue, top_color.green,
              	top_color.red);
           	image_it2.Increment();
         }
         image_it2.Set(0, 0, 0);
         image_it2.Increment();
         do
         {
            image_it2.Set(image_it1.GetBlue(), image_it1.GetGreen(),
            	image_it1.GetRed());
            image_it2.Increment();
            if (!image_it1.Increment())
            {
					image_it2.Set(0, 0, 0);
		         image_it2.Increment();
               do
               {
      	      	image_it2.Set(bottom_color.blue, bottom_color.green,
         	      	bottom_color.red);
					} while (image_it2.Increment());
               if (!image_it1.NextRow())
               	break;
               image_it2.NextRow();
	            row_count++;
   	         event_handler->Set(row_count);
					for (int n = 0; n < (size - 1); n++)
   		      {
      		     	image_it2.Set(top_color.blue, top_color.green,
         		     	top_color.red);
           			image_it2.Increment();
		         }
               image_it2.Set(0, 0, 0);
     			   image_it2.Increment();
            }
         } while (1);
         image_it2.NextRow();
         row_count++;
         event_handler->Set(row_count);
         for (int n = 0; n < size; n++)
         {
         	do
            {
            	image_it2.Set(bottom_color.blue, bottom_color.green,
               	bottom_color.red);
            } while (image_it2.Increment());
            image_it2.NextRow();
            row_count++;
            event_handler->Set(row_count);
         }
         image_it2.SetRowEnd();
         for (int n = 0; n < (size - 1); n++)
         {
         	image_it2.Set(0, 0, 0);
            image_it2.Decrement();
            image_it2.DecRow();
         }
      	do
         {
         	image_it2.Set(0, 0, 0);
         } while (image_it2.Decrement());
         image_it2.ResetRow();
         image_it2.SetLastRow();
         for (int n = 0; n < size; n++)
         {
	         image_it2.ResetRow();
         	for (int i = 0; i < n; i++)
            {
					image_it2.Set(top_color.blue, top_color.green,
               	top_color.red);
               image_it2.Increment();
				}
	        	image_it2.Set(0, 0, 0);
        	   image_it2.DecRow();
         }
      	event_handler->Stop();
         return new_data;
      }
      case 8:
      {
      	G42ImageData * new_data = new G42ImageData(data->GetData(),
  		     	data->GetWidth() + (size * 2), data->GetHeight() + (size * 2),
        	   8*TrueColorPixelSize, 0);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42LockedID8BitIterator image_it1(data);
			G42ImageData24BitIterator image_it2(new_data);
     	   for (int n = 0; n < size; n++)
        	{
        		do
            {
  	         	image_it2.Set(top_color.blue, top_color.green,
     	         	top_color.red);
        	   } while (image_it2.Increment());
           	image_it2.NextRow();
            row_count++;
            event_handler->Set(row_count);
         }
  	      image_it2.Reset();
     	   for (int n = 0; n < (size - 1); n++)
        	{
         	image_it2.Set(0, 0, 0);
  	         image_it2.Increment();
     	      image_it2.IncRow();
        	}
      	do
  	      {
     	   	image_it2.Set(0, 0, 0);
        	} while (image_it2.Increment());
         image_it2.Reset();
  	      for (int n = 0; n < size; n++)
     	   {
      	   image_it2.SetRowEnd();
        		for (int i = 0; i < n; i++)
            {
					image_it2.Set(bottom_color.blue, bottom_color.green,
     	         	bottom_color.red);
        	      image_it2.Decrement();
				}
	        	image_it2.Set(0, 0, 0);
  	     	   image_it2.IncRow();
     	   }
        	image_it2.ResetRow();
			for (int n = 0; n < (size - 1); n++)
  	      {
     	     	image_it2.Set(top_color.blue, top_color.green,
        	     	top_color.red);
        		image_it2.Increment();
         }
  	      image_it2.Set(0, 0, 0);
     	   image_it2.Increment();
         if (data->GetNumPalette() != 0)
         {
	         G42Color * palette = data->GetPalette();
          	do
	         {
            	unsigned char val = (unsigned char)image_it1;
   	         image_it2.Set(palette[val].blue, palette[val].green,
               	palette[val].red);
         	   image_it2.Increment();
            	if (!image_it1.Increment())
	            {
						image_it2.Set(0, 0, 0);
			         image_it2.Increment();
         	      do
            	   {
      	      		image_it2.Set(bottom_color.blue, bottom_color.green,
         	      		bottom_color.red);
						} while (image_it2.Increment());
   	            if (!image_it1.NextRow())
      	         	break;
         	      image_it2.NextRow();
		            row_count++;
      		      event_handler->Set(row_count);
						for (int n = 0; n < (size - 1); n++)
   		      	{
	      		     	image_it2.Set(top_color.blue, top_color.green,
   	      		     	top_color.red);
      	     			image_it2.Increment();
		   	      }
            	   image_it2.Set(0, 0, 0);
     			   	image_it2.Increment();
	            }
   	      } while (1);
         }
         else
         {
            do
	         {
            	unsigned char val = (unsigned char)image_it1;
   	         image_it2.Set(val, val, val);
         	   image_it2.Increment();
            	if (!image_it1.Increment())
	            {
						image_it2.Set(0, 0, 0);
			         image_it2.Increment();
         	      do
            	   {
      	      		image_it2.Set(bottom_color.blue, bottom_color.green,
         	      		bottom_color.red);
						} while (image_it2.Increment());
   	            if (!image_it1.NextRow())
      	         	break;
         	      image_it2.NextRow();
		            row_count++;
      		      event_handler->Set(row_count);
						for (int n = 0; n < (size - 1); n++)
   		      	{
	      		     	image_it2.Set(top_color.blue, top_color.green,
   	      		     	top_color.red);
      	     			image_it2.Increment();
		   	      }
            	   image_it2.Set(0, 0, 0);
     			   	image_it2.Increment();
	            }
   	      } while (1);
         }
     	   image_it2.NextRow();
         row_count++;
         event_handler->Set(row_count);
        	for (int n = 0; n < size; n++)
         {
  	      	do
     	      {
        	   	image_it2.Set(bottom_color.blue, bottom_color.green,
           	   	bottom_color.red);
            } while (image_it2.Increment());
  	         image_it2.NextRow();
            row_count++;
            event_handler->Set(row_count);
     	   }
        	image_it2.SetRowEnd();
         for (int n = 0; n < (size - 1); n++)
  	      {
     	   	image_it2.Set(0, 0, 0);
        	   image_it2.Decrement();
           	image_it2.DecRow();
         }
  	   	do
     	   {
        		image_it2.Set(0, 0, 0);
         } while (image_it2.Decrement());
  	      image_it2.ResetRow();
     	   image_it2.SetLastRow();
         for (int n = 0; n < size; n++)
  	      {
   	      image_it2.ResetRow();
        		for (int i = 0; i < n; i++)
            {
					image_it2.Set(top_color.blue, top_color.green,
     	         	top_color.red);
        	      image_it2.Increment();
				}
	        	image_it2.Set(0, 0, 0);
  	     	   image_it2.DecRow();
     	   }
      	event_handler->Stop();
         return new_data;
      }
      case 4:
      {
			G42ImageData * new_data;
         unsigned char top_index, bottom_index, black_index;
         if (data->GetNumPalette() != 0)
         {
	      	new_data = new G42ImageData(data->GetData(),
  			     	data->GetWidth() + (size * 2), data->GetHeight() + (size * 2),
      	  	   8, data->GetNumPalette() + 3);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
            G42Color * new_palette = new G42Color [new_data->GetNumPalette()];
            G42Color * palette = data->GetPalette();
            unsigned char n;
            for (n = 0; n < data->GetNumPalette(); n++)
            {
            	new_palette[n].red = palette[n].red;
            	new_palette[n].green = palette[n].green;
            	new_palette[n].blue = palette[n].blue;
            }
            top_index = n;
            bottom_index = top_index + 1;
            black_index = top_index + 2;
            new_palette[top_index].red = top_color.red;
            new_palette[top_index].green = top_color.green;
            new_palette[top_index].blue = top_color.blue;
            new_palette[bottom_index].red = bottom_color.red;
            new_palette[bottom_index].green = bottom_color.green;
            new_palette[bottom_index].blue = bottom_color.blue;
            new_palette[black_index].red = 0;
            new_palette[black_index].green = 0;
            new_palette[black_index].blue = 0;
            new_data->SetPalette(new_palette);
            delete [] new_palette;
			}
         else
         {
	      	new_data = new G42ImageData(data->GetData(),
  			     	data->GetWidth() + (size * 2), data->GetHeight() + (size * 2),
      	  	   8, 18);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
            G42Color * new_palette = new G42Color [new_data->GetNumPalette()];
            unsigned char n;
            for (n = 0; n < 16; n++)
            {
            	new_palette[n].red = ((n << 4) + n);
            	new_palette[n].green = ((n << 4) + n);
            	new_palette[n].blue = ((n << 4) + n);
            }
            top_index = n;
            bottom_index = top_index + 1;
            black_index = 0;
            new_palette[top_index].red = top_color.red;
            new_palette[top_index].green = top_color.green;
            new_palette[top_index].blue = top_color.blue;
            new_palette[bottom_index].red = bottom_color.red;
            new_palette[bottom_index].green = bottom_color.green;
            new_palette[bottom_index].blue = bottom_color.blue;
            new_data->SetPalette(new_palette);
            delete [] new_palette;
         }
			G42LockedID4BitIterator image_it1(data);
			G42ImageData8BitIterator image_it2(new_data);
     	   for (int n = 0; n < size; n++)
        	{
        		do
            {
  	         	image_it2.Set(top_index);
        	   } while (image_it2.Increment());
           	image_it2.NextRow();
            row_count++;
            event_handler->Set(row_count);
         }
  	      image_it2.Reset();
     	   for (int n = 0; n < (size - 1); n++)
        	{
         	image_it2.Set(black_index);
  	         image_it2.Increment();
     	      image_it2.IncRow();
        	}
      	do
  	      {
     	   	image_it2.Set(black_index);
        	} while (image_it2.Increment());
         image_it2.Reset();
  	      for (int n = 0; n < size; n++)
     	   {
      	   image_it2.SetRowEnd();
        		for (int i = 0; i < n; i++)
            {
					image_it2.Set(bottom_index);
        	      image_it2.Decrement();
				}
	        	image_it2.Set(black_index);
  	     	   image_it2.IncRow();
     	   }
        	image_it2.ResetRow();
			for (int n = 0; n < (size - 1); n++)
  	      {
     	     	image_it2.Set(top_index);
        		image_it2.Increment();
         }
  	      image_it2.Set(black_index);
     	   image_it2.Increment();
        	do
         {
  	         image_it2.Set((unsigned char)image_it1);
        	   image_it2.Increment();
           	if (!image_it1.Increment())
             {
 					image_it2.Set(black_index);
 		         image_it2.Increment();
        	      do
           	   {
     	      		image_it2.Set(bottom_index);
 					} while (image_it2.Increment());
  	            if (!image_it1.NextRow())
     	         	break;
        	      image_it2.NextRow();
	            row_count++;
   	         event_handler->Set(row_count);
 					for (int n = 0; n < (size - 1); n++)
  		      	{
       		     	image_it2.Set(top_index);
     	     			image_it2.Increment();
 	   	      }
           	   image_it2.Set(black_index);
    			   	image_it2.Increment();
             }
  	      } while (1);
     	   image_it2.NextRow();
         row_count++;
         event_handler->Set(row_count);
        	for (int n = 0; n < size; n++)
         {
  	      	do
     	      {
        	   	image_it2.Set(bottom_index);
            } while (image_it2.Increment());
  	         image_it2.NextRow();
            row_count++;
            event_handler->Set(row_count);
     	   }
        	image_it2.SetRowEnd();
         for (int n = 0; n < (size - 1); n++)
  	      {
     	   	image_it2.Set(black_index);
        	   image_it2.Decrement();
           	image_it2.DecRow();
         }
  	   	do
     	   {
        		image_it2.Set(black_index);
         } while (image_it2.Decrement());
  	      image_it2.ResetRow();
     	   image_it2.SetLastRow();
         for (int n = 0; n < size; n++)
  	      {
   	      image_it2.ResetRow();
        		for (int i = 0; i < n; i++)
            {
					image_it2.Set(top_index);
        	      image_it2.Increment();
				}
	        	image_it2.Set(black_index);
  	     	   image_it2.DecRow();
     	   }
      	event_handler->Stop();
         return new_data;
      }
      case 1:
      {
			G42ImageData * new_data;
         unsigned char top_index, bottom_index, black_index;
         if (data->GetNumPalette() != 0)
         {
	      	new_data = new G42ImageData(data->GetData(),
  			     	data->GetWidth() + (size * 2), data->GetHeight() + (size * 2),
      	  	   4, data->GetNumPalette() + 3);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
            G42Color * new_palette = new G42Color [new_data->GetNumPalette()];
            G42Color * palette = data->GetPalette();
            int n;
            for (n = 0; n < data->GetNumPalette(); n++)
            {
            	new_palette[n].red = palette[n].red;
            	new_palette[n].green = palette[n].green;
            	new_palette[n].blue = palette[n].blue;
            }
            top_index = n;
            bottom_index = top_index + 1;
            black_index = top_index + 2;
            new_palette[top_index].red = top_color.red;
            new_palette[top_index].green = top_color.green;
            new_palette[top_index].blue = top_color.blue;
            new_palette[bottom_index].red = bottom_color.red;
            new_palette[bottom_index].green = bottom_color.green;
            new_palette[bottom_index].blue = bottom_color.blue;
            new_palette[black_index].red = 0;
            new_palette[black_index].green = 0;
            new_palette[black_index].blue = 0;
            new_data->SetPalette(new_palette);
            delete [] new_palette;
			}
         else
         {
	      	new_data = new G42ImageData(data->GetData(),
  			     	data->GetWidth() + (size * 2), data->GetHeight() + (size * 2),
      	  	   4, 4);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
            G42Color * new_palette = new G42Color [new_data->GetNumPalette()];
           	new_palette[0].red = 0;
           	new_palette[0].green = 0;
           	new_palette[0].blue = 0;
           	new_palette[1].red = 0xff;
           	new_palette[1].green = 0xff;
           	new_palette[1].blue = 0xff;
            top_index = 2;
            bottom_index = top_index + 1;
            black_index = 0;
            new_palette[top_index].red = top_color.red;
            new_palette[top_index].green = top_color.green;
            new_palette[top_index].blue = top_color.blue;
            new_palette[bottom_index].red = bottom_color.red;
            new_palette[bottom_index].green = bottom_color.green;
            new_palette[bottom_index].blue = bottom_color.blue;
            new_data->SetPalette(new_palette);
            delete [] new_palette;
         }
			G42LockedID1BitIterator image_it1(data);
			G42ImageData4BitIterator image_it2(new_data);
     	   for (int n = 0; n < size; n++)
        	{
        		do
            {
  	         	image_it2.Set4(top_index);
        	   } while (image_it2.Increment());
           	image_it2.NextRow();
            row_count++;
            event_handler->Set(row_count);
         }
  	      image_it2.Reset();
     	   for (int n = 0; n < (size - 1); n++)
        	{
         	image_it2.Set4(black_index);
  	         image_it2.Increment();
     	      image_it2.IncRow();
        	}
      	do
  	      {
     	   	image_it2.Set4(black_index);
        	} while (image_it2.Increment());
         image_it2.Reset();
  	      for (int n = 0; n < size; n++)
     	   {
      	   image_it2.SetRowEnd();
        		for (int i = 0; i < n; i++)
            {
					image_it2.Set4(bottom_index);
        	      image_it2.Decrement();
				}
	        	image_it2.Set4(black_index);
  	     	   image_it2.IncRow();
     	   }
        	image_it2.ResetRow();
			for (int n = 0; n < (size - 1); n++)
  	      {
     	     	image_it2.Set4(top_index);
        		image_it2.Increment();
         }
  	      image_it2.Set4(black_index);
     	   image_it2.Increment();
        	do
         {
         	if ((unsigned char)image_it1)
            	image_it2.Set4(1);
            else
            	image_it2.Set4(0);
        	   image_it2.Increment();
           	if (!image_it1.Increment())
             {
 					image_it2.Set4(black_index);
 		         image_it2.Increment();
        	      do
           	   {
     	      		image_it2.Set4(bottom_index);
 					} while (image_it2.Increment());
  	            if (!image_it1.NextRow())
     	         	break;
        	      image_it2.NextRow();
	            row_count++;
   	         event_handler->Set(row_count);
 					for (int n = 0; n < (size - 1); n++)
  		      	{
       		     	image_it2.Set4(top_index);
     	     			image_it2.Increment();
 	   	      }
           	   image_it2.Set4(black_index);
    			   	image_it2.Increment();
             }
  	      } while (1);
     	   image_it2.NextRow();
         row_count++;
         event_handler->Set(row_count);
        	for (int n = 0; n < size; n++)
         {
  	      	do
     	      {
        	   	image_it2.Set4(bottom_index);
            } while (image_it2.Increment());
  	         image_it2.NextRow();
            row_count++;
            event_handler->Set(row_count);
     	   }
        	image_it2.SetRowEnd();
         for (int n = 0; n < (size - 1); n++)
  	      {
     	   	image_it2.Set4(black_index);
        	   image_it2.Decrement();
           	image_it2.DecRow();
         }
  	   	do
     	   {
        		image_it2.Set4(black_index);
         } while (image_it2.Decrement());
  	      image_it2.ResetRow();
     	   image_it2.SetLastRow();
         for (int n = 0; n < size; n++)
  	      {
   	      image_it2.ResetRow();
        		for (int i = 0; i < n; i++)
            {
					image_it2.Set4(top_index);
        	      image_it2.Increment();
				}
	        	image_it2.Set4(black_index);
  	     	   image_it2.DecRow();
     	   }
      	event_handler->Stop();
         return new_data;
      }
   }
}
G42ImageData *
ButtonizeNoColor(G42ImageData * data, int size, bool invert,
	G42EventUpdateHandler * event_handler)
{
	int offset = 64;
   if (invert)
   	offset = -offset;
   	
   	if ((size << 1) >= data->GetWidth())
   		size = ((data->GetWidth() - 1) >> 1);
   	if ((size << 1) >= data->GetHeight())
   		size = ((data->GetHeight() - 1) >> 1);
	event_handler->SetDenominator((int32)data->GetHeight());
   int32 row_count = 0L;
   event_handler->Start();
   event_handler->Set(0L);
	switch (data->GetDepth())
   {
   	case 32:
   	case 24:
      {
			G42ImageData24BitIterator image_it(data);
         for (int n = 0; n < size; n++)
         {
         	int x = 0;
            do
            {
            	int red = image_it.GetRed();
            	int green = image_it.GetGreen();
            	int blue = image_it.GetBlue();
            	if (x < n)
               {
               	red += offset;
                  green += offset;
                  blue += offset;
               }
               else if ((x > n) && (x < (data->GetWidth() - n - 1)))
               {
               	red += offset;
                  green += offset;
                  blue += offset;
               }
               else if (x > (data->GetWidth() - n - 1))
               {
               	red -= offset;
                  green -= offset;
                  blue -= offset;
               }
               if (red < 0)
               	red = 0;
               if (red > 255)
               	red = 255;
               if (green < 0)
               	green = 0;
               if (green > 255)
               	green = 255;
               if (blue < 0)
               	blue = 0;
               if (blue > 255)
               	blue = 255;
					x++;
               image_it.Set(blue, green, red);
            } while (image_it.Increment());
            image_it.NextRow();
            row_count++;
            event_handler->Set(row_count);
         }
         for (int n = 0; n < (data->GetHeight() - (size * 2)); n++)
         {
         	for (int x = 0; x < size; x++)
            {
               int red = image_it.GetRed() + offset;
            	int green = image_it.GetGreen() + offset;
            	int blue = image_it.GetBlue() + offset;
               if (red < 0)
               	red = 0;
               if (red > 255)
               	red = 255;
               if (green < 0)
               	green = 0;
               if (green > 255)
               	green = 255;
               if (blue < 0)
               	blue = 0;
               if (blue > 255)
               	blue = 255;
					image_it.Set(blue, green, red);
               image_it.Increment();
            }
            image_it.SetRowEnd();
         	for (int x = 0; x < size; x++)
            {
               int red = image_it.GetRed() - offset;
            	int green = image_it.GetGreen() - offset;
            	int blue = image_it.GetBlue() - offset;
               if (red < 0)
               	red = 0;
               if (red > 255)
               	red = 255;
               if (green < 0)
               	green = 0;
               if (green > 255)
               	green = 255;
               if (blue < 0)
               	blue = 0;
               if (blue > 255)
               	blue = 255;
					image_it.Set(blue, green, red);
               image_it.Decrement();
            }
            image_it.NextRow();
            row_count++;
            event_handler->Set(row_count);
         }
         image_it.SetLastRow();
         for (int n = 0; n < size; n++)
         {
         	int x = 0;
            do
            {
            	int red = image_it.GetRed();
            	int green = image_it.GetGreen();
            	int blue = image_it.GetBlue();
            	if (x < n)
               {
               	red += offset;
                  green += offset;
                  blue += offset;
               }
               else if ((x > n) && (x < (data->GetWidth() - n - 1)))
               {
               	red -= offset;
                  green -= offset;
                  blue -= offset;
               }
               else if (x > (data->GetWidth() - n - 1))
               {
               	red -= offset;
                  green -= offset;
                  blue -= offset;
               }
               if (red < 0)
               	red = 0;
               if (red > 255)
               	red = 255;
               if (green < 0)
               	green = 0;
               if (green > 255)
               	green = 255;
               if (blue < 0)
               	blue = 0;
               if (blue > 255)
               	blue = 255;
					x++;
               image_it.Set(blue, green, red);
            } while (image_it.Increment());
            image_it.ResetRow();
            image_it.DecRow();
            row_count++;
            event_handler->Set(row_count);
         }
      	event_handler->Stop();
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
				G42LockedID8BitIterator image_it1(data);
				G42ImageData24BitIterator image_it2(new_data);
   			G42Color * palette = data->GetPalette();
	         for (int n = 0; n < size; n++)
   	      {
      	   	int x = 0;
         	   do
            	{
               	unsigned char val = (unsigned char)image_it1;
	            	int red = palette[val].red;
   	         	int green = palette[val].green;
      	      	int blue = palette[val].blue;
         	   	if (x < n)
            	   {
               		red += offset;
                  	green += offset;
	                  blue += offset;
   	            }
      	         else if ((x > n) && (x < (data->GetWidth() - n - 1)))
         	      {
            	   	red += offset;
               	   green += offset;
                  	blue += offset;
	               }
   	            else if (x > (data->GetWidth() - n - 1))
      	         {
         	      	red -= offset;
            	      green -= offset;
               	   blue -= offset;
	               }
   	            if (red < 0)
      	         	red = 0;
         	      if (red > 255)
            	   	red = 255;
               	if (green < 0)
	               	green = 0;
   	            if (green > 255)
      	         	green = 255;
         	      if (blue < 0)
            	   	blue = 0;
               	if (blue > 255)
	               	blue = 255;
						x++;
      	         image_it2.Set(blue, green, red);
                  image_it2.Increment();
         	   } while (image_it1.Increment());
            	image_it2.NextRow();
            	image_it1.NextRow();
	            row_count++;
   	         event_handler->Set(row_count);
	         }
   	      for (int n = 0; n < (data->GetHeight() - (size * 2)); n++)
      	   {
               int x;
         		for (x = 0; x < size; x++)
	            {
               	unsigned char val = (unsigned char)image_it1;
   	            int red = palette[val].red + offset;
      	      	int green = palette[val].green + offset;
         	   	int blue = palette[val].blue + offset;
            	   if (red < 0)
               		red = 0;
	               if (red > 255)
   	            	red = 255;
      	         if (green < 0)
         	      	green = 0;
            	   if (green > 255)
               		green = 255;
	               if (blue < 0)
   	            	blue = 0;
      	         if (blue > 255)
         	      	blue = 255;
						image_it2.Set(blue, green, red);
               	image_it2.Increment();
                  image_it1.Increment();
	            }
               for (; x < (data->GetWidth() - size); x++)
   				{
               	unsigned char val = (unsigned char)image_it1;
                  image_it2.Set(palette[val].blue, palette[val].green,
                  	palette[val].red);
                  image_it1.Increment();
                  image_it2.Increment();
               }
	     	   	for (; x < data->GetWidth(); x++)
         	   {
               	unsigned char val = (unsigned char)image_it1;
            	   int red = palette[val].red - offset;
            		int green = palette[val].green - offset;
	            	int blue = palette[val].blue - offset;
   	            if (red < 0)
      	         	red = 0;
         	      if (red > 255)
            	   	red = 255;
               	if (green < 0)
	               	green = 0;
   	            if (green > 255)
      	         	green = 255;
         	      if (blue < 0)
            	   	blue = 0;
               	if (blue > 255)
	               	blue = 255;
						image_it2.Set(blue, green, red);
      	         image_it2.Increment();
      	         image_it1.Increment();
         	   }
            	image_it2.NextRow();
            	image_it1.NextRow();
	            row_count++;
   	         event_handler->Set(row_count);
	         }
   	      image_it2.SetLastRow();
   	      image_it1.SetLastRow();
      	   for (int n = 0; n < size; n++)
         	{
	         	int x = 0;
   	         do
      	      {
               	unsigned char val = (unsigned char)image_it1;
         	   	int red = palette[val].red;
            		int green = palette[val].green;
            		int blue = palette[val].blue;
	            	if (x < n)
   	            {
      	         	red += offset;
         	         green += offset;
            	      blue += offset;
               	}
	               else if ((x > n) && (x < (data->GetWidth() - n - 1)))
   	            {
      	         	red -= offset;
         	         green -= offset;
            	      blue -= offset;
               	}
	               else if (x > (data->GetWidth() - n - 1))
   	            {
      	         	red -= offset;
         	         green -= offset;
            	      blue -= offset;
               	}
	               if (red < 0)
   	            	red = 0;
      	         if (red > 255)
         	      	red = 255;
            	   if (green < 0)
               		green = 0;
	               if (green > 255)
   	            	green = 255;
      	         if (blue < 0)
         	      	blue = 0;
            	   if (blue > 255)
               		blue = 255;
						x++;
   	            image_it2.Set(blue, green, red);
                  image_it2.Increment();
      	      } while (image_it1.Increment());
         	   image_it2.ResetRow();
            	image_it2.DecRow();
         	   image_it1.ResetRow();
            	image_it1.DecRow();
	            row_count++;
   	         event_handler->Set(row_count);
	         }
	      	event_handler->Stop();
            return new_data;
         }
			G42ImageData8BitIterator image_it(data);
         for (int n = 0; n < size; n++)
         {
         	int x = 0;
            do
            {
            	int val = (unsigned char)image_it;
            	if (x < n)
               	val += offset;
               else if ((x > n) && (x < (data->GetWidth() - n - 1)))
               	val += offset;
               else if (x > (data->GetWidth() - n - 1))
               	val -= offset;
               if (val < 0)
               	val = 0;
               if (val > 255)
               	val = 255;
					x++;
               image_it.Set(val);
            } while (image_it.Increment());
            image_it.NextRow();
            row_count++;
            event_handler->Set(row_count);
         }
         for (int n = 0; n < (data->GetHeight() - (size * 2)); n++)
         {
         	for (int x = 0; x < size; x++)
            {
               int val = (unsigned char)image_it + offset;
               if (val < 0)
               	val = 0;
               if (val > 255)
               	val = 255;
					image_it.Set(val);
               image_it.Increment();
            }
            image_it.SetRowEnd();
         	for (int x = 0; x < size; x++)
            {
               int val = (unsigned char)image_it - offset;
               if (val < 0)
               	val = 0;
               if (val > 255)
               	val = 255;
					image_it.Set(val);
               image_it.Decrement();
            }
            image_it.NextRow();
            row_count++;
            event_handler->Set(row_count);
         }
         image_it.SetLastRow();
         for (int n = 0; n < size; n++)
         {
         	int x = 0;
            do
            {
            	int val = (unsigned char)image_it;
            	if (x < n)
               	val += offset;
               else if ((x > n) && (x < (data->GetWidth() - n - 1)))
               	val -= offset;
               else if (x > (data->GetWidth() - n - 1))
               	val -= offset;
               if (val < 0)
               	val = 0;
               if (val > 255)
               	val = 255;
					x++;
               image_it.Set(val);
            } while (image_it.Increment());
            image_it.ResetRow();
            image_it.DecRow();
            row_count++;
            event_handler->Set(row_count);
         }
      	event_handler->Stop();
         return data;
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
	         for (int n = 0; n < size; n++)
   	      {
      	   	int x = 0;
         	   do
            	{
               	unsigned char val = (unsigned char)image_it1;
	            	int red = palette[val].red;
   	         	int green = palette[val].green;
      	      	int blue = palette[val].blue;
         	   	if (x < n)
            	   {
               		red += offset;
                  	green += offset;
	                  blue += offset;
   	            }
      	         else if ((x > n) && (x < (data->GetWidth() - n - 1)))
         	      {
            	   	red += offset;
               	   green += offset;
                  	blue += offset;
	               }
   	            else if (x > (data->GetWidth() - n - 1))
      	         {
         	      	red -= offset;
            	      green -= offset;
               	   blue -= offset;
	               }
   	            if (red < 0)
      	         	red = 0;
         	      if (red > 255)
            	   	red = 255;
               	if (green < 0)
	               	green = 0;
   	            if (green > 255)
      	         	green = 255;
         	      if (blue < 0)
            	   	blue = 0;
               	if (blue > 255)
	               	blue = 255;
						x++;
      	         image_it2.Set(blue, green, red);
                  image_it2.Increment();
         	   } while (image_it1.Increment());
            	image_it2.NextRow();
            	image_it1.NextRow();
	            row_count++;
   	         event_handler->Set(row_count);
	         }
   	      for (int n = 0; n < (data->GetHeight() - (size * 2)); n++)
      	   {
               int x;
         		for (x = 0; x < size; x++)
	            {
               	unsigned char val = (unsigned char)image_it1;
   	            int red = palette[val].red + offset;
      	      	int green = palette[val].green + offset;
         	   	int blue = palette[val].blue + offset;
            	   if (red < 0)
               		red = 0;
	               if (red > 255)
   	            	red = 255;
      	         if (green < 0)
         	      	green = 0;
            	   if (green > 255)
               		green = 255;
	               if (blue < 0)
   	            	blue = 0;
      	         if (blue > 255)
         	      	blue = 255;
						image_it2.Set(blue, green, red);
               	image_it2.Increment();
                  image_it1.Increment();
	            }
               for (; x < (data->GetWidth() - size); x++)
   				{
               	unsigned char val = (unsigned char)image_it1;
                  image_it2.Set(palette[val].blue, palette[val].green,
                  	palette[val].red);
                  image_it1.Increment();
                  image_it2.Increment();
               }
	     	   	for (; x < data->GetWidth(); x++)
         	   {
               	unsigned char val = (unsigned char)image_it1;
            	   int red = palette[val].red - offset;
            		int green = palette[val].green - offset;
	            	int blue = palette[val].blue - offset;
   	            if (red < 0)
      	         	red = 0;
         	      if (red > 255)
            	   	red = 255;
               	if (green < 0)
	               	green = 0;
   	            if (green > 255)
      	         	green = 255;
         	      if (blue < 0)
            	   	blue = 0;
               	if (blue > 255)
	               	blue = 255;
						image_it2.Set(blue, green, red);
      	         image_it2.Increment();
      	         image_it1.Increment();
         	   }
            	image_it2.NextRow();
            	image_it1.NextRow();
	            row_count++;
   	         event_handler->Set(row_count);
	         }
   	      image_it2.SetLastRow();
   	      image_it1.SetLastRow();
      	   for (int n = 0; n < size; n++)
         	{
	         	int x = 0;
   	         do
      	      {
               	unsigned char val = (unsigned char)image_it1;
         	   	int red = palette[val].red;
            		int green = palette[val].green;
            		int blue = palette[val].blue;
	            	if (x < n)
   	            {
      	         	red += offset;
         	         green += offset;
            	      blue += offset;
               	}
	               else if ((x > n) && (x < (data->GetWidth() - n - 1)))
   	            {
      	         	red -= offset;
         	         green -= offset;
            	      blue -= offset;
               	}
	               else if (x > (data->GetWidth() - n - 1))
   	            {
      	         	red -= offset;
         	         green -= offset;
            	      blue -= offset;
               	}
	               if (red < 0)
   	            	red = 0;
      	         if (red > 255)
         	      	red = 255;
            	   if (green < 0)
               		green = 0;
	               if (green > 255)
   	            	green = 255;
      	         if (blue < 0)
         	      	blue = 0;
            	   if (blue > 255)
               		blue = 255;
						x++;
   	            image_it2.Set(blue, green, red);
                  image_it2.Increment();
      	      } while (image_it1.Increment());
         	   image_it2.ResetRow();
            	image_it2.DecRow();
         	   image_it1.ResetRow();
            	image_it1.DecRow();
	            row_count++;
   	         event_handler->Set(row_count);
	         }
	      	event_handler->Stop();
            return new_data;
         }
			G42ImageData4BitIterator image_it(data);
         for (int n = 0; n < size; n++)
         {
         	int x = 0;
            do
            {
            	int val = (unsigned char)image_it;
               val = ((val << 4) + val);
            	if (x < n)
               	val += offset;
               else if ((x > n) && (x < (data->GetWidth() - n - 1)))
               	val += offset;
               else if (x > (data->GetWidth() - n - 1))
               	val -= offset;
               if (val < 0)
               	val = 0;
               if (val > 255)
               	val = 255;
					x++;
               image_it.Set4((val >> 4));
            } while (image_it.Increment());
            image_it.NextRow();
            row_count++;
            event_handler->Set(row_count);
         }
         for (int n = 0; n < (data->GetHeight() - (size * 2)); n++)
         {
         	for (int x = 0; x < size; x++)
            {
               int val = (unsigned char)image_it;
               val = ((val << 4) + val) + offset;
               if (val < 0)
               	val = 0;
               if (val > 255)
               	val = 255;
					image_it.Set4((val >> 4));
               image_it.Increment();
            }
            image_it.SetRowEnd();
         	for (int x = 0; x < size; x++)
            {
               int val = (unsigned char)image_it;
               val = ((val << 4) + val) - offset;
               if (val < 0)
               	val = 0;
               if (val > 255)
               	val = 255;
					image_it.Set4((val >> 4));
               image_it.Decrement();
            }
            image_it.NextRow();
            row_count++;
            event_handler->Set(row_count);
         }
         image_it.SetLastRow();
         for (int n = 0; n < size; n++)
         {
         	int x = 0;
            do
            {
            	int val = (unsigned char)image_it;
               val = ((val << 4) + val);
            	if (x < n)
               	val += offset;
               else if ((x > n) && (x < (data->GetWidth() - n - 1)))
               	val -= offset;
               else if (x > (data->GetWidth() - n - 1))
               	val -= offset;
               if (val < 0)
               	val = 0;
               if (val > 255)
               	val = 255;
					x++;
               image_it.Set4((val >> 4));
            } while (image_it.Increment());
            image_it.ResetRow();
            image_it.DecRow();
            row_count++;
            event_handler->Set(row_count);
         }
      	event_handler->Stop();
         return data;
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
	         for (int n = 0; n < size; n++)
   	      {
      	   	int x = 0;
         	   do
            	{
               	unsigned char val;
                  if ((unsigned char)image_it1)
                  	val = 1;
                  else
                  	val = 0;
	            	int red = palette[val].red;
   	         	int green = palette[val].green;
      	      	int blue = palette[val].blue;
         	   	if (x < n)
            	   {
               		red += offset;
                  	green += offset;
	                  blue += offset;
   	            }
      	         else if ((x > n) && (x < (data->GetWidth() - n - 1)))
         	      {
            	   	red += offset;
               	   green += offset;
                  	blue += offset;
	               }
   	            else if (x > (data->GetWidth() - n - 1))
      	         {
         	      	red -= offset;
            	      green -= offset;
               	   blue -= offset;
	               }
   	            if (red < 0)
      	         	red = 0;
         	      if (red > 255)
            	   	red = 255;
               	if (green < 0)
	               	green = 0;
   	            if (green > 255)
      	         	green = 255;
         	      if (blue < 0)
            	   	blue = 0;
               	if (blue > 255)
	               	blue = 255;
						x++;
      	         image_it2.Set(blue, green, red);
                  image_it2.Increment();
         	   } while (image_it1.Increment());
            	image_it2.NextRow();
            	image_it1.NextRow();
	            row_count++;
   	         event_handler->Set(row_count);
	         }
   	      for (int n = 0; n < (data->GetHeight() - (size * 2)); n++)
      	   {
               int x;
         		for (x = 0; x < size; x++)
	            {
               	unsigned char val;
                  if ((unsigned char)image_it1)
                  	val = 1;
                  else
                  	val = 0;
   	            int red = palette[val].red + offset;
      	      	int green = palette[val].green + offset;
         	   	int blue = palette[val].blue + offset;
            	   if (red < 0)
               		red = 0;
	               if (red > 255)
   	            	red = 255;
      	         if (green < 0)
         	      	green = 0;
            	   if (green > 255)
               		green = 255;
	               if (blue < 0)
   	            	blue = 0;
      	         if (blue > 255)
         	      	blue = 255;
						image_it2.Set(blue, green, red);
               	image_it2.Increment();
                  image_it1.Increment();
	            }
               for (; x < (data->GetWidth() - size); x++)
   				{
               	unsigned char val;
                  if ((unsigned char)image_it1)
                  	val = 1;
                  else
                  	val = 0;
                  image_it2.Set(palette[val].blue, palette[val].green,
                  	palette[val].red);
                  image_it1.Increment();
                  image_it2.Increment();
               }
	     	   	for (; x < data->GetWidth(); x++)
         	   {
               	unsigned char val;
                  if ((unsigned char)image_it1)
                  	val = 1;
                  else
                  	val = 0;
            	   int red = palette[val].red - offset;
            		int green = palette[val].green - offset;
	            	int blue = palette[val].blue - offset;
   	            if (red < 0)
      	         	red = 0;
         	      if (red > 255)
            	   	red = 255;
               	if (green < 0)
	               	green = 0;
   	            if (green > 255)
      	         	green = 255;
         	      if (blue < 0)
            	   	blue = 0;
               	if (blue > 255)
	               	blue = 255;
						image_it2.Set(blue, green, red);
      	         image_it2.Increment();
      	         image_it1.Increment();
         	   }
            	image_it2.NextRow();
            	image_it1.NextRow();
	            row_count++;
   	         event_handler->Set(row_count);
	         }
   	      image_it2.SetLastRow();
   	      image_it1.SetLastRow();
      	   for (int n = 0; n < size; n++)
         	{
	         	int x = 0;
   	         do
      	      {
               	unsigned char val;
                  if ((unsigned char)image_it1)
                  	val = 1;
                  else
                  	val = 0;
         	   	int red = palette[val].red;
            		int green = palette[val].green;
            		int blue = palette[val].blue;
	            	if (x < n)
   	            {
      	         	red += offset;
         	         green += offset;
            	      blue += offset;
               	}
	               else if ((x > n) && (x < (data->GetWidth() - n - 1)))
   	            {
      	         	red -= offset;
         	         green -= offset;
            	      blue -= offset;
               	}
	               else if (x > (data->GetWidth() - n - 1))
   	            {
      	         	red -= offset;
         	         green -= offset;
            	      blue -= offset;
               	}
	               if (red < 0)
   	            	red = 0;
      	         if (red > 255)
         	      	red = 255;
            	   if (green < 0)
               		green = 0;
	               if (green > 255)
   	            	green = 255;
      	         if (blue < 0)
         	      	blue = 0;
            	   if (blue > 255)
               		blue = 255;
						x++;
   	            image_it2.Set(blue, green, red);
                  image_it2.Increment();
      	      } while (image_it1.Increment());
         	   image_it2.ResetRow();
            	image_it2.DecRow();
         	   image_it1.ResetRow();
            	image_it1.DecRow();
	            row_count++;
   	         event_handler->Set(row_count);
	         }
	      	event_handler->Stop();
            return new_data;
         }
         else
         {
	      	G42ImageData * new_data = new G42ImageData(data->GetData(),
			     	data->GetWidth(), data->GetHeight(), 24, 0);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
				G42LockedID1BitIterator image_it1(data);
				G42ImageData24BitIterator image_it2(new_data);
	         for (int n = 0; n < size; n++)
   	      {
      	   	int x = 0;
         	   do
            	{
               	unsigned char val = (unsigned char)image_it1;
	            	int red = val;
   	         	int green = val;
      	      	int blue = val;
         	   	if (x < n)
            	   {
               		red += offset;
                  	green += offset;
	                  blue += offset;
   	            }
      	         else if ((x > n) && (x < (data->GetWidth() - n - 1)))
         	      {
            	   	red += offset;
               	   green += offset;
                  	blue += offset;
	               }
   	            else if (x > (data->GetWidth() - n - 1))
      	         {
         	      	red -= offset;
            	      green -= offset;
               	   blue -= offset;
	               }
   	            if (red < 0)
      	         	red = 0;
         	      if (red > 255)
            	   	red = 255;
               	if (green < 0)
	               	green = 0;
   	            if (green > 255)
      	         	green = 255;
         	      if (blue < 0)
            	   	blue = 0;
               	if (blue > 255)
	               	blue = 255;
						x++;
      	         image_it2.Set(blue, green, red);
                  image_it2.Increment();
         	   } while (image_it1.Increment());
            	image_it2.NextRow();
            	image_it1.NextRow();
	            row_count++;
   	         event_handler->Set(row_count);
	         }
   	      for (int n = 0; n < (data->GetHeight() - (size * 2)); n++)
      	   {
               int x;
         		for (x = 0; x < size; x++)
	            {
               	unsigned char val = (unsigned char)image_it1;
   	            int red = val + offset;
      	      	int green = val + offset;
         	   	int blue = val + offset;
            	   if (red < 0)
               		red = 0;
	               if (red > 255)
   	            	red = 255;
      	         if (green < 0)
         	      	green = 0;
            	   if (green > 255)
               		green = 255;
	               if (blue < 0)
   	            	blue = 0;
      	         if (blue > 255)
         	      	blue = 255;
						image_it2.Set(blue, green, red);
               	image_it2.Increment();
                  image_it1.Increment();
	            }
               for (; x < (data->GetWidth() - size); x++)
   				{
               	unsigned char val = (unsigned char)image_it1;
                  image_it2.Set(val, val, val);
                  image_it1.Increment();
                  image_it2.Increment();
               }
	     	   	for (; x < data->GetWidth(); x++)
         	   {
               	unsigned char val = (unsigned char)image_it1;
            	   int red = val - offset;
            		int green = val - offset;
	            	int blue = val - offset;
   	            if (red < 0)
      	         	red = 0;
         	      if (red > 255)
            	   	red = 255;
               	if (green < 0)
	               	green = 0;
   	            if (green > 255)
      	         	green = 255;
         	      if (blue < 0)
            	   	blue = 0;
               	if (blue > 255)
	               	blue = 255;
						image_it2.Set(blue, green, red);
      	         image_it2.Increment();
      	         image_it1.Increment();
         	   }
            	image_it2.NextRow();
            	image_it1.NextRow();
	            row_count++;
   	         event_handler->Set(row_count);
	         }
   	      image_it2.SetLastRow();
   	      image_it1.SetLastRow();
      	   for (int n = 0; n < size; n++)
         	{
	         	int x = 0;
   	         do
      	      {
               	unsigned char val = (unsigned char)image_it1;
         	   	int red = val;
            		int green = val;
            		int blue = val;
	            	if (x < n)
   	            {
      	         	red += offset;
         	         green += offset;
            	      blue += offset;
               	}
	               else if ((x > n) && (x < (data->GetWidth() - n - 1)))
   	            {
      	         	red -= offset;
         	         green -= offset;
            	      blue -= offset;
               	}
	               else if (x > (data->GetWidth() - n - 1))
   	            {
      	         	red -= offset;
         	         green -= offset;
            	      blue -= offset;
               	}
	               if (red < 0)
   	            	red = 0;
      	         if (red > 255)
         	      	red = 255;
            	   if (green < 0)
               		green = 0;
	               if (green > 255)
   	            	green = 255;
      	         if (blue < 0)
         	      	blue = 0;
            	   if (blue > 255)
               		blue = 255;
						x++;
   	            image_it2.Set(blue, green, red);
                  image_it2.Increment();
      	      } while (image_it1.Increment());
         	   image_it2.ResetRow();
            	image_it2.DecRow();
         	   image_it1.ResetRow();
            	image_it1.DecRow();
	            row_count++;
   	         event_handler->Set(row_count);
	         }
	      	event_handler->Stop();
            return new_data;
         }
      }
	}
   return data;
}
void
G42Image::AddBorder(G42Color color, int width)
{
	G42ImageData * new_data = Data;
	if (Data)
   {
   	if (Mask)
      {
      	delete Mask;
         Mask = 0;
      }
		new_data = ::AddBorder(Data, color, width, GetEventHandler());
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
G42Image::Buttonize(G42Color top_color, G42Color bottom_color, G42ButtonType type,
	int size, bool has_color)
{
	G42ImageData * new_data = Data;
	bool invert = false;
	if (type != Raised)
		invert = true;
	if (Data)
   {
   	if (Mask)
      {
      	delete Mask;
      	Mask = 0;
      }
		new_data = ::Buttonize(Data, top_color, bottom_color, size, invert,
			has_color, GetEventHandler());
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
