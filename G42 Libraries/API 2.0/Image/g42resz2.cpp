#include "g42idata.h"
#include "g42iter.h"
#include "g42event.h"
void
ExpandImage(G42ImageData * data, int left, int top, int right, int bottom,
	G42ImageData * new_data, int left2, int top2, int right2, int bottom2,
   G42EventUpdateHandler * event_handler)
{
	int width = right - left + 1;
	int height = bottom - top + 1;
	int width2 = right2 - left2 + 1;
	int height2 = bottom2 - top2 + 1;
   double row = 0.0, column = 0.0;
   double pwidth = ((double)width - 1.0)/((double)width2 - 1.0);
   double pheight = ((double)height - 1.0)/((double)height2 - 1.0);
   double xpos = 0.0;
   double ypos = 0.0;
	event_handler->SetDenominator((int32)height2);
   int32 row_count = 0L;
   event_handler->Start();
   event_handler->Set(0L);
	switch (data->GetDepth())
   {
		case 32:
   	case 24:
      {
         G42LockedID24BitIterator image_it1(data);
         G42ImageData24BitIterator image_it2(new_data);
			int32 red_ref1 = image_it1.GetRed32();
			int32 green_ref1 = image_it1.GetGreen32();
			int32 blue_ref1 = image_it1.GetBlue32();
         image_it1.Increment();
			int32 red_ref2 = image_it1.GetRed32();
			int32 green_ref2 = image_it1.GetGreen32();
			int32 blue_ref2 = image_it1.GetBlue32();
			image_it1.IncRow();
			int32 red_ref4 = image_it1.GetRed32();
			int32 green_ref4 = image_it1.GetGreen32();
			int32 blue_ref4 = image_it1.GetBlue32();
         image_it1.Decrement();
			int32 red_ref3 = image_it1.GetRed32();
			int32 green_ref3 = image_it1.GetGreen32();
			int32 blue_ref3 = image_it1.GetBlue32();
         image_it1.DecRow();
         int32 y_offset = 0L;
         do
         {
           	int32 x_offset = (int32)((xpos - column) * 1024.0);
           	int32 tblue1 = (((blue_ref1 << 10) +
              	((blue_ref2 - blue_ref1) * x_offset)) >> 10);
           	int32 tblue2 = (((blue_ref3 << 10) +
              	((blue_ref4 - blue_ref3) * x_offset)) >> 10);
           	int32 tgreen1 = (((green_ref1 << 10) +
              	((green_ref2 - green_ref1) * x_offset)) >> 10);
           	int32 tgreen2 = (((green_ref3 << 10) +
              	((green_ref4 - green_ref3) * x_offset)) >> 10);
           	int32 tred1 = (((red_ref1 << 10) +
              	((red_ref2 - red_ref1) * x_offset)) >> 10);
           	int32 tred2 = (((red_ref3 << 10) +
              	((red_ref4 - red_ref3) * x_offset)) >> 10);
            int32 blue = (((tblue1 << 10)
              	+ ((tblue2 - tblue1) * y_offset)) >> 10);
            int32 green = (((tgreen1 << 10)
              	+ ((tgreen2 - tgreen1) * y_offset)) >> 10);
            int32 red = (((tred1 << 10)
              	+ ((tred2 - tred1) * y_offset)) >> 10);
            image_it2.Set(blue, green, red);
            xpos += pwidth;
            while (xpos >= (column + 1.0))
            {
              	image_it1.Increment();
              	column += 1.0;
               red_ref1 = red_ref2;
               green_ref1 = green_ref2;
               blue_ref1 = blue_ref2;
               red_ref3 = red_ref4;
               green_ref3 = green_ref4;
               blue_ref3 = blue_ref4;
               image_it1.Increment();
               red_ref2 = image_it1.GetRed32();
               green_ref2 = image_it1.GetGreen32();
               blue_ref2 = image_it1.GetBlue32();
               image_it1.IncRow();
               red_ref4 = image_it1.GetRed32();
               green_ref4 = image_it1.GetGreen32();
               blue_ref4 = image_it1.GetBlue32();
               image_it1.Decrement();
               image_it1.DecRow();
            }
            if (!image_it2.Increment())
            {
					if (!image_it2.NextRow())
                	break;
               row_count++;
               event_handler->Set(row_count);
					image_it1.ResetRow();
               column = 0.0;
               xpos = 0.0;
               ypos += pheight;
               while (ypos >= (row + 1.0))
               {
                 	image_it1.NextRow();
                  row += 1.0;
					}
  		         y_offset = (int32)((ypos - row) * 1024.0);
               red_ref1 = image_it1.GetRed32();
  	            green_ref1 = image_it1.GetGreen32();
     	         blue_ref1 = image_it1.GetBlue32();
					image_it1.Increment();
               red_ref2 = image_it1.GetRed32();
  	            green_ref2 = image_it1.GetGreen32();
    	         blue_ref2 = image_it1.GetBlue32();
               image_it1.IncRow();
               red_ref4 = image_it1.GetRed32();
 	            green_ref4 = image_it1.GetGreen32();
     	         blue_ref4 = image_it1.GetBlue32();
               image_it1.Decrement();
               red_ref3 = image_it1.GetRed32();
  	            green_ref3 = image_it1.GetGreen32();
     	         blue_ref3 = image_it1.GetBlue32();
               image_it1.DecRow();
            }
         } while (1);
      	break;
      }
      case 8:
      {
      	if (data->GetNumPalette() != 0)
         {
				G42Color * palette = data->GetPalette();
            G42LockedID8BitIterator image_it1(data);
            G42ImageData24BitIterator image_it2(new_data);
            unsigned char val = (unsigned char)image_it1;
				int32 red_ref1 = palette[val].red;
				int32 green_ref1 = palette[val].green;
				int32 blue_ref1 = palette[val].blue;
            image_it1.Increment();
            val = (unsigned char)image_it1;
				int32 red_ref2 = palette[val].red;
				int32 green_ref2 = palette[val].green;
				int32 blue_ref2 = palette[val].blue;
				image_it1.IncRow();
            val = (unsigned char)image_it1;
				int32 red_ref4 = palette[val].red;
				int32 green_ref4 = palette[val].green;
				int32 blue_ref4 = palette[val].blue;
            image_it1.Decrement();
            val = (unsigned char)image_it1;
				int32 red_ref3 = palette[val].red;
				int32 green_ref3 = palette[val].green;
				int32 blue_ref3 = palette[val].blue;
            image_it1.DecRow();
            int32 y_offset = 0L;
            do
            {
            	int32 x_offset = (int32)((xpos - column) * 1024.0);
             	int32 tblue1 = (((blue_ref1 << 10) +
               	((blue_ref2 - blue_ref1) * x_offset)) >> 10);
             	int32 tblue2 = (((blue_ref3 << 10) +
               	((blue_ref4 - blue_ref3) * x_offset)) >> 10);
             	int32 tgreen1 = (((green_ref1 << 10) +
               	((green_ref2 - green_ref1) * x_offset)) >> 10);
             	int32 tgreen2 = (((green_ref3 << 10) +
               	((green_ref4 - green_ref3) * x_offset)) >> 10);
             	int32 tred1 = (((red_ref1 << 10) +
               	((red_ref2 - red_ref1) * x_offset)) >> 10);
             	int32 tred2 = (((red_ref3 << 10) +
               	((red_ref4 - red_ref3) * x_offset)) >> 10);
               int32 blue = (((tblue1 << 10)
               	+ ((tblue2 - tblue1) * y_offset)) >> 10);
               int32 green = (((tgreen1 << 10)
               	+ ((tgreen2 - tgreen1) * y_offset)) >> 10);
               int32 red = (((tred1 << 10)
               	+ ((tred2 - tred1) * y_offset)) >> 10);
               image_it2.Set(blue, green, red);
               xpos += pwidth;
               while (xpos >= (column + 1.0))
               {
               	image_it1.Increment();
               	column += 1.0;
                  red_ref1 = red_ref2;
                  green_ref1 = green_ref2;
                  blue_ref1 = blue_ref2;
                  red_ref3 = red_ref4;
                  green_ref3 = green_ref4;
                  blue_ref3 = blue_ref4;
                  image_it1.Increment();
                  unsigned char val = (unsigned char)image_it1;
                  red_ref2 = palette[val].red;
                  green_ref2 = palette[val].green;
                  blue_ref2 = palette[val].blue;
                  image_it1.IncRow();
                  val = (unsigned char)image_it1;
                  red_ref4 = palette[val].red;
                  green_ref4 = palette[val].green;
                  blue_ref4 = palette[val].blue;
                  image_it1.Decrement();
                  image_it1.DecRow();
               }
               if (!image_it2.Increment())
               {
						if (!image_it2.NextRow())
                  	break;
                  row_count++;
                  event_handler->Set(row_count);
						image_it1.ResetRow();
                  column = 0.0;
                  xpos = 0.0;
                  ypos += pheight;
                  while (ypos >= (row + 1.0))
                  {
                  	image_it1.NextRow();
                     row += 1.0;
						}
   		         y_offset = (int32)((ypos - row) * 1024.0);
                  unsigned char val = (unsigned char)image_it1;
  	               red_ref1 = palette[val].red;
     	            green_ref1 = palette[val].green;
        	         blue_ref1 = palette[val].blue;
						image_it1.Increment();
                  val = (unsigned char)image_it1;
  	               red_ref2 = palette[val].red;
     	            green_ref2 = palette[val].green;
       	         blue_ref2 = palette[val].blue;
                  image_it1.IncRow();
                  val = (unsigned char)image_it1;
  	               red_ref4 = palette[val].red;
     	            green_ref4 = palette[val].green;
        	         blue_ref4 = palette[val].blue;
                  image_it1.Decrement();
                  val = (unsigned char)image_it1;
  	               red_ref3 = palette[val].red;
     	            green_ref3 = palette[val].green;
        	         blue_ref3 = palette[val].blue;
                  image_it1.DecRow();
               }
            } while (1);
          	break;
         }
         G42LockedID8BitIterator image_it1(data);
         G42ImageData8BitIterator image_it2(new_data);
         int32 val_ref1 = (int32)(unsigned char)image_it1;
         image_it1.Increment();
         int32 val_ref2 = (int32)(unsigned char)image_it1;
			image_it1.IncRow();
         int32 val_ref4 = (int32)(unsigned char)image_it1;
         image_it1.Decrement();
         int32 val_ref3 = (int32)(unsigned char)image_it1;
         image_it1.DecRow();
         int32 y_offset = 0L;
         do
         {
          	int32 x_offset = (int32)((xpos - column) * 1024.0);
           	int32 tval1 = (((val_ref1 << 10) +
              	((val_ref2 - val_ref1) * x_offset)) >> 10);
           	int32 tval2 = (((val_ref3 << 10) +
             	((val_ref4 - val_ref3) * x_offset)) >> 10);
            int32 val = (((tval1 << 10)
              	+ ((tval2 - tval1) * y_offset)) >> 10);
            image_it2.Set(val);
            xpos += pwidth;
            while (xpos >= (column + 1.0))
            {
             	image_it1.Increment();
              	column += 1.0;
               val_ref1 = val_ref2;
               val_ref3 = val_ref4;
               image_it1.Increment();
               val_ref2 = (int32)(unsigned char)image_it1;
               image_it1.IncRow();
               val_ref4 = (int32)(unsigned char)image_it1;
               image_it1.Decrement();
               image_it1.DecRow();
            }
            if (!image_it2.Increment())
            {
					if (!image_it2.NextRow())
                 	break;
               row_count++;
               event_handler->Set(row_count);
					image_it1.ResetRow();
               column = 0.0;
               xpos = 0.0;
               ypos += pheight;
               while (ypos >= (row + 1.0))
               {
                	image_it1.NextRow();
                  row += 1.0;
					}
  		         y_offset = (int32)((ypos - row) * 1024.0);
               val_ref1 = (int32)(unsigned char)image_it1;
 					image_it1.Increment();
               val_ref2 = (int32)(unsigned char)image_it1;
               image_it1.IncRow();
               val_ref4 = (int32)(unsigned char)image_it1;
               image_it1.Decrement();
               val_ref3 = (int32)(unsigned char)image_it1;
               image_it1.DecRow();
            }
         } while (1);
         break;
      }
      case 4:
      {
      	if (data->GetNumPalette() != 0)
         {
				G42Color * palette = data->GetPalette();
            G42LockedID4BitIterator image_it1(data);
            G42ImageData24BitIterator image_it2(new_data);
            unsigned char val = (unsigned char)image_it1;
				int32 red_ref1 = palette[val].red;
				int32 green_ref1 = palette[val].green;
				int32 blue_ref1 = palette[val].blue;
            image_it1.Increment();
            val = (unsigned char)image_it1;
				int32 red_ref2 = palette[val].red;
				int32 green_ref2 = palette[val].green;
				int32 blue_ref2 = palette[val].blue;
				image_it1.IncRow();
            val = (unsigned char)image_it1;
				int32 red_ref4 = palette[val].red;
				int32 green_ref4 = palette[val].green;
				int32 blue_ref4 = palette[val].blue;
            image_it1.Decrement();
            val = (unsigned char)image_it1;
				int32 red_ref3 = palette[val].red;
				int32 green_ref3 = palette[val].green;
				int32 blue_ref3 = palette[val].blue;
            image_it1.DecRow();
            int32 y_offset = 0L;
            do
            {
            	int32 x_offset = (int32)((xpos - column) * 1024.0);
             	int32 tblue1 = (((blue_ref1 << 10) +
               	((blue_ref2 - blue_ref1) * x_offset)) >> 10);
             	int32 tblue2 = (((blue_ref3 << 10) +
               	((blue_ref4 - blue_ref3) * x_offset)) >> 10);
             	int32 tgreen1 = (((green_ref1 << 10) +
               	((green_ref2 - green_ref1) * x_offset)) >> 10);
             	int32 tgreen2 = (((green_ref3 << 10) +
               	((green_ref4 - green_ref3) * x_offset)) >> 10);
             	int32 tred1 = (((red_ref1 << 10) +
               	((red_ref2 - red_ref1) * x_offset)) >> 10);
             	int32 tred2 = (((red_ref3 << 10) +
               	((red_ref4 - red_ref3) * x_offset)) >> 10);
               int32 blue = (((tblue1 << 10)
               	+ ((tblue2 - tblue1) * y_offset)) >> 10);
               int32 green = (((tgreen1 << 10)
               	+ ((tgreen2 - tgreen1) * y_offset)) >> 10);
               int32 red = (((tred1 << 10)
               	+ ((tred2 - tred1) * y_offset)) >> 10);
               image_it2.Set(blue, green, red);
               xpos += pwidth;
               while (xpos >= (column + 1.0))
               {
               	image_it1.Increment();
               	column += 1.0;
                  red_ref1 = red_ref2;
                  green_ref1 = green_ref2;
                  blue_ref1 = blue_ref2;
                  red_ref3 = red_ref4;
                  green_ref3 = green_ref4;
                  blue_ref3 = blue_ref4;
                  image_it1.Increment();
                  unsigned char val = (unsigned char)image_it1;
                  red_ref2 = palette[val].red;
                  green_ref2 = palette[val].green;
                  blue_ref2 = palette[val].blue;
                  image_it1.IncRow();
                  val = (unsigned char)image_it1;
                  red_ref4 = palette[val].red;
                  green_ref4 = palette[val].green;
                  blue_ref4 = palette[val].blue;
                  image_it1.Decrement();
                  image_it1.DecRow();
               }
               if (!image_it2.Increment())
               {
						if (!image_it2.NextRow())
                  	break;
                  row_count++;
                  event_handler->Set(row_count);
						image_it1.ResetRow();
                  column = 0.0;
                  xpos = 0.0;
                  ypos += pheight;
                  while (ypos >= (row + 1.0))
                  {
                  	image_it1.NextRow();
                     row += 1.0;
						}
   		         y_offset = (int32)((ypos - row) * 1024.0);
                  unsigned char val = (unsigned char)image_it1;
  	               red_ref1 = palette[val].red;
     	            green_ref1 = palette[val].green;
        	         blue_ref1 = palette[val].blue;
						image_it1.Increment();
                  val = (unsigned char)image_it1;
  	               red_ref2 = palette[val].red;
     	            green_ref2 = palette[val].green;
       	         blue_ref2 = palette[val].blue;
                  image_it1.IncRow();
                  val = (unsigned char)image_it1;
  	               red_ref4 = palette[val].red;
     	            green_ref4 = palette[val].green;
        	         blue_ref4 = palette[val].blue;
                  image_it1.Decrement();
                  val = (unsigned char)image_it1;
  	               red_ref3 = palette[val].red;
     	            green_ref3 = palette[val].green;
        	         blue_ref3 = palette[val].blue;
                  image_it1.DecRow();
               }
            } while (1);
          	break;
         }
         G42LockedID4BitIterator image_it1(data);
         G42ImageData8BitIterator image_it2(new_data);
         int32 val_ref1 = (int32)(unsigned char)image_it1;
         val_ref1 = ((val_ref1 << 4) + val_ref1);
         image_it1.Increment();
         int32 val_ref2 = (int32)(unsigned char)image_it1;
         val_ref2 = ((val_ref2 << 4) + val_ref2);
			image_it1.IncRow();
         int32 val_ref4 = (int32)(unsigned char)image_it1;
         val_ref4 = ((val_ref4 << 4) + val_ref4);
         image_it1.Decrement();
         int32 val_ref3 = (int32)(unsigned char)image_it1;
         val_ref3 = ((val_ref3 << 4) + val_ref3);
         image_it1.DecRow();
         int32 y_offset = 0L;
         do
         {
          	int32 x_offset = (int32)((xpos - column) * 1024.0);
           	int32 tval1 = (((val_ref1 << 10) +
              	((val_ref2 - val_ref1) * x_offset)) >> 10);
           	int32 tval2 = (((val_ref3 << 10) +
             	((val_ref4 - val_ref3) * x_offset)) >> 10);
            int32 val = (((tval1 << 10)
              	+ ((tval2 - tval1) * y_offset)) >> 10);
            image_it2.Set(val);
            xpos += pwidth;
            while (xpos >= (column + 1.0))
            {
             	image_it1.Increment();
              	column += 1.0;
               val_ref1 = val_ref2;
               val_ref3 = val_ref4;
               image_it1.Increment();
               val_ref2 = (int32)(unsigned char)image_it1;
		         val_ref2 = ((val_ref2 << 4) + val_ref2);
               image_it1.IncRow();
               val_ref4 = (int32)(unsigned char)image_it1;
		         val_ref4 = ((val_ref4 << 4) + val_ref4);
               image_it1.Decrement();
               image_it1.DecRow();
            }
            if (!image_it2.Increment())
            {
					if (!image_it2.NextRow())
                 	break;
               row_count++;
               event_handler->Set(row_count);
					image_it1.ResetRow();
               column = 0.0;
               xpos = 0.0;
               ypos += pheight;
               while (ypos >= (row + 1.0))
               {
                	image_it1.NextRow();
                  row += 1.0;
					}
  		         y_offset = (int32)((ypos - row) * 1024.0);
               val_ref1 = (int32)(unsigned char)image_it1;
		         val_ref1 = ((val_ref1 << 4) + val_ref1);
 					image_it1.Increment();
               val_ref2 = (int32)(unsigned char)image_it1;
		         val_ref2 = ((val_ref2 << 4) + val_ref2);
               image_it1.IncRow();
               val_ref4 = (int32)(unsigned char)image_it1;
		         val_ref4 = ((val_ref4 << 4) + val_ref4);
               image_it1.Decrement();
               val_ref3 = (int32)(unsigned char)image_it1;
		         val_ref3 = ((val_ref3 << 4) + val_ref3);
               image_it1.DecRow();
            }
         } while (1);
         break;
		}
      case 1:
      {
      	if (data->GetNumPalette() != 0)
         {
				G42Color * palette = data->GetPalette();
            G42LockedID1BitIterator image_it1(data);
            G42ImageData24BitIterator image_it2(new_data);
            unsigned char val = (unsigned char)image_it1;
            if (val)
            	val = 1;
				int32 red_ref1 = palette[val].red;
				int32 green_ref1 = palette[val].green;
				int32 blue_ref1 = palette[val].blue;
            image_it1.Increment();
            val = (unsigned char)image_it1;
            if (val)
            	val = 1;
				int32 red_ref2 = palette[val].red;
				int32 green_ref2 = palette[val].green;
				int32 blue_ref2 = palette[val].blue;
				image_it1.IncRow();
            val = (unsigned char)image_it1;
            if (val)
            	val = 1;
				int32 red_ref4 = palette[val].red;
				int32 green_ref4 = palette[val].green;
				int32 blue_ref4 = palette[val].blue;
            image_it1.Decrement();
            val = (unsigned char)image_it1;
            if (val)
            	val = 1;
				int32 red_ref3 = palette[val].red;
				int32 green_ref3 = palette[val].green;
				int32 blue_ref3 = palette[val].blue;
            image_it1.DecRow();
            int32 y_offset = 0L;
            do
            {
            	int32 x_offset = (int32)((xpos - column) * 1024.0);
             	int32 tblue1 = (((blue_ref1 << 10) +
               	((blue_ref2 - blue_ref1) * x_offset)) >> 10);
             	int32 tblue2 = (((blue_ref3 << 10) +
               	((blue_ref4 - blue_ref3) * x_offset)) >> 10);
             	int32 tgreen1 = (((green_ref1 << 10) +
               	((green_ref2 - green_ref1) * x_offset)) >> 10);
             	int32 tgreen2 = (((green_ref3 << 10) +
               	((green_ref4 - green_ref3) * x_offset)) >> 10);
             	int32 tred1 = (((red_ref1 << 10) +
               	((red_ref2 - red_ref1) * x_offset)) >> 10);
             	int32 tred2 = (((red_ref3 << 10) +
               	((red_ref4 - red_ref3) * x_offset)) >> 10);
               int32 blue = (((tblue1 << 10)
               	+ ((tblue2 - tblue1) * y_offset)) >> 10);
               int32 green = (((tgreen1 << 10)
               	+ ((tgreen2 - tgreen1) * y_offset)) >> 10);
               int32 red = (((tred1 << 10)
               	+ ((tred2 - tred1) * y_offset)) >> 10);
               image_it2.Set(blue, green, red);
               xpos += pwidth;
               while (xpos >= (column + 1.0))
               {
               	image_it1.Increment();
               	column += 1.0;
                  red_ref1 = red_ref2;
                  green_ref1 = green_ref2;
                  blue_ref1 = blue_ref2;
                  red_ref3 = red_ref4;
                  green_ref3 = green_ref4;
                  blue_ref3 = blue_ref4;
                  image_it1.Increment();
                  unsigned char val = (unsigned char)image_it1;
		            if (val)
      		      	val = 1;
                  red_ref2 = palette[val].red;
                  green_ref2 = palette[val].green;
                  blue_ref2 = palette[val].blue;
                  image_it1.IncRow();
                  val = (unsigned char)image_it1;
		            if (val)
      		      	val = 1;
                  red_ref4 = palette[val].red;
                  green_ref4 = palette[val].green;
                  blue_ref4 = palette[val].blue;
                  image_it1.Decrement();
                  image_it1.DecRow();
               }
               if (!image_it2.Increment())
               {
						if (!image_it2.NextRow())
                  	break;
                  row_count++;
                  event_handler->Set(row_count);
						image_it1.ResetRow();
                  column = 0.0;
                  xpos = 0.0;
                  ypos += pheight;
                  while (ypos >= (row + 1.0))
                  {
                  	image_it1.NextRow();
                     row += 1.0;
						}
   		         y_offset = (int32)((ypos - row) * 1024.0);
                  unsigned char val = (unsigned char)image_it1;
		            if (val)
      		      	val = 1;
  	               red_ref1 = palette[val].red;
     	            green_ref1 = palette[val].green;
        	         blue_ref1 = palette[val].blue;
						image_it1.Increment();
                  val = (unsigned char)image_it1;
		            if (val)
      		      	val = 1;
  	               red_ref2 = palette[val].red;
     	            green_ref2 = palette[val].green;
       	         blue_ref2 = palette[val].blue;
                  image_it1.IncRow();
                  val = (unsigned char)image_it1;
		            if (val)
      		      	val = 1;
  	               red_ref4 = palette[val].red;
     	            green_ref4 = palette[val].green;
        	         blue_ref4 = palette[val].blue;
                  image_it1.Decrement();
                  val = (unsigned char)image_it1;
		            if (val)
      		      	val = 1;
  	               red_ref3 = palette[val].red;
     	            green_ref3 = palette[val].green;
        	         blue_ref3 = palette[val].blue;
                  image_it1.DecRow();
               }
            } while (1);
          	break;
         }
         G42LockedID1BitIterator image_it1(data);
         G42ImageData8BitIterator image_it2(new_data);
         int32 val_ref1 = (int32)(unsigned char)image_it1;
         image_it1.Increment();
         int32 val_ref2 = (int32)(unsigned char)image_it1;
			image_it1.IncRow();
         int32 val_ref4 = (int32)(unsigned char)image_it1;
         image_it1.Decrement();
         int32 val_ref3 = (int32)(unsigned char)image_it1;
         image_it1.DecRow();
         int32 y_offset = 0L;
         do
         {
          	int32 x_offset = (int32)((xpos - column) * 1024.0);
           	int32 tval1 = (((val_ref1 << 10) +
              	((val_ref2 - val_ref1) * x_offset)) >> 10);
           	int32 tval2 = (((val_ref3 << 10) +
             	((val_ref4 - val_ref3) * x_offset)) >> 10);
            int32 val = (((tval1 << 10)
              	+ ((tval2 - tval1) * y_offset)) >> 10);
            image_it2.Set(val);
            xpos += pwidth;
            while (xpos >= (column + 1.0))
            {
             	image_it1.Increment();
              	column += 1.0;
               val_ref1 = val_ref2;
               val_ref3 = val_ref4;
               image_it1.Increment();
               val_ref2 = (int32)(unsigned char)image_it1;
               image_it1.IncRow();
               val_ref4 = (int32)(unsigned char)image_it1;
               image_it1.Decrement();
               image_it1.DecRow();
            }
            if (!image_it2.Increment())
            {
					if (!image_it2.NextRow())
                 	break;
               row_count++;
               event_handler->Set(row_count);
					image_it1.ResetRow();
               column = 0.0;
               xpos = 0.0;
               ypos += pheight;
               while (ypos >= (row + 1.0))
               {
                	image_it1.NextRow();
                  row += 1.0;
					}
  		         y_offset = (int32)((ypos - row) * 1024.0);
               val_ref1 = (int32)(unsigned char)image_it1;
 					image_it1.Increment();
               val_ref2 = (int32)(unsigned char)image_it1;
               image_it1.IncRow();
               val_ref4 = (int32)(unsigned char)image_it1;
               image_it1.Decrement();
               val_ref3 = (int32)(unsigned char)image_it1;
               image_it1.DecRow();
            }
         } while (1);
        	break;
      }
   }
   event_handler->Stop();
}
