// g42pbm.h - pbm version of the raster file base class
#ifndef G42WPBM_H
#define G42WPBM_H
/*************************************************************
	File:          g42pbm.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for bmp's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "g42wibas.h"
class G42PbmWriter : G42RasterWriterBase
{
	public:
		G42PbmWriter(G42RasterImageWriter & parent);
		virtual ~G42PbmWriter();
		virtual bool WriteInfo(G42ImageInfo * info);
		virtual bool WriteRow(const byte * row, const byte * mask);
	private:
		byte * TempRow;
		G42ImageType Type;
		G42ImageInfo::G42ColorType ColorType;
		const G42Color * ImagePalette; // reference only, do not delete
		uint NumPalette;
		uint TrueDepth;
};
#endif // G42PBM_H
