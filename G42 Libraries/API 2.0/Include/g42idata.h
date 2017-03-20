// g42idata.h
#ifndef G42IDATA_H
#define G42IDATA_H
#include "g42itype.h"
#include "g42memry.h"
/*************************************************************
	File:          g42idata.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   This class holds the true image data in memory
						or in a swap file, and takes care of reference
                  counting and swapping.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
		jrp	02/20/96		Added a selection of Iterators.
		ges01	04/16/96		Added a few calculation functions into g42idata
***************************************************************/
/*
This class is still in flux, but here is what I've worked out
so far.  The data is held in "blocks", each slightly less then
64K (for MS-Win).  An image is made up of an array of blocks.
However, the data is accessed through an array of row classes
that hold the information to find that particular row.  This
information is currently which block the row resides in, and
where in the block.  All access to the data will be through
these row classes.  The row will swap the block in from disk
if necessary before releasing access to the row pointer.
Each block creates the row classes for the data stored in
itself, and updates the row classes info when it gets swapped
in and out.  When the data class creates the blocks, it
makes copies of the row pointers created with the block, and
forms one large "row array" so it can find the row class for
each row.
*/
// forward declaration
class G42ImageRow;
// The data block is responsible for the data rows contained within
// the block
class G42ImageTile
{
private:
	G42ImageTile & operator= (G42ImageTile & Ref)
		{return *this;}
	protected:
		byte * Block; // the actual data, if loaded
		G42ImageRow ** RowArray; // the array of row classes
		uint32 ImageOffsetX; // where the tile starts in the image
		uint32 ImageOffsetY; // where tile starts in the image
		uint RowBytes; // bytes per row for tile
		bool Loaded; // whether the data block is loaded in memory or not
		bool Modified; // whether the data has been modified
		bool Valid; // true if valid
		bool HasDiskBlock; // true if block has a disk location
		uint DiskBlock; // block in swap file
		uint Width; // width of tile
		uint Height; // height of tile
		uint Depth; // depth of tile
		uint TileOffsetX; // actual x offset to start from
		uint TileOffsetY; // actual y offset to start from
		uint TileWidth; // actual width of data
		uint TileHeight; // actual height of data
		G42Data *	Data;
		uint			DataHandle;
      bool			NeedUpdate; // used for dithered images
	public:
		G42ImageTile(G42Data * data, uint width, uint height, uint depth,
			uint row_bytes, uint32 start_x, uint32 start_y,
			uint tile_offset_x, uint tile_offset_y,
			uint tile_width, uint tile_height);
		~G42ImageTile();
		G42ImageTile(G42ImageTile & Ref);
		G42ImageTile(G42ImageTile & Ref, uint32 start_x, uint32 start_y,
			uint tile_offset_x, uint tile_offset_y,
			uint tile_width, uint tile_height);
		G42ImageRow ** GetRowArray(void) const
			{ return RowArray; }
		G42ImageRow * GetRow(int row) const
			{ if (RowArray) return RowArray[row]; return 0; }
		bool IsLoaded(void) const {
			return Loaded;
		}
		void Load(void);
		void Unload(void);
		bool IsValid(void) const
			{ return Valid; }
		void MarkModified(void)
			{Data->MarkModified(DataHandle);}
		unsigned char * GetPtr(void)
			{ if (!Loaded) Load(); return Block; }
		uint GetWidth() // width of tile
			{ return Width; }
		uint GetHeight() // height of tile
			{ return Height; }
		uint GetDepth() // depth of tile
			{ return Depth; }
		uint GetRowBytes(void)
			{ return RowBytes; }
		uint GetTileOffsetX() // actual x offset to start from
			{ return TileOffsetX; }
		uint GetTileOffsetY() // actual y offset to start from
			{ return TileOffsetY; }
		uint GetTileWidth() // actual width of data
			{ return TileWidth; }
		uint GetTileHeight() // actual height of data
			{ return TileHeight; }
		uint32 GetImageOffsetX() // start of image data in image
			{ return ImageOffsetX; }
		uint32 GetImageOffsetY() // start of image data in image
			{ return ImageOffsetY; }
		G42Data * GetData(void) const
			{return Data;}
		uint GetDataHandle(void) const
			{return DataHandle;}
		void SetDataHandle(uint handle)
			{DataHandle = handle;}
		bool GetNeedUpdate(void)
			{return NeedUpdate;}
		void SetNeedUpdate(bool need_update)
			{NeedUpdate = need_update;}
};
class G42LockedImageTile
{
private:
	G42ImageTile *		Tile;
public:
	G42LockedImageTile(G42ImageTile * tile);
	~G42LockedImageTile(void);
	G42ImageTile * GetTile(void) const
   	{return Tile;}
};
class G42ModifiableImageTile
{
private:
	G42ImageTile *		Tile;
public:
	G42ModifiableImageTile(G42ImageTile * tile);
	~G42ModifiableImageTile(void);
	G42ImageTile * GetTile(void) const
		{return Tile;}
	void MarkModified(void)
   	{Tile->MarkModified();}
};
// an image data row.  Created by the data block, it checks the block
// to make sure it is in memory before releasing the pointer to the
// actual data.  The block's Load() function updates the Ptr variable
class G42ImageRow
{
	friend class G42ImageTile;
	friend class G42ImageRow24BitIterator;
	protected:
		G42ImageTile * Tile; // the tile that the row belongs in
		unsigned char * Ptr; // a pointer to the row data, only valid if
									// the block is loaded.
      uint				 Offset;
		// only a Block can create and destroy these
		G42ImageRow(G42ImageTile * tile) : Tile(tile) {}
		~G42ImageRow() {}
	public:
		// check to make sure the Ptr is valid before returning it
		operator unsigned char * () {
			if (!(Tile->IsLoaded()))
				Tile->Load();
         return (Tile->GetPtr() + Offset);
			//return Ptr;
		}
		void MarkModified(void) {Tile->MarkModified();}
};
// Holds the data and pixels for an image using the classes above.
// At this level, this is a rather easy class.  The tile class
// does all the nasty work
class G42ImageData
{
	friend class	G42ImageData24BitIterator;
	friend class	G42ImageData8BitIterator;
	friend class	G42ImageData4BitIterator;
	friend class	G42ImageData1BitIterator;
	G42ImageData & operator= (G42ImageData & Ref)
		{return *this;}
	protected:
		uint32 Width;
		uint32 Height;
		uint32 NumTiles;
		uint NumTilesAcross;
		uint NumTilesDown;
		uint Depth;
		uint NumPalette;
		uint TileWidth;
		uint TileHeight;
		uint TileRowBytes;
		uint TileSize;
		uint LastTileHeight;
		uint LastTileWidth;
		uint FirstTileOffsetX;
		uint FirstTileOffsetY;
		uint32 ValidStartRow;
		uint32 ValidNumRows;
		bool Finished;
		bool Valid;
		G42Color * DataPalette;
		G42ImageTile *** TileArray;
		G42Data *	Data;
		bool Partial;
		uint PartialTileStartX;
		uint PartialTileWidth;
		uint PartialTileStartY;
		uint PartialTileHeight;
		uint OldPartialTileStartX;
		uint OldPartialTileWidth;
		uint OldPartialTileStartY;
		uint OldPartialTileHeight;
	public:
		G42ImageData(G42Data * data, uint32 width, uint32 height, uint depth,
			uint num_palette, bool partial = false);
		~G42ImageData();
		G42ImageData(G42ImageData & Ref);
		G42ImageData(G42ImageData & Ref, uint32 x, uint32 y, uint32 width,
			uint32 height);
		G42Data * GetData(void) const
      	{return Data;}
		G42ImageTile * GetTile(uint row, uint column)
			{ if (TileArray && row >= PartialTileStartY &&
				row < PartialTileStartY + PartialTileHeight &&
				column >= PartialTileStartX &&
				column < PartialTileStartX + PartialTileWidth)
					return (TileArray[row - PartialTileStartY])
				[column - PartialTileStartX]; return 0; }
		G42Color * GetPalette(void)
			{ return DataPalette; }
		void SetPalette(G42Color * data_palette);
      void DeletePalette(void);
		uint32 GetWidth(void)
			{ return Width; }
		uint32 GetHeight(void)
			{ return Height; }
		uint32 GetNumTiles(void)
      	{ return NumTiles; }
		uint GetNumTilesDown(void)
			{ return NumTilesDown; }
		uint GetNumTilesAcross(void)
			{ return NumTilesAcross; }
		uint GetTileWidth(void)
			{ return TileWidth; }
		uint GetTileHeight(void)
			{ return TileHeight; }
		uint GetTileRowBytes(void)
			{ return TileRowBytes; }
		uint GetLastTileHeight(void)
			{ return LastTileHeight; }
		uint GetLastTileWidth(void)
			{ return LastTileWidth; }
   	uint GetFirstTileOffsetX(void)
			{ return FirstTileOffsetX; }
		uint GetFirstTileOffsetY(void)
			{ return FirstTileOffsetY; }
      uint GetTileRowIndex(uint32 y);
      uint GetTileColumnIndex(uint32 x);
		uint GetDepth(void)
			{ return Depth; }
		uint GetNumPalette(void)
			{ return NumPalette; }
		uint32 GetValidStartRow(void)
			{ return ValidStartRow; }
		uint32 GetValidNumRows(void)
			{ return ValidNumRows; }
		bool IsFinished(void)
			{ return Finished; }
		void MarkModified(uint row, uint column)
			{ G42ImageTile * tile = GetTile(row, column);
				if (tile) tile->MarkModified(); }
		void SetValidStartRow(int start)
			{ ValidStartRow = start; }
		void SetValidNumRows(int num)
			{ ValidNumRows = num; }
		void SetFinished(bool finished)
			{ Finished = finished; }
		void PrintToDisplay(G42Display display, int x, int y,
			int width, int height);
      void SetNumPalette(uint num_palette)
      	{NumPalette = num_palette;}
		// these next functions are common calculations that need to be
		// done to use the tiles - ges01
		uint GetTileStartX(uint row);
		uint GetTileStartY(uint column);
		uint GetTileEndX(uint row); // last pixel in tile, not one past
		uint GetTileEndY(uint column); // last pixel in tile, not one past
		uint32 GetImageStartX(uint row);
		uint32 GetImageStartY(uint column);
		uint32 GetImageEndX(uint row); // last pixel in tile, not one past
		uint32 GetImageEndY(uint column); // last pixel in tile, not one past
		// end ges01
		void SetPartial(uint x, uint y, uint width, uint height);
		bool NeedPartialUpdate(void);
		uint GetPartialUpdateTileX(void);
		uint GetPartialUpdateTileY(void);
		uint GetPartialUpdateTileWidth(void);
		uint GetPartialUpdateTileHeight(void);
		void UpdatePartial(void);
		uint ReferenceCount;
};
#endif // G42IDATA_H
