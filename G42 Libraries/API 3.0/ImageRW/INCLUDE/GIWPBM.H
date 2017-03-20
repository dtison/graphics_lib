// giwpbm.h - pbm version of the raster file base class
#ifndef GIWPBM_H
#define GIWPBM_H
/*************************************************************
	File:          giwpbm.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for pbm's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "giwbase.h"
#include "gitpbm.h"
class G42ImageWriterPbm : public G42ImageWriterBase
{
	public:
		G42ImageWriterPbm(void);
		G42ImageWriterPbm(G42Notifier & notifier,
			G42BaseOptions * options = 0);
		virtual ~G42ImageWriterPbm(void);
		virtual bool WriteInfo(G42ImageDescriptor * info);
		virtual bool WriteRow(const byte * row, const byte * mask);
	private:
		byte * TempRow;
		G42PbmType Type;
		const G42Color * ImagePalette; // reference only, do not delete
		uintf NumPalette;
		uintf TrueDepth;
};
#endif // GIWPBM_H
