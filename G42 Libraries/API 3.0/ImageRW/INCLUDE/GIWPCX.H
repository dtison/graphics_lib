// g42pcx.h - pcx version of the raster file base class
#ifndef GIWPCX_H
#define GIWPCX_H
/*************************************************************
	File:          g42pcx.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for pcx's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "giwbase.h"
#include "gitpcx.h"
class G42ImageWriterPcx : public G42ImageWriterBase
{
	public:
		G42ImageWriterPcx(void);
		G42ImageWriterPcx(G42Notifier & notifier,
			G42BaseOptions * options = 0);
		virtual ~G42ImageWriterPcx();
		virtual bool WriteInfo(G42ImageDescriptor * info);
		virtual bool WriteRow(const byte * row, const byte * mask);
		virtual bool WriteEnd(void);
	private:
		void CompressRow(const byte * row);
		byte * TempRow;
};
#endif // G42PCX_H
