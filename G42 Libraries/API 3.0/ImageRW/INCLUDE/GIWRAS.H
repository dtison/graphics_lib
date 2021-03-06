// giwras.h - ras version of the raster file base class
#ifndef GIWRAS_H
#define GIWRAS_H
/*************************************************************
	File:          giwras.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for ras's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "giwbase.h"
#include "gitras.h"
class G42ImageWriterRas : public G42ImageWriterBase
{
	public:
		G42ImageWriterRas(void);
		G42ImageWriterRas(G42Notifier & notifier,
			G42BaseOptions * options = 0);
		virtual ~G42ImageWriterRas(void);
		virtual bool WriteInfo(G42ImageDescriptor * info);
		virtual bool WriteRow(const byte * row, const byte * mask);
	private:
		void CompressRow(const byte * row);
		bool Compressed;
		intf TrueDepth;
		byte * TempRow;
};
#endif // G42RAS_H
