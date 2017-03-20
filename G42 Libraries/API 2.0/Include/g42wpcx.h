// g42pcx.h - pcx version of the raster file base class
#ifndef G42WPCX_H
#define G42WPCX_H
/*************************************************************
	File:          g42pcx.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for pcx's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "g42wibas.h"
class G42PcxWriter : G42RasterWriterBase
{
	public:
		G42PcxWriter(G42RasterImageWriter & parent);
		virtual ~G42PcxWriter();
		virtual bool WriteInfo(G42ImageInfo * info);
		virtual bool WriteRow(const byte * row, const byte * mask);
		virtual bool WriteEnd(G42ImageInfo * info);
	private:
		void CompressRow(const byte * row);
		byte * TempRow;
};
#endif // G42PCX_H
