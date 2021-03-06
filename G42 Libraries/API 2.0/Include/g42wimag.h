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
#ifndef G42_WIMAGE_H
#define G42_WIMAGE_H
#include "g42itype.h"
#include "g42iinfo.h"
class G42RasterWriterBase;
class G42RasterImageWriter
{
	public:
		G42RasterImageWriter();
		virtual ~G42RasterImageWriter();
		bool IsValid(void) const;
		bool WriteInfo(G42ImageInfo * info);
		bool WriteRow(const byte * row, const byte * mask);
		uint32 NextRow(void);
		bool NeedRow(void);
		bool WriteEnd(G42ImageInfo * info);
		int GetDepth(void);
		virtual bool Output(const byte * buffer, uint size) = 0;
		virtual void OutputFinished(void) = 0;
	protected:
		bool Valid;
	
	private:
		G42RasterWriterBase * Base;
};
#endif // G42_RIMAGE_H
