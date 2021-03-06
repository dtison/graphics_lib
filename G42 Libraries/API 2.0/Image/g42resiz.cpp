#include "g42idata.h"
#include "g42iter.h"
#include "g42event.h"
#include "g42image.h"
#include "g42iview.h"
void ShrinkImage(G42ImageData *, int, int, int, int, G42ImageData *, int, int,
	int, int, bool, G42EventUpdateHandler *);
void ExpandImage(G42ImageData *, int, int, int, int, G42ImageData *, int, int,
	int, int, G42EventUpdateHandler *);
int
GetResizeDepth(G42ImageData * data, uint32 width, uint32 height)
{
	if (width < data->GetWidth() && height < data->GetHeight())
	{
		if (data->GetDepth() > 8 || data->GetNumPalette())
			return 8 * TrueColorPixelSize;
			
		return 8;
	}
	return data->GetDepth();
}
void
StretchImage(G42ImageData * data, G42ImageData * new_data, int left,
	int top, int right, int bottom)
{
	double startx = ((double)left * (double)data->GetWidth())/(double)new_data->GetWidth();
	double starty = ((double)top * (double)data->GetHeight())/(double)new_data->GetHeight();
	double stopx = ((double)right * (double)data->GetWidth())/(double)new_data->GetWidth();
	double stopy = ((double)bottom * (double)data->GetHeight())/(double)new_data->GetHeight();
	int32 pwidth = (int32)(((stopx - startx) * 1024.0)/(double)(right - left));
	int32 pheight = (int32)(((stopy - starty) * 1024.0)/(double)(bottom - top));
   int32 xpos = (int32)(startx * 1024.0);
   int32 ypos = (int32)(starty * 1024.0);
   switch(data->GetDepth())
   {
		case 32:
   	case 24:
		{
			G42LockedID24BitIterator * image_it1
				= new G42LockedID24BitIterator(data, false);
			image_it1->SetArea((int)startx, (int)starty, (int)stopx, (int)stopy);
			G42ImageData24BitIterator * image_it2
				= new G42ImageData24BitIterator(new_data, false);
         image_it2->SetArea(left, top, right, bottom);
         image_it1->ResetRow();
         image_it2->ResetRow();
         int red = image_it1->GetRed();
         int green = image_it1->GetGreen();
         int blue = image_it1->GetBlue();
         int32 xref = (int32)(startx + 1.0) * 1024L;
         int32 yref = (int32)(starty + 1.0) * 1024L;
         do
         {
         	image_it2->Set(blue, green, red);
            if (!image_it2->Increment())
            {
            	if (!image_it2->NextRow())
               	break;
               xpos = (int32)(startx * 1024.0);
               xref = (int32)(startx + 1.0) * 1024L;
               ypos += pheight;
               if (ypos >= yref)
               {
               	image_it1->NextRow();
	               red = image_it1->GetRed();
   	            green = image_it1->GetGreen();
      	         blue = image_it1->GetBlue();
         	      yref += 1024L;
               }
               else
               {
               	image_it1->ResetRow();
  	               red = image_it1->GetRed();
   	            green = image_it1->GetGreen();
      	         blue = image_it1->GetBlue();
               }
               continue;
            }
            xpos += pwidth;
            if (xpos >= xref)
            {
            	image_it1->Increment();
               red = image_it1->GetRed();
               green = image_it1->GetGreen();
               blue = image_it1->GetBlue();
               xref += 1024L;
            }
         } while (1);
      	break;
      }
      case 8:
		{
			G42LockedID8BitIterator * image_it1
				= new G42LockedID8BitIterator(data, false);
			image_it1->SetArea((int)startx, (int)starty, (int)stopx, (int)stopy);
			G42ImageData8BitIterator * image_it2
				= new G42ImageData8BitIterator(new_data, false);
         image_it2->SetArea(left, top, right, bottom);
         unsigned char val = (unsigned char)(*image_it1);
         int32 xref = (int32)(startx + 1.0) * 1024L;
         int32 yref = (int32)(starty + 1.0) * 1024L;
         do
         {
         	image_it2->Set(val);
            if (!image_it2->Increment())
            {
            	if (!image_it2->NextRow())
               	break;
               xpos = (int32)(startx * 1024.0);
               xref = (int32)(startx + 1.0) * 1024L;
               ypos += pheight;
               if (ypos >= yref)
               {
               	image_it1->NextRow();
	               val = (unsigned char)(*image_it1);
         	      yref += 1024L;
               }
               else
               {
               	image_it1->ResetRow();
	               val = (unsigned char)(*image_it1);
               }
               continue;
            }
            xpos += pwidth;
            if (xpos >= xref)
            {
            	image_it1->Increment();
               val = (unsigned char)(*image_it1);
               xref += 1024L;
            }
         } while (1);
      	break;
      }
      case 4:
		{
			G42LockedID4BitIterator * image_it1
				= new G42LockedID4BitIterator(data, false);
			image_it1->SetArea((int)startx, (int)starty, (int)stopx, (int)stopy);
			G42ImageData4BitIterator * image_it2
				= new G42ImageData4BitIterator(new_data, false);
         image_it2->SetArea(left, top, right, bottom);
         unsigned char val = (unsigned char)(*image_it1);
         int32 xref = (int32)(startx + 1.0) * 1024L;
         int32 yref = (int32)(starty + 1.0) * 1024L;
         do
         {
         	image_it2->Set4(val);
            if (!image_it2->Increment())
            {
            	if (!image_it2->NextRow())
               	break;
               xpos = (int32)(startx * 1024.0);
               xref = (int32)(startx + 1.0) * 1024L;
               ypos += pheight;
               if (ypos >= yref)
               {
               	image_it1->NextRow();
	               val = (unsigned char)(*image_it1);
         	      yref += 1024L;
               }
               else
               {
               	image_it1->ResetRow();
	               val = (unsigned char)(*image_it1);
               }
               continue;
            }
            xpos += pwidth;
            if (xpos >= xref)
            {
            	image_it1->Increment();
               val = (unsigned char)(*image_it1);
               xref += 1024L;
            }
         } while (1);
      	break;
      }
      case 1:
		{
			G42LockedID1BitIterator * image_it1
				= new G42LockedID1BitIterator(data, false);
			image_it1->SetArea((int)startx, (int)starty, (int)stopx, (int)stopy);
			G42ImageData1BitIterator * image_it2
				= new G42ImageData1BitIterator(new_data, false);
         image_it2->SetArea(left, top, right, bottom);
         unsigned char val = (unsigned char)(*image_it1);
         int32 xref = (int32)(startx + 1.0) * 1024L;
         int32 yref = (int32)(starty + 1.0) * 1024L;
         do
         {
         	image_it2->Set8(val);
            if (!image_it2->Increment())
            {
            	if (!image_it2->NextRow())
               	break;
               xpos = (int32)(startx * 1024.0);
               xref = (int32)(startx + 1.0) * 1024L;
               ypos += pheight;
               if (ypos >= yref)
               {
               	image_it1->NextRow();
	               val = (unsigned char)(*image_it1);
         	      yref += 1024L;
               }
               else
               {
               	image_it1->ResetRow();
	               val = (unsigned char)(*image_it1);
               }
               continue;
            }
            xpos += pwidth;
            if (xpos >= xref)
            {
            	image_it1->Increment();
               val = (unsigned char)(*image_it1);
               xref += 1024L;
            }
         } while (1);
      	break;
      }
   }
   return;
#if 0
	int width = right2 - left2 + 1;
	int height = bottom2 - top2 + 1;
	int * wmap = new int [width];
	int * hmap = new int [height];
	int windex = 0;
	int32 n;
	int orig_width = right - left + 1;
	for (n = 0; n < orig_width; n++)
	{
		int u_limit = ((n * (width - 1)) + ((width - 1) >> 1))
			/ (int32)(orig_width - 1);
		if (u_limit >= width)
			u_limit = (width - 1);
		for (; windex < u_limit; windex++)
			wmap[windex] = n;
	}
	for (; windex < width; windex++)
		wmap[windex] = (orig_width - 1);
	int hindex = 0;
   int orig_height = bottom - top + 1;
	for (n = 0; n < orig_height; n++)
	{
		int u_limit = ((n * (height - 1)) + ((height - 1) >> 1))
			/ (int32)(orig_height - 1);
		if (u_limit >= height)
			u_limit = (height - 1);
		for (; hindex < u_limit; hindex++)
			hmap[hindex] = n;
	}
	for (; hindex < height; hindex++)
		hmap[hindex] = (orig_height - 1);
	switch (data->GetDepth())
	{
		case 24:
		{
			G42LockedID24BitIterator * image_it1
				= new G42LockedID24BitIterator(data, false);
			image_it1->SetArea(left, top, right, bottom);
			G42ImageData24BitIterator * image_it2
				= new G42ImageData24BitIterator(new_data, false);
         image_it2->SetArea(left2, top2, right2, bottom2);
			windex = 0;
			hindex = 0;
			int windex2 = 0;
         int hindex2 = 0;
			do
			{
				image_it2->Set(image_it1->GetBlue(), image_it1->GetGreen(),
					image_it1->GetRed());
				if (!image_it2->Increment())
				{
					if (!image_it2->NextRow())
						break;
					hindex++;
					image_it1->ResetRow();
					while (hindex2 < hmap[hindex])
					{
						image_it1->NextRow();
						hindex2++;
					}
					windex = windex2 = 0;
					continue;
				}
				windex++;
				while (windex2 < wmap[windex])
				{
					image_it1->Increment();
					windex2++;
				}
			} while (1);
			delete image_it1;
         delete image_it2;
			break;
		}
		case 8:
		{
			G42LockedID8BitIterator * image_it1
				 = new G42LockedID8BitIterator(data, false);
			image_it1->SetArea(left, top, right, bottom);
			G42ImageData8BitIterator * image_it2
				= new G42ImageData8BitIterator(new_data, false);
			image_it2->SetArea(left2, top2, right2, bottom2);
			windex = 0;
			hindex = 0;
			int windex2 = 0;
			int hindex2 = 0;
			do
			{
				image_it2->Set((unsigned char)(*image_it1));
				if (!image_it2->Increment())
				{
					if (!image_it2->NextRow())
						break;
					hindex++;
					image_it1->ResetRow();
					while (hindex2 < hmap[hindex])
					{
						image_it1->NextRow();
						hindex2++;
					}
					windex = windex2 = 0;
					continue;
				}
				windex++;
				while (windex2 < wmap[windex])
				{
					image_it1->Increment();
					windex2++;
				}
			} while (1);
			delete image_it1;
			delete image_it2;
			break;
		}
		case 4:
		{
			G42LockedID4BitIterator * image_it1 =
				new G42LockedID4BitIterator(data, false);
			image_it1->SetArea(left, top, right, bottom);
			G42ImageData4BitIterator * image_it2
				= new G42ImageData4BitIterator(new_data, false);
			image_it2->SetArea(left2, top2, right2, bottom2);
			windex = 0;
			hindex = 0;
			int windex2 = 0;
			int hindex2 = 0;
			do
			{
				image_it2->Set4((unsigned char)(*image_it1));
				if (!image_it2->Increment())
				{
					if (!image_it2->NextRow())
						break;
					hindex++;
					image_it1->ResetRow();
					while (hindex2 < hmap[hindex])
					{
						image_it1->NextRow();
						hindex2++;
					}
					windex = windex2 = 0;
					continue;
				}
				windex++;
				while (windex2 < wmap[windex])
				{
					image_it1->Increment();
					windex2++;
				}
			} while (1);
			delete image_it1;
			delete image_it2;
			break;
		}
		case 1:
		{
			G42LockedID1BitIterator * image_it1
				= new G42LockedID1BitIterator(data, false);
			image_it1->SetArea(left, top, right, bottom);
			G42ImageData1BitIterator * image_it2
				= new G42ImageData1BitIterator(new_data, false);
			image_it2->SetArea(left2, top2, right2, bottom2);
			windex = 0;
			hindex = 0;
			int windex2 = 0;
			int hindex2 = 0;
			do
			{
				image_it2->Set8((unsigned char)(*image_it1));
				if (!image_it2->Increment())
				{
					if (!image_it2->NextRow())
						break;
					hindex++;
					image_it1->ResetRow();
					while (hindex2 < hmap[hindex])
					{
						image_it1->NextRow();
						hindex2++;
					}
					windex = windex2 = 0;
					continue;
				}
				windex++;
				while (windex2 < wmap[windex])
				{
					if (!image_it1->Increment())
               	break;
					windex2++;
				}
			} while (1);
			delete image_it1;
			delete image_it2;
			break;
		}
	}
	delete [] wmap;
	delete [] hmap;
#endif
}
G42ImageData *
ResizeEntireImage(G42ImageData * data, int width, int height,
	G42EventUpdateHandler * event_handler)
{
	if ((width == data->GetWidth()) && (height == data->GetHeight()))
		return data;
	switch (data->GetDepth())
	{
		case 32:
		case 24:
		{
			if ((width <= data->GetWidth()) && (height <= data->GetHeight()))
         {
				G42ImageData * new_data = new G42ImageData(data->GetData(),
					width, height, 8*TrueColorPixelSize, 0);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
            ShrinkImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
					new_data, 0, 0, width - 1, height - 1, true, event_handler);
            return new_data;
         }
			if ((width >= data->GetWidth()) && (height >= data->GetHeight()))
         {
			   G42ImageData * new_data = new G42ImageData(data->GetData(),
					width, height, 8*TrueColorPixelSize, 0);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
            ExpandImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
					new_data, 0, 0, width - 1, height - 1, event_handler);
            return new_data;
         }
			else if ((width < data->GetWidth()) && (height > data->GetHeight()))
  			{
				G42ImageData * new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), height, 8*TrueColorPixelSize, 0);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
            ExpandImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
					new_data, 0, 0, data->GetWidth() - 1, height - 1, event_handler);
            G42ImageData * new_data2 = new G42ImageData(new_data->GetData(),
              	width, new_data->GetHeight(), 8*TrueColorPixelSize, 0);
				new_data2->ReferenceCount++;
				new_data2->SetFinished(true);
				new_data2->SetValidStartRow(0);
				new_data2->SetValidNumRows(new_data2->GetHeight());
            ShrinkImage(new_data, 0, 0, new_data->GetWidth() - 1, new_data->GetHeight() - 1,
					new_data2, 0, 0, width - 1, new_data->GetHeight() - 1, true, event_handler);
            delete new_data;
            return new_data2;
			}
			G42ImageData * new_data = new G42ImageData(data->GetData(),
				width, data->GetHeight(), 8*TrueColorPixelSize, 0);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
         ExpandImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
				new_data, 0, 0, width - 1, data->GetHeight() - 1, event_handler);
         G42ImageData * new_data2 = new G42ImageData(new_data->GetData(),
           	new_data->GetWidth(), height, 8*TrueColorPixelSize, 0);
			new_data2->ReferenceCount++;
			new_data2->SetFinished(true);
			new_data2->SetValidStartRow(0);
			new_data2->SetValidNumRows(new_data2->GetHeight());
         ShrinkImage(new_data, 0, 0, new_data->GetWidth() - 1, new_data->GetHeight() - 1,
				new_data2, 0, 0, new_data->GetWidth() - 1, height - 1, true, event_handler);
         delete new_data;
         return new_data2;
		}
		case 8:
		{
			if (data->GetNumPalette() != 0)
			{
				if ((width <= data->GetWidth()) && (height <= data->GetHeight()))
            {
					G42ImageData * new_data = new G42ImageData(data->GetData(),
						width, height, 8*TrueColorPixelSize, 0);
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
               ShrinkImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
						new_data, 0, 0, width - 1, height - 1, true, event_handler);
               return new_data;
            }
				if ((width >= data->GetWidth()) && (height >= data->GetHeight()))
            {
				   G42ImageData * new_data = new G42ImageData(data->GetData(),
						width, height, 8*TrueColorPixelSize, 0);
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
               ExpandImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
						new_data, 0, 0, width - 1, height - 1, event_handler);
               return new_data;
            }
				else if ((width < data->GetWidth()) && (height > data->GetHeight()))
   			{
					G42ImageData * new_data = new G42ImageData(data->GetData(),
						data->GetWidth(), height, 8*TrueColorPixelSize, 0);
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
               ExpandImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
						new_data, 0, 0, data->GetWidth() - 1, height - 1, event_handler);
               G42ImageData * new_data2 = new G42ImageData(new_data->GetData(),
               	width, new_data->GetHeight(), 8*TrueColorPixelSize, 0);
					new_data2->ReferenceCount++;
					new_data2->SetFinished(true);
					new_data2->SetValidStartRow(0);
					new_data2->SetValidNumRows(new_data2->GetHeight());
               ShrinkImage(new_data, 0, 0, new_data->GetWidth() - 1, new_data->GetHeight() - 1,
						new_data2, 0, 0, width - 1, new_data->GetHeight() - 1, true, event_handler);
               delete new_data;
               return new_data2;
				}
				G42ImageData * new_data = new G42ImageData(data->GetData(),
					width, data->GetHeight(), 8*TrueColorPixelSize, 0);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
            ExpandImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
					new_data, 0, 0, width - 1, data->GetHeight() - 1, event_handler);
            G42ImageData * new_data2 = new G42ImageData(new_data->GetData(),
              	new_data->GetWidth(), height, 8*TrueColorPixelSize, 0);
				new_data2->ReferenceCount++;
				new_data2->SetFinished(true);
				new_data2->SetValidStartRow(0);
				new_data2->SetValidNumRows(new_data2->GetHeight());
            ShrinkImage(new_data, 0, 0, new_data->GetWidth() - 1, new_data->GetHeight() - 1,
					new_data2, 0, 0, new_data->GetWidth() - 1, height - 1, true, event_handler);
            delete new_data;
            return new_data2;
			}
			else
			{
				if ((width <= data->GetWidth()) && (height <= data->GetHeight()))
            {
					G42ImageData * new_data = new G42ImageData(data->GetData(),
						width, height, 8, 0);
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
               ShrinkImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
						new_data, 0, 0, width - 1, height - 1, true, event_handler);
               return new_data;
            }
				if ((width >= data->GetWidth()) && (height >= data->GetHeight()))
            {
				   G42ImageData * new_data = new G42ImageData(data->GetData(),
						width, height, 8, 0);
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
               ExpandImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
						new_data, 0, 0, width - 1, height - 1, event_handler);
               return new_data;
            }
				else if ((width < data->GetWidth()) && (height > data->GetHeight()))
   			{
					G42ImageData * new_data = new G42ImageData(data->GetData(),
						data->GetWidth(), height, 8, 0);
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
               ExpandImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
						new_data, 0, 0, data->GetWidth() - 1, height - 1, event_handler);
               G42ImageData * new_data2 = new G42ImageData(new_data->GetData(),
               	width, new_data->GetHeight(), 8, 0);
					new_data2->ReferenceCount++;
					new_data2->SetFinished(true);
					new_data2->SetValidStartRow(0);
					new_data2->SetValidNumRows(new_data2->GetHeight());
               ShrinkImage(new_data, 0, 0, new_data->GetWidth() - 1, new_data->GetHeight() - 1,
						new_data2, 0, 0, width - 1, new_data->GetHeight() - 1, true, event_handler);
               delete new_data;
               return new_data2;
				}
				G42ImageData * new_data = new G42ImageData(data->GetData(),
					width, data->GetHeight(), 8, 0);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
            ExpandImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
					new_data, 0, 0, width - 1, data->GetHeight() - 1, event_handler);
            G42ImageData * new_data2 = new G42ImageData(new_data->GetData(),
              	new_data->GetWidth(), height, 8, 0);
				new_data2->ReferenceCount++;
				new_data2->SetFinished(true);
				new_data2->SetValidStartRow(0);
				new_data2->SetValidNumRows(new_data2->GetHeight());
            ShrinkImage(new_data, 0, 0, new_data->GetWidth() - 1, new_data->GetHeight() - 1,
					new_data2, 0, 0, new_data->GetWidth() - 1, height - 1, true, event_handler);
            delete new_data;
            return new_data2;
			}
		}
		case 4:
		{
			if (data->GetNumPalette() != 0)
			{
				if ((width <= data->GetWidth()) && (height <= data->GetHeight()))
            {
					G42ImageData * new_data = new G42ImageData(data->GetData(),
						width, height, 8*TrueColorPixelSize, 0);
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
               ShrinkImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
						new_data, 0, 0, width - 1, height - 1, true, event_handler);
               return new_data;
            }
				if ((width >= data->GetWidth()) && (height >= data->GetHeight()))
            {
				   G42ImageData * new_data = new G42ImageData(data->GetData(),
						width, height, 8*TrueColorPixelSize, 0);
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
               ExpandImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
						new_data, 0, 0, width - 1, height - 1, event_handler);
               return new_data;
            }
				else if ((width < data->GetWidth()) && (height > data->GetHeight()))
   			{
					G42ImageData * new_data = new G42ImageData(data->GetData(),
						data->GetWidth(), height, 8*TrueColorPixelSize, 0);
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
               ExpandImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
						new_data, 0, 0, data->GetWidth() - 1, height - 1, event_handler);
               G42ImageData * new_data2 = new G42ImageData(new_data->GetData(),
               	width, new_data->GetHeight(), 8*TrueColorPixelSize, 0);
					new_data2->ReferenceCount++;
					new_data2->SetFinished(true);
					new_data2->SetValidStartRow(0);
					new_data2->SetValidNumRows(new_data2->GetHeight());
               ShrinkImage(new_data, 0, 0, new_data->GetWidth() - 1, new_data->GetHeight() - 1,
						new_data2, 0, 0, width - 1, new_data->GetHeight() - 1, true, event_handler);
               delete new_data;
               return new_data2;
				}
				G42ImageData * new_data = new G42ImageData(data->GetData(),
					width, data->GetHeight(), 8*TrueColorPixelSize, 0);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
            ExpandImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
					new_data, 0, 0, width - 1, data->GetHeight() - 1, event_handler);
            G42ImageData * new_data2 = new G42ImageData(new_data->GetData(),
              	new_data->GetWidth(), height, 8*TrueColorPixelSize, 0);
				new_data2->ReferenceCount++;
				new_data2->SetFinished(true);
				new_data2->SetValidStartRow(0);
				new_data2->SetValidNumRows(new_data2->GetHeight());
            ShrinkImage(new_data, 0, 0, new_data->GetWidth() - 1, new_data->GetHeight() - 1,
					new_data2, 0, 0, new_data->GetWidth() - 1, height - 1, true, event_handler);
            delete new_data;
            return new_data2;
			}
			else
			{
				if ((width <= data->GetWidth()) && (height <= data->GetHeight()))
            {
					G42ImageData * new_data = new G42ImageData(data->GetData(),
						width, height, 8, 0);
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
               ShrinkImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
						new_data, 0, 0, width - 1, height - 1, true, event_handler);
               return new_data;
            }
				if ((width >= data->GetWidth()) && (height >= data->GetHeight()))
            {
				   G42ImageData * new_data = new G42ImageData(data->GetData(),
						width, height, 8, 0);
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
               ExpandImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
						new_data, 0, 0, width - 1, height - 1, event_handler);
               return new_data;
            }
				else if ((width < data->GetWidth()) && (height > data->GetHeight()))
   			{
					G42ImageData * new_data = new G42ImageData(data->GetData(),
						data->GetWidth(), height, 8, 0);
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
               ExpandImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
						new_data, 0, 0, data->GetWidth() - 1, height - 1, event_handler);
               G42ImageData * new_data2 = new G42ImageData(new_data->GetData(),
               	width, new_data->GetHeight(), 8, 0);
					new_data2->ReferenceCount++;
					new_data2->SetFinished(true);
					new_data2->SetValidStartRow(0);
					new_data2->SetValidNumRows(new_data2->GetHeight());
               ShrinkImage(new_data, 0, 0, new_data->GetWidth() - 1, new_data->GetHeight() - 1,
						new_data2, 0, 0, width - 1, new_data->GetHeight() - 1, true, event_handler);
               delete new_data;
               return new_data2;
				}
				G42ImageData * new_data = new G42ImageData(data->GetData(),
					width, data->GetHeight(), 8, 0);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
            ExpandImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
					new_data, 0, 0, width - 1, data->GetHeight() - 1, event_handler);
            G42ImageData * new_data2 = new G42ImageData(new_data->GetData(),
              	new_data->GetWidth(), height, 8, 0);
				new_data2->ReferenceCount++;
				new_data2->SetFinished(true);
				new_data2->SetValidStartRow(0);
				new_data2->SetValidNumRows(new_data2->GetHeight());
            ShrinkImage(new_data, 0, 0, new_data->GetWidth() - 1, new_data->GetHeight() - 1,
					new_data2, 0, 0, new_data->GetWidth() - 1, height - 1, true, event_handler);
            delete new_data;
            return new_data2;
			}
		}
		case 1:
		{
			if (data->GetNumPalette() != 0)
			{
				if ((width <= data->GetWidth()) && (height <= data->GetHeight()))
            {
					G42ImageData * new_data = new G42ImageData(data->GetData(),
						width, height, 8*TrueColorPixelSize, 0);
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
               ShrinkImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
						new_data, 0, 0, width - 1, height - 1, true, event_handler);
               return new_data;
            }
				if ((width >= data->GetWidth()) && (height >= data->GetHeight()))
            {
				   G42ImageData * new_data = new G42ImageData(data->GetData(),
						width, height, 8*TrueColorPixelSize, 0);
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
               ExpandImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
						new_data, 0, 0, width - 1, height - 1, event_handler);
               return new_data;
            }
				else if ((width < data->GetWidth()) && (height > data->GetHeight()))
   			{
					G42ImageData * new_data = new G42ImageData(data->GetData(),
						data->GetWidth(), height, 8*TrueColorPixelSize, 0);
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
               ExpandImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
						new_data, 0, 0, data->GetWidth() - 1, height - 1, event_handler);
               G42ImageData * new_data2 = new G42ImageData(new_data->GetData(),
               	width, new_data->GetHeight(), 8*TrueColorPixelSize, 0);
					new_data2->ReferenceCount++;
					new_data2->SetFinished(true);
					new_data2->SetValidStartRow(0);
					new_data2->SetValidNumRows(new_data2->GetHeight());
               ShrinkImage(new_data, 0, 0, new_data->GetWidth() - 1, new_data->GetHeight() - 1,
						new_data2, 0, 0, width - 1, new_data->GetHeight() - 1, true, event_handler);
               delete new_data;
               return new_data2;
				}
				G42ImageData * new_data = new G42ImageData(data->GetData(),
					width, data->GetHeight(), 8*TrueColorPixelSize, 0);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
            ExpandImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
					new_data, 0, 0, width - 1, data->GetHeight() - 1, event_handler);
            G42ImageData * new_data2 = new G42ImageData(new_data->GetData(),
              	new_data->GetWidth(), height, 8*TrueColorPixelSize, 0);
				new_data2->ReferenceCount++;
				new_data2->SetFinished(true);
				new_data2->SetValidStartRow(0);
				new_data2->SetValidNumRows(new_data2->GetHeight());
            ShrinkImage(new_data, 0, 0, new_data->GetWidth() - 1, new_data->GetHeight() - 1,
					new_data2, 0, 0, new_data->GetWidth() - 1, height - 1, true, event_handler);
            delete new_data;
            return new_data2;
			}
			else
			{
				if ((width <= data->GetWidth()) && (height <= data->GetHeight()))
            {
					G42ImageData * new_data = new G42ImageData(data->GetData(),
						width, height, 8, 0);
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
               ShrinkImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
						new_data, 0, 0, width - 1, height - 1, true, event_handler);
               return new_data;
            }
				if ((width >= data->GetWidth()) && (height >= data->GetHeight()))
            {
				   G42ImageData * new_data = new G42ImageData(data->GetData(),
						width, height, 8, 0);
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
               ExpandImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
						new_data, 0, 0, width - 1, height - 1, event_handler);
               return new_data;
            }
				else if ((width < data->GetWidth()) && (height > data->GetHeight()))
   			{
					G42ImageData * new_data = new G42ImageData(data->GetData(),
						data->GetWidth(), height, 8, 0);
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
               ExpandImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
						new_data, 0, 0, data->GetWidth() - 1, height - 1, event_handler);
               G42ImageData * new_data2 = new G42ImageData(new_data->GetData(),
               	width, new_data->GetHeight(), 8, 0);
					new_data2->ReferenceCount++;
					new_data2->SetFinished(true);
					new_data2->SetValidStartRow(0);
					new_data2->SetValidNumRows(new_data2->GetHeight());
               ShrinkImage(new_data, 0, 0, new_data->GetWidth() - 1, new_data->GetHeight() - 1,
						new_data2, 0, 0, width - 1, new_data->GetHeight() - 1, true, event_handler);
               delete new_data;
               return new_data2;
				}
				G42ImageData * new_data = new G42ImageData(data->GetData(),
					width, data->GetHeight(), 8, 0);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
            ExpandImage(data, 0, 0, data->GetWidth() - 1, data->GetHeight() - 1,
					new_data, 0, 0, width - 1, data->GetHeight() - 1, event_handler);
            G42ImageData * new_data2 = new G42ImageData(new_data->GetData(),
              	new_data->GetWidth(), height, 8, 0);
				new_data2->ReferenceCount++;
				new_data2->SetFinished(true);
				new_data2->SetValidStartRow(0);
				new_data2->SetValidNumRows(new_data2->GetHeight());
            ShrinkImage(new_data, 0, 0, new_data->GetWidth() - 1, new_data->GetHeight() - 1,
					new_data2, 0, 0, new_data->GetWidth() - 1, height - 1, true, event_handler);
            delete new_data;
            return new_data2;
			}
		}
	}
}
void
FastShrinkImage(G42ImageData * data, int left, int top, int right, int bottom,
	G42ImageData * new_data, int left2, int top2, int right2, int bottom2)
{
	int width = right - left + 1;
	int height = bottom - top + 1;
	int * wmap = new int [width];
	int * hmap = new int [height];
	int32 n;
	int new_width = right2 - left2;
	for (n = 0; n < width; n++)
	{
		wmap[n] = (int)(((float)(n * new_width) + ((float)(width-1)/2.0))
			/ (float)(width-1));
	}
	int new_height = bottom2 - top2;
	for (n = 0; n < height; n++)
	{
		hmap[n] = (int)(((float)(n * new_height) + ((float)(height-1)/2.0))
			/ (float)(height-1));
	}
	switch (data->GetDepth())
	{
		case 1:
		{
 			G42LockedID1BitIterator * image_it1
 				= new G42LockedID1BitIterator(data, false);
 			image_it1->SetArea(left, top, right, bottom);
 			G42ImageData1BitIterator * image_it2
 				= new G42ImageData1BitIterator(new_data, false);
 			image_it2->SetArea(left2, top2, right2, bottom2);
 			int windex = 0;
 			int hindex = 0;
 			int windex2 = 0, hindex2 = 0;
 			do
 			{
 				int32 black_count = 0L;
 				int pix_count = 0;
 				do
 				{
 					int count = 0;
 					do
 					{
 						unsigned char val = (unsigned char)(*image_it1);
 						if ((unsigned char)(*image_it1))
                    	black_count++;
 						pix_count++;
 						hindex2++;
 						if ((hindex2 >= height) || (hmap[hindex2] != hindex))
 						{
 							hindex2--;
 							break;
 						}
 						image_it1->IncRow();
 						count++;
 					} while (1);
 					for (int n = 0; n < count; n++)
 					{
 						hindex2--;
 						image_it1->DecRow();
 					}
 					windex2++;
 					if ((windex2 >= width) || (wmap[windex2] != windex))
 					{
 						windex2--;
 						break;
 					}
 					image_it1->Increment();
 				} while (1);
            if (black_count >= ((pix_count + 1) >> 1))
 					image_it2->Set8(1);
            else
             	image_it2->Set8(0);
				image_it1->Increment();
				windex++;
				windex2++;
				if (!image_it2->Increment())
				{
					if (!image_it2->NextRow())
						break;
					windex = 0;
					windex2 = 0;
					hindex++;
					do
					{
						image_it1->NextRow();
						hindex2++;
					} while (hmap[hindex2] != hindex);
				}
			} while (1);
			delete image_it1;
			delete image_it2;
			break;
		}
	}
	delete [] wmap;
	delete [] hmap;
}
void
ShrinkImage(G42ImageData * data, int left, int top, int right, int bottom,
	G42ImageData * new_data, int left2, int top2, int right2, int bottom2,
   bool update, G42EventUpdateHandler * event_handler)
{
	int width = right - left + 1;
	int height = bottom - top + 1;
	int * wmap = new int [width];
	int * hmap = new int [height];
	int32 n;
	int new_width = right2 - left2;
	for (n = 0; n < width; n++)
	{
		wmap[n] = (int)(((float)(n * new_width) + ((float)(width-1)/2.0))
			/ (float)(width-1));
	}
	int new_height = bottom2 - top2;
	for (n = 0; n < height; n++)
	{
		hmap[n] = (int)(((float)(n * new_height) + ((float)(height-1)/2.0))
			/ (float)(height-1));
	}
   int32 row_count = 0L;
	if (update)
   {
		event_handler->SetDenominator((int32)new_height);
   	event_handler->Start();
   	event_handler->Set(0L);
   }
	switch (data->GetDepth())
	{
		case 32:
		case 24:
		{
			G42LockedID24BitIterator * image_it1
				= new G42LockedID24BitIterator(data, false);
			image_it1->SetArea(left, top, right, bottom);
			G42ImageData24BitIterator * image_it2
				= new G42ImageData24BitIterator(new_data, false);
			image_it2->SetArea(left2, top2, right2, bottom2);
			int windex = 0;
			int hindex = 0;
			int windex2 = 0, hindex2 = 0;
			do
			{
				int32 red = 0L, green = 0L, blue = 0L;
				int pix_count = 0;
				do
				{
					int count = 0;
					do
					{
						red += image_it1->GetRed32();
						green += image_it1->GetGreen32();
						blue += image_it1->GetBlue32();
						pix_count++;
						hindex2++;
						if ((hindex2 >= height) || (hmap[hindex2] != hindex))
						{
							hindex2--;
							break;
						}
						image_it1->IncRow();
						count++;
					} while (1);
					for (int n = 0; n < count; n++)
					{
						hindex2--;
						image_it1->DecRow();
					}
					windex2++;
					if ((windex2 >= width) || (wmap[windex2] != windex))
					{
						windex2--;
						break;
					}
					image_it1->Increment();
				} while (1);
				red /= pix_count;
				green /= pix_count;
				blue /= pix_count;
				if (red > 255)
					red = 255;
				if (red < 0)
					red = 0;
				if (green > 255)
					green = 255;
				if (green < 0)
					green = 0;
				if (blue > 255)
					blue = 255;
				if (blue < 0)
					blue = 0;
				image_it2->Set((unsigned char)blue, (unsigned char)green,
					(unsigned char)red);
				image_it1->Increment();
				windex++;
            windex2++;
				if (!image_it2->Increment())
				{
					if (!image_it2->NextRow())
						break;
               if (update)
               {
	               row_count++;
   	            event_handler->Set(row_count);
               }
					windex = 0;
               windex2 = 0;
					hindex++;
					do
					{
						image_it1->NextRow();
                  hindex2++;
					} while (hmap[hindex2] != hindex);
				}
			} while (1);
			delete image_it1;
			delete image_it2;
			break;
		}
		case 8:
		{
			if (data->GetNumPalette() != 0)
			{
				G42Color * palette = data->GetPalette();
				G42LockedID8BitIterator * image_it1
					= new G42LockedID8BitIterator(data, false);
				image_it1->SetArea(left, top, right, bottom);
				G42ImageData24BitIterator * image_it2
					= new G42ImageData24BitIterator(new_data, false);
				image_it2->SetArea(left2, top2, right2, bottom2);
				int windex = 0;
				int hindex = 0;
				int windex2 = 0, hindex2 = 0;
				do
				{
					int32 red = 0L, green = 0L, blue = 0L;
					int pix_count = 0;
					do
					{
						int count = 0;
						do
						{
                  	unsigned char val = (unsigned char)(*image_it1);
							red += palette[val].red;
							green += palette[val].green;
							blue += palette[val].blue;
							pix_count++;
							hindex2++;
							if ((hindex2 >= height) || (hmap[hindex2] != hindex))
							{
								hindex2--;
								break;
							}
							image_it1->IncRow();
							count++;
						} while (1);
						for (int n = 0; n < count; n++)
						{
							hindex2--;
							image_it1->DecRow();
						}
						windex2++;
						if ((windex2 >= width) || (wmap[windex2] != windex))
						{
							windex2--;
							break;
						}
						image_it1->Increment();
					} while (1);
					red /= pix_count;
					green /= pix_count;
					blue /= pix_count;
					if (red > 255)
						red = 255;
					if (red < 0)
						red = 0;
					if (green > 255)
						green = 255;
					if (green < 0)
						green = 0;
					if (blue > 255)
						blue = 255;
					if (blue < 0)
						blue = 0;
					image_it2->Set((unsigned char)blue, (unsigned char)green,
						(unsigned char)red);
					image_it1->Increment();
					windex++;
					windex2++;
					if (!image_it2->Increment())
					{
						if (!image_it2->NextRow())
							break;
	               if (update)
   	            {
      	        		row_count++;
         	         event_handler->Set(row_count);
                  }
						windex = 0;
						windex2 = 0;
						hindex++;
						do
						{
							image_it1->NextRow();
							hindex2++;
						} while (hmap[hindex2] != hindex);
					}
				} while (1);
				delete image_it1;
				delete image_it2;
			}
			else
			{
				G42LockedID8BitIterator * image_it1
					= new G42LockedID8BitIterator(data, false);
				image_it1->SetArea(left, top, right, bottom);
				G42ImageData8BitIterator * image_it2
					= new G42ImageData8BitIterator(new_data, false);
				image_it2->SetArea(left2, top2, right2, bottom2);
				int windex = 0;
				int hindex = 0;
				int windex2 = 0, hindex2 = 0;
				do
				{
					int32 pixel = 0;
					int pix_count = 0;
					do
					{
						int count = 0;
						do
						{
							pixel += (unsigned char)(*image_it1);
							pix_count++;
							hindex2++;
							if ((hindex2 >= height) || (hmap[hindex2] != hindex))
							{
								hindex2--;
								break;
							}
							image_it1->IncRow();
							count++;
						} while (1);
						for (int n = 0; n < count; n++)
						{
							hindex2--;
							image_it1->DecRow();
						}
						windex2++;
						if ((windex2 >= width) || (wmap[windex2] != windex))
						{
							windex2--;
							break;
						}
						image_it1->Increment();
					} while (1);
					pixel /= pix_count;
					if (pixel > 255)
						pixel = 255;
					if (pixel < 0)
						pixel = 0;
					image_it2->Set((unsigned char)pixel);
					image_it1->Increment();
					windex++;
					windex2++;
					if (!image_it2->Increment())
					{
						if (!image_it2->NextRow())
							break;
   	            if (update)
	               {
      	            row_count++;
         	         event_handler->Set(row_count);
                  }
						windex = 0;
						windex2 = 0;
						hindex++;
						do
						{
							image_it1->NextRow();
							hindex2++;
						} while (hmap[hindex2] != hindex);
					}
				} while (1);
				delete image_it1;
				delete image_it2;
			}
			break;
		}
		case 4:
		{
			if (data->GetNumPalette() != 0)
			{
				G42Color * palette = data->GetPalette();
				G42LockedID4BitIterator * image_it1
					= new G42LockedID4BitIterator(data, false);
				image_it1->SetArea(left, top, right, bottom);
				G42ImageData24BitIterator * image_it2
					= new G42ImageData24BitIterator(new_data, false);
				image_it2->SetArea(left2, top2, right2, bottom2);
				int windex = 0;
				int hindex = 0;
				int windex2 = 0, hindex2 = 0;
				do
				{
					int32 red = 0L, green = 0L, blue = 0L;
					int pix_count = 0;
					do
					{
						int count = 0;
						do
						{
                  	unsigned char val = (unsigned char)(*image_it1);
							red += palette[val].red;
							green += palette[val].green;
							blue += palette[val].blue;
							pix_count++;
							hindex2++;
							if ((hindex2 >= height) || (hmap[hindex2] != hindex))
							{
								hindex2--;
								break;
							}
							image_it1->IncRow();
							count++;
						} while (1);
						for (int n = 0; n < count; n++)
						{
							hindex2--;
							image_it1->DecRow();
						}
						windex2++;
						if ((windex2 >= width) || (wmap[windex2] != windex))
						{
							windex2--;
							break;
						}
						image_it1->Increment();
					} while (1);
					red /= pix_count;
					green /= pix_count;
					blue /= pix_count;
					if (red > 255)
						red = 255;
					if (red < 0)
						red = 0;
					if (green > 255)
						green = 255;
					if (green < 0)
						green = 0;
					if (blue > 255)
						blue = 255;
					if (blue < 0)
						blue = 0;
					image_it2->Set((unsigned char)blue, (unsigned char)green,
						(unsigned char)red);
					image_it1->Increment();
					windex++;
					windex2++;
					if (!image_it2->Increment())
					{
						if (!image_it2->NextRow())
							break;
	               if (update)
   	            {
      	            row_count++;
         	         event_handler->Set(row_count);
                  }
						windex = 0;
						windex2 = 0;
						hindex++;
						do
						{
							image_it1->NextRow();
							hindex2++;
						} while (hmap[hindex2] != hindex);
					}
				} while (1);
				delete image_it1;
				delete image_it2;
			}
			else
			{
				G42LockedID4BitIterator * image_it1
					= new G42LockedID4BitIterator(data, false);
				image_it1->SetArea(left, top, right, bottom);
				G42ImageData8BitIterator * image_it2
					= new G42ImageData8BitIterator(new_data, false);
				image_it2->SetArea(left2, top2, right2, bottom2);
				int windex = 0;
				int hindex = 0;
				int windex2 = 0, hindex2 = 0;
				do
				{
					int32 pixel = 0;
					int pix_count = 0;
					do
					{
						int count = 0;
						do
						{
							int val = (unsigned char)(*image_it1);
							pixel += ((val << 4) + val);
							pix_count++;
							hindex2++;
							if ((hindex2 >= height) || (hmap[hindex2] != hindex))
							{
								hindex2--;
								break;
							}
							image_it1->IncRow();
							count++;
						} while (1);
						for (int n = 0; n < count; n++)
						{
							hindex2--;
							image_it1->DecRow();
						}
						windex2++;
						if ((windex2 >= width) || (wmap[windex2] != windex))
						{
							windex2--;
							break;
						}
						image_it1->Increment();
					} while (1);
					pixel /= pix_count;
					if (pixel > 255)
						pixel = 255;
					if (pixel < 0)
						pixel = 0;
					image_it2->Set((unsigned char)pixel);
					image_it1->Increment();
					windex++;
					windex2++;
					if (!image_it2->Increment())
					{
						if (!image_it2->NextRow())
							break;
	               if (update)
   	            {
      	            row_count++;
         	         event_handler->Set(row_count);
                  }
						windex = 0;
						windex2 = 0;
						hindex++;
						do
						{
							image_it1->NextRow();
							hindex2++;
						} while (hmap[hindex2] != hindex);
					}
				} while (1);
				delete image_it1;
				delete image_it2;
			}
			break;
		}
		case 1:
		{
			if (data->GetNumPalette() != 0)
			{
				G42Color * palette = data->GetPalette();
				G42LockedID1BitIterator * image_it1
					= new G42LockedID1BitIterator(data, false);
				image_it1->SetArea(left, top, right, bottom);
				G42ImageData24BitIterator * image_it2
					= new G42ImageData24BitIterator(new_data, false);
				image_it2->SetArea(left2, top2, right2, bottom2);
				int windex = 0;
				int hindex = 0;
				int windex2 = 0, hindex2 = 0;
				do
				{
					int32 red = 0L, green = 0L, blue = 0L;
					int pix_count = 0;
					do
					{
						int count = 0;
						do
						{
							unsigned char val = (unsigned char)(*image_it1);
							if ((unsigned char)(*image_it1))
							{
								red += palette[1].red;
								green += palette[1].green;
								blue += palette[1].blue;
							}
							else
							{
								red += palette[0].red;
								green += palette[0].green;
								blue += palette[0].blue;
							}
							pix_count++;
							hindex2++;
							if ((hindex2 >= height) || (hmap[hindex2] != hindex))
							{
								hindex2--;
								break;
							}
							image_it1->IncRow();
							count++;
						} while (1);
						for (int n = 0; n < count; n++)
						{
							hindex2--;
							image_it1->DecRow();
						}
						windex2++;
						if ((windex2 >= width) || (wmap[windex2] != windex))
						{
							windex2--;
							break;
						}
						image_it1->Increment();
					} while (1);
					red /= pix_count;
					green /= pix_count;
					blue /= pix_count;
					if (red > 255)
						red = 255;
					if (red < 0)
						red = 0;
					if (green > 255)
						green = 255;
					if (green < 0)
						green = 0;
					if (blue > 255)
						blue = 255;
					if (blue < 0)
						blue = 0;
					image_it2->Set((unsigned char)blue, (unsigned char)green,
						(unsigned char)red);
					image_it1->Increment();
					windex++;
					windex2++;
					if (!image_it2->Increment())
					{
						if (!image_it2->NextRow())
							break;
	               if (update)
   	            {
      	            row_count++;
         	         event_handler->Set(row_count);
                  }
						windex = 0;
						windex2 = 0;
						hindex++;
						do
						{
							image_it1->NextRow();
							hindex2++;
						} while (hmap[hindex2] != hindex);
					}
				} while (1);
				delete image_it1;
				delete image_it2;
			}
			else
			{
				G42LockedID1BitIterator * image_it1
					= new G42LockedID1BitIterator(data, false);
				image_it1->SetArea(left, top, right, bottom);
				G42ImageData8BitIterator * image_it2
					= new G42ImageData8BitIterator(new_data, false);
				image_it2->SetArea(left2, top2, right2, bottom2);
				int windex = 0;
				int hindex = 0;
				int windex2 = 0, hindex2 = 0;
				do
				{
					int32 pixel = 0;
					int pix_count = 0;
					do
					{
						int count = 0;
						do
						{
							pixel += (unsigned char)(*image_it1);
							pix_count++;
							hindex2++;
							if ((hindex2 >= height) || (hmap[hindex2] != hindex))
							{
								hindex2--;
								break;
							}
							image_it1->IncRow();
							count++;
						} while (1);
						for (int n = 0; n < count; n++)
						{
							hindex2--;
							image_it1->DecRow();
						}
						windex2++;
						if ((windex2 >= width) || (wmap[windex2] != windex))
						{
							windex2--;
							break;
						}
						image_it1->Increment();
					} while (1);
					pixel /= pix_count;
					if (pixel > 255)
						pixel = 255;
					if (pixel < 0)
						pixel = 0;
					image_it2->Set((unsigned char)pixel);
					image_it1->Increment();
					windex++;
					windex2++;
					if (!image_it2->Increment())
					{
						if (!image_it2->NextRow())
							break;
	               if (update)
   	            {
      	            row_count++;
         	         event_handler->Set(row_count);
                  }
						windex = 0;
						windex2 = 0;
						hindex++;
						do
						{
							image_it1->NextRow();
							hindex2++;
						} while (hmap[hindex2] != hindex);
					}
				} while (1);
				delete image_it1;
				delete image_it2;
			}
			break;
		}
	}
	delete [] wmap;
	delete [] hmap;
	if (update)
	   event_handler->Stop();
}
void
ResizeImage(G42ImageData * data, int left, int top, int right, int bottom,
	G42ImageData * new_data, int left2, int top2, int right2, int bottom2)
{
	int MinRow = new_data->GetTileRowIndex(top2);
	int MaxRow = new_data->GetTileRowIndex(bottom2);
	int MinColumn = new_data->GetTileColumnIndex(left2);
	int MaxColumn = new_data->GetTileColumnIndex(right2);
	for (int n = MinRow; n <= MaxRow; n++)
   {
   	for (int i = MinColumn; i <= MaxColumn; i++)
      {
    		G42ImageTile * tile = new_data->GetTile(n, i);
         if (!tile)
         	return;
      }
   }
	if ((right2 - left2) <= (right - left) && (bottom2 - top2) <= (bottom - top))
	{
		ShrinkImage(data, left, top, right, bottom,
			new_data, left2, top2, right2, bottom2, false, 0);
		return;
	}
	StretchImage(data, new_data, left2, top2, right2, bottom2);
}
void
G42Image::ResizeImage(int width, int height)
{
	if ((width <= 0) || (height <= 0))
   	return;
	G42ImageData * new_data = Data;
	if (Data)
   {
   	if (Mask)
      {
      	delete Mask;
         Mask = 0;
      }
		new_data = ::ResizeEntireImage(Data, width, height, GetEventHandler());
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
