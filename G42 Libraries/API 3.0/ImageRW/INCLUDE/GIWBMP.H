// g42bmp.h - bmp version of the raster file base class
#ifndef G42WBMP_H
#define G42WBMP_H
/*************************************************************
	File:          g42bmp.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for bmp's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
      Code   Date   Name and Description
***************************************************************/
#include "giwbase.h"
#include "gitbmp.h"
class G42ImageWriterBmp : public G42ImageWriterBase
{
	public:
		G42ImageWriterBmp(void);
		G42ImageWriterBmp(G42Notifier & notifier,
			G42BaseOptions * options = 0);
		virtual ~G42ImageWriterBmp(void);
		virtual bool WriteInfo(G42ImageDescriptor * info);
		virtual bool WriteRow(const byte * row, const byte * mask);
		virtual uint32 NextRow(void);
		virtual bool NeedRow(void);
	private:
		byte * TempRow;
		int32 CurrentBmpRow;
};
#endif // G42WBMP_H
