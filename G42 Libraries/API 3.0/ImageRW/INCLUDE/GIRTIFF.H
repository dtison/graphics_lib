// girtiff.h - tiff version of the raster file base class
#ifndef GIRTIFF_H
#define GIRTIFF_H
/*************************************************************
	File:          girtiff.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for tiff's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "girbase.h"
#include "giteps.h"
#include "gittiff.h"
class G42ImageReaderTiffBase : public G42ImageReaderBase
{
	public:
		G42ImageReaderTiffBase(void);
		G42ImageReaderTiffBase(G42HeapManager & heap, G42Notifier & notifier,
			G42File * file = 0, G42BaseOptions * options = 0);
		virtual ~G42ImageReaderTiffBase();
		virtual void ProcessStreamData(void);
		virtual void BeginProcessing(void);
		virtual void SendExtendedInfo(void);
		bool ShouldAbort(void) const // what?
			{return (ReadMode != ReadImageRowsMode);}
		virtual void ForceFileEnd(void); // force image to be finished
		virtual bool DecompressorOutputRow(byte * row);
	protected:
		intf LastStoreBlock;
		intf MaxStoreBlock;
//		G42ImageTile ** StoreBlockArray;
		byte ** StoreBlockArray;
      uint16 StoreBufferSize;
		uint16 StoreBufferNext;
		uint16 StoreBufferLeft;
		uint32 TotalFileRead;
		uint32 WaitOffset;
		uint32 IFDOffset;
		uint32 StripOffset;
      uint32 TiffSkip;
		uint32 ArrayOffset;
      uint32 StripBytesLeft;
		uint32 * StripSizeArray;
		uint32 * StripArray;
		intf StripRow;
		intf CurrentRow;
		uintf Tag;
		uintf Type;
		uintf Count;
		uintf RowsPerStrip;
		bool NeedMoreData;
		intf NumIFDEntries;
		intf CurIFDEntry;
		intf BitsPerSample;
		intf Compression;
		intf ColorType;
		intf FillOrder;
		intf SamplesPerPixel;
		intf T4Options;
		intf T6Options;
		intf Predictor;
		intf ExtraSamples;
		intf CurrentStrip;
		intf TrueRowBytes;
		intf InternalRowBytes;
		intf TiffDepth;
		byte * TiffRowBuffer;
		size_t OrigBufferSize;
		const byte * OrigBuffer;
		G42APtr<G42Color> TiffPalette;
		enum
		{
			TagImageWidth = 256,
			TagImageLength = 257,
			TagBitsPerSample = 258,
			TagCompression = 259,
			TagPhotometricInterpretation = 262,
			TagFillOrder = 266,
			TagStripOffsets = 273,
			TagSamplesPerPixel = 277,
			TagRowsPerStrip = 278,
			TagStripByteCounts = 279,
			TagT4Options = 292,
			TagT6Options = 293,
			TagPredictor = 317,
			TagColorMap = 320
		};
		enum
		{
			TypeByte = 1,
			TypeAscii = 2,
			TypeShort = 3,
			TypeLong = 4,
			TypeRational = 5,
			TypeSByte = 6,
			TypeUndefined = 7,
			TypeSShort = 8,
			TypeSLong = 9,
			TypeSRational = 10,
			TypeFloat = 11,
			TypeDouble = 12
		};
		enum
		{
			ColorTypeWhiteIsZero = 0,
			ColorTypeBlackIsZero = 1,
			ColorTypeRGB = 2,
			ColorTypePalette = 3
		};
		enum
		{
			NoCompression = 1,
			CCITT = 2,
			Group3 = 3,
			Group4 = 4,
			LZW = 5,
			PackBits = 32773
		};
		enum
		{
			PredictionNone = 1,
			PredictionHorizontal = 2
		};
		enum G42ReadMode {ReadHeaderMode, ReadImageRowsMode, ReadImageStripMode,
			ReadWaitMode, ReadIFDMode, ReadIFDEntryMode, ReadStripOffsetsMode,
			ReadBitsPerSampleMode, ReadStripByteCountsMode, ReadColorMapMode,
         ReadTiffHeaderMode,
			ReadDoneMode, ReadErrorMode};
		G42ReadMode ReadMode;
		G42ReadMode WaitMode;
		virtual void ConvertRow(void);
		void ReadHeader(void);
		void ReadTiffHeader(void);
		void ReadIFD(void);
		void ReadIFDEntry(void);
		void ReadStripOffsets(void);
		void ReadColorMap(void);
		void ReadStripByteCounts(void);
		void ReadBitsPerSample(void);
		int32 GetTiffValue(void);
		intf GetValueSize(void);
		bool NeedTiffPtr(void);
		void FinishIFDEntry(void);
		void ReadImageStrip(void);
		void ReadImageRows(void);
		bool LoadBuffer(uint32 offset, uint32 buffer_size);
		void AddBuffer(const byte * buffer, uint32 size);
		void ClearBuffer(void);
		void SetupDecompressor(void);
};
class G42ImageReaderTiff : public G42ImageReaderTiffBase
{
public:
	G42ImageReaderTiff(void);
	G42ImageReaderTiff(G42HeapManager & heap, G42Notifier & notifier,
		G42File * file = 0, G42BaseOptions * options = 0);
	virtual ~G42ImageReaderTiff();
	virtual bool TestType(void);
	virtual bool TestType(const byte * buffer, uint32 buffer_size);
};
class G42ImageReaderEps : public G42ImageReaderTiffBase
{
public:
	G42ImageReaderEps(void);
	G42ImageReaderEps(G42HeapManager & heap, G42Notifier & notifier,
		G42File * file = 0, G42BaseOptions * options = 0);
	virtual ~G42ImageReaderEps();
};
#endif // GIRTIFF_H
