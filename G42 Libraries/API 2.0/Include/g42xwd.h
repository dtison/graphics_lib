// g42pcx.h - pcx version of the raster file base class
#ifndef G42XWD_H
#define G42XWD_H
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
class G42XwdImage : G42RasterBase
{
	public:
		void ProcessData(byte * buffer, size_t buffer_size);
		G42XwdImage(G42RasterImage & parent);
		virtual ~G42XwdImage();
	protected:
		enum G42ReadMode {ReadHeaderMode, ReadImageMode, ReadInfoMode,
			ReadHeaderSizeMode, ReadHasInfoMode, ReadPaletteMode,
			ReadDoneMode, ReadErrorMode};
		G42ReadMode ReadMode;
		uint32 HeaderSize;
		uint Version;
		uint VisualType;
		uint CurrentPalette;
		uint32 CurrentRow;
		void ReadPalette(void);
		void ReadHasInfo(void);
		void ReadHeader(void);
		void ReadImage(void);
		void ReadHeaderSize(void);
		virtual void OutputRow(void);
};
extern bool G42IsXwd(byte * buffer, size_t buffer_size);
#endif // G42XWD_H
