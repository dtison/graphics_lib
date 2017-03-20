// g42pcx.h - pcx version of the raster file base class
#ifndef G42TARGA_H
#define G42TARGA_H
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
class G42TargaImage : G42RasterBase
{
	public:
		void ProcessData(byte * buffer, size_t buffer_size);
		G42TargaImage(G42RasterImage & parent);
		virtual ~G42TargaImage();
	protected:
		enum G42ReadMode {ReadHeaderMode, ReadImageMode, ReadPaletteMode,
			ReadIdMode, ReadDoneMode, ReadErrorMode};
		G42ReadMode ReadMode;
		void ReadHeader(void);
		void ReadPalette(void);
		void ReadImage(void);
		void ReadId(void);
		virtual void OutputRow(void);
		enum G42DecompressionMode { StartDecompression, GetRunValue,
			GetSequence };
		G42DecompressionMode DecompressionMode;
		int CurrentPalette;
		int RunCount;
		uint32 CurrentRow;
		int IdLength;
		int ImageType;
		int ColorMapStart;
		int ColorMapLength;
		int ColorMapDepth;
		bool UpsideDown;
		bool Backwards;
		int Length;
		int Offset;
		byte Ref[4];
};
extern bool G42IsTarga(byte * buffer, size_t buffer_size);
#endif // G42TARGA_H
