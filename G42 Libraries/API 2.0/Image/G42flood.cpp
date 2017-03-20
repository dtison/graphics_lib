#include "g42idata.h"
#include "g42iter.h"
#include "g42image.h"
#include "g42iview.h"
G42ImageData *
ButtonBar(G42ImageData * data, G42Color top_color, G42Color bottom_color,
	G42Color middle_color, int width, int rows, int columns)
{
	switch (data->GetDepth())
   {
		case 32:
   	case 24:
      	break;
      case 8:
      	break;
      case 4:
      {
      	G42ImageData * new_data = new G42ImageData(data->GetData(),
         	(data->GetWidth() * columns), (data->GetHeight() * rows), 4, 4);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42Color * new_palette = new G42Color [4];
         new_palette[0].red = top_color.red;
         new_palette[0].green = top_color.green;
         new_palette[0].blue = top_color.blue;
         new_palette[1].red = middle_color.red;
         new_palette[1].green = middle_color.green;
         new_palette[1].blue = middle_color.blue;
         new_palette[2].red = bottom_color.red;
         new_palette[2].green = bottom_color.green;
         new_palette[2].blue = bottom_color.blue;
         new_palette[3].red = 0;
         new_palette[3].green = 0;
         new_palette[3].blue = 0;
         new_data->SetPalette(new_palette);
         delete [] new_palette;
      	G42ImageData4BitIterator image_it(new_data);
         do
         {
				image_it.Set4(1);
            if (!image_it.Increment())
            {
            	if (!image_it.NextRow())
               	break;
            }
         } while (1);
         for (int x = 0; x < rows; x++)
         {
         	uint32 top = x * data->GetHeight();
            uint32 bottom = top + data->GetHeight() - 1;
         	for (int y = 0; y < columns; y++)
            {
            	uint32 left = y * data->GetWidth();
            	uint32 right = left + data->GetWidth() - 1;
               image_it.SetArea(left, top, right, bottom);
               image_it.Reset();
               int n;
		     	   for (n = 0; n < (width - 1); n++)
   		     	{
               	int i = 0;
      		  		do
         		   {
	         	   	if (i < n)
			        			image_it.Set4(0);
	      	         else if ((i > n) && (i < (data->GetWidth() - n - 1)))
                     	image_it.Set4(0);
	   	            else if (i > (data->GetWidth() - n - 1))
                     	image_it.Set4(2);
                     else
                     	image_it.Set4(3);
                     i++;
	        	   	} while (image_it.Increment());
	   	        	image_it.NextRow();
   	   	   }
              	int i = 0;
     		  		do
        		   {
         	   	if (i < n)
		        			image_it.Set4(0);
      	         else if ((i > n) && (i < (data->GetWidth() - n - 1)))
                    	image_it.Set4(3);
   	            else if (i > (data->GetWidth() - n - 1))
                    	image_it.Set4(2);
                  else
                    	image_it.Set4(3);
                    i++;
        	   	} while (image_it.Increment());
   	        	image_it.NextRow();
               n++;
               for (; n < (data->GetHeight() - width); n++)
               {
               	for (int i = 0; i < (width - 1); i++, image_it.Increment())
                  	image_it.Set4(0);
                  image_it.Set4(3);
                  image_it.SetRowEnd();
               	for (int i = 0; i < (width - 1); i++, image_it.Decrement())
                  	image_it.Set4(2);
                  image_it.Set4(3);
                  image_it.NextRow();
               }
               image_it.SetLastRow();
		     	   for (n = 0; n < (width - 1); n++)
   		     	{
               	int i = 0;
      		  		do
         		   {
	         	   	if (i < n)
			        			image_it.Set4(0);
	      	         else if ((i > n) && (i < (data->GetWidth() - n - 1)))
                     	image_it.Set4(2);
	   	            else if (i > (data->GetWidth() - n - 1))
                     	image_it.Set4(2);
                     else
                     	image_it.Set4(3);
                     i++;
	        	   	} while (image_it.Increment());
	   	        	image_it.ResetRow();
                  image_it.DecRow();
   	   	   }
					i = 0;
     		  		do
        		   {
         	   	if (i < n)
		        			image_it.Set4(0);
      	         else if ((i > n) && (i < (data->GetWidth() - n - 1)))
                    	image_it.Set4(3);
   	            else if (i > (data->GetWidth() - n - 1))
                    	image_it.Set4(2);
                  else
                    	image_it.Set4(3);
                    i++;
        	   	} while (image_it.Increment());
            }
         }
      	return new_data;
      }
      case 1:
      	break;
   }
	return data;
}
G42ImageData *
FloodImage(G42ImageData * data, G42Color top_color, G42Color bottom_color,
	G42Color middle_color, int width, int rows, int columns)
{
	if (rows)
   	return ButtonBar(data, top_color, middle_color, bottom_color, width, rows, columns);
   switch (data->GetDepth())
   {
   	case 32:
   	case 24:
      {
      	G42ImageData24BitIterator image_it(data);
         do
         {
         	image_it.Set(top_color.blue, top_color.green, top_color.red);
            if (!image_it.Increment())
            {
            	if (!image_it.NextRow())
               	break;
            }
         } while (1);
         return data;
      }
      case 8:
      {
      	G42ImageData * new_data = new G42ImageData(data->GetData(),
         	data->GetWidth(), data->GetHeight(), 8, 1);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42Color * new_palette = new G42Color [1];
         new_palette[0].red = top_color.red;
         new_palette[0].green = top_color.green;
         new_palette[0].blue = top_color.blue;
         new_data->SetPalette(new_palette);
         delete [] new_palette;
      	G42ImageData8BitIterator image_it(new_data);
         do
         {
         	image_it.Set(0);
            {
            	if (!image_it.NextRow())
               	break;
            }
         } while (1);
         return new_data;
      }
      case 4:
      {
      	G42ImageData * new_data = new G42ImageData(data->GetData(),
         	data->GetWidth(), data->GetHeight(), 4, 1);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42Color * new_palette = new G42Color [1];
         new_palette[0].red = top_color.red;
         new_palette[0].green = top_color.green;
         new_palette[0].blue = top_color.blue;
         new_data->SetPalette(new_palette);
         delete [] new_palette;
      	G42ImageData4BitIterator image_it(new_data);
         do
         {
         	image_it.Set4(0);
            {
            	if (!image_it.NextRow())
               	break;
            }
         } while (1);
         return new_data;
      }
      case 1:
      {
      	G42ImageData * new_data = new G42ImageData(data->GetData(),
         	data->GetWidth(), data->GetHeight(), 1, 1);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42Color * new_palette = new G42Color [1];
         new_palette[0].red = top_color.red;
         new_palette[0].green = top_color.green;
         new_palette[0].blue = top_color.blue;
         new_data->SetPalette(new_palette);
         delete [] new_palette;
      	G42ImageData1BitIterator image_it(new_data);
         do
         {
         	image_it.Set8(0);
            {
            	if (!image_it.NextRow())
               	break;
            }
         } while (1);
         return new_data;
      }
   }
   return data;
}
void
G42Image::FloodFill(G42Color top_color, G42Color middle_color, G42Color bottom_color,
	int width, int rows, int columns)
{
	G42ImageData * new_data = Data;
	if (Data)
		new_data = ::FloodImage(Data, top_color, middle_color, bottom_color, width, rows, columns);
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
