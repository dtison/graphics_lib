// girpcx.h - pcx version of the raster file base class
#ifndef GIRPCX_H
#define GIRPCX_H
/*************************************************************
	File:          girpcx.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for pcx's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "girbase.h"
#include "gidcomp.h"
#include "gitpcx.h"
class G42ImageReaderPcx : public G42ImageReaderBase
{
	public:
		G42ImageReaderPcx(void);
		G42ImageReaderPcx(G42HeapManager & heap, G42Notifier & notifier,
			G42File * file = 0, G42BaseOptions * options = 0);
		virtual ~G42ImageReaderPcx();
		virtual void ProcessStreamData(void);
		virtual void BeginProcessing(void);
		virtual void SendExtendedInfo(void);
		virtual void ForceFileEnd(void);
		
	protected:
		enum G42ReadMode {ReadHeaderMode, ReadImageMode, ReadInfoMode,
			ReadPaletteMode, CheckTrailerMode, ReadDoneMode, ReadErrorMode};
		G42ReadMode ReadMode;
		void ReadHeader(void);
		void ReadImage(void);
		virtual void ConvertRow(void);
		void ReadPalette(void);
		void CheckTrailer(void);
		uint32 CurrentRow;
		byte * PcxRowBuffer;
		intf CurrentPalette;
		intf PlaneBytes;
		int32 ImagePosition; // to seek back
};
class G42PcxRLEDecompressor : public G42Decompressor
{
public:
	G42PcxRLEDecompressor(G42DecompressorOutput & output, uintf row_bytes);
	virtual ~G42PcxRLEDecompressor(void);
	virtual void ProcessBuffer(void);
private:
	enum G42DecompressionMode { StartDecompression, GetRunValue };
	G42DecompressionMode DecompressionMode;
	uintf RunCount;
};
#endif // GIRPCX_H
