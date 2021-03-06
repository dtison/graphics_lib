// g42pcx.h - pcx version of the raster file base class
#ifndef G42RAS_H
#define G42RAS_H
/*************************************************************
	File:          g42pcx.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for pcx's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "g42ribas.h"
class G42RasImage : G42RasterBase
{
	public:
		void ProcessData(byte * buffer, size_t buffer_size);
		G42RasImage(G42RasterImage & parent);
		virtual ~G42RasImage();
	protected:
		enum G42ReadMode {ReadHeaderMode, ReadImageMode, ReadPaletteMode,
			ReadDoneMode, ReadErrorMode};
		G42ReadMode ReadMode;
		void ReadHeader(void);
		void ReadPalette(void);
		void ReadImage(void);
		virtual void OutputRow(void);
		enum G42DecompressionMode { StartDecompression, GetRunCount, GetRunValue };
		G42DecompressionMode DecompressionMode;
		bool ColorOrderRGB;
		int CurrentPalette;
		int RunCount;
		int CurrentRow;
};
extern bool G42IsRas(byte * buffer, size_t buffer_size);
#endif // G42RAS_H
