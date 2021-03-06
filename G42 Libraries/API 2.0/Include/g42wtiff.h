// g42tiff.h - tiff version of the raster file base class
#ifndef G42WTIFF_H
#define G42WTIFF_H
/*************************************************************
	File:          g42tiff.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for tiff's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "g42wibas.h"
#include "g42wcomp.h"
#include "g42idata.h"
class G42TiffCompressOutput;
class G42TiffWriter : G42RasterWriterBase
{
	public:
		G42TiffWriter(G42RasterImageWriter & parent);
		virtual ~G42TiffWriter();
		virtual bool WriteInfo(G42ImageInfo * info);
		virtual bool WriteRow(const byte * row, const byte * mask);
		virtual uint32 NextRow(void);
		virtual bool NeedRow(void);
		void HandleOutputBuffer(byte * buffer, uint length);
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
		G42TiffCompressOutput * CompressOutput;
		G42WriteCompression * Compress;
		uint32 TiffCurrentRow;
		G42ImageInfo * Info;
		void WriteTag(uint16 tag, uint16 type,
			uint32 length, uint32 offset);
		void CreateCompress(void);
		void WriteTags(void);
};
class G42TiffCompressOutput : public G42WriteCompressionOutput
{
	public:
		G42TiffCompressOutput(G42TiffWriter * parent);
		virtual ~G42TiffCompressOutput();
		virtual void OutputBuffer(byte * buffer, uint length);
	private:
		G42TiffWriter * Parent;
};
#endif // G42WTIFF_H
