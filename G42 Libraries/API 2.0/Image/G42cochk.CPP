#include "g42idata.h"
#include "g42image.h"
#include "g42iview.h"
bool
CheckForGrayScale(G42ImageData * data)
{
	G42Color * palette = data->GetPalette();
   bool NotGray = false;
	for (int i = 0; i < data->GetNumPalette(); i++)
   {
		if ((palette[i].blue != palette[i].green)
      	|| (palette[i].blue != palette[i].red))
      {
      	NotGray = true;
         break;
      }
   }
   if (NotGray)
   	return false;
	unsigned char * map = new unsigned char [data->GetNumPalette()];
   for (int n = 0; n < data->GetNumPalette(); n++)
   {
		map[n] = (unsigned char)((((int32)palette[n].blue * (int32)58)
      	+ ((int32)palette[n].green * (int32)301)
			+ ((int32)palette[n].red * (int32)153)) >> 9);
   }
	data->DeletePalette();
  	for (int i = 0; i < data->GetNumTilesDown(); i++)
   {
     	for (int j = 0; j < data->GetNumTilesAcross(); j++)
      {
        	G42ImageTile * tile = data->GetTile(i, j);
         for (int n = 0; n < tile->GetHeight(); n++)
         {
          	G42ImageRow * row = tile->GetRow(n);
            unsigned char * ptr = *row;
            for (int m = 0; m < tile->GetTileWidth(); m++, ptr++)
            {
					*ptr = map[*ptr];
            }
         }
      }
   }
	delete [] map;
	return true;
}
G42ImageData *
CheckColorCount(G42ImageData * data)
{
	if (data->GetNumPalette() == 0)
   	return data;
	int * TestArray = new int [256];
   memset(TestArray, 0, 256 * sizeof(int));
	switch (data->GetDepth())
   {
   	case 8:
      {
			if (CheckForGrayScale(data))
      	{
         	delete [] TestArray;
            return data;
         }
      	for (int i = 0; i < data->GetNumTilesDown(); i++)
         {
         	for (int j = 0; j < data->GetNumTilesAcross(); j++)
            {
            	G42ImageTile * tile = data->GetTile(i, j);
               for (int n = 0; n < tile->GetHeight(); n++)
               {
               	G42ImageRow * row = tile->GetRow(n);
                  unsigned char * ptr = *row;
                  for (int m = 0; m < tile->GetTileWidth(); m++, ptr++)
                  {
                      TestArray[*ptr]++;
                  }
               }
            }
         }
         int count = 0;
         for (int i = 0; i < 256; i++)
         {
         	if (TestArray[i])
            	count++;
         }
         if (count == data->GetNumPalette())
         {
         	delete [] TestArray;
            return data;
         }
         G42Color * old_pal = data->GetPalette();
         G42Color * new_pal = new G42Color [count];
         int * map = new int [256];
         memset(map, 0, 256 * sizeof(int));
         for (int i = 0, count = 0; i < 256; i++)
         {
          	if (TestArray[i])
            {
					map[i] = count;
            	new_pal[count].blue = old_pal[i].blue;
            	new_pal[count].green = old_pal[i].green;
            	new_pal[count].red = old_pal[i].red;
               count++;
            }
         }
#if 0
         if (count <= 2)
         {
				G42ImageData * new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), data->GetHeight(), 1, 2);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
            new_data->SetPalette(new_pal);
         }
         else if (count <= 16)
         {
				G42ImageData * new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), data->GetHeight(), 4, count);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
            new_data->SetPalette(new_pal);
         }
         else
#endif
         {
	         data->SetNumPalette(count);
   	      data->SetPalette(new_pal);
	      	for (int i = 0; i < data->GetNumTilesDown(); i++)
   	      {
      	   	for (int j = 0; j < data->GetNumTilesAcross(); j++)
         	   {
            		G42ImageTile * tile = data->GetTile(i, j);
               	for (int n = 0; n < tile->GetHeight(); n++)
	               {
   	            	G42ImageRow * row = tile->GetRow(n);
      	            unsigned char * ptr = *row;
         	         for (int m = 0; m < tile->GetTileWidth(); m++, ptr++)
            	      {
								*ptr = map[*ptr];
	                  }
   	            }
      	      }
            }
         }
         delete [] map;
         delete [] TestArray;
         return data;
      }
      case 4:
      {
      	for (int i = 0; i < data->GetNumTilesDown(); i++)
         {
         	for (int j = 0; j < data->GetNumTilesAcross(); j++)
            {
            	G42ImageTile * tile = data->GetTile(i, j);
               for (int n = 0; n < tile->GetHeight(); n++)
               {
               	G42ImageRow * row = tile->GetRow(n);
                  unsigned char * ptr = *row;
                  for (int m = 0; m < (tile->GetTileWidth() >> 1); m++, ptr++)
  		            {
                 		unsigned char val = (unsigned char)(*ptr >> 4);
                     TestArray[val]++;
                     val = (unsigned char)(*ptr & 0x0f);
                     TestArray[val]++;
                  }
                  if (tile->GetTileWidth() & 0x01)
                  {
                 		unsigned char val = (unsigned char)(*ptr >> 4);
                     TestArray[val]++;
                  }
               }
            }
         }
         int count = 0;
         for (int i = 0; i < 256; i++)
         {
         	if (TestArray[i])
            	count++;
         }
         if (count == data->GetNumPalette())
         {
         	delete [] TestArray;
            return data;
         }
         G42Color * old_pal = data->GetPalette();
         G42Color * new_pal = new G42Color [count];
         int * map = new int [256];
         memset(map, 0, 256 * sizeof(int));
         for (int i = 0, count = 0; i < 256; i++)
         {
          	if (TestArray[i])
            {
					map[i] = count;
            	new_pal[count].blue = old_pal[i].blue;
            	new_pal[count].green = old_pal[i].green;
            	new_pal[count].red = old_pal[i].red;
               count++;
            }
         }
         data->SetNumPalette(count);
  	      data->SetPalette(new_pal);
      	for (int i = 0; i < data->GetNumTilesDown(); i++)
  	      {
     	   	for (int j = 0; j < data->GetNumTilesAcross(); j++)
        	   {
           		G42ImageTile * tile = data->GetTile(i, j);
              	for (int n = 0; n < tile->GetHeight(); n++)
               {
  	            	G42ImageRow * row = tile->GetRow(n);
     	            unsigned char * ptr = *row;
        	         for (int m = 0; m < (tile->GetTileWidth() >> 1); m++, ptr++)
           	      {
                  	unsigned char val = (unsigned char)(*ptr >> 4);
                     unsigned char result = map[val];
                     val = (unsigned char)(*ptr & 0x0f);
                     val = map[val];
							*ptr = (unsigned char)((result << 4) | val);
                  }
                  if (tile->GetTileWidth() & 0x01)
                  {
                  	unsigned char val = (unsigned char)(*ptr >> 4);
                     unsigned char result = map[val];
							*ptr = (unsigned char)((result << 4) & 0xf0);
                  }
  	            }
     	      }
         }
         delete [] map;
         delete [] TestArray;
         return data;
      	break;
      }
      case 1:
      	break;
   }
   delete [] TestArray;
   return data;
}
void
G42Image::CheckColorCount(void)
{
	G42ImageData * new_data = Data;
	if (Data)
   {
   	new_data = ::CheckColorCount(Data);
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
}
