// giwtiff.h - tiff version of the raster file base class
#ifndef GIWTIFF_H
#define GIWTIFF_H
/*************************************************************
	File:          giwtiff.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for tiff's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "giwbase.h"
#include "gittiff.h"
#include "gicomp.h"
class G42ImageWriterTiff : public G42ImageWriterBase, public G42CompressorOutput
{
	public:
		G42ImageWriterTiff(void);
		G42ImageWriterTiff(G42Notifier & notifier,
			G42BaseOptions * options = 0);
		virtual ~G42ImageWriterTiff(void);
		virtual bool WriteInfo(G42ImageDescriptor * info);
		virtual bool WriteRow(const byte * row, const byte * mask);
		virtual uint32 NextRow(void);
		virtual bool NeedRow(void);
		virtual void CompressorOutputBuffer(byte * buffer, uintf length);
	private:
		byte * TempRow;
		int32 Compression;
		enum
		{
			TagNewSubFile = 254,
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
			TagPlanarConfiguration = 284,
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
			CCITTCompression = 2,
			Group3Compression = 3,
			Group4Compression = 4,
			LZWCompression = 5,
			PackBitsCompression = 32773
		};
		enum
		{
			PredictionNone = 1,
			PredictionHorizontal = 2
		};
		uint32 ImageSize;
		enum G42WriteMode { Count, Write };
		G42WriteMode WriteMode;
		G42Compressor * Compress;
		uint32 TiffCurrentRow;
		G42ImageDescriptor * Info;
		void WriteTag(uint16 tag, uint16 type,
			uint32 length, uint32 offset);
		void CreateCompress(void);
		void WriteTags(void);
};
#endif // GIWTIFF_H
