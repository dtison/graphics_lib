// g42tiff.cpp - Tiff File
/*************************************************************
	File:          g42tiff.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of g42ribase that handles TIFF files
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "g42itype.h"
#include "g42wtiff.h"
#include "g42iter.h"
#include "g42wlzw.h"
#include "g42wpbts.h"
#include "g42wcctt.h"
G42TiffWriter::G42TiffWriter(G42RasterImageWriter & parent) :
	G42RasterWriterBase(parent), TempRow(0), Compression(0),
	Compress(0), CompressOutput(0), ImageSize(0), WriteMode(Count),
	TiffCurrentRow(0), Info(0)
{
}
G42TiffWriter::~G42TiffWriter()
{
	if (Compress)
	{
		delete Compress;
		Compress = 0;
	}
	if (CompressOutput)
	{
		delete CompressOutput;
		CompressOutput = 0;
	}
	if (Info)
	{
		delete Info;
		Info = 0;
	}
	if (TempRow)
	{
		delete [] TempRow;
		TempRow = 0;
	}
	Valid = false;
}
void
G42TiffWriter::WriteTag(uint16 tag, uint16 type,
	uint32 length, uint32 offset)
{
	PutUInt16(tag);
	PutUInt16(type);
	PutUInt32(length);
	PutUInt32(offset);
}
bool
G42TiffWriter::WriteInfo(G42ImageInfo * info)
{
	G42RasterWriterBase::WriteInfo(info); // set up base stuff
	if (Depth > 8)
	{
		Depth = 24;
		RowBytes = Width * 3;
	}
	TempRow = new byte [(size_t)RowBytes];
	if (!TempRow)
	{
		Valid = false;
		return false;
	}
	Info = new G42ImageInfo(*info);
	switch (info->Compressed)
	{
		case CCITT:
			Compression = CCITTCompression;
			break;
		case Group3:
			Compression = Group3Compression;
			break;
		case Group4:
			Compression = Group4Compression;
			break;
		case LZW:
			Compression = LZWCompression;
			break;
		case RLE:
			Compression = PackBitsCompression;
			break;
		default:
			Compression = NoCompression;
			break;
	}
	IntegerFormat = Intel;
	PutUInt16(0x4949);
	PutUInt16(42);
	PutUInt32(8);
	if (Compression == NoCompression)
	{
		ImageSize = RowBytes * Height;
		WriteTags();
		WriteMode = Write;
	}
	else
	{
		WriteMode = Count;
		ImageSize = 0;
	}
	CompressOutput = new G42TiffCompressOutput(this);
	CreateCompress();
	return Valid;
}
void
G42TiffWriter::CreateCompress(void)
{
	if (Compress)
	{
		delete Compress;
		Compress = 0;
	}
	switch (Compression)
	{
		case NoCompression:
			Compress = new G42WriteNoCompression(CompressOutput);
			break;
		case PackBitsCompression:
			Compress = new G42WritePackBits(CompressOutput);
			break;
		case LZWCompression:
			Compress = new G42LzwOutput(CompressOutput);
			break;
		case CCITTCompression:
		case Group3Compression:
		case Group4Compression:
			Compress = new G42CcittOutput(CompressOutput, Compression, Width);
			break;
		default:
			Valid = false;
			break;
	}
}
void
G42TiffWriter::WriteTags(void)
{
	int num_tags;
	if (Depth == 1)
	{
		if (Compression == Group3Compression || Compression == Group4Compression)
			num_tags = 13;
		else
			num_tags = 11;
	}
	else if (Depth <= 8)
		num_tags = 12;
	else
		num_tags = 11;
	int data_start = (8 + 2 + 12 * num_tags + 4);
	if (Depth > 8)
		data_start += 6;
	if (Depth > 1 && Depth <= 4)
		data_start += 96;
	if (Depth > 4 && Depth <= 8)
		data_start += 1536;
	PutUInt16((uint16)num_tags);
	/* write the tags */
	WriteTag(TagNewSubFile, TypeShort, 1L, 0L);
	WriteTag(TagImageWidth, TypeShort, 1L, Width);
	WriteTag(TagImageLength, TypeShort, 1L, Height);
	if (Depth == 1)
	{
		WriteTag(TagBitsPerSample, TypeShort, 1L, 1L);
		WriteTag(TagCompression, TypeShort, 1L, Compression);
		if (Compression >= CCITTCompression && Compression <= Group4Compression)
			WriteTag(TagPhotometricInterpretation, TypeShort, 1L, 0L);
		else
			WriteTag(TagPhotometricInterpretation, TypeShort, 1L, 1L);
		if (Compression == Group3Compression || Compression == Group4Compression)
			WriteTag(TagFillOrder, TypeShort, 1L, 1L);
		WriteTag(TagStripOffsets, TypeLong, 1L, data_start);
		WriteTag(TagSamplesPerPixel, TypeShort, 1L, 1L);
		WriteTag(TagRowsPerStrip, TypeLong, 1L, Height);
		WriteTag(TagStripByteCounts, TypeLong, 1L, ImageSize);
		WriteTag(TagPlanarConfiguration, TypeShort, 1L, 1L);
		if (Compression == Group3Compression || Compression == Group4Compression)
			WriteTag(TagT4Options, TypeLong, 1L, 4L);
	}
	else if (Depth == 4)
	{
		WriteTag(TagBitsPerSample, TypeShort, 1L, 4L);
		WriteTag(TagCompression, TypeShort, 1L, Compression);
		WriteTag(TagPhotometricInterpretation, TypeShort, 1L, 3L);
		WriteTag(TagStripOffsets, TypeLong, 1L, data_start);
		WriteTag(TagSamplesPerPixel, TypeShort, 1L, 1L);
		WriteTag(TagRowsPerStrip, TypeLong, 1L, Height);
		WriteTag(TagStripByteCounts, TypeLong, 1L, ImageSize);
		WriteTag(TagPlanarConfiguration, TypeShort, 1L, 1L);
		WriteTag(TagColorMap, TypeShort, 48L, data_start - 96L);
	}
	else if (Depth == 8)
	{
		WriteTag(TagBitsPerSample, TypeShort, 1L, 8L);
		WriteTag(TagCompression, TypeShort, 1L, Compression);
		WriteTag(TagPhotometricInterpretation, TypeShort, 1L, 3L);
		WriteTag(TagStripOffsets, TypeLong, 1L, data_start);
		WriteTag(TagSamplesPerPixel, TypeShort, 1L, 1L);
		WriteTag(TagRowsPerStrip, TypeLong, 1L, Height);
		WriteTag(TagStripByteCounts, TypeLong, 1L, ImageSize);
		WriteTag(TagPlanarConfiguration, TypeShort, 1L, 1L);
		WriteTag(TagColorMap, TypeShort, 768L, data_start - 1536L);
	}
	else if (Depth == 24)
	{
		WriteTag(TagBitsPerSample, TypeShort, 3L, data_start - 6L);
		WriteTag(TagCompression, TypeShort, 1L, Compression);
		WriteTag(TagPhotometricInterpretation, TypeShort, 1L, 2L);
		WriteTag(TagStripOffsets, TypeLong, 1L, data_start);
		WriteTag(TagSamplesPerPixel, TypeShort, 1L, 3L);
		WriteTag(TagRowsPerStrip, TypeLong, 1L, Height);
		WriteTag(TagStripByteCounts, TypeLong, 1L, ImageSize);
		WriteTag(TagPlanarConfiguration, TypeShort, 1L, 1L);
	}
	PutUInt32(0);
	if (Info->ColorType == G42ImageInfo::ColorTypeGrayscale)
	{
		if (Depth == 4)
		{
			for (int i = 0; i < 16; i++)
				PutUInt16((uint16)(i * 0x1111));
			for (int i = 0; i < 16; i++)
				PutUInt16((uint16)(i * 0x1111));
			for (int i = 0; i < 16; i++)
				PutUInt16((uint16)(i * 0x1111));
		}
		else if (Depth == 8)
		{
			for (int i = 0; i < 256; i++)
				PutUInt16((uint16)(i * 0x101));
			for (int i = 0; i < 256; i++)
				PutUInt16((uint16)(i * 0x101));
			for (int i = 0; i < 256; i++)
				PutUInt16((uint16)(i * 0x101));
		}
	}
	else if (Depth <= 8)
	{
		const G42Color * pal = Info->ImagePalette;
		int num_palette = Info->NumPalette;
		for (int i = 0; i < (int)(1 << (uint)Depth); i++)
		{
			if (i < num_palette)
				PutUInt16((uint16)(pal[i].red * 0x101));
			else
				PutUInt16(0);
		}
		for (int i = 0; i < (int)(1 << (uint)Depth); i++)
		{
			if (i < num_palette)
				PutUInt16((uint16)(pal[i].green * 0x101));
			else
				PutUInt16(0);
		}
		for (int i = 0; i < (int)(1 << (uint)Depth); i++)
		{
			if (i < num_palette)
				PutUInt16((uint16)(pal[i].blue * 0x101));
			else
				PutUInt16(0);
		}
	}
	else if (Depth > 8)
	{
		PutUInt16(8);
		PutUInt16(8);
		PutUInt16(8);
	}
}
bool
G42TiffWriter::WriteRow(const byte * row, const byte *)
{
	if (Depth == 24)
	{
		G42ConstByte24BitIterator sp(row);
		byte * dp = TempRow;
		uint32 i;
		for (i = 0; i < Width; i++)
		{
			*dp++ = (byte)sp.GetRed();
			*dp++ = (byte)sp.GetGreen();
			*dp++ = (byte)sp.GetBlue();
			sp.Increment();
		}
	}
	else
	{
		memcpy(TempRow, row, RowBytes);
	}
	Compress->ProcessBuffer(TempRow, RowBytes);
	TiffCurrentRow++;
	if (TiffCurrentRow >= Height)
	{
		if (WriteMode == Count)
		{
			Compress->Flush();
			WriteTags();
			CreateCompress();
			WriteMode = Write;
			TiffCurrentRow = 0;
		}
		else
		{
			Compress->Flush();
		}
	}
	return Valid;
}
uint32
G42TiffWriter::NextRow(void)
{
	return TiffCurrentRow;
}
bool
G42TiffWriter::NeedRow(void)
{
	return TiffCurrentRow < Height;
}
void
G42TiffWriter::HandleOutputBuffer(byte * buffer, uint length)
{
	if (WriteMode == Count)
		ImageSize += length;
	else
		PutBytes(buffer, length);
}
G42TiffCompressOutput::G42TiffCompressOutput(G42TiffWriter * parent) :
	Parent(parent)
{
}
G42TiffCompressOutput::~G42TiffCompressOutput()
{
}
void
G42TiffCompressOutput::OutputBuffer(byte * buffer, uint length)
{
	Parent->HandleOutputBuffer(buffer, length);
}
