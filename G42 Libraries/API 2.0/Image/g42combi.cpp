#include "g42idata.h"
#include "g42iter.h"
#include "g42image.h"
#include "g42iview.h"
bool MakeCompatible(G42ImageData **, G42ImageData **);
bool PalettesAreIdentical(G42Color *, int, G42Color *, int);
G42ImageData *
CombineImage(G42ImageData * data, G42ImageData * new_data, int32 x, int32 y)
{
	int32 right = x + new_data->GetWidth() - 1;
	int32 bottom = y + new_data->GetHeight() - 1;
	if ((right < 0) || (bottom < 0) || (x > (int32)data->GetWidth()) ||
		(y > (int32)data->GetHeight()))
		return data;
  	bool must_delete = MakeCompatible(&data, &new_data);
	int32 b1l = x, b1t = y, b1r = right, b1b = bottom;
	if (b1l < 0)
		b1l = 0;
	if (b1t < 0)
		b1t = 0;
	if (b1r >= data->GetWidth())
		b1r = data->GetWidth() - 1;
	if (b1b >= data->GetHeight())
		b1b = data->GetHeight() - 1;
	int32 b2l = 0, b2t = 0, b2r = new_data->GetWidth() - 1;
	int32 b2b = new_data->GetHeight() - 1;
	if (x < 0)
		b2l = -x;
	if (y < 0)
		b2t = -y;
	if (right >= data->GetWidth())
		b2r -= (right - data->GetWidth() + 1);
	if (bottom >= data->GetHeight())
		b2b -= (bottom - data->GetHeight() + 1);
	switch (data->GetDepth())
	{
		case 32:
		case 24:
		{
			G42ImageData24BitIterator image_it1(data, false);
			image_it1.SetArea(b1l, b1t, b1r, b1b);
			G42LockedID24BitIterator image_it2(new_data, false);
			image_it2.SetArea(b2l, b2t, b2r, b2b);
			do
			{
				image_it1.Set(image_it2.GetBlue(), image_it2.GetGreen(),
					image_it2.GetRed());
				image_it2.Increment();
				if (!image_it1.Increment())
				{
					if (!image_it1.NextRow())
						break;
					image_it2.NextRow();
				}
			} while (1);
         if (must_delete)
         	delete new_data;
			return data;
		}
		case 8:
		{
			G42ImageData8BitIterator image_it1(data, false);
			image_it1.SetArea(b1l, b1t, b1r, b1b);
			G42LockedID8BitIterator image_it2(new_data, false);
			image_it2.SetArea(b2l, b2t, b2r, b2b);
			do
			{
				image_it1.Set((unsigned char)image_it2);
				image_it2.Increment();
				if (!image_it1.Increment())
				{
					if (!image_it1.NextRow())
						break;
					image_it2.NextRow();
				}
			} while (1);
			return data;
		}
		case 4:
		{
			G42ImageData4BitIterator image_it1(data, false);
			image_it1.SetArea(b1l, b1t, b1r, b1b);
			G42LockedID4BitIterator image_it2(new_data, false);
			image_it2.SetArea(b2l, b2t, b2r, b2b);
			do
			{
				image_it1.Set4((unsigned char)image_it2);
				image_it2.Increment();
				if (!image_it1.Increment())
				{
					if (!image_it1.NextRow())
						break;
					image_it2.NextRow();
				}
			} while (1);
			return data;
		}
		case 1:
		{
			G42ImageData1BitIterator image_it1(data, false);
			image_it1.SetArea(b1l, b1t, b1r, b1b);
			G42LockedID1BitIterator image_it2(new_data, false);
			image_it2.SetArea(b2l, b2t, b2r, b2b);
			do
			{
				image_it1.Set8((unsigned char)image_it2);
				image_it2.Increment();
				if (!image_it1.Increment())
				{
					if (!image_it1.NextRow())
						break;
					image_it2.NextRow();
				}
			} while (1);
			return data;
		}
	}
	return data;
}
bool
MakeCompatible(G42ImageData ** data, G42ImageData ** new_data)
{
	switch ((*data)->GetDepth())
   {
		case 32:
   	case 24:
      {
      	switch ((*new_data)->GetDepth())
         {
				case 32:
         	case 24:
            {
            	return false;
            }
            case 8:
            {
              	G42ImageData * local_data = new G42ImageData((*new_data)->GetData(),
						(*new_data)->GetWidth(), (*new_data)->GetHeight(), 8*TrueColorPixelSize, 0);
					local_data->ReferenceCount++;
					local_data->SetFinished(true);
					local_data->SetValidStartRow(0);
					local_data->SetValidNumRows(local_data->GetHeight());
					G42LockedID8BitIterator image_it1((*new_data));
					G42ImageData24BitIterator image_it2(local_data);
            	if ((*new_data)->GetNumPalette() != 0)
               {
						G42Color * palette = (*new_data)->GetPalette();
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
                  		if (!image_it1.NextRow())
	                     	break;
   	                	image_it2.NextRow();
      	            }
         	      } while (1);
               }
               *new_data = local_data;
               return true;
            }
            case 4:
            {
              	G42ImageData * local_data = new G42ImageData((*new_data)->GetData(),
						(*new_data)->GetWidth(), (*new_data)->GetHeight(), 8*TrueColorPixelSize, 0);
					local_data->ReferenceCount++;
					local_data->SetFinished(true);
					local_data->SetValidStartRow(0);
					local_data->SetValidNumRows(local_data->GetHeight());
					G42LockedID4BitIterator image_it1((*new_data));
					G42ImageData24BitIterator image_it2(local_data);
            	if ((*new_data)->GetNumPalette() != 0)
               {
						G42Color * palette = (*new_data)->GetPalette();
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
               }
					else
               {
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
               }
               *new_data = local_data;
               return true;
            }
            case 1:
            {
              	G42ImageData * local_data = new G42ImageData((*new_data)->GetData(),
						(*new_data)->GetWidth(), (*new_data)->GetHeight(), 8*TrueColorPixelSize, 0);
					local_data->ReferenceCount++;
					local_data->SetFinished(true);
					local_data->SetValidStartRow(0);
					local_data->SetValidNumRows(local_data->GetHeight());
					G42LockedID1BitIterator image_it1((*new_data));
					G42ImageData24BitIterator image_it2(local_data);
            	if ((*new_data)->GetNumPalette() != 0)
               {
						G42Color * palette = (*new_data)->GetPalette();
						do
                  {
							unsigned char val = (unsigned char)image_it1;
                     if (val)
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
	                  	if (!image_it1.NextRow())
   	                  	break;
      	             	image_it2.NextRow();
         	         }
            	   } while (1);
            	}
               *new_data = local_data;
               return true;
            }
         }
      }
      case 8:
      {
      	switch ((*new_data)->GetDepth())
         {
				case 32:
         	case 24:
            {
              	G42ImageData * local_data = new G42ImageData((*data)->GetData(),
						(*data)->GetWidth(), (*data)->GetHeight(), 8*TrueColorPixelSize, 0);
					local_data->ReferenceCount++;
					local_data->SetFinished(true);
					local_data->SetValidStartRow(0);
					local_data->SetValidNumRows(local_data->GetHeight());
					G42LockedID8BitIterator image_it1((*data));
					G42ImageData24BitIterator image_it2(local_data);
            	if ((*data)->GetNumPalette() != 0)
               {
						G42Color * palette = (*data)->GetPalette();
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
                  		if (!image_it1.NextRow())
	                     	break;
   	                	image_it2.NextRow();
      	            }
         	      } while (1);
               }
               *data = local_data;
               return false;
            }
            case 8:
            {
					if ((*data)->GetNumPalette() != 0)
               {
               	if (PalettesAreIdentical((*data)->GetPalette(), (*data)->GetNumPalette(),
                  	(*new_data)->GetPalette(), (*new_data)->GetNumPalette()))
                     return false;
	              	G42ImageData * local_data = new G42ImageData((*data)->GetData(),
							(*data)->GetWidth(), (*data)->GetHeight(), 8*TrueColorPixelSize, 0);
						local_data->ReferenceCount++;
						local_data->SetFinished(true);
						local_data->SetValidStartRow(0);
						local_data->SetValidNumRows(local_data->GetHeight());
						G42LockedID8BitIterator image_it1((*data));
						G42ImageData24BitIterator image_it2(local_data);
						G42Color * palette = (*data)->GetPalette();
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
                  *data = local_data;
						if ((*new_data)->GetNumPalette() != 0)
                  {
		              	local_data = new G42ImageData((*new_data)->GetData(),
								(*new_data)->GetWidth(), (*new_data)->GetHeight(), 8*TrueColorPixelSize, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID8BitIterator image_it3((*new_data));
							G42ImageData24BitIterator image_it4(local_data);
							G42Color * palette = (*new_data)->GetPalette();
							do
            	      {
								unsigned char val = (unsigned char)image_it3;
                  	   image_it4.Set(palette[val].blue, palette[val].green,
                     		palette[val].red);
	                     image_it4.Increment();
   	                  if (!image_it3.Increment())
      	               {
         	            	if (!image_it3.NextRow())
            	            	break;
               	         image_it4.NextRow();
                  	   }
	                  } while (1);
	                  *new_data = local_data;
   	               return true;
                  }
                  else
                  {
		              	local_data = new G42ImageData((*new_data)->GetData(),
								(*new_data)->GetWidth(), (*new_data)->GetHeight(), 8*TrueColorPixelSize, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID8BitIterator image_it3((*new_data));
							G42ImageData24BitIterator image_it4(local_data);
							do
            	      {
								unsigned char val = (unsigned char)image_it3;
                  	   image_it4.Set(val, val, val);
	                     image_it4.Increment();
   	                  if (!image_it3.Increment())
      	               {
         	            	if (!image_it3.NextRow())
            	            	break;
               	         image_it4.NextRow();
                  	   }
	                  } while (1);
	                  *new_data = local_data;
   	               return true;
                  }
               }
               else
               {
               	if ((*new_data)->GetNumPalette() != 0)
                  {
		              	G42ImageData * local_data = new G42ImageData((*data)->GetData(),
								(*data)->GetWidth(), (*data)->GetHeight(), 8*TrueColorPixelSize, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID8BitIterator image_it1((*data));
							G42ImageData24BitIterator image_it2(local_data);
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
	                  *data = local_data;
		              	local_data = new G42ImageData((*new_data)->GetData(),
								(*new_data)->GetWidth(), (*new_data)->GetHeight(), 8*TrueColorPixelSize, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID8BitIterator image_it3((*new_data));
							G42ImageData24BitIterator image_it4(local_data);
							G42Color * palette = (*new_data)->GetPalette();
							do
            	      {
								unsigned char val = (unsigned char)image_it3;
                  	   image_it4.Set(palette[val].blue, palette[val].green,
                     		palette[val].red);
	                     image_it4.Increment();
   	                  if (!image_it3.Increment())
      	               {
         	            	if (!image_it3.NextRow())
            	            	break;
               	         image_it4.NextRow();
                  	   }
	                  } while (1);
	                  *new_data = local_data;
   	               return true;
                  }
                  else
							return false;
               }
            }
            case 4:
            {
					if ((*data)->GetNumPalette() != 0)
               {
	              	G42ImageData * local_data = new G42ImageData((*data)->GetData(),
							(*data)->GetWidth(), (*data)->GetHeight(), 8*TrueColorPixelSize, 0);
						local_data->ReferenceCount++;
						local_data->SetFinished(true);
						local_data->SetValidStartRow(0);
						local_data->SetValidNumRows(local_data->GetHeight());
						G42LockedID8BitIterator image_it1((*data));
						G42ImageData24BitIterator image_it2(local_data);
						G42Color * palette = (*data)->GetPalette();
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
                  *data = local_data;
	              	local_data = new G42ImageData((*new_data)->GetData(),
							(*new_data)->GetWidth(), (*new_data)->GetHeight(), 8*TrueColorPixelSize, 0);
						local_data->ReferenceCount++;
						local_data->SetFinished(true);
						local_data->SetValidStartRow(0);
						local_data->SetValidNumRows(local_data->GetHeight());
						G42LockedID4BitIterator image_it3((*new_data));
						G42ImageData24BitIterator image_it4(local_data);
	            	if ((*new_data)->GetNumPalette() != 0)
   	            {
							G42Color * palette = (*new_data)->GetPalette();
							do
            	      {
								unsigned char val = (unsigned char)image_it3;
                  	   image_it4.Set(palette[val].blue, palette[val].green,
                     		palette[val].red);
	                     image_it4.Increment();
   	                  if (!image_it3.Increment())
      	               {
         	            	if (!image_it3.NextRow())
            	            	break;
               	         image_it4.NextRow();
                  	   }
	                  } while (1);
		            }
      	         else
         	      {
	         	      do
   	         	   {
								unsigned char val = (unsigned char)image_it3;
                        val = ((val << 4) + val);
	         	         image_it4.Set(val, val, val);
   	         	      image_it4.Increment();
      	         	   if (!image_it3.Increment())
         	         	{
            	      		if (!image_it3.NextRow())
	            	         	break;
   	            	    	image_it4.NextRow();
	      	            }
   	      	      } while (1);
      	         }
                  *new_data = local_data;
                  return true;
               }
               else
					{
						if ((*new_data)->GetNumPalette() != 0)
                  {
		              	G42ImageData * local_data = new G42ImageData((*data)->GetData(),
								(*data)->GetWidth(), (*data)->GetHeight(), 8*TrueColorPixelSize, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID8BitIterator image_it1((*data));
							G42ImageData24BitIterator image_it2(local_data);
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
                     *data = local_data;
		              	local_data = new G42ImageData((*new_data)->GetData(),
								(*new_data)->GetWidth(), (*new_data)->GetHeight(), 8*TrueColorPixelSize, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID4BitIterator image_it3((*new_data));
							G42ImageData24BitIterator image_it4(local_data);
							G42Color * palette = (*new_data)->GetPalette();
							do
         	  	      {
								unsigned char val = (unsigned char)image_it3;
               	  	   image_it4.Set(palette[val].blue, palette[val].green,
                        	palette[val].red);
                  	   image_it4.Increment();
  	                  	if (!image_it3.Increment())
	     	               {
   	     	            	if (!image_it3.NextRow())
      	     	            	break;
         	     	         image_it4.NextRow();
            	     	   }
               	   } while (1);
	                  *new_data = local_data;
   	               return true;
                  }
                  else
                  {
		              	G42ImageData * local_data = new G42ImageData((*new_data)->GetData(),
								(*new_data)->GetWidth(), (*new_data)->GetHeight(), 8, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID4BitIterator image_it1((*new_data));
							G42ImageData8BitIterator image_it2(local_data);
							do
         	  	      {
								unsigned char val = (unsigned char)image_it1;
               	  	   image_it2.Set(((val << 4) + val));
                  	   image_it2.Increment();
  	                  	if (!image_it1.Increment())
	     	               {
   	     	            	if (!image_it1.NextRow())
      	     	            	break;
         	     	         image_it2.NextRow();
            	     	   }
               	   } while (1);
	                  *new_data = local_data;
   	               return true;
                  }
               }
            }
            case 1:
            {
					if ((*data)->GetNumPalette() != 0)
               {
	              	G42ImageData * local_data = new G42ImageData((*data)->GetData(),
							(*data)->GetWidth(), (*data)->GetHeight(), 8*TrueColorPixelSize, 0);
						local_data->ReferenceCount++;
						local_data->SetFinished(true);
						local_data->SetValidStartRow(0);
						local_data->SetValidNumRows(local_data->GetHeight());
						G42LockedID8BitIterator image_it1((*data));
						G42ImageData24BitIterator image_it2(local_data);
						G42Color * palette = (*data)->GetPalette();
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
                  *data = local_data;
	              	local_data = new G42ImageData((*new_data)->GetData(),
							(*new_data)->GetWidth(), (*new_data)->GetHeight(), 8*TrueColorPixelSize, 0);
						local_data->ReferenceCount++;
						local_data->SetFinished(true);
						local_data->SetValidStartRow(0);
						local_data->SetValidNumRows(local_data->GetHeight());
						G42LockedID1BitIterator image_it3((*new_data));
						G42ImageData24BitIterator image_it4(local_data);
	            	if ((*new_data)->GetNumPalette() != 0)
   	            {
							G42Color * palette = (*new_data)->GetPalette();
							do
            	      {
								if ((unsigned char)image_it3)
	                  	   image_it4.Set(palette[1].blue, palette[1].green,
   	                  		palette[1].red);
                        else
	                  	   image_it4.Set(palette[0].blue, palette[0].green,
   	                  		palette[0].red);
	                     image_it4.Increment();
   	                  if (!image_it3.Increment())
      	               {
         	            	if (!image_it3.NextRow())
            	            	break;
               	         image_it4.NextRow();
                  	   }
	                  } while (1);
		            }
      	         else
         	      {
	         	      do
   	         	   {
								unsigned char val = (unsigned char)image_it3;
	         	         image_it4.Set(val, val, val);
   	         	      image_it4.Increment();
      	         	   if (!image_it3.Increment())
         	         	{
            	      		if (!image_it3.NextRow())
	            	         	break;
   	            	    	image_it4.NextRow();
	      	            }
   	      	      } while (1);
      	         }
                  *new_data = local_data;
                  return true;
               }
               else
					{
						if ((*new_data)->GetNumPalette() != 0)
                  {
		              	G42ImageData * local_data = new G42ImageData((*data)->GetData(),
								(*data)->GetWidth(), (*data)->GetHeight(), 8*TrueColorPixelSize, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID8BitIterator image_it1((*data));
							G42ImageData24BitIterator image_it2(local_data);
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
                     *data = local_data;
		              	local_data = new G42ImageData((*new_data)->GetData(),
								(*new_data)->GetWidth(), (*new_data)->GetHeight(), 8*TrueColorPixelSize, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID1BitIterator image_it3((*new_data));
							G42ImageData24BitIterator image_it4(local_data);
							G42Color * palette = (*new_data)->GetPalette();
							do
         	  	      {
								if ((unsigned char)image_it3)
               	  	   	image_it4.Set(palette[1].blue, palette[1].green,
                        		palette[1].red);
                        else
               	  	   	image_it4.Set(palette[0].blue, palette[0].green,
                        		palette[0].red);
                  	   image_it4.Increment();
  	                  	if (!image_it3.Increment())
	     	               {
   	     	            	if (!image_it3.NextRow())
      	     	            	break;
         	     	         image_it4.NextRow();
            	     	   }
               	   } while (1);
	                  *new_data = local_data;
   	               return true;
                  }
                  else
                  {
		              	G42ImageData * local_data = new G42ImageData((*new_data)->GetData(),
								(*new_data)->GetWidth(), (*new_data)->GetHeight(), 8, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID1BitIterator image_it1((*new_data));
							G42ImageData8BitIterator image_it2(local_data);
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
	                  *new_data = local_data;
   	               return true;
                  }
               }
            }
         }
      }
      case 4:
      {
      	switch ((*new_data)->GetDepth())
         {
				case 32:
         	case 24:
            {
             	G42ImageData * local_data = new G42ImageData((*data)->GetData(),
						(*data)->GetWidth(), (*data)->GetHeight(), 8*TrueColorPixelSize, 0);
					local_data->ReferenceCount++;
					local_data->SetFinished(true);
					local_data->SetValidStartRow(0);
					local_data->SetValidNumRows(local_data->GetHeight());
					G42LockedID4BitIterator image_it1((*data));
					G42ImageData24BitIterator image_it2(local_data);
            	if ((*data)->GetNumPalette() != 0)
               {
               	G42Color * palette = (*data)->GetPalette();
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
                  *data = local_data;
                  return false;
               }
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
               *data = local_data;
               return false;
            }
            case 8:
            {
					if ((*data)->GetNumPalette() != 0)
               {
	              	G42ImageData * local_data = new G42ImageData((*data)->GetData(),
							(*data)->GetWidth(), (*data)->GetHeight(), 8*TrueColorPixelSize, 0);
						local_data->ReferenceCount++;
						local_data->SetFinished(true);
						local_data->SetValidStartRow(0);
						local_data->SetValidNumRows(local_data->GetHeight());
						G42LockedID4BitIterator image_it1((*data));
						G42ImageData24BitIterator image_it2(local_data);
						G42Color * palette = (*data)->GetPalette();
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
                  *data = local_data;
						if ((*new_data)->GetNumPalette() != 0)
                  {
		              	local_data = new G42ImageData((*new_data)->GetData(),
								(*new_data)->GetWidth(), (*new_data)->GetHeight(), 8*TrueColorPixelSize, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID8BitIterator image_it3((*new_data));
							G42ImageData24BitIterator image_it4(local_data);
							G42Color * palette = (*new_data)->GetPalette();
							do
            	      {
								unsigned char val = (unsigned char)image_it3;
                  	   image_it4.Set(palette[val].blue, palette[val].green,
                     		palette[val].red);
	                     image_it4.Increment();
   	                  if (!image_it3.Increment())
      	               {
         	            	if (!image_it3.NextRow())
            	            	break;
               	         image_it4.NextRow();
                  	   }
	                  } while (1);
	                  *new_data = local_data;
   	               return true;
                  }
                  else
                  {
		              	local_data = new G42ImageData((*new_data)->GetData(),
								(*new_data)->GetWidth(), (*new_data)->GetHeight(), 8*TrueColorPixelSize, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID8BitIterator image_it3((*new_data));
							G42ImageData24BitIterator image_it4(local_data);
							do
            	      {
								unsigned char val = (unsigned char)image_it3;
                  	   image_it4.Set(val, val, val);
	                     image_it4.Increment();
   	                  if (!image_it3.Increment())
      	               {
         	            	if (!image_it3.NextRow())
            	            	break;
               	         image_it4.NextRow();
                  	   }
	                  } while (1);
	                  *new_data = local_data;
   	               return true;
                  }
               }
               else
               {
               	if ((*new_data)->GetNumPalette() != 0)
                  {
		              	G42ImageData * local_data = new G42ImageData((*data)->GetData(),
								(*data)->GetWidth(), (*data)->GetHeight(), 8*TrueColorPixelSize, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID4BitIterator image_it1((*data));
							G42ImageData24BitIterator image_it2(local_data);
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
	                  *data = local_data;
		              	local_data = new G42ImageData((*new_data)->GetData(),
								(*new_data)->GetWidth(), (*new_data)->GetHeight(), 8*TrueColorPixelSize, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID8BitIterator image_it3((*new_data));
							G42ImageData24BitIterator image_it4(local_data);
							G42Color * palette = (*new_data)->GetPalette();
							do
            	      {
								unsigned char val = (unsigned char)image_it3;
                  	   image_it4.Set(palette[val].blue, palette[val].green,
                     		palette[val].red);
	                     image_it4.Increment();
   	                  if (!image_it3.Increment())
      	               {
         	            	if (!image_it3.NextRow())
            	            	break;
               	         image_it4.NextRow();
                  	   }
	                  } while (1);
	                  *new_data = local_data;
   	               return true;
                  }
                  else
						{
		              	G42ImageData * local_data = new G42ImageData((*data)->GetData(),
								(*data)->GetWidth(), (*data)->GetHeight(), 8, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID4BitIterator image_it1((*data));
							G42ImageData8BitIterator image_it2(local_data);
							do
     	   	  	      {
								unsigned char val = (unsigned char)image_it1;
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
	                  *data = local_data;
                  }
              	}
            }
            case 4:
            {
					if ((*data)->GetNumPalette() != 0)
               {
               	if (PalettesAreIdentical((*data)->GetPalette(), (*data)->GetNumPalette(),
                  	(*new_data)->GetPalette(), (*new_data)->GetNumPalette()))
                     return false;
	              	G42ImageData * local_data = new G42ImageData((*data)->GetData(),
							(*data)->GetWidth(), (*data)->GetHeight(), 8*TrueColorPixelSize, 0);
						local_data->ReferenceCount++;
						local_data->SetFinished(true);
						local_data->SetValidStartRow(0);
						local_data->SetValidNumRows(local_data->GetHeight());
						G42LockedID4BitIterator image_it1((*data));
						G42ImageData24BitIterator image_it2(local_data);
						G42Color * palette = (*data)->GetPalette();
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
                  *data = local_data;
	              	local_data = new G42ImageData((*new_data)->GetData(),
							(*new_data)->GetWidth(), (*new_data)->GetHeight(), 8*TrueColorPixelSize, 0);
						local_data->ReferenceCount++;
						local_data->SetFinished(true);
						local_data->SetValidStartRow(0);
						local_data->SetValidNumRows(local_data->GetHeight());
						G42LockedID4BitIterator image_it3((*new_data));
						G42ImageData24BitIterator image_it4(local_data);
	            	if ((*new_data)->GetNumPalette() != 0)
   	            {
							G42Color * palette = (*new_data)->GetPalette();
							do
            	      {
								unsigned char val = (unsigned char)image_it3;
                  	   image_it4.Set(palette[val].blue, palette[val].green,
                     		palette[val].red);
	                     image_it4.Increment();
   	                  if (!image_it3.Increment())
      	               {
         	            	if (!image_it3.NextRow())
            	            	break;
               	         image_it4.NextRow();
                  	   }
	                  } while (1);
		            }
      	         else
         	      {
	         	      do
   	         	   {
								unsigned char val = (unsigned char)image_it3;
                        val = ((val << 4) + val);
	         	         image_it4.Set(val, val, val);
   	         	      image_it4.Increment();
      	         	   if (!image_it3.Increment())
         	         	{
            	      		if (!image_it3.NextRow())
	            	         	break;
   	            	    	image_it4.NextRow();
	      	            }
   	      	      } while (1);
      	         }
                  *new_data = local_data;
                  return true;
               }
               else
					{
						if ((*new_data)->GetNumPalette() != 0)
                  {
		              	G42ImageData * local_data = new G42ImageData((*data)->GetData(),
								(*data)->GetWidth(), (*data)->GetHeight(), 8*TrueColorPixelSize, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID4BitIterator image_it1((*data));
							G42ImageData24BitIterator image_it2(local_data);
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
                     *data = local_data;
		              	local_data = new G42ImageData((*new_data)->GetData(),
								(*new_data)->GetWidth(), (*new_data)->GetHeight(), 8*TrueColorPixelSize, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID4BitIterator image_it3((*new_data));
							G42ImageData24BitIterator image_it4(local_data);
							G42Color * palette = (*new_data)->GetPalette();
							do
         	  	      {
								unsigned char val = (unsigned char)image_it3;
               	  	   image_it4.Set(palette[val].blue, palette[val].green,
                        	palette[val].red);
                  	   image_it4.Increment();
  	                  	if (!image_it3.Increment())
	     	               {
   	     	            	if (!image_it3.NextRow())
      	     	            	break;
         	     	         image_it4.NextRow();
            	     	   }
               	   } while (1);
	                  *new_data = local_data;
   	               return true;
                  }
                  else
							return false;
               }
            }
            case 1:
            {
					if ((*data)->GetNumPalette() != 0)
               {
	              	G42ImageData * local_data = new G42ImageData((*data)->GetData(),
							(*data)->GetWidth(), (*data)->GetHeight(), 8*TrueColorPixelSize, 0);
						local_data->ReferenceCount++;
						local_data->SetFinished(true);
						local_data->SetValidStartRow(0);
						local_data->SetValidNumRows(local_data->GetHeight());
						G42LockedID4BitIterator image_it1((*data));
						G42ImageData24BitIterator image_it2(local_data);
						G42Color * palette = (*data)->GetPalette();
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
                  *data = local_data;
	              	local_data = new G42ImageData((*new_data)->GetData(),
							(*new_data)->GetWidth(), (*new_data)->GetHeight(), 8*TrueColorPixelSize, 0);
						local_data->ReferenceCount++;
						local_data->SetFinished(true);
						local_data->SetValidStartRow(0);
						local_data->SetValidNumRows(local_data->GetHeight());
						G42LockedID1BitIterator image_it3((*new_data));
						G42ImageData24BitIterator image_it4(local_data);
	            	if ((*new_data)->GetNumPalette() != 0)
   	            {
							G42Color * palette = (*new_data)->GetPalette();
							do
            	      {
								if ((unsigned char)image_it3)
	                  	   image_it4.Set(palette[1].blue, palette[1].green,
   	                  		palette[1].red);
                        else
	                  	   image_it4.Set(palette[0].blue, palette[0].green,
   	                  		palette[0].red);
	                     image_it4.Increment();
   	                  if (!image_it3.Increment())
      	               {
         	            	if (!image_it3.NextRow())
            	            	break;
               	         image_it4.NextRow();
                  	   }
	                  } while (1);
		            }
      	         else
         	      {
	         	      do
   	         	   {
								unsigned char val = (unsigned char)image_it3;
	         	         image_it4.Set(val, val, val);
   	         	      image_it4.Increment();
      	         	   if (!image_it3.Increment())
         	         	{
            	      		if (!image_it3.NextRow())
	            	         	break;
   	            	    	image_it4.NextRow();
	      	            }
   	      	      } while (1);
      	         }
                  *new_data = local_data;
                  return true;
               }
               else
					{
						if ((*new_data)->GetNumPalette() != 0)
                  {
		              	G42ImageData * local_data = new G42ImageData((*data)->GetData(),
								(*data)->GetWidth(), (*data)->GetHeight(), 8*TrueColorPixelSize, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID4BitIterator image_it1((*data));
							G42ImageData24BitIterator image_it2(local_data);
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
                     *data = local_data;
		              	local_data = new G42ImageData((*new_data)->GetData(),
								(*new_data)->GetWidth(), (*new_data)->GetHeight(), 8*TrueColorPixelSize, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID1BitIterator image_it3((*new_data));
							G42ImageData24BitIterator image_it4(local_data);
							G42Color * palette = (*new_data)->GetPalette();
							do
         	  	      {
								if ((unsigned char)image_it3)
               	  	   	image_it4.Set(palette[1].blue, palette[1].green,
                        		palette[1].red);
                        else
               	  	   	image_it4.Set(palette[0].blue, palette[0].green,
                        		palette[0].red);
                  	   image_it4.Increment();
  	                  	if (!image_it3.Increment())
	     	               {
   	     	            	if (!image_it3.NextRow())
      	     	            	break;
         	     	         image_it4.NextRow();
            	     	   }
               	   } while (1);
	                  *new_data = local_data;
   	               return true;
                  }
                  else
                  {
		              	G42ImageData * local_data = new G42ImageData((*new_data)->GetData(),
								(*new_data)->GetWidth(), (*new_data)->GetHeight(), 4, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID1BitIterator image_it1((*new_data));
							G42ImageData4BitIterator image_it2(local_data);
							do
        	  	      	{
								if ((unsigned char)image_it1)
	               	  	   image_it2.Set4(0x0f);
                        else
                        	image_it2.Set4(0);
                  	   image_it2.Increment();
  	                  	if (!image_it1.Increment())
	     	               {
   	     	            	if (!image_it1.NextRow())
      	     	            	break;
         	     	         image_it2.NextRow();
            	     	   }
               	   } while (1);
	                  *new_data = local_data;
   	               return true;
                  }
               }
            }
         }
      }
      case 1:
      {
      	switch ((*new_data)->GetDepth())
         {
				case 32:
        		case 24:
            {
             	G42ImageData * local_data = new G42ImageData((*data)->GetData(),
						(*data)->GetWidth(), (*data)->GetHeight(), 8*TrueColorPixelSize, 0);
					local_data->ReferenceCount++;
					local_data->SetFinished(true);
					local_data->SetValidStartRow(0);
					local_data->SetValidNumRows(local_data->GetHeight());
					G42LockedID1BitIterator image_it1((*data));
					G42ImageData24BitIterator image_it2(local_data);
            	if ((*data)->GetNumPalette() != 0)
               {
               	G42Color * palette = (*data)->GetPalette();
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
                  *data = local_data;
                  return false;
               }
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
               *data = local_data;
               return false;
            }
            case 8:
            {
					if ((*data)->GetNumPalette() != 0)
               {
	              	G42ImageData * local_data = new G42ImageData((*data)->GetData(),
							(*data)->GetWidth(), (*data)->GetHeight(), 8*TrueColorPixelSize, 0);
						local_data->ReferenceCount++;
						local_data->SetFinished(true);
						local_data->SetValidStartRow(0);
						local_data->SetValidNumRows(local_data->GetHeight());
						G42LockedID1BitIterator image_it1((*data));
						G42ImageData24BitIterator image_it2(local_data);
						G42Color * palette = (*data)->GetPalette();
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
                  *data = local_data;
						if ((*new_data)->GetNumPalette() != 0)
                  {
		              	local_data = new G42ImageData((*new_data)->GetData(),
								(*new_data)->GetWidth(), (*new_data)->GetHeight(), 8*TrueColorPixelSize, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID8BitIterator image_it3((*new_data));
							G42ImageData24BitIterator image_it4(local_data);
							G42Color * palette = (*new_data)->GetPalette();
							do
            	      {
								unsigned char val = (unsigned char)image_it3;
                  	   image_it4.Set(palette[val].blue, palette[val].green,
                     		palette[val].red);
	                     image_it4.Increment();
   	                  if (!image_it3.Increment())
      	               {
         	            	if (!image_it3.NextRow())
            	            	break;
               	         image_it4.NextRow();
                  	   }
	                  } while (1);
	                  *new_data = local_data;
   	               return true;
                  }
                  else
                  {
		              	local_data = new G42ImageData((*new_data)->GetData(),
								(*new_data)->GetWidth(), (*new_data)->GetHeight(), 8*TrueColorPixelSize, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID8BitIterator image_it3((*new_data));
							G42ImageData24BitIterator image_it4(local_data);
							do
            	      {
								unsigned char val = (unsigned char)image_it3;
                  	   image_it4.Set(val, val, val);
	                     image_it4.Increment();
   	                  if (!image_it3.Increment())
      	               {
         	            	if (!image_it3.NextRow())
            	            	break;
               	         image_it4.NextRow();
                  	   }
	                  } while (1);
	                  *new_data = local_data;
   	               return true;
                  }
               }
               else
               {
               	if ((*new_data)->GetNumPalette() != 0)
                  {
		              	G42ImageData * local_data = new G42ImageData((*data)->GetData(),
								(*data)->GetWidth(), (*data)->GetHeight(), 8*TrueColorPixelSize, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID1BitIterator image_it1((*data));
							G42ImageData24BitIterator image_it2(local_data);
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
	                  *data = local_data;
		              	local_data = new G42ImageData((*new_data)->GetData(),
								(*new_data)->GetWidth(), (*new_data)->GetHeight(), 8*TrueColorPixelSize, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID8BitIterator image_it3((*new_data));
							G42ImageData24BitIterator image_it4(local_data);
							G42Color * palette = (*new_data)->GetPalette();
							do
            	      {
								unsigned char val = (unsigned char)image_it3;
                  	   image_it4.Set(palette[val].blue, palette[val].green,
                     		palette[val].red);
	                     image_it4.Increment();
   	                  if (!image_it3.Increment())
      	               {
         	            	if (!image_it3.NextRow())
            	            	break;
               	         image_it4.NextRow();
                  	   }
	                  } while (1);
	                  *new_data = local_data;
   	               return true;
                  }
                  else
						{
		              	G42ImageData * local_data = new G42ImageData((*data)->GetData(),
								(*data)->GetWidth(), (*data)->GetHeight(), 8, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID1BitIterator image_it1((*data));
							G42ImageData8BitIterator image_it2(local_data);
							do
     	   	  	      {
								unsigned char val = (unsigned char)image_it1;
           	   	  	   image_it2.Set(val);
                 	   	image_it2.Increment();
	  	                  if (!image_it1.Increment())
  		  	               {
     		  	            	if (!image_it1.NextRow())
        		  	            	break;
           		  	         image_it2.NextRow();
              		  	   }
                 		} while (1);
	                  *data = local_data;
                     return false;
                  }
              	}
            }
            case 4:
            {
					if ((*data)->GetNumPalette() != 0)
               {
	              	G42ImageData * local_data = new G42ImageData((*data)->GetData(),
							(*data)->GetWidth(), (*data)->GetHeight(), 8*TrueColorPixelSize, 0);
						local_data->ReferenceCount++;
						local_data->SetFinished(true);
						local_data->SetValidStartRow(0);
						local_data->SetValidNumRows(local_data->GetHeight());
						G42LockedID1BitIterator image_it1((*data));
						G42ImageData24BitIterator image_it2(local_data);
						G42Color * palette = (*data)->GetPalette();
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
                  *data = local_data;
	              	local_data = new G42ImageData((*new_data)->GetData(),
							(*new_data)->GetWidth(), (*new_data)->GetHeight(), 8*TrueColorPixelSize, 0);
						local_data->ReferenceCount++;
						local_data->SetFinished(true);
						local_data->SetValidStartRow(0);
						local_data->SetValidNumRows(local_data->GetHeight());
						G42LockedID4BitIterator image_it3((*new_data));
						G42ImageData24BitIterator image_it4(local_data);
	            	if ((*new_data)->GetNumPalette() != 0)
   	            {
							G42Color * palette = (*new_data)->GetPalette();
							do
            	      {
								unsigned char val = (unsigned char)image_it3;
                  	   image_it4.Set(palette[val].blue, palette[val].green,
                     		palette[val].red);
	                     image_it4.Increment();
   	                  if (!image_it3.Increment())
      	               {
         	            	if (!image_it3.NextRow())
            	            	break;
               	         image_it4.NextRow();
                  	   }
	                  } while (1);
		            }
      	         else
         	      {
	         	      do
   	         	   {
								unsigned char val = (unsigned char)image_it3;
                        val = ((val << 4) + val);
	         	         image_it4.Set(val, val, val);
   	         	      image_it4.Increment();
      	         	   if (!image_it3.Increment())
         	         	{
            	      		if (!image_it3.NextRow())
	            	         	break;
   	            	    	image_it4.NextRow();
	      	            }
   	      	      } while (1);
      	         }
                  *new_data = local_data;
                  return true;
               }
               else
					{
						if ((*new_data)->GetNumPalette() != 0)
                  {
		              	G42ImageData * local_data = new G42ImageData((*data)->GetData(),
								(*data)->GetWidth(), (*data)->GetHeight(), 8*TrueColorPixelSize, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID1BitIterator image_it1((*data));
							G42ImageData24BitIterator image_it2(local_data);
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
                     *data = local_data;
		              	local_data = new G42ImageData((*new_data)->GetData(),
								(*new_data)->GetWidth(), (*new_data)->GetHeight(), 8*TrueColorPixelSize, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID4BitIterator image_it3((*new_data));
							G42ImageData24BitIterator image_it4(local_data);
							G42Color * palette = (*new_data)->GetPalette();
							do
         	  	      {
								unsigned char val = (unsigned char)image_it3;
               	  	   image_it4.Set(palette[val].blue, palette[val].green,
                        	palette[val].red);
                  	   image_it4.Increment();
  	                  	if (!image_it3.Increment())
	     	               {
   	     	            	if (!image_it3.NextRow())
      	     	            	break;
         	     	         image_it4.NextRow();
            	     	   }
               	   } while (1);
	                  *new_data = local_data;
   	               return true;
                  }
                  else
                  {
		              	G42ImageData * local_data = new G42ImageData((*data)->GetData(),
								(*data)->GetWidth(), (*data)->GetHeight(), 4, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID1BitIterator image_it1((*data));
							G42ImageData4BitIterator image_it2(local_data);
							do
     	   	  	      {
								if ((unsigned char)image_it1)
	          	   	  	   image_it2.Set4(0x0f);
                        else
                        	image_it2.Set4(0);
                 	   	image_it2.Increment();
	  	                  if (!image_it1.Increment())
  		  	               {
     		  	            	if (!image_it1.NextRow())
        		  	            	break;
           		  	         image_it2.NextRow();
              		  	   }
                 		} while (1);
	                  *data = local_data;
                     return false;
                  }
               }
            }
            case 1:
            {
					if ((*data)->GetNumPalette() != 0)
               {
               	if (PalettesAreIdentical((*data)->GetPalette(), (*data)->GetNumPalette(),
                  	(*new_data)->GetPalette(), (*new_data)->GetNumPalette()))
                     return false;
	              	G42ImageData * local_data = new G42ImageData((*data)->GetData(),
							(*data)->GetWidth(), (*data)->GetHeight(), 8*TrueColorPixelSize, 0);
						local_data->ReferenceCount++;
						local_data->SetFinished(true);
						local_data->SetValidStartRow(0);
						local_data->SetValidNumRows(local_data->GetHeight());
						G42LockedID1BitIterator image_it1((*data));
						G42ImageData24BitIterator image_it2(local_data);
						G42Color * palette = (*data)->GetPalette();
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
                  *data = local_data;
	              	local_data = new G42ImageData((*new_data)->GetData(),
							(*new_data)->GetWidth(), (*new_data)->GetHeight(), 8*TrueColorPixelSize, 0);
						local_data->ReferenceCount++;
						local_data->SetFinished(true);
						local_data->SetValidStartRow(0);
						local_data->SetValidNumRows(local_data->GetHeight());
						G42LockedID1BitIterator image_it3((*new_data));
						G42ImageData24BitIterator image_it4(local_data);
	            	if ((*new_data)->GetNumPalette() != 0)
   	            {
							G42Color * palette = (*new_data)->GetPalette();
							do
            	      {
								if ((unsigned char)image_it3)
	                  	   image_it4.Set(palette[1].blue, palette[1].green,
   	                  		palette[1].red);
                        else
	                  	   image_it4.Set(palette[0].blue, palette[0].green,
   	                  		palette[0].red);
	                     image_it4.Increment();
   	                  if (!image_it3.Increment())
      	               {
         	            	if (!image_it3.NextRow())
            	            	break;
               	         image_it4.NextRow();
                  	   }
	                  } while (1);
		            }
      	         else
         	      {
	         	      do
   	         	   {
								unsigned char val = (unsigned char)image_it3;
	         	         image_it4.Set(val, val, val);
   	         	      image_it4.Increment();
      	         	   if (!image_it3.Increment())
         	         	{
            	      		if (!image_it3.NextRow())
	            	         	break;
   	            	    	image_it4.NextRow();
	      	            }
   	      	      } while (1);
      	         }
                  *new_data = local_data;
                  return true;
               }
               else
					{
						if ((*new_data)->GetNumPalette() != 0)
                  {
		              	G42ImageData * local_data = new G42ImageData((*data)->GetData(),
								(*data)->GetWidth(), (*data)->GetHeight(), 8*TrueColorPixelSize, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID1BitIterator image_it1((*data));
							G42ImageData24BitIterator image_it2(local_data);
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
                     *data = local_data;
		              	local_data = new G42ImageData((*new_data)->GetData(),
								(*new_data)->GetWidth(), (*new_data)->GetHeight(), 8*TrueColorPixelSize, 0);
							local_data->ReferenceCount++;
							local_data->SetFinished(true);
							local_data->SetValidStartRow(0);
							local_data->SetValidNumRows(local_data->GetHeight());
							G42LockedID1BitIterator image_it3((*new_data));
							G42ImageData24BitIterator image_it4(local_data);
							G42Color * palette = (*new_data)->GetPalette();
							do
         	  	      {
								if ((unsigned char)image_it3)
               	  	   	image_it4.Set(palette[1].blue, palette[1].green,
                        		palette[1].red);
                        else
               	  	   	image_it4.Set(palette[0].blue, palette[0].green,
                        		palette[0].red);
                  	   image_it4.Increment();
  	                  	if (!image_it3.Increment())
	     	               {
   	     	            	if (!image_it3.NextRow())
      	     	            	break;
         	     	         image_it4.NextRow();
            	     	   }
               	   } while (1);
	                  *new_data = local_data;
   	               return true;
                  }
                  else
							return false;
               }
            }
         }
      }
   }
}
bool
PalettesAreIdentical(G42Color * palette1, int num_palette1, G42Color * palette2,
	int num_palette2)
{
	if (!palette1 || !palette2)
   	return false;
   if (num_palette1 != num_palette2)
   	return false;
	for (int n = 0; n < num_palette1; n++)
   {
   	if ((palette1[n].red != palette2[n].red) ||
      	(palette1[n].green != palette2[n].green) ||
         (palette1[n].blue != palette2[n].blue))
      	return false;
   }
   return true;
}
void
G42Image::CombineImage(G42Image * image, int32 x, int32 y)
{
	G42ImageData * new_data = Data;
	G42ImageData * new_mask = Mask;
	if (Data)
		new_data = ::CombineImage(Data, image->GetData(), x, y);
	if (Mask)
	{
		if (image->GetMask())
		{
			new_mask = ::CombineImage(Mask, image->GetMask(), x, y);
		}
		else
		{
			new_mask = 0;
			delete Mask;
			Mask = 0;
		}
	}
	bool rebuild = false;
	if (new_data && Data != new_data)
	{
		delete Data;
		Data = new_data;
		rebuild = true;
	}
	
	if (new_mask && Mask != new_mask)
	{
		if (Mask)
			delete Mask;
		Mask = new_mask;
		rebuild = true;
	}
	
	if (rebuild)
	{
		RebuildInfo();
		for (int i = 0; i < NumViewArray; i++)
		{
			ViewArray[i]->ImageChanged();
		}
	}
	Dirty = true;
}
