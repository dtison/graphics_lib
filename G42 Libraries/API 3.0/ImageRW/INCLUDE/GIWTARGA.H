// g42pcx.h - pcx version of the raster file base class
#ifndef GIWTARGA_H
#define GIWTARGA_H
/*************************************************************
	File:          giwtarga.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for targa's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "giwbase.h"
#include "gittarga.h"
class G42ImageWriterTarga : public G42ImageWriterBase
{
	public:
		G42ImageWriterTarga(void);
		G42ImageWriterTarga(G42Notifier & notifier,
			G42BaseOptions * options = 0);
		virtual ~G42ImageWriterTarga(void);
		virtual bool WriteInfo(G42ImageDescriptor * info);
		virtual bool WriteRow(const byte * row, const byte * mask);
	private:
		void CompressRow(const byte * row);
		bool Compressed;
		byte TrueColorDepth;
		byte * TempRow;
//		G42ImageDescriptor::G42ImageType ColorType;
};
#endif // GIWTARGA_H
