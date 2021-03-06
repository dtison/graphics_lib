#include "g42idata.h"
#include "g42iter.h"
#include "g42event.h"
#include "g42image.h"
#include "g42iview.h"
class G42BGRLum
{
private:
	unsigned char	Blue;
	unsigned char	Green;
	unsigned char	Red;
	unsigned char	Lum;
	unsigned char	Index;
public:
	G42BGRLum(void)
		: Blue(0), Green(0), Red(0), Lum(0), Index(0) {}
	~G42BGRLum(void)
   	{}
	void Set(unsigned char blue, unsigned char green, unsigned char red,
		unsigned char index = 0);
	unsigned char GetBlue(void) const
		{return Blue;}
	unsigned char GetGreen(void) const
		{return Green;}
	unsigned char GetRed(void) const
		{return Red;}
	unsigned char GetLum(void) const
		{return Lum;}
	unsigned char GetIndex(void) const
		{return Index;}
};
inline void
G42BGRLum::Set(unsigned char blue, unsigned char green, unsigned char red,
	unsigned char index)
{
	Blue = blue;
	Green = green;
	Red = red;
	Index = index;
	Lum = (unsigned char)((((int32)blue * (int32)58) + ((int32)green * (int32)301)
		+ ((int32)red * (int32)153)) >> 9);
}
G42ImageData *
RemoveNoise(G42ImageData* data, int threshold, G42EventUpdateHandler * event_handler)
{
	event_handler->SetDenominator((int32)data->GetHeight());
   int32 row_count = 0L;
	if (data->GetNumPalette() == 0)
	{
		if (data->GetDepth() >= 8)
		{
			int32 row_width = data->GetWidth();
			G42ImageData24BitIterator image_it(data);
			row_width *= image_it.GetCellSize();
			unsigned char * trow1 = new unsigned char [row_width];
			unsigned char * trow2 = new unsigned char [row_width];
			memset(trow1, 0, row_width);
			memset(trow2, 0, row_width);
			unsigned char * trow2_ptr = trow2;
			memcpy(trow2_ptr, image_it.GetPtr(), image_it.GetCellSize());
			while (image_it.Increment()) // Put first row in buffer2
			{
				trow2_ptr += image_it.GetCellSize();
				memcpy(trow2_ptr, image_it.GetPtr(), image_it.GetCellSize());
			}
			G42BGRLum * cache1 = new G42BGRLum [9];
			G42BGRLum * cache1_ptr = cache1;
			image_it.ResetRow();
			cache1_ptr->Set(image_it.GetBlue(), image_it.GetGreen(), image_it.GetRed());
			cache1_ptr++;
			image_it.IncRow();
			cache1_ptr->Set(image_it.GetBlue(), image_it.GetGreen(), image_it.GetRed());
			cache1_ptr++;
			image_it.IncRow();
			cache1_ptr->Set(image_it.GetBlue(), image_it.GetGreen(), image_it.GetRed());
			cache1_ptr++;
			image_it.Increment();
			image_it.DecRow();
			image_it.DecRow();
			cache1_ptr->Set(image_it.GetBlue(), image_it.GetGreen(), image_it.GetRed());
			cache1_ptr++;
			image_it.IncRow();
			cache1_ptr->Set(image_it.GetBlue(), image_it.GetGreen(), image_it.GetRed());
			cache1_ptr++;
			image_it.IncRow();
			cache1_ptr->Set(image_it.GetBlue(), image_it.GetGreen(), image_it.GetRed());
			cache1_ptr++;
			image_it.DecRow();
			image_it.Decrement();
			unsigned char * trow1_ptr = trow1;
			memcpy(trow1_ptr, image_it.GetPtr(), image_it.GetCellSize());
			trow1_ptr += image_it.GetCellSize();
			image_it.Increment();
			int center = 1;
			int lookup [9];
			for (int n = 0; n < 9; n++)
				lookup[n] = n;
         event_handler->Start();
         event_handler->Set(0L);
			while (1)
			{
				if (!image_it.Increment())
				{
					memcpy(trow1_ptr, image_it.GetPtr(), image_it.GetCellSize());
					image_it.ResetRow();
					image_it.DecRow();
					trow2_ptr = trow2;
					memcpy(image_it.GetPtr(), trow2_ptr, image_it.GetCellSize());
					trow2_ptr += image_it.GetCellSize();
					while (image_it.Increment()) //Roll buffer2 back into image
					{
						memcpy(image_it.GetPtr(), trow2_ptr, image_it.GetCellSize());
						trow2_ptr += image_it.GetCellSize();
					}
					image_it.NextRow();
					image_it.NextRow();
               row_count++;
               event_handler->Set(row_count);
					unsigned char * swap = trow2;
					trow2 = trow1;
					trow1 = swap;
					trow1_ptr = trow1;
					memcpy(trow1_ptr, image_it.GetPtr(), image_it.GetCellSize());
					trow1_ptr += image_it.GetCellSize();
					image_it.DecRow();
					cache1_ptr = cache1;
					center = 1;
					cache1_ptr->Set(image_it.GetBlue(), image_it.GetGreen(), image_it.GetRed());
					cache1_ptr++;
					image_it.IncRow();
					cache1_ptr->Set(image_it.GetBlue(), image_it.GetGreen(), image_it.GetRed());
					cache1_ptr++;
					if (!image_it.IncRow())
					{
						// We're past the last row.
						image_it.DecRow();
						image_it.ResetRow();
						trow2_ptr = trow2;
						memcpy(image_it.GetPtr(), trow2_ptr, image_it.GetCellSize());
						trow2_ptr += image_it.GetCellSize();
						while (image_it.Increment()) //Roll buffer2 back into image
						{
							memcpy(image_it.GetPtr(), trow2_ptr, image_it.GetCellSize());
							trow2_ptr += image_it.GetCellSize();
						}
						break;
					}
					cache1_ptr->Set(image_it.GetBlue(), image_it.GetGreen(), image_it.GetRed());
					cache1_ptr++;
					image_it.Increment();
					image_it.DecRow();
					image_it.DecRow();
					cache1_ptr->Set(image_it.GetBlue(), image_it.GetGreen(), image_it.GetRed());
					cache1_ptr++;
					image_it.IncRow();
					cache1_ptr->Set(image_it.GetBlue(), image_it.GetGreen(), image_it.GetRed());
					cache1_ptr++;
					image_it.IncRow();
					cache1_ptr->Set(image_it.GetBlue(), image_it.GetGreen(), image_it.GetRed());
					cache1_ptr++;
					image_it.DecRow();
					for (int n = 0; n < 9; n++)
						lookup[n] = n;
					continue;
				}
				image_it.DecRow();
				cache1_ptr->Set(image_it.GetBlue(), image_it.GetGreen(), image_it.GetRed());
				cache1_ptr++;
				image_it.IncRow();
				cache1_ptr->Set(image_it.GetBlue(), image_it.GetGreen(), image_it.GetRed());
				cache1_ptr++;
				image_it.IncRow();
				cache1_ptr->Set(image_it.GetBlue(), image_it.GetGreen(), image_it.GetRed());
				cache1_ptr++;
				if (cache1_ptr >= (cache1 + 9))
					cache1_ptr = cache1;
				center += 3;
				if (center >= 9)
					center = 1;
				image_it.DecRow();
				image_it.Decrement(); // Now pointing at the right spot.
				/* Sort mask buffer using lookup pointers. */
				for(int n = 0; n < 5; n++)
				{
					int lowindex = n;
					unsigned char lowkey = cache1[lookup[n]].GetLum();
					for(int i = n + 1; i < 9; i++)
					{
						if(cache1[lookup[i]].GetLum() < lowkey)
						{
							lowkey = cache1[lookup[i]].GetLum();
							lowindex = i;
						}
					}
					int tval = lookup[n];
					lookup[n] = lookup[lowindex];
					lookup[lowindex] = tval;
				}
				if(((cache1[center].GetLum() - cache1[lookup[4]].GetLum()) > threshold)
					|| ((cache1[lookup[4]].GetLum() - cache1[center].GetLum()) > threshold))
				{
					*(trow1_ptr + image_it.GetBlueOffset()) = cache1[lookup[4]].GetBlue();
					*(trow1_ptr + image_it.GetGreenOffset()) = cache1[lookup[4]].GetGreen();
					*(trow1_ptr + image_it.GetRedOffset()) = cache1[lookup[4]].GetRed();
               trow1_ptr += image_it.GetCellSize();
				}
				else
				{
					memcpy(trow1_ptr, image_it.GetPtr(), image_it.GetCellSize());
					trow1_ptr += image_it.GetCellSize();
				}
				image_it.Increment();
			}
			delete [] trow1;
			delete [] trow2;
			delete [] cache1;
      	event_handler->Stop();
			return data;
		}
		if (data->GetDepth() == 8)
		{
			int32 row_width = 0L;
			for (uint y = 0; y < data->GetNumTilesAcross(); y++)
			{
				G42ImageTile * tile = data->GetTile(0, y);
				row_width += tile->GetTileWidth();
			}
			unsigned char * trow1 = new unsigned char [row_width];
			unsigned char * trow2 = new unsigned char [row_width];
			memset(trow1, 0, row_width);
			memset(trow2, 0, row_width);
			G42ImageData8BitIterator image_it(data);
			unsigned char * trow2_ptr = trow2;
			memcpy(trow2_ptr, image_it.GetPtr(), 1);
			while (image_it.Increment()) // Put first row in buffer2
			{
				trow2_ptr++;
				memcpy(trow2_ptr, image_it.GetPtr(), 1);
			}
			unsigned char * cache1 = new unsigned char [9];
			unsigned char * cache1_ptr = cache1;
			image_it.ResetRow();
			*cache1_ptr = (unsigned char)image_it;
			cache1_ptr++;
			image_it.IncRow();
			*cache1_ptr = (unsigned char)image_it;
			cache1_ptr++;
			image_it.IncRow();
			*cache1_ptr = (unsigned char)image_it;
			cache1_ptr++;
			image_it.Increment();
			image_it.DecRow();
			image_it.DecRow();
			*cache1_ptr = (unsigned char)image_it;
			cache1_ptr++;
			image_it.IncRow();
			*cache1_ptr = (unsigned char)image_it;
			cache1_ptr++;
			image_it.IncRow();
			*cache1_ptr = (unsigned char)image_it;
			cache1_ptr++;
			image_it.DecRow();
			image_it.Decrement();
			unsigned char * trow1_ptr = trow1;
			memcpy(trow1_ptr, image_it.GetPtr(), 1);
			trow1_ptr++;
			image_it.Increment();
			int center = 1;
			int lookup [9];
			for (int n = 0; n < 9; n++)
				lookup[n] = n;
			event_handler->Start();
         event_handler->Set(0L);
			while (1)
			{
				if (!image_it.Increment())
				{
					memcpy(trow1_ptr, image_it.GetPtr(), 1);
					image_it.ResetRow();
					image_it.DecRow();
					trow2_ptr = trow2;
					memcpy(image_it.GetPtr(), trow2_ptr, 1);
					trow2_ptr++;
					while (image_it.Increment()) //Roll buffer2 back into image
					{
						memcpy(image_it.GetPtr(), trow2_ptr, 1);
						trow2_ptr++;
					}
					image_it.NextRow();
					image_it.NextRow();
               row_count++;
               event_handler->Set(row_count);
					unsigned char * swap = trow2;
					trow2 = trow1;
					trow1 = swap;
					trow1_ptr = trow1;
					memcpy(trow1_ptr, image_it.GetPtr(), 1);
					trow1_ptr++;
					image_it.DecRow();
					cache1_ptr = cache1;
					center = 1;
					*cache1_ptr = (unsigned char)image_it;
					cache1_ptr++;
					image_it.IncRow();
					*cache1_ptr = (unsigned char)image_it;
					cache1_ptr++;
					if (!image_it.IncRow())
					{
						// We're past the last row.
						image_it.DecRow();
						image_it.ResetRow();
						trow2_ptr = trow2;
						memcpy(image_it.GetPtr(), trow2_ptr, 1);
						trow2_ptr++;
						while (image_it.Increment()) //Roll buffer2 back into image
						{
							memcpy(image_it.GetPtr(), trow2_ptr, 1);
							trow2_ptr++;
						}
						break;
					}
					*cache1_ptr = (unsigned char)image_it;
					cache1_ptr++;
					image_it.Increment();
					image_it.DecRow();
					image_it.DecRow();
					*cache1_ptr = (unsigned char)image_it;
					cache1_ptr++;
					image_it.IncRow();
					*cache1_ptr = (unsigned char)image_it;
					cache1_ptr++;
					image_it.IncRow();
					*cache1_ptr = (unsigned char)image_it;
					cache1_ptr++;
					image_it.DecRow();
					for (int n = 0; n < 9; n++)
						lookup[n] = n;
					continue;
				}
				image_it.DecRow();
				*cache1_ptr = (unsigned char)image_it;
				cache1_ptr++;
				image_it.IncRow();
				*cache1_ptr = (unsigned char)image_it;
				cache1_ptr++;
				image_it.IncRow();
				*cache1_ptr = (unsigned char)image_it;
				cache1_ptr++;
				if (cache1_ptr >= (cache1 + 9))
					cache1_ptr = cache1;
				center += 3;
				if (center >= 9)
					center = 1;
				image_it.DecRow();
				image_it.Decrement(); // Now pointing at the right spot.
				/* Sort mask buffer using lookup pointers. */
				for(int n = 0; n < 5; n++)
				{
					int lowindex = n;
					unsigned char lowkey = cache1[lookup[n]];
					for(int i = n + 1; i < 9; i++)
					{
						if(cache1[lookup[i]] < lowkey)
						{
							lowkey = cache1[lookup[i]];
							lowindex = i;
						}
					}
					int tval = lookup[n];
					lookup[n] = lookup[lowindex];
					lookup[lowindex] = tval;
				}
				if(((cache1[center] - cache1[lookup[4]]) > threshold)
					|| ((cache1[lookup[4]] - cache1[center]) > threshold))
				{
					*trow1_ptr = cache1[lookup[4]];
					trow1_ptr++;
				}
				else
				{
					memcpy(trow1_ptr, image_it.GetPtr(), 1);
					trow1_ptr++;
				}
				image_it.Increment();
			}
			delete [] trow1;
			delete [] trow2;
			delete [] cache1;
			event_handler->Stop();
			return data;
		}
		if (data->GetDepth() == 4)
		{
			G42ImageData * new_data = new G42ImageData(data->GetData(),
				data->GetWidth(), data->GetHeight(), 8, 0);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42LockedID4BitIterator image_it1(data);
			G42ImageData8BitIterator image_it2(new_data);
			do
			{
				int val = (int)(unsigned char)image_it1;
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
			return RemoveNoise(new_data, threshold, event_handler);
		}
		if (data->GetDepth() == 1)
		{
			int32 row_width = 0L;
			for (uint y = 0; y < data->GetNumTilesAcross(); y++)
			{
				G42ImageTile * tile = data->GetTile(0, y);
				row_width += tile->GetTileWidth();
			}
			int32 max_count = row_width - 2;
			row_width = ((row_width + 7) >> 3);
			unsigned char * trow1 = new unsigned char [row_width + 1];
			unsigned char * trow2 = new unsigned char [row_width + 1];
			unsigned char * trow3 = new unsigned char [row_width + 1];
			unsigned char * trow4 = new unsigned char [row_width + 1];
			unsigned char * trow5 = new unsigned char [row_width + 1];
			threshold = 12 - (((threshold - 1) * 12) / 128);
			if (threshold > 12)
				threshold = 12;
			if (threshold < 1)
				threshold = 1;
			memset(trow1, 0, row_width);
			memset(trow2, 0, row_width);
			memset(trow3, 0, row_width);
			memset(trow4, 0, row_width);
			memset(trow5, 0, row_width);
			G42ImageData1BitIterator image_it(data);
			unsigned char * trow1_ptr = trow1;
			*trow1_ptr = *(image_it.GetPtr());
			while (image_it.Increment8())
			{
				trow1_ptr++;
				*trow1_ptr = *(image_it.GetPtr());
			}
			image_it.NextRow();
			unsigned char * trow2_ptr = trow2;
			*trow2_ptr = *(image_it.GetPtr());
			while (image_it.Increment8())
			{
				trow2_ptr++;
				*trow2_ptr = *(image_it.GetPtr());
			}
			image_it.NextRow();
			unsigned char * trow3_ptr = trow3;
			*trow3_ptr = *(image_it.GetPtr());
			while (image_it.Increment8())
			{
				trow3_ptr++;
				*trow3_ptr = *(image_it.GetPtr());
			}
			image_it.NextRow();
			unsigned char * trow4_ptr = trow4;
			*trow4_ptr = *(image_it.GetPtr());
			while (image_it.Increment8())
			{
				trow4_ptr++;
				*trow4_ptr = *(image_it.GetPtr());
			}
			image_it.NextRow();
			unsigned char * trow5_ptr = trow5;
			*trow5_ptr = *(image_it.GetPtr());
			while (image_it.Increment8())
			{
				trow5_ptr++;
				*trow5_ptr = *(image_it.GetPtr());
			}
			trow1_ptr = trow1;
			trow2_ptr = trow2;
			trow3_ptr = trow3;
			trow4_ptr = trow4;
			trow5_ptr = trow5;
			unsigned char * cache1 = new unsigned char [25];
         memset(cache1, 0, 25);
			unsigned char * cache1_ptr = cache1;
			unsigned char mask = 0x80;
			int nwhite = 0;
			for (int n = 0; n < 4; n++)
			{
				if (*trow1_ptr & mask)
				{
					*cache1_ptr = 1;
					nwhite++;
				}
				else
					*cache1_ptr = 0;
				cache1_ptr++;
				if (*trow2_ptr & mask)
				{
					*cache1_ptr = 1;
					nwhite++;
				}
				else
					*cache1_ptr = 0;
				cache1_ptr++;
				if (*trow3_ptr & mask)
				{
					*cache1_ptr = 1;
					nwhite++;
				}
				else
					*cache1_ptr = 0;
				cache1_ptr++;
				if (*trow4_ptr & mask)
				{
					*cache1_ptr = 1;
					nwhite++;
				}
				else
					*cache1_ptr = 0;
				cache1_ptr++;
				if (*trow5_ptr & mask)
				{
					*cache1_ptr = 1;
					nwhite++;
				}
				else
					*cache1_ptr = 0;
				cache1_ptr++;
            mask >>= 1;
			}
			unsigned char cmask = 0x20;
         unsigned char * current_ptr = trow3;
			int32 count = 2;
         int nblack = 0;
         event_handler->Start();
         event_handler->Set(0L);
			while (1)
			{
				if (cmask == 0x00)
				{
					current_ptr++;
					cmask = 0x80;
				}
				if (cache1_ptr >= (cache1 + 25))
					cache1_ptr = cache1;
				int n;
				for (n = 0; n < 5; n++, cache1_ptr++)
					nwhite -= *cache1_ptr;
				cache1_ptr -= 5;
				if (mask == 0x00)
				{
					trow1_ptr++;
					trow2_ptr++;
					trow3_ptr++;
					trow4_ptr++;
					trow5_ptr++;
					mask = 0x80;
				}
				if (*trow1_ptr & mask)
				{
					*cache1_ptr = 1;
					nwhite++;
				}
				else
					*cache1_ptr = 0;
				cache1_ptr++;
				if (*trow2_ptr & mask)
				{
					*cache1_ptr = 1;
					nwhite++;
				}
				else
					*cache1_ptr = 0;
				cache1_ptr++;
				if (*trow3_ptr & mask)
				{
					*cache1_ptr = 1;
					nwhite++;
				}
				else
					*cache1_ptr = 0;
				cache1_ptr++;
				if (*trow4_ptr & mask)
				{
					*cache1_ptr = 1;
					nwhite++;
				}
				else
					*cache1_ptr = 0;
				cache1_ptr++;
				if (*trow5_ptr & mask)
				{
					*cache1_ptr = 1;
					nwhite++;
				}
				else
					*cache1_ptr = 0;
				cache1_ptr++;
            mask >>= 1;
				nblack = 25 - nwhite;
				if ((*current_ptr & cmask) && (nwhite < threshold))
					*current_ptr &= ~cmask;
				else if ((!(*current_ptr & cmask)) && (nblack < threshold))
					*current_ptr |= cmask;
            cmask >>= 1;
				count++;
				if (count >= max_count)
				{
					trow3_ptr = trow3;
					image_it.ResetRow();
					image_it.DecRow();
					image_it.DecRow();
					*(image_it.GetPtr()) = *trow3_ptr;
					while (image_it.Increment8())
					{
						trow3_ptr++;
						*(image_it.GetPtr()) = *trow3_ptr;
					}
					image_it.NextRow();
					image_it.NextRow();
					if (!image_it.NextRow())
   					break;
               row_count++;
               event_handler->Set(row_count);
					unsigned char * swap = trow1;
					trow1 = trow2;
					trow2 = trow3;
					trow3 = trow4;
					trow4 = trow5;
					trow5 = swap;
					trow1_ptr = trow1;
					trow2_ptr = trow2;
					trow3_ptr = trow3;
					trow4_ptr = trow4;
					trow5_ptr = trow5;
					*trow5_ptr = *(image_it.GetPtr());
					while (image_it.Increment8())
					{
						trow5_ptr++;
						*trow5_ptr = *(image_it.GetPtr());
					}
					trow5_ptr = trow5;
					count = 2;
					cmask = 0x20;
					current_ptr = trow3;
					cache1_ptr = cache1;
					mask = 0x80;
					nwhite = 0;
					int n;
					for (n = 0; n < 4; n++)
					{
						if (*trow1_ptr & mask)
						{
							*cache1_ptr = 1;
							nwhite++;
						}
						else
							*cache1_ptr = 0;
						cache1_ptr++;
						if (*trow2_ptr & mask)
						{
							*cache1_ptr = 1;
							nwhite++;
						}
						else
							*cache1_ptr = 0;
						cache1_ptr++;
						if (*trow3_ptr & mask)
						{
							*cache1_ptr = 1;
							nwhite++;
						}
						else
							*cache1_ptr = 0;
						cache1_ptr++;
						if (*trow4_ptr & mask)
						{
							*cache1_ptr = 1;
							nwhite++;
						}
						else
							*cache1_ptr = 0;
						cache1_ptr++;
						if (*trow5_ptr & mask)
						{
							*cache1_ptr = 1;
							nwhite++;
						}
						else
							*cache1_ptr = 0;
						cache1_ptr++;
						mask >>= 1;
					}
					for (n = 0; n < 5; n++, cache1_ptr++)
						*cache1_ptr = 0;
					cache1_ptr -= 5;
				}
			}
			delete [] trow1;
			delete [] trow2;
			delete [] trow3;
			delete [] trow4;
			delete [] trow5;
			delete [] cache1;
      	event_handler->Stop();
			return data;
		}
	}
	if (data->GetDepth() == 8)
	{
		G42Color * palette = data->GetPalette();
		int32 row_width = data->GetWidth();
		unsigned char * trow1 = new unsigned char [row_width];
		unsigned char * trow2 = new unsigned char [row_width];
		memset(trow1, 0, row_width);
		memset(trow2, 0, row_width);
		G42ImageData8BitIterator image_it(data);
		unsigned char * trow2_ptr = trow2;
		memcpy(trow2_ptr, image_it.GetPtr(), 1);
		while (image_it.Increment()) // Put first row in buffer2
		{
			trow2_ptr++;
			memcpy(trow2_ptr, image_it.GetPtr(), 1);
		}
		G42BGRLum * cache1 = new G42BGRLum [9];
		G42BGRLum * cache1_ptr = cache1;
		image_it.ResetRow();
		cache1_ptr->Set(palette[(unsigned char)image_it].blue,
			palette[(unsigned char)image_it].green,
			palette[(unsigned char)image_it].red, (unsigned char)image_it);
		cache1_ptr++;
		image_it.IncRow();
		cache1_ptr->Set(palette[(unsigned char)image_it].blue,
			palette[(unsigned char)image_it].green,
			palette[(unsigned char)image_it].red, (unsigned char)image_it);
		cache1_ptr++;
		image_it.IncRow();
		cache1_ptr->Set(palette[(unsigned char)image_it].blue,
			palette[(unsigned char)image_it].green,
			palette[(unsigned char)image_it].red, (unsigned char)image_it);
		cache1_ptr++;
		image_it.Increment();
		image_it.DecRow();
		image_it.DecRow();
		cache1_ptr->Set(palette[(unsigned char)image_it].blue,
			palette[(unsigned char)image_it].green,
			palette[(unsigned char)image_it].red, (unsigned char)image_it);
		cache1_ptr++;
		image_it.IncRow();
		cache1_ptr->Set(palette[(unsigned char)image_it].blue,
			palette[(unsigned char)image_it].green,
			palette[(unsigned char)image_it].red, (unsigned char)image_it);
		cache1_ptr++;
		image_it.IncRow();
		cache1_ptr->Set(palette[(unsigned char)image_it].blue,
			palette[(unsigned char)image_it].green,
			palette[(unsigned char)image_it].red, (unsigned char)image_it);
		cache1_ptr++;
		image_it.DecRow();
		image_it.Decrement();
		unsigned char * trow1_ptr = trow1;
		memcpy(trow1_ptr, image_it.GetPtr(), 1);
		trow1_ptr++;
		image_it.Increment();
		int center = 1;
		int lookup [9];
		for (int n = 0; n < 9; n++)
			lookup[n] = n;
      event_handler->Start();
      event_handler->Set(0L);
		while (1)
		{
			if (!image_it.Increment())
			{
				memcpy(trow1_ptr, image_it.GetPtr(), 1);
				image_it.ResetRow();
				image_it.DecRow();
				trow2_ptr = trow2;
				memcpy(image_it.GetPtr(), trow2_ptr, 1);
				trow2_ptr++;
				while (image_it.Increment()) //Roll buffer2 back into image
				{
					memcpy(image_it.GetPtr(), trow2_ptr, 1);
					trow2_ptr++;
				}
				image_it.NextRow();
				image_it.NextRow();
            row_count++;
            event_handler->Set(row_count);
				unsigned char * swap = trow2;
				trow2 = trow1;
				trow1 = swap;
				trow1_ptr = trow1;
				memcpy(trow1_ptr, image_it.GetPtr(), 1);
				trow1_ptr++;
				image_it.DecRow();
				cache1_ptr = cache1;
				center = 1;
				cache1_ptr->Set(palette[(unsigned char)image_it].blue,
					palette[(unsigned char)image_it].green,
					palette[(unsigned char)image_it].red, (unsigned char)image_it);
				cache1_ptr++;
				image_it.IncRow();
				cache1_ptr->Set(palette[(unsigned char)image_it].blue,
					palette[(unsigned char)image_it].green,
					palette[(unsigned char)image_it].red, (unsigned char)image_it);
				cache1_ptr++;
				if (!image_it.IncRow())
				{
					// We're past the last row.
					image_it.DecRow();
					image_it.ResetRow();
					trow2_ptr = trow2;
					memcpy(image_it.GetPtr(), trow2_ptr, 1);
					trow2_ptr++;
					while (image_it.Increment()) //Roll buffer2 back into image
					{
						memcpy(image_it.GetPtr(), trow2_ptr, 1);
						trow2_ptr++;
					}
					break;
				}
				cache1_ptr->Set(palette[(unsigned char)image_it].blue,
					palette[(unsigned char)image_it].green,
					palette[(unsigned char)image_it].red, (unsigned char)image_it);
				cache1_ptr++;
				image_it.Increment();
				image_it.DecRow();
				image_it.DecRow();
				cache1_ptr->Set(palette[(unsigned char)image_it].blue,
					palette[(unsigned char)image_it].green,
					palette[(unsigned char)image_it].red, (unsigned char)image_it);
				cache1_ptr++;
				image_it.IncRow();
				cache1_ptr->Set(palette[(unsigned char)image_it].blue,
					palette[(unsigned char)image_it].green,
					palette[(unsigned char)image_it].red, (unsigned char)image_it);
				cache1_ptr++;
				image_it.IncRow();
				cache1_ptr->Set(palette[(unsigned char)image_it].blue,
					palette[(unsigned char)image_it].green,
					palette[(unsigned char)image_it].red, (unsigned char)image_it);
				cache1_ptr++;
				image_it.DecRow();
				for (int n = 0; n < 9; n++)
					lookup[n] = n;
				continue;
			}
			image_it.DecRow();
			cache1_ptr->Set(palette[(unsigned char)image_it].blue,
				palette[(unsigned char)image_it].green,
				palette[(unsigned char)image_it].red, (unsigned char)image_it);
			cache1_ptr++;
			image_it.IncRow();
			cache1_ptr->Set(palette[(unsigned char)image_it].blue,
				palette[(unsigned char)image_it].green,
				palette[(unsigned char)image_it].red, (unsigned char)image_it);
			cache1_ptr++;
			image_it.IncRow();
			cache1_ptr->Set(palette[(unsigned char)image_it].blue,
				palette[(unsigned char)image_it].green,
				palette[(unsigned char)image_it].red, (unsigned char)image_it);
			cache1_ptr++;
			if (cache1_ptr >= (cache1 + 9))
				cache1_ptr = cache1;
			center += 3;
			if (center >= 9)
				center = 1;
			image_it.DecRow();
			image_it.Decrement(); // Now pointing at the right spot.
			/* Sort mask buffer using lookup pointers. */
			for (int n = 0; n < 5; n++)
			{
				int lowindex = n;
				unsigned char lowkey = cache1[lookup[n]].GetLum();
				for(int i = n + 1; i < 9; i++)
				{
					if(cache1[lookup[i]].GetLum() < lowkey)
					{
						lowkey = cache1[lookup[i]].GetLum();
						lowindex = i;
					}
				}
				int tval = lookup[n];
				lookup[n] = lookup[lowindex];
				lookup[lowindex] = tval;
			}
			if(((cache1[center].GetLum() - cache1[lookup[4]].GetLum()) > threshold)
				|| ((cache1[lookup[4]].GetLum() - cache1[center].GetLum()) > threshold))
			{
				*trow1_ptr = cache1[lookup[4]].GetIndex();
				trow1_ptr++;
			}
			else
			{
				memcpy(trow1_ptr, image_it.GetPtr(), 1);
				trow1_ptr++;
			}
			image_it.Increment();
		}
		delete [] trow1;
		delete [] trow2;
		delete [] cache1;
   	event_handler->Stop();
		return data;
	}
	if (data->GetDepth() == 4)
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
			unsigned char result = (unsigned char)image_it1;
			image_it2.Set(palette[result].blue, palette[result].green,
				palette[result].red);
			image_it2.Increment();
			if (!image_it1.Increment())
			{
				if (!image_it1.NextRow())
					break;
				image_it2.NextRow();
			}
		} while (1);
		return RemoveNoise(new_data, threshold, event_handler);
	}
	if (data->GetDepth() == 1)
	{
		int32 row_width = 0L;
		for (uint y = 0; y < data->GetNumTilesAcross(); y++)
		{
			G42ImageTile * tile = data->GetTile(0, y);
			row_width += tile->GetTileWidth();
		}
		int32 max_count = row_width - 2;
		row_width = ((row_width + 7) >> 3);
		unsigned char * trow1 = new unsigned char [row_width + 1];
		unsigned char * trow2 = new unsigned char [row_width + 1];
		unsigned char * trow3 = new unsigned char [row_width + 1];
		unsigned char * trow4 = new unsigned char [row_width + 1];
		unsigned char * trow5 = new unsigned char [row_width + 1];
		threshold = 12 - (((threshold - 1) * 12) / 128);
		if (threshold > 12)
			threshold = 12;
		if (threshold < 1)
			threshold = 1;
		memset(trow1, 0, row_width);
		memset(trow2, 0, row_width);
		memset(trow3, 0, row_width);
		memset(trow4, 0, row_width);
		memset(trow5, 0, row_width);
		G42ImageData1BitIterator image_it(data);
		unsigned char * trow1_ptr = trow1;
		*trow1_ptr = *(image_it.GetPtr());
		while (image_it.Increment8())
		{
			trow1_ptr++;
			*trow1_ptr = *(image_it.GetPtr());
		}
		image_it.NextRow();
		unsigned char * trow2_ptr = trow2;
		*trow2_ptr = *(image_it.GetPtr());
		while (image_it.Increment8())
		{
			trow2_ptr++;
			*trow2_ptr = *(image_it.GetPtr());
		}
		image_it.NextRow();
		unsigned char * trow3_ptr = trow3;
		*trow3_ptr = *(image_it.GetPtr());
		while (image_it.Increment8())
		{
			trow3_ptr++;
			*trow3_ptr = *(image_it.GetPtr());
		}
		image_it.NextRow();
		unsigned char * trow4_ptr = trow4;
		*trow4_ptr = *(image_it.GetPtr());
		while (image_it.Increment8())
		{
			trow4_ptr++;
			*trow4_ptr = *(image_it.GetPtr());
		}
		image_it.NextRow();
		unsigned char * trow5_ptr = trow5;
		*trow5_ptr = *(image_it.GetPtr());
		while (image_it.Increment8())
		{
			trow5_ptr++;
			*trow5_ptr = *(image_it.GetPtr());
		}
		trow1_ptr = trow1;
		trow2_ptr = trow2;
		trow3_ptr = trow3;
		trow4_ptr = trow4;
		trow5_ptr = trow5;
		unsigned char * cache1 = new unsigned char [25];
		memset(cache1, 0, 25);
		unsigned char * cache1_ptr = cache1;
		unsigned char mask = 0x80;
		int nwhite = 0;
		for (int n = 0; n < 4; n++)
		{
			if (*trow1_ptr & mask)
			{
				*cache1_ptr = 1;
				nwhite++;
			}
			else
				*cache1_ptr = 0;
			cache1_ptr++;
			if (*trow2_ptr & mask)
			{
				*cache1_ptr = 1;
				nwhite++;
			}
			else
				*cache1_ptr = 0;
			cache1_ptr++;
			if (*trow3_ptr & mask)
			{
				*cache1_ptr = 1;
				nwhite++;
			}
			else
				*cache1_ptr = 0;
			cache1_ptr++;
			if (*trow4_ptr & mask)
			{
				*cache1_ptr = 1;
				nwhite++;
			}
			else
				*cache1_ptr = 0;
			cache1_ptr++;
			if (*trow5_ptr & mask)
			{
				*cache1_ptr = 1;
				nwhite++;
			}
			else
				*cache1_ptr = 0;
			cache1_ptr++;
			  mask >>= 1;
		}
		unsigned char cmask = 0x20;
		  unsigned char * current_ptr = trow3;
		int32 count = 2;
		  int nblack = 0;
      event_handler->Start();
      event_handler->Set(0L);
		while (1)
		{
			if (cmask == 0x00)
			{
				current_ptr++;
				cmask = 0x80;
			}
				if (cache1_ptr >= (cache1 + 25))
				cache1_ptr = cache1;
			int n;
			for (n = 0; n < 5; n++, cache1_ptr++)
				nwhite -= *cache1_ptr;
			cache1_ptr -= 5;
			if (mask == 0x00)
			{
				trow1_ptr++;
				trow2_ptr++;
				trow3_ptr++;
				trow4_ptr++;
				trow5_ptr++;
				mask = 0x80;
			}
			if (*trow1_ptr & mask)
			{
				*cache1_ptr = 1;
				nwhite++;
			}
			else
				*cache1_ptr = 0;
			cache1_ptr++;
			if (*trow2_ptr & mask)
			{
				*cache1_ptr = 1;
				nwhite++;
			}
			else
				*cache1_ptr = 0;
			cache1_ptr++;
			if (*trow3_ptr & mask)
			{
				*cache1_ptr = 1;
				nwhite++;
			}
			else
				*cache1_ptr = 0;
			cache1_ptr++;
			if (*trow4_ptr & mask)
			{
				*cache1_ptr = 1;
				nwhite++;
			}
			else
				*cache1_ptr = 0;
			cache1_ptr++;
			if (*trow5_ptr & mask)
			{
				*cache1_ptr = 1;
				nwhite++;
			}
			else
				*cache1_ptr = 0;
			cache1_ptr++;
			mask >>= 1;
			nblack = 25 - nwhite;
			if ((*current_ptr & cmask) && (nwhite < threshold))
				*current_ptr &= ~cmask;
			else if ((!(*current_ptr & cmask)) && (nblack < threshold))
				*current_ptr |= cmask;
			cmask >>= 1;
			count++;
			if (count >= max_count)
			{
				trow3_ptr = trow3;
				image_it.ResetRow();
				image_it.DecRow();
				image_it.DecRow();
				*(image_it.GetPtr()) = *trow3_ptr;
				while (image_it.Increment8())
				{
					trow3_ptr++;
					*(image_it.GetPtr()) = *trow3_ptr;
				}
				image_it.NextRow();
				image_it.NextRow();
				if (!image_it.NextRow())
					break;
            row_count++;
            event_handler->Set(row_count);
				unsigned char * swap = trow1;
				trow1 = trow2;
				trow2 = trow3;
				trow3 = trow4;
				trow4 = trow5;
				trow5 = swap;
				trow1_ptr = trow1;
				trow2_ptr = trow2;
				trow3_ptr = trow3;
				trow4_ptr = trow4;
				trow5_ptr = trow5;
				*trow5_ptr = *(image_it.GetPtr());
				while (image_it.Increment8())
				{
					trow5_ptr++;
					*trow5_ptr = *(image_it.GetPtr());
				}
				trow5_ptr = trow5;
				count = 2;
				cmask = 0x20;
				current_ptr = trow3;
				cache1_ptr = cache1;
				mask = 0x80;
				nwhite = 0;
				int n;
				for (n = 0; n < 4; n++)
				{
					if (*trow1_ptr & mask)
					{
						*cache1_ptr = 1;
						nwhite++;
					}
					else
						*cache1_ptr = 0;
					cache1_ptr++;
					if (*trow2_ptr & mask)
					{
						*cache1_ptr = 1;
						nwhite++;
					}
					else
						*cache1_ptr = 0;
					cache1_ptr++;
					if (*trow3_ptr & mask)
					{
						*cache1_ptr = 1;
						nwhite++;
					}
					else
						*cache1_ptr = 0;
					cache1_ptr++;
					if (*trow4_ptr & mask)
					{
						*cache1_ptr = 1;
						nwhite++;
					}
					else
						*cache1_ptr = 0;
					cache1_ptr++;
					if (*trow5_ptr & mask)
					{
						*cache1_ptr = 1;
						nwhite++;
					}
					else
						*cache1_ptr = 0;
					cache1_ptr++;
					mask >>= 1;
				}
				for (n = 0; n < 5; n++, cache1_ptr++)
					*cache1_ptr = 0;
				cache1_ptr -= 5;
			}
		}
		delete [] trow1;
		delete [] trow2;
		delete [] trow3;
		delete [] trow4;
		delete [] trow5;
		delete [] cache1;
		event_handler->Stop();
		return data;
	}
}
void
G42Image::RemoveNoise(int noise)
{
	G42ImageData * new_data = Data;
	if (Data)
   {
   	if (Mask)
      {
      	delete Mask;
         Mask = 0;
      }
		new_data = ::RemoveNoise(Data, noise, GetEventHandler());
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
