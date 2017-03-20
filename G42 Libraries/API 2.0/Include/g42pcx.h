// g42pcx.h - pcx version of the raster file base class
#ifndef G42PCX_H
#define G42PCX_H
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
class G42PcxImage : G42RasterBase
{
	public:
		void ProcessData(byte * buffer, size_t buffer_size);
		G42PcxImage(G42RasterImage & parent);
		virtual ~G42PcxImage();
	protected:
		enum G42ReadMode {ReadHeaderMode, ReadImageMode, ReadInfoMode,
			ReadPaletteMode, CheckTrailerMode, ReadDoneMode, ReadErrorMode};
		G42ReadMode ReadMode;
		void ReadHeader(void);
		void ReadImage(void);
		virtual void OutputRow(void);
		void ReadPalette(void);
		void CheckTrailer(void);
		uint32 CurrentRow;
		byte * PcxRowBuffer;
		int CurrentPalette;
		int PlaneBytes;
		enum G42DecompressionMode { StartDecompression, GetRunValue };
		G42DecompressionMode DecompressionMode;
		int RunCount;
};
extern bool G42IsPcx(byte * buffer, size_t buffer_size);
#endif // G42PCX_H
