// g42idata.cpp - image data
/*************************************************************
	File:          g42idata.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   This class holds the true image data in memory
						or in a swap file, and takes care of reference
                  counting and swapping.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "g42itype.h"
#include "g42idata.h"
#include <math.h>
#if !defined(MACOS)
#include <dir.h>
#endif
// create the data
G42ImageData::G42ImageData(G42Data * data, uint32 width, uint32 height, uint depth,
	uint num_palette, bool partial) :
		Width(width), Height(height), Depth(depth), NumPalette(num_palette),
		Data(data), DataPalette(0), TileArray(0), NumTiles(0),
		ReferenceCount(0), Finished(false), ValidStartRow(0), ValidNumRows(0),
		Valid(true),
		Partial(partial), PartialTileStartX(0), PartialTileStartY(0),
		PartialTileWidth(0), PartialTileHeight(0),
		OldPartialTileStartX(0), OldPartialTileStartY(0),
		OldPartialTileWidth(0), OldPartialTileHeight(0)
{
	// create the palette if there is one
	if (NumPalette)
	{
		DataPalette = new G42Color [NumPalette];
		if (!DataPalette)
		{
			Valid = false;
			return;
		}
	}
	uint32 row_bytes = ((Width * (uint32)Depth + 7) >> 3);
	uint32 image_size = row_bytes * Height;
	uint memory_block_size = Data->GetBlockSize();
	NumTiles = image_size / (uint32)(memory_block_size) + 1;
	// we want to create squarish tiles, pixel wise at least
	uint tile_side = (uint)sqrt((double)((memory_block_size * 8L) / (long)Depth));
	#define LONG_TILES
	#ifdef LONG_TILES
	tile_side <<= 1;
	#endif
	TileWidth = tile_side;
	if (Depth < 8)
		TileWidth = ((TileWidth + 7) & ~7);
	NumTilesAcross = (uint)((Width - 1) / (uint32)TileWidth) + 1;
	TileWidth = (uint)((Width - 1) / (uint32)NumTilesAcross) + 1;
	if (Depth < 8)
		TileWidth = ((TileWidth + 7) & ~7);
	TileRowBytes = (uint)(((((uint32)TileWidth * (uint32)Depth +
		7) >> 3) + 3) & ~3);
	TileHeight = memory_block_size / TileRowBytes;
	NumTilesDown = (uint)((Height - 1) / (uint32)TileHeight) + 1;
	TileHeight = (uint)((Height - 1) / (uint32)NumTilesDown) + 1;
	NumTiles = (uint32)NumTilesAcross * (uint32)NumTilesDown;
	TileSize = TileRowBytes * TileHeight;
	LastTileWidth = (uint)(Width -
		(uint32)TileWidth * (uint32)(NumTilesAcross - 1));
	LastTileHeight = (uint)(Height -
		(uint32)TileHeight * (uint32)(NumTilesDown - 1));
	FirstTileOffsetX = 0;
	FirstTileOffsetY = 0;
	if (!Partial) // partial tiles get created at the SetPartial call
	{
		// set up with and height so we don't have to check Partial
		PartialTileWidth = OldPartialTileWidth = NumTilesAcross;
		PartialTileHeight = OldPartialTileHeight = NumTilesDown;
		TileArray = new G42ImageTile ** [NumTilesDown];
		if (!TileArray)
		{
			Valid = false;
			return;
		}
		memset(TileArray, 0, NumTilesDown * sizeof (G42ImageTile **));
		uint32 tile_start_y = 0;
		uint cur_tile_height = TileHeight;
		// pointer to fill in the row array as we go
		for (uint tile_row = 0; tile_row < NumTilesDown; tile_row++)
		{
			if (tile_start_y + (uint32)cur_tile_height > Height)
				cur_tile_height = (uint)(Height - tile_start_y);
			TileArray[tile_row] = new G42ImageTile * [NumTilesAcross];
			if (!TileArray[tile_row])
			{
				Valid = false;
				return;
			}
			memset(TileArray[tile_row], 0,
				NumTilesAcross * sizeof (G42ImageTile **));
			uint32 tile_start_x = 0;
			uint cur_tile_width = TileWidth;
			for (int tile_column = 0; tile_column < NumTilesAcross; tile_column++)
			{
				if (tile_start_x + (uint32)cur_tile_width > Width)
					cur_tile_width = (uint)(Width - tile_start_x);
				TileArray[tile_row][tile_column] =
					new G42ImageTile(Data, TileWidth, TileHeight, Depth,
						TileRowBytes, tile_start_x, tile_start_y, 0, 0,
						cur_tile_width, cur_tile_height);
				if (!TileArray[tile_row][tile_column] ||
					!TileArray[tile_row][tile_column]->IsValid())
				{
					Valid = false;
					return;
				}
				tile_start_x += (uint32)TileWidth;
			}
			tile_start_y += (uint32)TileHeight;
		}
	}
}
G42ImageData::~G42ImageData()
{
	if (DataPalette)
	{
		delete[] DataPalette;
		DataPalette = 0;
	}
	if (TileArray)
	{
		for (int tile_row = 0; tile_row < PartialTileHeight; tile_row++)
		{
			if (TileArray[tile_row])
			{
				for (int tile_column = 0; tile_column < PartialTileWidth;
					tile_column++)
				{
					if (TileArray[tile_row][tile_column])
						delete TileArray[tile_row][tile_column];
				}
				delete[] TileArray[tile_row];
			}
		}
		delete[] TileArray;
		TileArray = 0;
	}
   Valid = false;
}
G42ImageData::G42ImageData(G42ImageData & Ref)
	:
	Width(Ref.Width), Height(Ref.Height), Depth(Ref.Depth),
	NumPalette(Ref.NumPalette),
	Data(Ref.Data), DataPalette(0), TileArray(0), NumTiles(Ref.NumTiles),
	ReferenceCount(1), Finished(Ref.Finished), ValidStartRow(Ref.ValidStartRow),
	ValidNumRows(Ref.ValidNumRows), Valid(Ref.Valid),
	NumTilesAcross(Ref.NumTilesAcross),
	NumTilesDown(Ref.NumTilesDown), TileWidth(Ref.TileWidth),
	TileHeight(Ref.TileHeight),
	TileRowBytes(Ref.TileRowBytes), TileSize(Ref.TileSize),
	LastTileHeight(Ref.LastTileHeight),
	LastTileWidth(Ref.LastTileWidth), FirstTileOffsetX(Ref.FirstTileOffsetX),
	FirstTileOffsetY(Ref.FirstTileOffsetY), Partial(Ref.Partial),
	PartialTileStartX(Ref.PartialTileStartX),
	PartialTileStartY(Ref.PartialTileStartY),
	PartialTileWidth(Ref.PartialTileWidth),
	PartialTileHeight(Ref.PartialTileHeight),
	OldPartialTileStartX(Ref.OldPartialTileStartX),
	OldPartialTileStartY(Ref.OldPartialTileStartY),
	OldPartialTileWidth(Ref.OldPartialTileWidth),
	OldPartialTileHeight(Ref.OldPartialTileHeight)
{
	if (Ref.DataPalette)
	{
		DataPalette = new G42Color [NumPalette];
		SetPalette(Ref.DataPalette);
	}
	TileArray = new G42ImageTile ** [PartialTileHeight];
	if (!TileArray)
	{
		Valid = false;
		return;
	}
	memset(TileArray, 0, PartialTileHeight * sizeof (G42ImageTile **));
	// pointer to fill in the row array as we go
	for (uint tile_row = 0; tile_row < PartialTileHeight; tile_row++)
	{
		TileArray[tile_row] = new G42ImageTile * [PartialTileWidth];
		if (!TileArray[tile_row])
		{
			Valid = false;
			return;
		}
		memset(TileArray[tile_row], 0,
			PartialTileWidth * sizeof (G42ImageTile **));
		for (int tile_column = 0; tile_column < PartialTileWidth; tile_column++)
		{
			TileArray[tile_row][tile_column] =
				new G42ImageTile(*(Ref.GetTile(tile_row, tile_column)));
			if (!TileArray[tile_row][tile_column] ||
				!TileArray[tile_row][tile_column]->IsValid())
			{
				Valid = false;
				return;
			}
		}
	}
}
G42ImageData::G42ImageData(G42ImageData & Ref, uint32 x, uint32 y,
		uint32 width, uint32 height)
	:
	Width(Ref.Width), Height(Ref.Height), Depth(Ref.Depth),
	NumPalette(Ref.NumPalette),
	Data(Ref.Data), DataPalette(0), TileArray(0), NumTiles(Ref.NumTiles),
	ReferenceCount(1), Finished(Ref.Finished), ValidStartRow(Ref.ValidStartRow),
	ValidNumRows(Ref.ValidNumRows), Valid(Ref.Valid),
	NumTilesAcross(Ref.NumTilesAcross),
	NumTilesDown(Ref.NumTilesDown), TileWidth(Ref.TileWidth),
	TileHeight(Ref.TileHeight),
	TileRowBytes(Ref.TileRowBytes), TileSize(Ref.TileSize),
	LastTileHeight(Ref.LastTileHeight),
	LastTileWidth(Ref.LastTileWidth), FirstTileOffsetX(Ref.FirstTileOffsetX),
	FirstTileOffsetY(Ref.FirstTileOffsetY), Partial(Ref.Partial),
	PartialTileStartX(Ref.PartialTileStartX),
	PartialTileStartY(Ref.PartialTileStartY),
	PartialTileWidth(Ref.PartialTileWidth),
	PartialTileHeight(Ref.PartialTileHeight),
	OldPartialTileStartX(Ref.OldPartialTileStartX),
	OldPartialTileStartY(Ref.OldPartialTileStartY),
	OldPartialTileWidth(Ref.OldPartialTileWidth),
	OldPartialTileHeight(Ref.OldPartialTileHeight)
{
	if (Ref.DataPalette)
	{
		DataPalette = new G42Color [NumPalette];
		SetPalette(Ref.DataPalette);
	}
	Width = width;
	Height = height;
	ValidStartRow = 0;
	ValidNumRows = height;
	// shrink the number of tiles and set first offsets based upon
	// the new size
	uint tile_x_offset;
	if (x < (uint32)(Ref.TileWidth - Ref.FirstTileOffsetX))
	{
		FirstTileOffsetX = Ref.FirstTileOffsetX + (uint)x;
      tile_x_offset = 0;
	}
	else
	{
		tile_x_offset = (uint)((x - (uint32)Ref.FirstTileOffsetX) /
			(uint32)Ref.TileWidth);
		FirstTileOffsetX = (uint)(x - ((uint32)tile_x_offset *
			(uint32)Ref.TileWidth + (uint32)Ref.FirstTileOffsetX));
	}
	uint tile_y_offset;
	if (y < (uint32)(Ref.TileHeight - Ref.FirstTileOffsetY))
	{
		FirstTileOffsetY = Ref.FirstTileOffsetY + (uint)y;
		tile_y_offset = 0;
	}
	else
	{
		tile_y_offset = (uint)((y - (uint32)Ref.FirstTileOffsetY) /
			(uint32)Ref.TileHeight);
		FirstTileOffsetY = (uint)(y - ((uint32)tile_y_offset *
			(uint32)Ref.TileHeight + (uint32)Ref.FirstTileOffsetY));
	}
	if (width > (uint32)(TileWidth - FirstTileOffsetX))
	{
		NumTilesAcross = (uint)((width + (uint32)FirstTileOffsetX - 1) /
			(uint32)TileWidth + 1);
		LastTileWidth = (uint)(width - ((uint32)(NumTilesAcross - 1) *
			(uint32)TileWidth - (uint32)FirstTileOffsetX));
	}
	else
	{
		NumTilesAcross = 1;
      LastTileWidth = (uint)width;
	}
	if (height > (uint32)(TileHeight - FirstTileOffsetY))
	{
		NumTilesDown = (uint)((height + (uint32)FirstTileOffsetY - 1) /
			(uint32)TileHeight + 1);
		LastTileHeight = (uint)(height - ((uint32)(NumTilesDown - 1) *
			(uint32)TileHeight - (uint32)FirstTileOffsetY));
	}
	else
	{
		NumTilesDown = 1;
		LastTileHeight = (uint)height;
	}
	NumTiles = (uint32)NumTilesAcross * (uint32)NumTilesDown;
	if (!Partial) // partial tiles get created at the SetPartial call
	{
		// set up with and height so we don't have to check Partial
		PartialTileWidth = OldPartialTileWidth = NumTilesAcross;
		PartialTileHeight = OldPartialTileHeight = NumTilesDown;
		PartialTileStartX = PartialTileStartY = 0;
		OldPartialTileStartX = OldPartialTileStartY = 0;
		TileArray = new G42ImageTile ** [NumTilesDown];
		if (!TileArray)
		{
			Valid = false;
			return;
		}
		memset(TileArray, 0, NumTilesDown * sizeof (G42ImageTile **));
		uint32 tile_start_y = 0;
		uint cur_tile_height;
		uint first_tile_start_y = (uint)FirstTileOffsetY;
		// pointer to fill in the row array as we go
		for (uint tile_row = 0; tile_row < NumTilesDown; tile_row++)
		{
      	cur_tile_height = TileHeight - first_tile_start_y;
			if (tile_start_y + (uint32)cur_tile_height > Height)
				cur_tile_height = (uint)(Height - tile_start_y);
			TileArray[tile_row] = new G42ImageTile * [NumTilesAcross];
			if (!TileArray[tile_row])
			{
				Valid = false;
				return;
			}
			memset(TileArray[tile_row], 0,
				NumTilesAcross * sizeof (G42ImageTile **));
			uint32 tile_start_x = 0;
			uint first_tile_start_x = (uint)FirstTileOffsetX;
			uint cur_tile_width;
			for (int tile_column = 0; tile_column < NumTilesAcross; tile_column++)
			{
	      	cur_tile_width = TileWidth - first_tile_start_x;
				if (tile_start_x + (uint32)cur_tile_width > Width)
					cur_tile_width = (uint)(Width - tile_start_x);
				G42ImageTile * old_tile = Ref.GetTile(tile_row + tile_y_offset,
					tile_column + tile_x_offset);
				if (old_tile)
				{
					TileArray[tile_row][tile_column] =
						new G42ImageTile(*old_tile,
							tile_start_x, tile_start_y, first_tile_start_x,
							first_tile_start_y, cur_tile_width, cur_tile_height);
				}
				else
				{
					TileArray[tile_row][tile_column] =
						new G42ImageTile(Data, TileWidth, TileHeight, Depth,
							TileRowBytes,
							tile_start_x, tile_start_y, first_tile_start_x,
							first_tile_start_y, cur_tile_width, cur_tile_height);
				}
				if (!TileArray[tile_row][tile_column] ||
					!TileArray[tile_row][tile_column]->IsValid())
				{
					Valid = false;
					return;
				}
				tile_start_x += (uint32)cur_tile_width;
				first_tile_start_x = 0;
			}
			tile_start_y += (uint32)cur_tile_height;
			first_tile_start_y = 0;
		}
	}
}
void
G42ImageData::SetPalette(G42Color * pal)
{
	if (DataPalette && pal)
	{
		for (uint i = 0; i < NumPalette; i++)
		{
      	DataPalette[i] = pal[i];
		}
	}
}
void
G42ImageData::DeletePalette(void)
{
	if (DataPalette)
   {
   	delete [] DataPalette;
      DataPalette = 0;
      NumPalette = 0;
   }
}
void
G42ImageData::SetPartial(uint x, uint y, uint width, uint height)
{
	if (!Partial || !Valid)
		return;
	if (x == PartialTileStartX && y == PartialTileStartY &&
		width == PartialTileWidth && height == PartialTileHeight &&
		TileArray)
	{
		OldPartialTileStartX = PartialTileStartX;
		OldPartialTileStartY = PartialTileStartY;
		OldPartialTileWidth = PartialTileWidth;
		OldPartialTileHeight = PartialTileHeight;
		return;
	}
	G42ImageTile *** old_tile_array = TileArray;
	if (!TileArray || width != PartialTileWidth ||
		height != PartialTileHeight)
	{
		TileArray = new G42ImageTile ** [height];
		if (!TileArray)
		{
			Valid = false;
			return;
		}
		memset(TileArray, 0, height * sizeof (G42ImageTile **));
		for (uint tile_row = 0; tile_row < height; tile_row++)
		{
			TileArray[tile_row] = new G42ImageTile * [width];
			if (!TileArray[tile_row])
			{
				Valid = false;
				return;
			}
			memset(TileArray[tile_row], 0,
				width * sizeof (G42ImageTile **));
		}
	}
	OldPartialTileWidth = PartialTileWidth;
	OldPartialTileStartX = PartialTileStartX;
	if (OldPartialTileStartX < x)
	{
		if (OldPartialTileWidth < (x - OldPartialTileStartX))
			OldPartialTileWidth = 0;
		else
			OldPartialTileWidth -= (x - OldPartialTileStartX);
		OldPartialTileStartX = x;
	}
	if (OldPartialTileStartX + OldPartialTileWidth > x + width)
	{
		if (OldPartialTileStartX > x + width)
			OldPartialTileWidth = 0;
		else
			OldPartialTileWidth = x + width - OldPartialTileStartX;
	}
	OldPartialTileHeight = PartialTileHeight;
	OldPartialTileStartY = PartialTileStartY;
	if (OldPartialTileStartY < y)
	{
		if (OldPartialTileHeight < (y - OldPartialTileStartY))
			OldPartialTileHeight = 0;
		else
			OldPartialTileHeight -= (y - OldPartialTileStartY);
		OldPartialTileStartY = y;
	}
	if (OldPartialTileStartY + OldPartialTileHeight > y + height)
	{
		if (OldPartialTileStartY > y + height)
			OldPartialTileHeight = 0;
		else
			OldPartialTileHeight = y + height - OldPartialTileStartY;
	}
	if (old_tile_array && OldPartialTileWidth && OldPartialTileHeight)
	{
		// copy old tiles
		if (y < PartialTileStartY || (y == PartialTileStartY &&
			x < PartialTileStartX))
		{
			int dy = height - 1;
			int sy = OldPartialTileHeight - 1;
			for (int y = 0; y < OldPartialTileHeight; y++)
			{
				int sx = OldPartialTileWidth - 1;
				int dx = OldPartialTileWidth - 1;
				if (x < PartialTileStartX)
					dx = width - 1;
				else
					sx = PartialTileWidth - 1;
				for (int x = 0; x < OldPartialTileWidth; x++)
				{
					if (TileArray[dy][dx])
						delete TileArray[dy][dx];
					TileArray[dy][dx] = old_tile_array[sy][sx];
					old_tile_array[sy][sx] = 0;
					sx--;
					dx--;
				}
				sy--;
				dy--;
			}
		}
		else if (y > PartialTileStartY || (y == PartialTileStartY &&
			x > PartialTileStartX))
		{
			int dy = 0;
			int sy = y - PartialTileStartY;
			for (int y = 0; y < OldPartialTileHeight; y++)
			{
				int sx = 0;
				int dx = 0;
				if (x < PartialTileStartX)
					dx = PartialTileStartX - x;
				else
					sx = x - PartialTileStartX;
				for (int x = 0; x < OldPartialTileWidth; x++)
				{
					if (TileArray[dy][dx])
						delete TileArray[dy][dx];
					TileArray[dy][dx] = old_tile_array[sy][sx];
					old_tile_array[sy][sx] = 0;
					sx++;
					dx++;
				}
				sy++;
				dy++;
			}
		}
		else if (old_tile_array != TileArray)
		{
			int dy = 0;
			int sy = 0;
			for (int y = 0; y < OldPartialTileHeight; y++)
			{
				int sx = 0;
				int dx = 0;
				for (int x = 0; x < OldPartialTileWidth; x++)
				{
					if (TileArray[dy][dx])
						delete TileArray[dy][dx];
					TileArray[dy][dx] = old_tile_array[sy][sx];
					old_tile_array[sy][sx] = 0;
					sx++;
					dx++;
				}
				sy++;
				dy++;
			}
		}
	}
	// remove old tiles and old array
	if (old_tile_array && old_tile_array != TileArray)
	{
		for (uint tile_row = 0; tile_row < PartialTileHeight; tile_row++)
		{
			for (uint tile_column = 0; tile_column < PartialTileWidth;
				tile_column++)
			{
				if (old_tile_array[tile_row][tile_column])
					delete old_tile_array[tile_row][tile_column];
			}
			delete [] old_tile_array[tile_row];
		}
		delete [] old_tile_array;
	}
	uint32 tile_start_y;
	if (y)
		tile_start_y = y * TileHeight - FirstTileOffsetY;
	else
		tile_start_y = 0;
	uint cur_tile_height = TileHeight;
   // fill in any new tiles
	for (uint tile_row = 0; tile_row < height; tile_row++)
	{
		if (tile_start_y + (uint32)cur_tile_height > Height)
			cur_tile_height = (uint)(Height - tile_start_y);
		uint32 tile_start_x;
		if (x)
			tile_start_x = x * TileWidth - FirstTileOffsetX;
		else
			tile_start_x = 0;
		uint cur_tile_width = TileWidth;
		for (int tile_column = 0; tile_column < width; tile_column++)
		{
			if (tile_start_x + (uint32)cur_tile_width > Width)
				cur_tile_width = (uint)(Width - tile_start_x);
			if (!TileArray[tile_row][tile_column])
			{
				TileArray[tile_row][tile_column] =
					new G42ImageTile(Data, TileWidth, TileHeight, Depth,
						TileRowBytes, tile_start_x, tile_start_y, 0, 0,
						cur_tile_width, cur_tile_height);
				if (!TileArray[tile_row][tile_column] ||
					!TileArray[tile_row][tile_column]->IsValid())
				{
					Valid = false;
					return;
				}
			}
			tile_start_x += (uint32)TileWidth;
		}
		tile_start_y += (uint32)TileHeight;
	}
	PartialTileStartX = x;
	PartialTileStartY = y;
	PartialTileWidth = width;
   PartialTileHeight = height;
}
bool
G42ImageData::NeedPartialUpdate(void)
{
	return (!OldPartialTileWidth || !OldPartialTileHeight ||
		(PartialTileStartX != OldPartialTileStartX) ||
		(PartialTileStartY != OldPartialTileStartY) ||
		(PartialTileWidth != OldPartialTileWidth) ||
		(PartialTileHeight != OldPartialTileHeight));
}
uint
G42ImageData::GetPartialUpdateTileX(void)
{
	if (!OldPartialTileWidth || !OldPartialTileHeight)
		return PartialTileStartX;
	if (PartialTileStartX != OldPartialTileStartX) // left
		return PartialTileStartX;
	if (PartialTileWidth != OldPartialTileWidth) // right
		return OldPartialTileStartX + OldPartialTileWidth;
	if (PartialTileStartY != OldPartialTileStartY) // top
		return PartialTileStartX;
	if (PartialTileHeight != OldPartialTileHeight) // bottom
		return PartialTileStartX;
	return 0;
}
uint
G42ImageData::GetPartialUpdateTileY(void)
{
	if (!OldPartialTileWidth || !OldPartialTileHeight)
		return PartialTileStartY;
	if (PartialTileStartX != OldPartialTileStartX) // left
		return OldPartialTileStartY;
	if (PartialTileWidth != OldPartialTileWidth) // right
		return OldPartialTileStartY;
	if (PartialTileStartY != OldPartialTileStartY) // top
		return PartialTileStartY;
	if (PartialTileHeight != OldPartialTileHeight) // bottom
		return OldPartialTileStartY + OldPartialTileHeight;
	return 0;
}
uint
G42ImageData::GetPartialUpdateTileWidth(void)
{
	if (!OldPartialTileWidth || !OldPartialTileHeight)
		return PartialTileWidth;
	if (PartialTileStartX != OldPartialTileStartX) // left
		return OldPartialTileStartX - PartialTileStartX;
	if (PartialTileWidth != OldPartialTileWidth) // right
		return PartialTileWidth - OldPartialTileWidth;
	if (PartialTileStartY != OldPartialTileStartY) // top
		return PartialTileWidth;
	if (PartialTileHeight != OldPartialTileHeight) // bottom
		return PartialTileWidth;
	return 0;
}
uint
G42ImageData::GetPartialUpdateTileHeight(void)
{
	if (!OldPartialTileWidth || !OldPartialTileHeight)
		return PartialTileHeight;
	if (PartialTileStartX != OldPartialTileStartX) // left
		return OldPartialTileHeight;
	if (PartialTileWidth != OldPartialTileWidth) // right
		return OldPartialTileHeight;
	if (PartialTileStartY != OldPartialTileStartY) // top
		return OldPartialTileStartY - PartialTileStartY;
	if (PartialTileHeight != OldPartialTileHeight) // bottom
		return PartialTileHeight - OldPartialTileHeight;
	return 0;
}
void
G42ImageData::UpdatePartial(void)
{
	if (!OldPartialTileWidth || !OldPartialTileHeight)
	{
		OldPartialTileWidth = PartialTileWidth;
		OldPartialTileStartX = PartialTileStartX;
		OldPartialTileHeight = PartialTileHeight;
		OldPartialTileStartY = PartialTileStartY;
      return;
	}
	if (PartialTileStartX != OldPartialTileStartX) // left
	{
		OldPartialTileWidth += (OldPartialTileStartX - PartialTileStartX);
		OldPartialTileStartX = PartialTileStartX;
		return;
	}
	if (PartialTileWidth != OldPartialTileWidth) // bottom
	{
		OldPartialTileWidth = PartialTileWidth;
		return;
	}
	if (PartialTileStartY != OldPartialTileStartY) // top
	{
		OldPartialTileHeight += (OldPartialTileStartY - PartialTileStartY);
		OldPartialTileStartY = PartialTileStartY;
		return;
	}
	if (PartialTileHeight != OldPartialTileHeight) // bottom
	{
		OldPartialTileHeight = PartialTileHeight;
		return;
	}
}
// create a new tile
G42ImageTile::G42ImageTile(G42Data * data, uint width, uint height, uint depth,
			uint row_bytes, uint32 start_x, uint32 start_y,
			uint tile_offset_x, uint tile_offset_y,
			uint tile_width, uint tile_height) :
	Data(data), Block(0), RowArray(0), Height(height), RowBytes(row_bytes),
	Width(width), Depth(depth), ImageOffsetX(start_x),
	ImageOffsetY(start_y), Valid(true), HasDiskBlock(false),
	Loaded(false), DiskBlock(0), Modified(false),
	TileOffsetX(tile_offset_x), TileOffsetY(tile_offset_y),
	TileWidth(tile_width), TileHeight(tile_height), NeedUpdate(false)
{
	// Here is the new memory stuff
	DataHandle = Data->AddPage(RowBytes * Height);
	// create the row classes associated with this block
	RowArray = new G42ImageRow * [Height];
	if (!RowArray)
	{
		Valid = false;
      return;
	}
   memset(RowArray, 0, Height * sizeof (G42ImageRow *));
	for (int i = 0; i < Height; i++)
	{
		RowArray[i] = new G42ImageRow(this);
		if (!RowArray[i])
		{
			Valid = false;
         return;
		}
	}
	uint ptr = 0;
	for (int i = 0; i < Height; i++)
	{
		#ifdef MSWIN // MSWIN does things upside down
		RowArray[Height - i - 1]->Offset = ptr;
		#else
		RowArray[i]->Offset = ptr;
		#endif
		ptr += RowBytes;
	}
}
// delete the block and the row classes
G42ImageTile::~G42ImageTile()
{
	// New Memory Stuff
	Data->DeletePage(DataHandle);
	if (RowArray)
	{
		for (int i = 0; i < Height; i++)
		{
			delete RowArray[i];
		}
		delete[] RowArray;
	}
}
G42ImageTile::G42ImageTile(G42ImageTile & Ref)
	: Data(Ref.Data), Block(0), RowArray(0), Height(Ref.Height),
	RowBytes(Ref.RowBytes), DataHandle(Ref.DataHandle),
	Width(Ref.Width), Depth(Ref.Depth), ImageOffsetX(Ref.ImageOffsetX),
	ImageOffsetY(Ref.ImageOffsetY), Valid(Ref.Valid), HasDiskBlock(false),
	Loaded(false), DiskBlock(0), Modified(false),
	TileOffsetX(Ref.TileOffsetX), TileOffsetY(Ref.TileOffsetY),
	TileWidth(Ref.TileWidth), TileHeight(Ref.TileHeight),
	NeedUpdate(Ref.NeedUpdate)
{
	Data->IncRefCount(DataHandle);
	RowArray = new G42ImageRow * [Height];
	if (!RowArray)
	{
		Valid = false;
		return;
	}
	memset(RowArray, 0, Height * sizeof (G42ImageRow *));
	for (int i = 0; i < Height; i++)
	{
		RowArray[i] = new G42ImageRow(this);
		if (!RowArray[i])
		{
			Valid = false;
			return;
		}
	}
	uint ptr = 0;
	for (int i = 0; i < Height; i++)
	{
		#ifdef MSWIN // MSWIN does things upside down
		RowArray[Height - i - 1]->Offset = ptr;
		#else
		RowArray[i]->Offset = ptr;
		#endif
		ptr += RowBytes;
	}
}
G42ImageTile::G42ImageTile(G42ImageTile & Ref,
		uint32 start_x, uint32 start_y,
		uint tile_offset_x, uint tile_offset_y,
		uint tile_width, uint tile_height)
	: Data(Ref.Data), Block(0), RowArray(0), Height(Ref.Height),
	RowBytes(Ref.RowBytes), DataHandle(Ref.DataHandle),
	Width(Ref.Width), Depth(Ref.Depth), ImageOffsetX(start_x),
	ImageOffsetY(start_y), Valid(Ref.Valid), HasDiskBlock(false),
	Loaded(false), DiskBlock(0), Modified(false),
	TileOffsetX(tile_offset_x), TileOffsetY(tile_offset_y),
	TileWidth(tile_width), TileHeight(tile_height),
	NeedUpdate(Ref.NeedUpdate)
{
	Data->IncRefCount(DataHandle);
	RowArray = new G42ImageRow * [Height];
	if (!RowArray)
	{
		Valid = false;
		return;
	}
	memset(RowArray, 0, Height * sizeof (G42ImageRow *));
	for (int i = 0; i < Height; i++)
	{
		RowArray[i] = new G42ImageRow(this);
		if (!RowArray[i])
		{
			Valid = false;
			return;
		}
	}
	uint ptr = 0;
	for (int i = 0; i < Height; i++)
	{
		#ifdef MSWIN // MSWIN does things upside down
		RowArray[Height - i - 1]->Offset = ptr;
		#else
		RowArray[i]->Offset = ptr;
		#endif
		ptr += RowBytes;
	}
}
void
G42ImageTile::Unload(void)
{
// new Memory Stuff
	if (!Valid)
		return;
	if (!Loaded)
		return;
	Data->Unlock(DataHandle);
	Loaded = false;
}
void
G42ImageTile::Load(void)
{
// new Memory Stuff
	if (!Valid)
		return;
	if (Loaded)
		return;
	Block = (*Data)[DataHandle];
   Data->Lock(DataHandle);
#if 0
	unsigned char * ptr = Block;
	for (int i = 0; i < Height; i++)
	{
		#ifdef MSWIN // MSWIN does things upside down
		RowArray[Height - i - 1]->Ptr = ptr;
		#else
		RowArray[i]->Ptr = ptr;
		#endif
		ptr += RowBytes;
	}
#endif
	Loaded = true;
	return;
}
G42LockedImageTile::G42LockedImageTile(G42ImageTile * tile)
	: Tile(tile)
{}
G42LockedImageTile::~G42LockedImageTile(void)
{
	Tile->Unload();
}
G42ModifiableImageTile::G42ModifiableImageTile(G42ImageTile * tile)
	: Tile(tile)
{
	int new_data_handle = Tile->GetData()->ForkPage(Tile->GetDataHandle());
	Tile->SetDataHandle(new_data_handle);
	Tile->Load();
}
G42ModifiableImageTile::~G42ModifiableImageTile(void)
{
	Tile->Unload();
}
void
G42ImageData::PrintToDisplay(G42Display, int, int, int, int)
{
#if 0
#ifdef MSWIN
	int pal_space = (NumPalette) ? NumPalette : 1;
	int dib_size = sizeof (BITMAPINFOHEADER) + pal_space * sizeof (RGBQUAD);
	BITMAPINFO * dib_header = (BITMAPINFO *)(new char [dib_size]);
	RGBQUAD * pal_header = (RGBQUAD *)(&(dib_header->bmiColors[0]));
	int row_bytes = ((((Width * Depth + 7) >> 3) + 3) & ~3);
	dib_header->bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
	dib_header->bmiHeader.biWidth = Width;
	dib_header->bmiHeader.biPlanes = 1;
	dib_header->bmiHeader.biBitCount = Depth;
	dib_header->bmiHeader.biCompression = BI_RGB;
	dib_header->bmiHeader.biXPelsPerMeter = 0;
	dib_header->bmiHeader.biYPelsPerMeter = 0;
	dib_header->bmiHeader.biClrUsed = NumPalette;
	dib_header->bmiHeader.biClrImportant = 0;
	for (int pal = 0; pal < NumPalette; pal++)
	{
		pal_header[pal].rgbRed = DataPalette[pal].red;
		pal_header[pal].rgbGreen = DataPalette[pal].green;
		pal_header[pal].rgbBlue = DataPalette[pal].blue;
		pal_header[pal].rgbReserved = 0;
	}
	for (int block = 0; block < NumBlockArray; block++)
	{
		int num_rows;
		if (block == NumBlockArray - 1)
			num_rows = LastBlockHeight;
		else
			num_rows = RowsPerBlock;
		int dest_y = y + (int)(((int32)(block * RowsPerBlock) *
			(int32)height) / (int32)Height);
		int dest_h = y + (int)(((int32)(block * RowsPerBlock + num_rows) *
			(int32)height) / (int32)Height) - dest_y;
		dib_header->bmiHeader.biHeight = num_rows;
		dib_header->bmiHeader.biSizeImage = (DWORD)num_rows * row_bytes;
		StretchDIBits(display,
			x, dest_y, width, dest_h, 0, 0, Width, num_rows,
			BlockArray[block]->GetPtr(), dib_header, DIB_RGB_COLORS, SRCCOPY);
	}
	delete [] (char *)(dib_header);
#endif // MSWIN
#endif // if 0
}
uint
G42ImageData::GetTileRowIndex(uint32 y)
{
	uint index = (uint)((y + (uint32)FirstTileOffsetY) / (uint32)TileHeight);
	if (index >= NumTilesDown)
		index = NumTilesDown - 1;
	return index;
}
uint
G42ImageData::GetTileColumnIndex(uint32 x)
{
	uint index = (uint)((x + (uint32)FirstTileOffsetX) / (uint32)TileWidth);
	if (index >= NumTilesAcross)
		index = NumTilesAcross - 1;
	return index;
}
uint
G42ImageData::GetTileStartX(uint column)
{
	if (column)
		return 0;
	return FirstTileOffsetX;
}
uint
G42ImageData::GetTileStartY(uint row)
{
	if (row)
		return 0;
	return FirstTileOffsetY;
}
uint
G42ImageData::GetTileEndX(uint column)
{
	if (!column && NumTilesAcross == 1)
		return LastTileWidth + FirstTileOffsetX - 1;
	if (column == NumTilesAcross - 1)
		return LastTileWidth - 1;
	return TileWidth - 1;
}
uint
G42ImageData::GetTileEndY(uint row)
{
	if (!row && NumTilesDown == 1)
		return LastTileHeight + FirstTileOffsetY - 1;
	if (row == NumTilesDown - 1)
		return LastTileHeight - 1;
	return TileHeight - 1;
}
uint32
G42ImageData::GetImageStartX(uint column)
{
	uint32 x = 0;
	if (column)
	{
		x = (uint32)column * (uint32)TileWidth - (uint32)FirstTileOffsetX;
		if (x >= Width)
			x = Width - 1;
	}
	return x;
}
uint32
G42ImageData::GetImageStartY(uint row)
{
	uint32 y = 0;
	if (row)
	{
		y = (uint32)row * (uint32)TileHeight -
			(uint32)FirstTileOffsetY;
		if (y >= Height)
			y = Height - 1;
	}
	return y;
}
uint32
G42ImageData::GetImageEndX(uint column)
{
	uint32 x = (uint32)(column + 1) * (uint32)TileWidth -
		(uint32)FirstTileOffsetX - 1;
	if (x >= Width)
		x = Width - 1;
	return x;
}
uint32
G42ImageData::GetImageEndY(uint row)
{
	uint32 y = (uint32)(row + 1) * (uint32)TileHeight -
		(uint32)FirstTileOffsetY - 1;
	if (y >= Height)
		y = Height - 1;
	return y;
}
