// girras.h - sun raster (ras) version of the raster file base class
#ifndef GIRRAS_H
#define GIRRAS_H
/*************************************************************
	File:          girras.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for ras's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "girbase.h"
#include "gidcomp.h"
#include "gitras.h"
class G42ImageReaderRas : public G42ImageReaderBase
{
	public:
		G42ImageReaderRas(void);
		G42ImageReaderRas(G42HeapManager & heap, G42Notifier & notifier,
			G42File * file = 0, G42BaseOptions * options = 0);
		virtual ~G42ImageReaderRas();
		virtual void ProcessStreamData(void);
		virtual void BeginProcessing(void);
		virtual void SendExtendedInfo(void);
		virtual bool TestType(void);
		virtual bool TestType(const byte * buffer, uint32 buffer_size);
	protected:
		enum G42ReadMode {ReadHeaderMode, ReadImageMode, ReadPaletteMode,
			ReadDoneMode, ReadErrorMode};
		G42ReadMode ReadMode;
		void ReadHeader(void);
		void ReadPalette(void);
		void ReadImage(void);
		virtual void ConvertRow(void);
		bool ColorOrderRGB;
		intf CurrentPalette;
};
class G42RasRLEDecompressor : public G42Decompressor
{
public:
	G42RasRLEDecompressor(G42DecompressorOutput & output, uintf row_bytes);
	virtual ~G42RasRLEDecompressor(void);
	virtual void ProcessBuffer(void);
private:
	enum G42DecompressionMode { StartDecompression, GetRunCount, GetRunValue };
	G42DecompressionMode DecompressionMode;
	uintf RunCount;
};
#endif // GIRRAS_H
