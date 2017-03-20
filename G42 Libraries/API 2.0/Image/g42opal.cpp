// g42opal.cpp - optimized palette
/*************************************************************
	File:          g42opal.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Optimize a palette for an image
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "g42itype.h"
#include "g42opal.h"
#include "gesdebug.h"
G42OptimizedPalette::~G42OptimizedPalette()
{
#ifdef MSWIN
	if (DitherUnique)
		farfree(DitherUnique);
	if (DitherTable)
		farfree(DitherTable);
#else
	if (DitherUnique)
		delete[] DitherUnique;
	if (DitherTable)
		delete[] DitherTable;
#endif
	if (DitherPalette)
		delete[] DitherPalette;
}
void G42OptimizedPalette::G42PaletteBox::ResetBoxLimits(void)
{
	rmin = gmin = bmin = 0x1f;
	rmax = gmax = bmax = 0;
	for (int j = 0; j < nh; j++)
	{
		int i = h[j];
		int r = (i >> 8) & 0xf;
		int g = (i >> 3) & 0x1f;
		int b = i & 0x7;
		if (r > rmax)
			rmax = r;
		if (r < rmin)
			rmin = r;
		if (g > gmax)
			gmax = g;
		if (g < gmin)
			gmin = g;
		if (b > bmax)
			bmax = b;
		if (b < bmin)
			bmin = b;
	}
}
G42OptimizedPalette::G42OptimizedPalette(
	G42ImageData * data, int num_palette, bool need_white,
	bool standard_palette, bool full_color_only, G42ImageData * sub_data) :
		MaxPalette(num_palette), Data(data), NumPalette(0), SubData(sub_data),
		DitherTable(0), DitherUnique(0),
		DitherPalette(0), WhitePixel(0), NeedWhite(need_white),
		FullColorOnly(full_color_only)
{
GESTRACESTART("Optimize Palette Start");
#if 0
#ifdef MSWIN
	h = (uint16 *)farmalloc((int32)32768U * (int32)sizeof (uint16));
	for (uint16 i = 0; i <= 32767U; i++)
		h[i] = 0;
	DitherUnique = (byte *)farmalloc(32768L);
	DitherTable = (byte *)farmalloc(32768L);
#endif
#endif
	h = new uint16 [4096];
	memset(h, 0, 4096 * sizeof (uint16));
	DitherUnique = new byte [4096];
	DitherTable = new byte [4096];
	NeedOptimize = false;
	DitherPalette = new G42Color[256];
	if (standard_palette)
	{
		CreateStandardPalette();
	}
	else // not a standard palette
	{
		if (Data)
			InvestigateData(Data);
		if (SubData)
			InvestigateData(SubData);
	}
	if (NeedOptimize)
		OptimizeData();
	if (NumPalette)
		PrepareTable();
//#ifdef MSWIN
//	farfree(h);
//#else
	delete[] h;
//#endif
GESTRACEEND("Optimize Palette End");
GESTRACEADD(GesTraceOPal, ges_trace_end - ges_trace);
}
void
G42OptimizedPalette::CreateStandardPalette(void)
{
	if (MaxPalette < 16)
	{
		DitherPalette[0].red = 0;
		DitherPalette[0].green = 0;
		DitherPalette[0].blue = 0;
		DitherPalette[1].red = 0xff;
		DitherPalette[1].green = 0xff;
		DitherPalette[1].blue = 0xff;
		NumPalette = 2;
		WhitePixel = 1;
	}
	else if (MaxPalette < 216)
	{
		DitherPalette[0].red =
		DitherPalette[0].green =
		DitherPalette[0].blue = 0x0;
		DitherPalette[1].red = 0x80;
		DitherPalette[1].green = 0x00;
		DitherPalette[1].blue = 0x00;
		DitherPalette[2].red = 0x00;
		DitherPalette[2].green = 0x80;
		DitherPalette[2].blue = 0x00;
		DitherPalette[3].red = 0x80;
		DitherPalette[3].green = 0x80;
		DitherPalette[3].blue = 0x00;
		DitherPalette[4].red = 0x00;
		DitherPalette[4].green = 0x00;
		DitherPalette[4].blue = 0x80;
		DitherPalette[5].red = 0x80;
		DitherPalette[5].green = 0x00;
		DitherPalette[5].blue = 0x80;
		DitherPalette[6].red = 0x00;
		DitherPalette[6].green = 0x80;
		DitherPalette[6].blue = 0x80;
		DitherPalette[7].red = 0x80;
		DitherPalette[7].green = 0x80;
		DitherPalette[7].blue = 0x80;
		DitherPalette[8].red =
		DitherPalette[8].green =
		DitherPalette[8].blue = 0xc0;
		DitherPalette[9].red = 0xff;
		DitherPalette[9].green = 0x00;
		DitherPalette[9].blue = 0x00;
		DitherPalette[10].red = 0x00;
		DitherPalette[10].green = 0xff;
		DitherPalette[10].blue = 0x00;
		DitherPalette[11].red = 0xff;
		DitherPalette[11].green = 0xff;
		DitherPalette[11].blue = 0x00;
		DitherPalette[12].red = 0x00;
		DitherPalette[12].green = 0x00;
		DitherPalette[12].blue = 0xff;
		DitherPalette[13].red = 0xff;
		DitherPalette[13].green = 0x00;
		DitherPalette[13].blue = 0xff;
		DitherPalette[14].red = 0x00;
		DitherPalette[14].green = 0xff;
		DitherPalette[14].blue = 0xff;
		DitherPalette[15].red = 0xff;
		DitherPalette[15].green = 0xff;
		DitherPalette[15].blue = 0xff;
		NumPalette = 16;
		WhitePixel = 15;
	}
	else
	{
		int i;
		for (i = 0; i < 192; i++)
		{
			int rp = (i >> 5);
			DitherPalette[i].red =
				(byte)(((rp << 5) + (rp << 4) + (rp << 1) + rp));
			int gp = ((i >> 2) & 7);
			DitherPalette[i].green =
				(byte)((gp << 5) | (gp << 2) | (gp >> 1));
			int bp = (i & 3);
			DitherPalette[i].blue =
				(byte)((bp << 6) | (bp << 4) | (bp << 2) | bp);
		}
		for (i = 192; i < 208; i++)
		{
			int gp = (i - 192);
			DitherPalette[i].red =
			DitherPalette[i].green =
			DitherPalette[i].blue = (byte)((gp << 4) | gp);
		}
		DitherPalette[208].red =
		DitherPalette[208].green =
		DitherPalette[208].blue = 0xc0; // special one, because black is pixel 0
		DitherPalette[209].red = 0x00;
		DitherPalette[209].green = 0x00;
		DitherPalette[209].blue = 0x80;
		DitherPalette[210].red = 0x00;
		DitherPalette[210].green = 0x80;
		DitherPalette[210].blue = 0x00;
		DitherPalette[211].red = 0x00;
		DitherPalette[211].green = 0x80;
		DitherPalette[211].blue = 0x80;
		DitherPalette[212].red = 0x80;
		DitherPalette[212].green = 0x00;
		DitherPalette[212].blue = 0x00;
		DitherPalette[213].red = 0x80;
		DitherPalette[213].green = 0x00;
		DitherPalette[213].blue = 0x80;
		DitherPalette[214].red = 0x80;
		DitherPalette[214].green = 0x80;
		DitherPalette[214].blue = 0x00;
		DitherPalette[215].red = 0x80;
		DitherPalette[215].green = 0x80;
		DitherPalette[215].blue = 0x80;
		NumPalette = 216;
		WhitePixel = 207;
	}
}
void
G42OptimizedPalette::InvestigateData(G42ImageData * data)
{
	// this first case could probably be simplified greatly, but it should work
	// this way also, so I'm not going to take the chance of introducing
	// bugs - ges
	int d = data->GetDepth();
	if (d == 1)
	{
		G42Color * palette = data->GetPalette();
		bool has_white = false;
		uint16 i;
		for (i = 0; i < 2; i++)
		{
			DitherUnique[i] = 0;
		}
		int count = 0;
		for (int ty = 0; ty < data->GetNumTilesDown(); ty++)
		{
			for (int tx = 0; tx < data->GetNumTilesAcross(); tx++)
			{
				G42LockedImageTile tile(data->GetTile(ty, tx));
				//if (!tile || !tile.GetTile()->IsValid())
					//continue;
				for (int y = tile.GetTile()->GetTileOffsetY();
					y < tile.GetTile()->GetTileOffsetY() +
						tile.GetTile()->GetTileHeight(); y++)
				{
					byte * cp1 = *tile.GetTile()->GetRow(y) +
						(tile.GetTile()->GetTileOffsetX() >> 3);
					int s = 7 - (tile.GetTile()->GetTileOffsetX() & 7);
					for (int x = tile.GetTile()->GetTileOffsetX();
						x < tile.GetTile()->GetTileOffsetX() +
							tile.GetTile()->GetTileWidth(); x++)
					{
						int v = ((*cp1) >> s) & 1;
						if (!DitherUnique[v])
						{
							DitherUnique[v] = 1;
							if (palette && palette[v].red == 0xff &&
								palette[v].green == 0xff && palette[v].blue == 0xff)
									has_white = true;
							count++;
							if (count == 2)
								break;
						}
						if (s)
						{
							s--;
						}
						else
						{
							s = 7;
							cp1++;
						}
					}
					if (count == 2)
						break;
				}
				if (count == 2)
					break;
			}
			if (count == 2)
				break;
		}
		if (palette)
		{
			for (i = 0; i < 2; i++)
			{
				if (DitherUnique[i])
					AddColor(palette[i]);
			}
			if (NeedWhite && !has_white)
			{
				AddGray(0xff);
			}
		}
		else
		{
			for (i = 0; i < 2; i++)
			{
				if (DitherUnique[i])
					AddGray((byte)(i * 0xff));
			}
			if (NeedWhite && !DitherUnique[1])
				AddGray(0xff);
		}
	}
	else if (d == 4)
	{
		G42Color * palette = data->GetPalette();
		bool has_white = false;
		for (int i = 0; i < 16; i++)
		{
			DitherUnique[i] = 0;
		}
		int count = 0;
		for (int ty = 0; ty < data->GetNumTilesDown(); ty++)
		{
			for (int tx = 0; tx < data->GetNumTilesAcross(); tx++)
			{
				G42LockedImageTile tile(data->GetTile(ty, tx));
				//if (!tile || !tile.GetTile()->IsValid())
					//continue;
				for (int y = tile.GetTile()->GetTileOffsetY();
					y < tile.GetTile()->GetTileOffsetY() +
						tile.GetTile()->GetTileHeight(); y++)
				{
					byte * cp1 = *tile.GetTile()->GetRow(y) +
						(tile.GetTile()->GetTileOffsetX() >> 1);
					int s = ((tile.GetTile()->GetTileOffsetX() & 1) ? 0 : 4);
					for (int x = tile.GetTile()->GetTileOffsetX();
						x < tile.GetTile()->GetTileOffsetX() +
							tile.GetTile()->GetTileWidth(); x++)
					{
						int v = ((*cp1) >> s) & 0xff;
						if (!DitherUnique[v])
						{
							DitherUnique[v] = 1;
							if (palette && palette[v].red == 0xff &&
								palette[v].green == 0xff && palette[v].blue == 0xff)
									has_white = true;
							count++;
							if (count == 16)
								break;
						}
						if (s == 4)
						{
							s = 0;
						}
						else
						{
							s = 4;
							cp1++;
						}
					}
					if (count == 16)
						break;
				}
				if (count == 16)
					break;
			}
			if (count == 16)
				break;
		}
		if (palette)
		{
			for (int i = 0; i < 16; i++)
			{
				if (DitherUnique[i])
					AddColor(palette[i]);
			}
			if (NeedWhite && !has_white)
			{
				AddGray(0xff);
			}
		}
		else
		{
			for (int i = 0; i < 16; i++)
			{
				if (DitherUnique[i])
					AddGray((byte)(i * 0x11));
			}
			if (NeedWhite && !DitherUnique[15])
				AddGray(0xff);
		}
	}
	else if (d == 8)
	{
		G42Color * palette = data->GetPalette();
		bool has_white = false;
		for (int i = 0; i < 256; i++)
		{
			DitherUnique[i] = 0;
		}
		int count = 0;
		for (int ty = 0; ty < data->GetNumTilesDown(); ty++)
		{
			for (int tx = 0; tx < data->GetNumTilesAcross(); tx++)
			{
				G42LockedImageTile tile(data->GetTile(ty, tx));
				//if (!tile || !tile.GetTile()->IsValid())
					//continue;
				for (int y = tile.GetTile()->GetTileOffsetY();
					y < tile.GetTile()->GetTileOffsetY() + tile.GetTile()->GetTileHeight(); y++)
				{
					byte * cp1 = *tile.GetTile()->GetRow(y) + tile.GetTile()->GetTileOffsetX();
					for (int x = tile.GetTile()->GetTileOffsetX();
						x < tile.GetTile()->GetTileOffsetX() + tile.GetTile()->GetTileWidth(); x++)
					{
						int v = *cp1++;
						if (!DitherUnique[v])
						{
							DitherUnique[v] = 1;
							if (palette && palette[v].red == 0xff &&
								palette[v].green == 0xff && palette[v].blue == 0xff)
									has_white = true;
							count++;
							if (count == 256)
								break;
						}
					}
					if (count == 256)
						break;
				}
				if (count == 256)
					break;
			}
			if (count == 256)
				break;
		}
		if (palette)
		{
			for (int i = 0; i < 256; i++)
			{
				if (DitherUnique[i])
					AddColor(palette[i]);
			}
			if (NeedWhite && !has_white)
			{
				AddGray(0xff);
			}
		}
		else
		{
			for (int i = 0; i < 256; i++)
			{
				if (DitherUnique[i])
					AddGray((byte)i);
			}
			if (NeedWhite && !DitherUnique[255])
				AddGray(0xff);
		}
	}
	else if (d > 8)
	{
		if (NumPalette && !NeedOptimize)
		{
			for (int i = 0; i < NumPalette; i++)
			{
				h[ColorTo16(DitherPalette[i])] = 1;
			}
		}
		NeedOptimize = true;
		if (NeedWhite)
		{
			G42Color c(0xff, 0xff, 0xff);
			int v = ColorTo16(c);
			h[v] = 1;
		}
		for (int ty = 0; ty < data->GetNumTilesDown(); ty++)
		{
			for (int tx = 0; tx < data->GetNumTilesAcross(); tx++)
			{
				G42LockedImageTile tile(data->GetTile(ty, tx));
				if (!tile.GetTile() || !tile.GetTile()->IsValid())
					continue;
				for (int y = tile.GetTile()->GetTileOffsetY();
					y < tile.GetTile()->GetTileOffsetY() + tile.GetTile()->GetTileHeight(); y++)
				{
					byte * cp1 = *tile.GetTile()->GetRow(y) + tile.GetTile()->GetTileOffsetX() * 3;
					for (int x = tile.GetTile()->GetTileOffsetX();
						x < tile.GetTile()->GetTileOffsetX() + tile.GetTile()->GetTileWidth(); x++)
					{
						G42Color c;
						#ifdef MSWIN
						c.blue = *cp1++;
						c.green = *cp1++;
						c.red = *cp1++;
						#endif
						#ifdef MACOS
						c.red = *cp1++;
						c.green = *cp1++;
						c.blue = *cp1++;
						#endif						
						int v = ColorTo16(c);
						h[v] = 1;
					}
				}
			}
		}
	}
}
void
G42OptimizedPalette::AddColor(G42Color c)
{
	if (NeedOptimize)
	{
		h[ColorTo16(c)] = 1;
		return;
	}
	bool found = false;
	for (int i = 0; i < NumPalette; i++)
	{
		if (DitherPalette[i] == c)
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		if (NumPalette < MaxPalette)
		{
			DitherPalette[NumPalette++] = c;
		}
		else
		{
			NeedOptimize = true;
			for (int i = 0; i < NumPalette; i++)
			{
				h[ColorTo16(DitherPalette[i])] = 1;
			}
			h[ColorTo16(c)] = 1;
		}
	}
}
void
G42OptimizedPalette::OptimizeData(void)
{
	if (NeedOptimize)
	{
		uint16 i;
		for (i = 0, nh = 0; i < 4096; i++)
		{
			if (h[i])
				h[nh++] = i;
			DitherUnique[i] = 2;
			DitherTable[i] = 0;
		}
		if (nh <= MaxPalette)
		{
			for (i = 0; i < nh; i++)
			{
				DitherPalette[i].red =
					(byte)(((h[i] >> 4) & 0xf0) |
					((h[i] >> 8) & 0xf));
				DitherPalette[i].green =
					(byte)(((h[i]) & 0xf8) |
					((h[i] >> 5) & 7));
				DitherPalette[i].blue =
					(byte)(((h[i] << 5) & 0xe0) |
					((h[i] << 2) & 0x1c) |
					((h[i] >> 1) & 3));
			}
			NumPalette = nh;
		}
		else
		{
			int num_box, cur_box;
			num_box = 1;
			PaletteBox[0].nh = nh;
			PaletteBox[0].h = h;
			PaletteBox[0].ResetBoxLimits();
			while (num_box < MaxPalette)
			{
				int dr, dg, db, ctype;
				uint16 *lptr, *uptr;
				uint16 t;
				uint16 max_nh, max_side;
				int i, split, start, end;
				int count[32];
				unsigned int total_count;
				/* select next PaletteBox to split */
				max_nh = 1;
				max_side = 0;
				for (i = 0; i < num_box; i++)
				{
					uint16 cur_nh = PaletteBox[i].nh;
					uint16 cur_side = (uint16)CalcDistance(
						(int)((PaletteBox[i].rmax - PaletteBox[i].rmin)),
						(int)(PaletteBox[i].gmax - PaletteBox[i].gmin),
						(int)((PaletteBox[i].bmax - PaletteBox[i].bmin)));
#if 0
					if (((PaletteBox[i].rmax - PaletteBox[i].rmin) >> 1) > cur_side)
					{
						cur_side = (uint16)((PaletteBox[i].rmax -
							PaletteBox[i].rmin) >> 1);
					}
					if (PaletteBox[i].gmax - PaletteBox[i].gmin > cur_side)
					{
						cur_side = (uint16)(PaletteBox[i].gmax -
							PaletteBox[i].gmin);
					}
					if (((PaletteBox[i].bmax - PaletteBox[i].bmin) >> 2) > cur_side)
					{
						cur_side = (uint16)((PaletteBox[i].bmax -
							PaletteBox[i].bmin) >> 2);
					}
#endif
					if (num_box & 1)
					{
						if (cur_nh > max_nh ||
							(cur_nh == max_nh && cur_side > max_side))
						{
							cur_box = i;
							max_nh = cur_nh;
							max_side = cur_side;
						}
					}
					else
					{
						if (cur_nh > 1 && (cur_side > max_side ||
							(cur_side == max_side && cur_nh > max_nh)))
						{
							cur_box = i;
							max_nh = cur_nh;
							max_side = cur_side;
						}
					}
				}
				if (max_nh == 1)
					break;
				/* figure out which color to move */
				dr = PaletteBox[cur_box].rmax - PaletteBox[cur_box].rmin;
				dg = PaletteBox[cur_box].gmax - PaletteBox[cur_box].gmin;
				db = PaletteBox[cur_box].bmax - PaletteBox[cur_box].bmin;
				/* weight red and blue */
//				db >>= 2;
//				dr >>= 1;
				int cmask;
				if (db > dg && db > dr)
				{
					ctype = 0;
					cmask = 0x7;
				}
				else if (dr > dg)
				{
					ctype = 8;
					cmask = 0xf;
				}
				else
				{
					ctype = 3;
					cmask = 0x1f;
				}
				total_count = 0;
				memset(count, 0, 32 * sizeof (int));
				for (i = 0; i < PaletteBox[cur_box].nh; i++)
				{
					count[((PaletteBox[cur_box].h[i] >> ctype) & cmask)]++;
					total_count++;
				}
				total_count >>= 1;
				for (start = 0; !count[start] && start < 31; start++)
					/* empty loop */ ;
				for (end = 31; !count[end] && end; end--)
					/* empty loop */ ;
				for (split = 0, i = start;
					split < total_count && i < end;
					i++)
				{
					split += count[i];
				}
				split = i;
				lptr = PaletteBox[cur_box].h;
				uptr = PaletteBox[cur_box].h + PaletteBox[cur_box].nh - 1;
				while (lptr < uptr)
				{
					while ((((*lptr) >> ctype) & cmask) < split &&
						lptr < uptr)
						lptr++;
					while ((((*uptr) >> ctype) & cmask) >= split &&
						uptr > lptr)
						uptr--;
					if (lptr < uptr)
					{
						t = *lptr;
						*lptr = *uptr;
						*uptr = t;
					}
				}
				PaletteBox[num_box].nh = (uint16)(PaletteBox[cur_box].nh -
					(uint16)(lptr - PaletteBox[cur_box].h));
				PaletteBox[num_box].h = lptr;
				PaletteBox[cur_box].nh = (uint16)(lptr - PaletteBox[cur_box].h);
				PaletteBox[cur_box].ResetBoxLimits();
				PaletteBox[num_box].ResetBoxLimits();
				num_box++;
			}
			for (i = 0; i < num_box; i++)
			{
				if (PaletteBox[i].rmin == 0 && PaletteBox[i].rmax == 15)
					DitherPalette[i].red = 0x7f;
				else if (PaletteBox[i].rmin == 0)
					DitherPalette[i].red = 0;
				else if (PaletteBox[i].rmax == 15)
					DitherPalette[i].red = 0xff;
				else
				{
					int vmax, vmin;
					vmax = PaletteBox[i].rmax;
					vmax = (vmax << 4) | (vmax);
					vmin = PaletteBox[i].rmin;
					vmin = (vmin << 4) | (vmin);
					DitherPalette[i].red = (byte)((vmax + vmin) >> 1);
				}
				if (PaletteBox[i].gmin == 0 && PaletteBox[i].gmax == 31)
					DitherPalette[i].green = 0x7f;
				else if (PaletteBox[i].gmin == 0)
					DitherPalette[i].green = 0;
				else if (PaletteBox[i].gmax == 31)
					DitherPalette[i].green = 0xff;
				else
				{
					int vmax, vmin;
					vmax = PaletteBox[i].gmax;
					vmax = (vmax << 3) | (vmax >> 2);
					vmin = PaletteBox[i].gmin;
					vmin = (vmin << 3) | (vmin >> 2);
					DitherPalette[i].green = (byte)((vmax + vmin) >> 1);
				}
				if (PaletteBox[i].bmin == 0 && PaletteBox[i].bmax == 7)
					DitherPalette[i].blue = 0x7f;
				else if (PaletteBox[i].bmin == 0)
					DitherPalette[i].blue = 0;
				else if (PaletteBox[i].bmax == 7)
					DitherPalette[i].blue = 0xff;
				else
				{
					int vmax, vmin;
					vmax = PaletteBox[i].bmax;
					vmax = (vmax << 5) | (vmax << 2) | (vmax >> 1);
					vmin = PaletteBox[i].bmin;
					vmin = (vmin << 5) | (vmin << 2) | (vmin >> 1);
					DitherPalette[i].blue = (byte)((vmax + vmin) >> 1);
				}
			}
			NumPalette = num_box;
		}
	}
}
#define DISTANCE_STEP 32
void
G42OptimizedPalette::PrepareTable(void)
{
	uint i;
	if (NumPalette)
	{
		if (FullColorOnly || NeedOptimize)
		{
//			DWORD start_time = GetTickCount();
			int min_dist = 0;
			int max_dist = DISTANCE_STEP - 1;
			for (i = 0; i < 4096; i++)
				h[i] = 0x7fff;
			bool * palette_finished = new bool [256];
			bool * side_finished = new bool [256 * 8];
			memset(palette_finished, 0, 256 * sizeof (bool));
			memset(side_finished, 0, 256 * 8 * sizeof (bool));
			while (1)
			{
				bool need_more = false;
				for (i = 0; i < NumPalette; i++)
				{
					if (palette_finished[i])
						break;
					int r = (DitherPalette[i].red >> 4);
					int dr = ((DitherPalette[i].red & 0xf) << 1);
					if (dr > 15)
						dr -= 16;
					else
						dr -= 14;
					int g = (DitherPalette[i].green >> 3);
					int dg = ((DitherPalette[i].green & 7) << 2);
					if (dg > 15)
						dg -= 16;
					else
						dg -= 12;
					int b = (DitherPalette[i].blue >> 5);
					int db = ((DitherPalette[i].blue) & 0x1f);
					if (db > 15)
						db -= 16;
					else
						db -= 15;
					int sfoffset = (i << 3);
					bool is_any_used = false;
					if (!side_finished[sfoffset])
					{
						for (int dist = min_dist; dist <= max_dist; dist++)
						{
							bool is_used = false;
							int max_ir = min(r, dist);
							for (int ir = 0; ir <= max_ir; ir++)
							{
								int index_r = ((r - ir) << 8);
								int dist_r = (dr + (ir << 5));
								int max_ig = min(g, dist);
								for (int ig = 0; ig <= max_ig; ig++)
								{
									int index_g = index_r | ((g - ig) << 3);
									int dist_g = dg + (ig << 5);
									int ib;
									if (ig == dist || ir == dist)
										ib = 0;
									else
										ib = dist;
									int max_ib = min(b, dist);
									for (; ib <= max_ib; ib++)
									{
										int index = index_g | (b - ib);
										int dist_b = (db + (ib << 5));
										int d = CalcDistance(dist_r, dist_g, dist_b);
										if (h[index] >= d)
										{
											h[index] = (uint16)d;
											DitherTable[index] = (byte)i;
											is_used = true;
										}
									} // ib
								} // ig
							} // ir
							if (!is_used)
							{
								side_finished[sfoffset] = true;
								break;
							}
						} // dist
						if (!side_finished[sfoffset])
							is_any_used = true;
					}
					if (!side_finished[sfoffset + 1])
					{
						for (int dist = min_dist; dist <= max_dist; dist++)
						{
							bool is_used = false;
							int max_ir = min(r, dist);
							for (int ir = 0; ir <= max_ir; ir++)
							{
								int index_r = ((r - ir) << 8);
								int dist_r = (dr + (ir << 5));
								int max_ig = min(g, dist);
								for (int ig = 0; ig <= max_ig; ig++)
								{
									int index_g = index_r | ((g - ig) << 3);
									int dist_g = dg + (ig << 5);
									int ib;
									if (ig == dist || ir == dist)
										ib = 0;
									else
										ib = dist;
									int max_ib = min(6 - b, dist);
									for (; ib <= max_ib; ib++)
									{
										int index = index_g | (b + ib + 1);
										int dist_b = ((32 - db + (ib << 5)));
										int d = CalcDistance(dist_r, dist_g, dist_b);
										if (h[index] >= d)
										{
											h[index] = (uint16)d;
											DitherTable[index] = (byte)i;
											is_used = true;
										}
									} // ib
								} // ig
							} // ir
							if (!is_used)
							{
								side_finished[sfoffset + 1] = true;
								break;
							}
						} // dist
						if (!side_finished[sfoffset + 1])
							is_any_used = true;
					}
					if (!side_finished[sfoffset + 2])
					{
						for (int dist = min_dist; dist <= max_dist; dist++)
						{
							bool is_used = false;
							int max_ir = min(r, dist);
							for (int ir = 0; ir <= max_ir; ir++)
							{
								int index_r = ((r - ir) << 8);
								int dist_r = (dr + (ir << 5));
								int max_ig = min(30 - g, dist);
								for (int ig = 0; ig <= max_ig; ig++)
								{
									int index_g = index_r | ((g + ig + 1) << 3);
									int dist_g = 32 - dg + (ig << 5);
									int ib;
									if (ig == dist || ir == dist)
										ib = 0;
									else
										ib = dist;
									int max_ib = min(b, dist);
									for (; ib <= max_ib; ib++)
									{
										int index = index_g | (b - ib);
										int dist_b = (db + (ib << 5));
										int d = CalcDistance(dist_r, dist_g, dist_b);
										if (h[index] >= d)
										{
											h[index] = (uint16)d;
											DitherTable[index] = (byte)i;
											is_used = true;
										}
									} // ib
								} // ig
							} // ir
							if (!is_used)
							{
								side_finished[sfoffset + 2] = true;
								break;
							}
						} // dist
						if (!side_finished[sfoffset + 2])
							is_any_used = true;
					}
					if (!side_finished[sfoffset + 3])
					{
						for (int dist = min_dist; dist <= max_dist; dist++)
						{
							bool is_used = false;
							int max_ir = min(r, dist);
							for (int ir = 0; ir <= max_ir; ir++)
							{
								int index_r = ((r - ir) << 8);
								int dist_r = ((dr + (ir << 5)));
								int max_ig = min(30 - g, dist);
								for (int ig = 0; ig <= max_ig; ig++)
								{
									int index_g = index_r | ((g + ig + 1) << 3);
									int dist_g = 32 - dg + (ig << 5);
									int ib;
									if (ig == dist || ir == dist)
										ib = 0;
									else
										ib = dist;
									int max_ib = min(6 - b, dist);
									for (; ib <= max_ib; ib++)
									{
										int index = index_g | (b + ib + 1);
										int dist_b = ((32 - db + (ib << 5)));
										int d = CalcDistance(dist_r, dist_g, dist_b);
										if (h[index] >= d)
										{
											h[index] = (uint16)d;
											DitherTable[index] = (byte)i;
											is_used = true;
										}
									} // ib
								} // ig
							} // ir
							if (!is_used)
							{
								side_finished[sfoffset + 3] = true;
								break;
							}
						} // dist
						if (!side_finished[sfoffset + 3])
							is_any_used = true;
					}
					if (!side_finished[sfoffset + 4])
					{
						for (int dist = min_dist; dist <= max_dist; dist++)
						{
							bool is_used = false;
							int max_ir = min(14 - r, dist);
							for (int ir = 0; ir <= max_ir; ir++)
							{
								int index_r = ((r + ir + 1) << 8);
								int dist_r = (32 - dr + (ir << 5));
								int max_ig = min(g, dist);
								for (int ig = 0; ig <= max_ig; ig++)
								{
									int index_g = index_r | ((g - ig) << 3);
									int dist_g = dg + (ig << 5);
									int ib;
									if (ig == dist || ir == dist)
										ib = 0;
									else
										ib = dist;
									int max_ib = min(b, dist);
									for (; ib <= max_ib; ib++)
									{
										int index = index_g | (b - ib);
										int dist_b = (db + (ib << 5));
										int d = CalcDistance(dist_r, dist_g, dist_b);
										if (h[index] >= d)
										{
											h[index] = (uint16)d;
											DitherTable[index] = (byte)i;
											is_used = true;
										}
									} // ib
								} // ig
							} // ir
							if (!is_used)
							{
								side_finished[sfoffset + 4] = true;
								break;
							}
						} // dist
						if (!side_finished[sfoffset + 4])
							is_any_used = true;
					}
					if (!side_finished[sfoffset + 5])
					{
						for (int dist = min_dist; dist <= max_dist; dist++)
						{
							bool is_used = false;
							int max_ir = min(14 - r, dist);
							for (int ir = 0; ir <= max_ir; ir++)
							{
								int index_r = ((r + ir + 1) << 8);
								int dist_r = (32 - dr + (ir << 5));
								int max_ig = min(g, dist);
								for (int ig = 0; ig <= max_ig; ig++)
								{
									int index_g = index_r | ((g - ig) << 3);
									int dist_g = dg + (ig << 5);
									int ib;
									if (ig == dist || ir == dist)
										ib = 0;
									else
										ib = dist;
									int max_ib = min(6 - b, dist);
									for (; ib <= max_ib; ib++)
									{
										int index = index_g | (b + ib + 1);
										int dist_b = ((32 - db + (ib << 5)));
										int d = CalcDistance(dist_r, dist_g, dist_b);
										if (h[index] >= d)
										{
											h[index] = (uint16)d;
											DitherTable[index] = (byte)i;
											is_used = true;
										}
									} // ib
								} // ig
							} // ir
							if (!is_used)
							{
								side_finished[sfoffset + 5] = true;
								break;
							}
						} // dist
						if (!side_finished[sfoffset + 5])
							is_any_used = true;
					}
					if (!side_finished[sfoffset + 6])
					{
						for (int dist = min_dist; dist <= max_dist; dist++)
						{
							bool is_used = false;
							int max_ir = min(14 - r, dist);
							for (int ir = 0; ir <= max_ir; ir++)
							{
								int index_r = ((r + ir + 1) << 8);
								int dist_r = (32 - dr + (ir << 5));
								int max_ig = min(30 - g, dist);
								for (int ig = 0; ig <= max_ig; ig++)
								{
									int index_g = index_r | ((g + ig + 1) << 3);
									int dist_g = 32 - dg + (ig << 5);
									int ib;
									if (ig == dist || ir == dist)
										ib = 0;
									else
										ib = dist;
									int max_ib = min(b, dist);
									for (; ib <= max_ib; ib++)
									{
										int index = index_g | (b - ib);
										int dist_b = (db + (ib << 5));
										int d = CalcDistance(dist_r, dist_g, dist_b);
										if (h[index] >= d)
										{
											h[index] = (uint16)d;
											DitherTable[index] = (byte)i;
											is_used = true;
										}
									} // ib
								} // ig
							} // ir
							if (!is_used)
							{
								side_finished[sfoffset + 6] = true;
								break;
							}
						} // dist
						if (!side_finished[sfoffset + 6])
							is_any_used = true;
					}
					if (!side_finished[sfoffset + 7])
					{
						for (int dist = min_dist; dist <= max_dist; dist++)
						{
							bool is_used = false;
							int max_ir = min(14 - r, dist);
							for (int ir = 0; ir <= max_ir; ir++)
							{
								int index_r = ((r + ir + 1) << 8);
								int dist_r = (32 - dr + (ir << 5));
								int max_ig = min(30 - g, dist);
								for (int ig = 0; ig <= max_ig; ig++)
								{
									int index_g = index_r | ((g + ig + 1) << 3);
									int dist_g = 32 - dg + (ig << 5);
									int ib;
									if (ig == dist || ir == dist)
										ib = 0;
									else
										ib = dist;
									int max_ib = min(6 - b, dist);
									for (; ib <= max_ib; ib++)
									{
										int index = index_g | (b + ib + 1);
										int dist_b = ((32 - db + (ib << 5)));
										int d = CalcDistance(dist_r, dist_g, dist_b);
										if (h[index] >= d)
										{
											h[index] = (uint16)d;
											DitherTable[index] = (byte)i;
											is_used = true;
										}
									} // ib
								} // ig
							} // ir
							if (!is_used)
							{
								side_finished[sfoffset + 7] = true;
								break;
							}
						} // dist
						if (!side_finished[sfoffset + 7])
							is_any_used = true;
					}
					if (is_any_used)
					{
						need_more = true;
					}
					else
					{
						palette_finished[i] = true;
					}
				} // for num palette
				if (!need_more)
				{
					break; // from distance loop
				}
				min_dist = max_dist + 1;
				max_dist += DISTANCE_STEP;
				if (min_dist >= 32)
					break;
			}
			WhitePixel = DitherTable[RGBTo16(0xff, 0xff, 0xff)];
			delete [] palette_finished;
			delete [] side_finished;
//			DWORD stop_time = GetTickCount();
//GES((ges, "Elapsed time (milliseconds): %ld\n", stop_time - start_time));
		}
		else
		{
			// set up translations for data and sub_data
			memset(DitherTable, 0, 512 * sizeof (uint16));
			if (Data)
			{
				if (Data->GetNumPalette() && Data->GetPalette())
				{
					G42Color * pal = Data->GetPalette();
					for (int i = 0; i < Data->GetNumPalette(); i++)
					{
						for (int j = 0; j < NumPalette; j++)
						{
							if (pal[i] == DitherPalette[j])
							{
								DitherTable[i] = (byte)j;
								i = Data->GetNumPalette();
								break;
							}
						}
					}
				}
				else
				{
					for (int i = 0; i < (1 << Data->GetDepth()); i++)
					{
						byte g = (byte)i;
						if (Data->GetDepth() == 1)
							g *= (byte)0xff;
						else if (Data->GetDepth() == 4)
							g *= (byte)0x11;
						G42Color c(g, g, g);
						for (int j = 0; j < NumPalette; j++)
						{
							if (c == DitherPalette[j])
							{
								DitherTable[i] = (byte)j;
								i = 0xff;
								break;
							}
						}
					}
				}
			}
			if (SubData)
			{
				if (SubData->GetNumPalette() && SubData->GetPalette())
				{
					G42Color * pal = SubData->GetPalette();
					for (int i = 0; i < SubData->GetNumPalette(); i++)
					{
						for (int j = 0; j < NumPalette; j++)
						{
							if (pal[i] == DitherPalette[j])
							{
								DitherTable[i + 256] = (byte)j;
								i = SubData->GetNumPalette();
								break;
							}
						}
					}
				}
				else
				{
					for (int i = 0; i < (1 << SubData->GetDepth()); i++)
					{
						byte g = (byte)i;
						if (SubData->GetDepth() == 1)
							g *= (byte)0xff;
						else if (SubData->GetDepth() == 4)
							g *= (byte)0x11;
						G42Color c(g, g, g);
						for (int j = 0; j < NumPalette; j++)
						{
							if (c == DitherPalette[j])
							{
								DitherTable[i + 256] = (byte)j;
								i = 0xff;
								break;
							}
						}
					}
				}
			}
		}
	}
}
#if 0
// save off old, slow, but accurate version
void
G42OptimizedPalette::PrepareTable(void)
{
	uint i;
	if (NumPalette)
	{
		if (FullColorOnly || NeedOptimize)
		{
			bool finished = false;
			int dist = 0;
			for (i = 0; i <= 32767; i++)
				h[i] = 0x7fff;
			bool palette_finished[256];
//			bool palette_used[256];
			for (i = 0; i < 256; i++)
				palette_finished[i] = false;
			while (!finished)
			{
				bool need_more = false;
//				for (int i = 0; i < 256; i++)
//					palette_used[i] = false;
				for (i = 0; i < NumPalette; i++)
				{
				if (palette_finished[i])
					continue;
				bool is_used = false;
							int r = (DitherPalette[i].red >> 3);
							int dr = (DitherPalette[i].red & 7);
							if (dr > 3)
								dr -= 4;
							else
								dr -= 3;
							int g = (DitherPalette[i].green >> 3);
							int dg = (DitherPalette[i].green & 7);
							if (dg > 3)
								dg -= 4;
							else
								dg -= 3;
							int b = (DitherPalette[i].blue >> 3);
							int db = (DitherPalette[i].blue & 7);
							if (db > 3)
								db -= 4;
							else
								db -= 3;
#if 0
				for (int ir = 0; ir <= dist; ir++)
				for (int ig = 0; ig <= dist; ig++)
				{
					int ib;
					if (ir == dist || ig == dist)
						ib = 0;
					else
						ib = dist;
					for (; ib <= dist; ib++)
					{
#endif
				for (int ir = 0; ir <= dist; ir++)
				{
					for (int ig = 0; ig <= dist - ir; ig++)
					{
						int ib = dist - (ir + ig);
#if 0
						for (i = 0; i < NumPalette; i++)
						{
							if (palette_finished[i])
								continue;
							bool is_used = false;
							int r = (DitherPalette[i].red >> 3);
							int dr = (DitherPalette[i].red & 7);
							if (dr > 3)
								dr -= 4;
							else
								dr -= 3;
							int g = (DitherPalette[i].green >> 3);
							int dg = (DitherPalette[i].green & 7);
							if (dg > 3)
								dg -= 4;
							else
								dg -= 3;
							int b = (DitherPalette[i].blue >> 3);
							int db = (DitherPalette[i].blue & 7);
							if (db > 3)
								db -= 4;
							else
								db -= 3;
						}
#endif
							if (r - ir >= 0 && g - ig >= 0 && b - ib >= 0)
							{
								int index = ((r - ir) << 10) | ((g - ig) << 5) |
									(b - ib);
								int dist_b = ((db + (ib << 3)) >> 2);
								int dist_r = ((dr + (ir << 3)) >> 1);
								int dist_g = dg + (ig << 3);
								int d = CalcDistance(dist_r, dist_g, dist_b);
								if (h[index] >= d)
								{
									h[index] = (uint16)d;
									DitherTable[index] = (byte)i;
									is_used = true;
								}
							}
							if (r - ir >= 0 && g - ig >= 0 && b + ib + 1 < 32)
							{
								int index = ((r - ir) << 10) | ((g - ig) << 5) |
									(b + ib + 1);
								int dist_b = ((8 - db + (ib << 3)) >> 2);
								int dist_r = ((dr + (ir << 3)) >> 1);
								int dist_g = dg + (ig << 3);
								int d = CalcDistance(dist_r, dist_g, dist_b);
								if (h[index] >= d)
								{
									h[index] = (uint16)d;
									DitherTable[index] = (byte)i;
									is_used = true;
								}
							}
							if (r - ir >= 0 && g + ig + 1 < 32 && b - ib >= 0)
							{
								int index = ((r - ir) << 10) | ((g + ig + 1) << 5) |
									(b - ib);
								int dist_b = ((db + (ib << 3)) >> 2);
								int dist_r = ((dr + (ir << 3)) >> 1);
								int dist_g = 8 - dg + (ig << 3);
								int d = CalcDistance(dist_r, dist_g, dist_b);
								if (h[index] >= d)
								{
									h[index] = (uint16)d;
									DitherTable[index] = (byte)i;
									is_used = true;
								}
							}
							if (r - ir >= 0 && g + ig + 1 < 32 && b + ib + 1 < 32)
							{
								int index = ((r - ir) << 10) | ((g + ig + 1) << 5) |
									(b + ib + 1);
								int dist_b = ((8 - db + (ib << 3)) >> 2);
								int dist_r = ((dr + (ir << 3)) >> 1);
								int dist_g = 8 - dg + (ig << 3);
								int d = CalcDistance(dist_r, dist_g, dist_b);
								if (h[index] >= d)
								{
									h[index] = (uint16)d;
									DitherTable[index] = (byte)i;
									is_used = true;
								}
							}
							if (r + ir + 1 < 32 && g - ig >= 0 && b - ib >= 0)
							{
								int index = ((r + ir + 1) << 10) | ((g - ig) << 5) |
									(b - ib);
								int dist_b = ((db + (ib << 3)) >> 2);
								int dist_r = ((8 - dr + (ir << 3)) >> 1);
								int dist_g = dg + (ig << 3);
								int d = CalcDistance(dist_r, dist_g, dist_b);
								if (h[index] >= d)
								{
									h[index] = (uint16)d;
									DitherTable[index] = (byte)i;
									is_used = true;
								}
							}
							if (r + ir + 1 < 32 && g - ig >= 0 && b + ib + 1 < 32)
							{
								int index = ((r + ir + 1) << 10) | ((g - ig) << 5) |
									(b + ib + 1);
								int dist_b = ((8 - db + (ib << 3)) >> 2);
								int dist_r = ((8 - dr + (ir << 3)) >> 1);
								int dist_g = dg + (ig << 3);
								int d = CalcDistance(dist_r, dist_g, dist_b);
								if (h[index] >= d)
								{
									h[index] = (uint16)d;
									DitherTable[index] = (byte)i;
									is_used = true;
								}
							}
							if (r + ir + 1 < 32 && g + ig + 1 < 32 && b - ib >= 0)
							{
								int index = ((r + ir + 1) << 10) | ((g + ig + 1) << 5) |
									(b - ib);
								int dist_b = ((db + (ib << 3)) >> 2);
								int dist_r = ((8 - dr + (ir << 3)) >> 1);
								int dist_g = 8 - dg + (ig << 3);
								int d = CalcDistance(dist_r, dist_g, dist_b);
								if (h[index] >= d)
								{
									h[index] = (uint16)d;
									DitherTable[index] = (byte)i;
									is_used = true;
								}
							}
							if (r + ir + 1 < 32 && g + ig + 1 < 32 &&
								b + ib + 1 < 32)
							{
								int index = ((r + ir + 1) << 10) | ((g + ig + 1) << 5) |
									(b + ib + 1);
								int dist_b = ((8 - db + (ib << 3)) >> 2);
								int dist_r = ((8 - dr + (ir << 3)) >> 1);
								int dist_g = 8 - dg + (ig << 3);
								int d = CalcDistance(dist_r, dist_g, dist_b);
								if (h[index] >= d)
								{
									h[index] = (uint16)d;
									DitherTable[index] = (byte)i;
									is_used = true;
								}
							}
						}
					}
					if (is_used)
						need_more = true;
					else
						palette_finished[i] = true;
				}
				if (!need_more)
				{
					finished = true;
				}
#if 0
				else
				{
					for (int i = 0; i < 256; i++)
						if (!palette_used[i])
							palette_finished[i] = true;
				}
#endif				
				dist++;
				if (dist > 96)
					break;
			}
			WhitePixel = DitherTable[RGBTo16(0xff, 0xff, 0xff)];
		}
		else
		{
			// set up translations for data and sub_data
      	memset(DitherTable, 0, 512 * sizeof (uint16));
			if (Data)
			{
				if (Data->GetNumPalette() && Data->GetPalette())
				{
					G42Color * pal = Data->GetPalette();
					for (int i = 0; i < Data->GetNumPalette(); i++)
					{
						for (int j = 0; j < NumPalette; j++)
						{
							if (pal[i] == DitherPalette[j])
							{
								DitherTable[i] = (byte)j;
								i = Data->GetNumPalette();
								break;
							}
						}
					}
				}
				else
				{
					for (int i = 0; i < (1 << Data->GetDepth()); i++)
					{
						byte g = (byte)i;
						if (Data->GetDepth() == 1)
							g *= (byte)0xff;
						else if (Data->GetDepth() == 4)
							g *= (byte)0x11;
						G42Color c(g, g, g);
						for (int j = 0; j < NumPalette; j++)
						{
							if (c == DitherPalette[j])
							{
								DitherTable[i] = (byte)j;
								i = 0xff;
								break;
							}
						}
					}
				}
			}
			if (SubData)
			{
				if (SubData->GetNumPalette() && SubData->GetPalette())
				{
					G42Color * pal = SubData->GetPalette();
					for (int i = 0; i < SubData->GetNumPalette(); i++)
					{
						for (int j = 0; j < NumPalette; j++)
						{
							if (pal[i] == DitherPalette[j])
							{
								DitherTable[i + 256] = (byte)j;
								i = SubData->GetNumPalette();
								break;
							}
						}
					}
				}
				else
				{
					for (int i = 0; i < (1 << SubData->GetDepth()); i++)
					{
						byte g = (byte)i;
						if (SubData->GetDepth() == 1)
							g *= (byte)0xff;
						else if (SubData->GetDepth() == 4)
							g *= (byte)0x11;
						G42Color c(g, g, g);
						for (int j = 0; j < NumPalette; j++)
						{
							if (c == DitherPalette[j])
							{
								DitherTable[i + 256] = (byte)j;
								i = 0xff;
								break;
							}
						}
					}
				}
			}
		}
	}
}
#endif
