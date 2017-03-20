// girtarga.h - pcx version of the raster file base class
#ifndef GIRTARGA_H
#define GIRTARGA_H
/*************************************************************
	File:          girtarga.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for targas derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "girbase.h"
#include "gidcomp.h"
#include "gittarga.h"
class G42ImageReaderTarga : public G42ImageReaderBase
{
	public:
		G42ImageReaderTarga(void);
		G42ImageReaderTarga(G42HeapManager & heap, G42Notifier & notifier,
			G42File * file = 0, G42BaseOptions * options = 0);
		virtual ~G42ImageReaderTarga();
		virtual void ProcessStreamData(void);
		virtual void BeginProcessing(void);
		virtual void SendExtendedInfo(void);
		virtual bool TestType(void);
		virtual bool TestType(const byte * buffer, uint32 buffer_size);
	protected:
		enum G42ReadMode {ReadHeaderMode, ReadImageMode, ReadPaletteMode,
			ReadIdMode, ReadDoneMode, ReadErrorMode};
		G42ReadMode ReadMode;
		void ReadHeader(void);
		void ReadPalette(void);
		void ReadImage(void);
		void ReadId(void);
		virtual void ConvertRow(void);
		intf CurrentPalette;
		uint32 CurrentRow;
		intf IdLength;
		intf ImageType;
		intf ColorMapStart;
		intf ColorMapLength;
		intf ColorMapDepth;
		bool UpsideDown;
		bool Backwards;
};
class G42TargaRLEDecompressor : public G42Decompressor
{
public:
	G42TargaRLEDecompressor(G42DecompressorOutput & output, uintf row_bytes,
		uintf depth);
	virtual ~G42TargaRLEDecompressor(void);
	virtual void ProcessBuffer(void);
private:
	enum G42DecompressionMode { StartDecompression, GetRunValue,
		GetSequence };
	G42DecompressionMode DecompressionMode;
	uintf RunCount;
	uintf Length;
	uintf Offset;
	byte Ref[4];
	uintf BytesPerPixel;
};
#endif // GIRTARGA_H
