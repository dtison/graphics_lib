
#if !defined(G42IMAGE_H)
#define G42IMAGE_H
#include "g42idata.h"
#include "g42iinfo.h"
#include "g42memry.h"
#include "g42opal.h"
#include "g42event.h"
class G42ImageViewer;
class G42Image
{
public:
	G42Image(G42Data * data_core, G42EventUpdateHandler * event_handler = 0);
	~G42Image();
	G42Image(G42Image & Ref);
	G42Image(G42Image & Ref, uint32 x, uint32 y, uint32 width, uint32 height);
	G42Image & operator= (G42Image & Ref);
	G42Image(G42Data * data_core, const G42ImageInfo * info,
		bool partial = false, G42EventUpdateHandler * event_handler = 0);
	void SetPartial(uint x, uint y, uint width, uint height);
	bool NeedPartialUpdate(void);
	uint GetPartialUpdateTileX(void);
	uint GetPartialUpdateTileY(void);
	uint GetPartialUpdateTileWidth(void);
	uint GetPartialUpdateTileHeight(void);
	void UpdatePartial(void);
	void AddView(G42ImageViewer * view);
	void RemoveView(G42ImageViewer * view);
	void SetInfo(G42ImageInfo & info);
	void MarkRowValid(uint32 row);
	byte * GetRow(uint32 row_num, uint32 offset, uint length,
		int depth = 0); // memcpy's tiles into row - note: slowish
	byte * GetMaskRow(uint32 row_num, uint32 offset, uint length); // memcpy's tiles into row - note: slowish
	void SetRow(byte * row, uint32 row_num, uint32 offset, // rows in any order
		uint length); // offset and length in pixels
	void SetMaskRow(byte * row, uint32 row_num, uint32 offset, // rows in any order
		uint length); // offset and length in pixels
	void EndImage(G42ImageInfo & info);
	G42ImageData * GetData(void)
		{ return Data; }
	G42ImageData * GetMask(void)
		{ return Mask; }
	G42Data * GetMemoryManager(void)
		{ return DataCore; }
	bool IsValid(void) const
		{ return Valid; }
	bool HasInfo(void) const
   	{ return (Info != 0); }
	const G42ImageInfo & GetInfo(void) const
		{ return *Info; }
   bool IsDirty(void) const
		{ return Dirty; }
	void SetDirty(bool dirty = true)
		{ Dirty = dirty; }
	bool IsFinished(void) const
		{ if (Data) return Data->IsFinished(); return false; }
	uint32 GetValidStartRow(void) const
		{ if (Data) return Data->GetValidStartRow(); return 0; }
	uint32 GetValidNumRows(void) const
		{ if (Data) return Data->GetValidNumRows(); return 0; }
	uint32 GetNumTiles(void) const
		{ if (Data) return Data->GetNumTiles(); return 0; }
	uint GetNumTilesAcross(void) const
		{ if (Data) return Data->GetNumTilesAcross(); return 0; }
	uint GetNumTilesDown(void) const
		{ if (Data) return Data->GetNumTilesDown(); return 0; }
	uint GetTileWidth(void) const
		{ if (Data) return Data->GetTileWidth(); return 0; }
	uint GetTileHeight(void) const
		{ if (Data) return Data->GetTileHeight(); return 0; }
	uint GetTileRowBytes(void) const
		{ if (Data) return Data->GetTileRowBytes(); return 0; }
	uint GetFirstTileOffsetX(void) const
		{ if (Data) return Data->GetFirstTileOffsetX(); return 0; }
	uint GetFirstTileOffsetY(void) const
		{ if (Data) return Data->GetFirstTileOffsetY(); return 0; }
	uint GetLastTileWidth(void) const
		{ if (Data) return Data->GetLastTileWidth(); return 0; }
	uint GetLastTileHeight(void) const
		{ if (Data) return Data->GetLastTileHeight(); return 0; }
	G42ImageTile * GetTile(uint row, uint column) const
		{ if (Data) return Data->GetTile(row, column); return 0; }
	uint GetTileRowIndex(uint32 y)
		{ if (Data) return Data->GetTileRowIndex(y); return 0; }
	uint GetTileColumnIndex(uint32 x)
		{ if (Data) return Data->GetTileColumnIndex(x); return 0; }
	uint GetTileStartX(uint row)
		{ if (Data) return Data->GetTileStartX(row); return 0; }
	uint GetTileStartY(uint column)
		{ if (Data) return Data->GetTileStartY(column); return 0; }
	uint GetTileEndX(uint row) // last pixel in tile, not one past
		{ if (Data) return Data->GetTileEndX(row); return 0; }
	uint GetTileEndY(uint column) // last pixel in tile, not one past
		{ if (Data) return Data->GetTileEndY(column); return 0; }
	uint32 GetImageStartX(uint row)
		{ if (Data) return Data->GetImageStartX(row); return 0; }
	uint32 GetImageStartY(uint column)
		{ if (Data) return Data->GetImageStartY(column); return 0; }
	uint32 GetImageEndX(uint row) // last pixel in tile, not one past
		{ if (Data) return Data->GetImageEndX(row); return 0; }
	uint32 GetImageEndY(uint column) // last pixel in tile, not one past
		{ if (Data) return Data->GetImageEndY(column); return 0; }
	void RebuildInfo(void);
	void Invert(void);
	void SwapRB(void);
	void Equalize(void);
	void ColorAdjust(int bright, int gamma, int contrast, int channel);
	void ColorReduce(int method, G42PaletteType pal_type, int color_cnt);
	void ColorReduce(G42OptimizedPalette * pal, bool match = false);
	void ResizeImage(int width, int height);
	void Emboss(int angle);
	void Sharpness(int sharp);
	void RemoveNoise(int noise);
   void MirrorX(void);
	void MirrorY(void);
   void Rotate(int angle);
   void CheckColorCount(void);
   void CombineImage(G42Image * image, int32 x, int32 y);
   void AddBorder(G42Color color, int width);
	void Buttonize(G42Color top_color, G42Color bottom_color, G42ButtonType type,
   	int size, bool has_color);
	void FloodFill(G42Color top_color, G42Color middle_color, G42Color bottom_color,
   	int width, int rows,	int columns);
   void DrawColorMask(byte * mask, int width, int height, G42Color color,
   	int x, int y);
   void DrawColorMask8(byte * mask, int width, int height, G42Color color,
   	int x, int y);
	void SetInfoFileSize(uint32 size)
		{ if (Info) Info->FileSize = size; }
	void SetInfoModifiedTime(time_t modified_time)
		{ if (Info) Info->ModifiedTime = modified_time; }
	void SetImageType(G42ImageType type)
		{ if (Info) Info->ImageType = type; }
	void SetProgressive(bool progressive)
		{ if (Info) Info->Progressive = progressive; }
	void SetCompression(int compression)
		{ if (Info) Info->Compressed = compression; }
	void SetJpegQuality(byte quality)
		{ if (Info) Info->JpegQuality = quality; }
	void SetJpegSubSampling(bool sub_sampling)
		{ if (Info) Info->JpegSubSampling = sub_sampling; }
	void SetTargaDepth(int depth)
		{ if (Info) Info->TargaDepth = depth; }
	void SetTransparency(G42Color color);
	G42Color GetColorAtPoint(uint32 x, uint32 y);
	void ClearTransparency(void);
	void PreLoad(uint * index_array, uint size)
		{DataCore->PreLoad(index_array, size);}
   bool OwnsEventHandler(void) const
   	{return OwnEventHandler;}
   G42EventUpdateHandler * GetEventHandler(void) const
   	{return EventHandler;}
protected:
	G42Array<G42ImageViewer *> ViewArray;
	int NumViewArray;
	G42ImageData * Data;
	G42ImageData * Mask; // for simple transparency, a kludge
	G42ImageInfo * Info;
	G42Data *		DataCore;
	G42EventUpdateHandler *		EventHandler;
   bool								OwnEventHandler;
	byte * GetRowBuffer;
	uint32 GetRowBufferSize;
	byte * GetMaskRowBuffer;
	uint32 GetMaskRowBufferSize;
	bool Valid;
   bool Dirty;
};
#endif G42IMAGE_H
