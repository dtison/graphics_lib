// g42pcx.h - pcx version of the raster file base class
#ifndef G42WTGA_H
#define G42WTGA_H
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
class G42TargaWriter : G42RasterWriterBase
{
	public:
		G42TargaWriter(G42RasterImageWriter & parent);
		virtual ~G42TargaWriter();
		virtual bool WriteInfo(G42ImageInfo * info);
		virtual bool WriteRow(const byte * row, const byte * mask);
	private:
		void CompressRow(const byte * row);
		bool Compressed;
		byte * TempRow;
		G42ImageInfo::G42ColorType ColorType;
};
#endif // G42TARGA_H
