// g42rimag.h - raster file factory class
/*************************************************************
	File:          g42rimag.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   Determine which class a file is and build that
						raster type.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#ifndef G42_RIMAGE_H
#define G42_RIMAGE_H
#include "g42itype.h"
#include "g42iinfo.h"
class G42RasterBase;
class G42RasterImage
{
   public:
		G42RasterImage();
      virtual ~G42RasterImage();
      virtual bool IsValid(void);
		virtual bool IsFinished(void);
		virtual void ProcessData(byte * buffer, size_t buffer_size);
      virtual void ForceFileEnd(void);
		virtual uint32 GetWidth(void);
		virtual uint32 GetHeight(void);
		virtual uint GetDepth(void);
		virtual uint GetNumPalette(void);
      virtual G42Color * GetPalette(void);
		virtual bool IsColor(void);
		virtual bool IsTransparent(void);
		virtual bool TopToBottom(void);
		virtual void HasRow(byte * row, byte * mask = 0);
		virtual void HasInfo(void);
		virtual void HasInfo(G42ImageInfo & info) = 0;
		virtual void HasRow(byte * row, uint32 row_num, uint32 offset,
			uint length, byte * mask = 0) = 0;
		virtual void HasEnd(G42ImageInfo & info) = 0;
		virtual void CombineRow(byte * old_row, byte * new_row, int32 rowbytes);
		virtual void CombineMask(byte * old_row, byte * new_row, int32 rowbytes);
		virtual byte * GetRow(uint32 row) = 0;
		G42ImageType GetImageType(void) const
			{ return Type; }
   protected:
		G42RasterBase *Base;
      enum G42Mode {NotEnoughData, KnownType, UnknownType};
		G42Mode Mode;
		G42ImageType Type;
};
#endif // G42_RIMAGE_H
