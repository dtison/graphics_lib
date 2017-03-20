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
#include "giwtiff.h"
#include "giclzw.h"
#include "gicpbits.h"
#include "gicccitt.h"
G42ImageWriterTiff::G42ImageWriterTiff(void)
:
	G42ImageWriterBase(), TempRow(0), Compression(0),
	Compress(0), ImageSize(0), WriteMode(Count),
	TiffCurrentRow(0), Info(0)
{
	SetInfo(G42TiffTypeId, "tif", "Tiff", true);
}
G42ImageWriterTiff::G42ImageWriterTiff(G42Notifier & notifier,
	G42BaseOptions * options)
:
	G42ImageWriterBase(notifier, options), TempRow(0), Compression(0),
	Compress(0), ImageSize(0), WriteMode(Count),
	TiffCurrentRow(0), Info(0)
{
	SetInfo(G42TiffTypeId, "tif", "Tiff", true);
}
G42ImageWriterTiff::~G42ImageWriterTiff()
{
	if (Compress)
	{
		delete Compress;
		Compress = 0;
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
G42ImageWriterTiff::WriteTag(uint16 tag, uint16 type,
	uint32 length, uint32 offset)
{
	PutUInt16(tag);
	PutUInt16(type);
	PutUInt32(length);
	PutUInt32(offset);
}
bool
G42ImageWriterTiff::WriteInfo(G42ImageDescriptor * info)
{
	G42ImageWriterBase::WriteInfo(info); // set up base stuff
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
	Info = new G42ImageDescriptor(*info);
	Compression = NoCompression;
	if (Options && dynamic_cast<G42ImageReadWriteOptions *>(Options))
	{
		G42ImageReadWriteOptions * options =
			dynamic_cast<G42ImageReadWriteOptions *>(Options);
		if (options && options->IsCompressed())
		{
			Compression = PackBitsCompression;
		}
	}
	if (!Options || !dynamic_cast<G42ImageTiffOptions *>(Options))
	{
		G42ImageTiffOptions * options =
			new G42ImageTiffOptions();
		G42ThrowIfNull(options);
		AskForOptions(options);
	}
	G42ImageTiffOptions * options;
	if (Options)
	{
		options = dynamic_cast<G42ImageTiffOptions *>(Options);
	}
	if (options)
	{
		Compression = options->GetTiffCompressionType();
	}
	else
	{
		if (Options && dynamic_cast<G42ImageReadWriteOptions *>(Options))
		{
			G42ImageReadWriteOptions * options =
				dynamic_cast<G42ImageReadWriteOptions *>(Options);
			if (options && options->IsCompressed())
			{
				Compression = PackBitsCompression;
			}
		}
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
	CreateCompress();
	return Valid;
}
void
G42ImageWriterTiff::CreateCompress(void)
{
	if (Compress)
	{
		delete Compress;
		Compress = 0;
	}
	switch (Compression)
	{
		case NoCompression:
			Compress = new G42NoCompressor(*this);
			break;
		case PackBitsCompression:
			Compress = new G42PackBitsCompressor(*this);
			break;
		case LZWCompression:
			Compress = new G42LzwCompressor(*this);
			break;
		case CCITTCompression:
		case Group3Compression:
		case Group4Compression:
			Compress = new G42CcittCompressor(*this, Compression, Width);
			break;
		default:
			Valid = false;
			break;
	}
}
void
G42ImageWriterTiff::WriteTags(void)
{
	intf num_tags;
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
	intf data_start = (8 + 2 + 12 * num_tags + 4);
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
	if (!Info->IsColor())
	{
		if (Depth == 4)
		{
			intf i;
			for (i = 0; i < 16; i++)
				PutUInt16((uint16)(i * 0x1111));
			for (i = 0; i < 16; i++)
				PutUInt16((uint16)(i * 0x1111));
			for (i = 0; i < 16; i++)
				PutUInt16((uint16)(i * 0x1111));
		}
		else if (Depth == 8)
		{
			intf i;
			for (i = 0; i < 256; i++)
				PutUInt16((uint16)(i * 0x101));
			for (i = 0; i < 256; i++)
				PutUInt16((uint16)(i * 0x101));
			for (i = 0; i < 256; i++)
				PutUInt16((uint16)(i * 0x101));
		}
	}
	else if (Depth <= 8)
	{
		const G42Color * pal = Info->GetPalette();
		intf num_palette = Info->GetNumPalette();
		intf i;
		for (i = 0; i < (intf)(1 << (uintf)Depth); i++)
		{
			if (i < num_palette)
				PutUInt16((uint16)(pal[i].Red * 0x101));
			else
				PutUInt16(0);
		}
		for (i = 0; i < (intf)(1 << (uintf)Depth); i++)
		{
			if (i < num_palette)
				PutUInt16((uint16)(pal[i].Green * 0x101));
			else
				PutUInt16(0);
		}
		for (i = 0; i < (intf)(1 << (uintf)Depth); i++)
		{
			if (i < num_palette)
				PutUInt16((uint16)(pal[i].Blue * 0x101));
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
G42ImageWriterTiff::WriteRow(const byte * row, const byte *)
{
	if (Depth == 24)
	{
		if (Info->GetDepth() == 24)
		{
			G42ConstColor24Iterator sp(row);
			byte * dp = TempRow;
			uint32 i;
			for (i = 0; i < Width; i++)
			{
				G42ConstColor24 c(*sp);
				*dp++ = (byte)c.GetRed();
				*dp++ = (byte)c.GetGreen();
				*dp++ = (byte)c.GetBlue();
				sp++;
			}
		}
		else
		{
			G42ConstColor32Iterator sp(row);
			byte * dp = TempRow;
			uint32 i;
			for (i = 0; i < Width; i++)
			{
				G42ConstColor32 c(*sp);
				*dp++ = (byte)c.GetRed();
				*dp++ = (byte)c.GetGreen();
				*dp++ = (byte)c.GetBlue();
				sp++;
			}
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
G42ImageWriterTiff::NextRow(void)
{
	return TiffCurrentRow;
}
bool
G42ImageWriterTiff::NeedRow(void)
{
	return TiffCurrentRow < Height;
}
void
G42ImageWriterTiff::CompressorOutputBuffer(byte * buffer, uintf length)
{
	if (WriteMode == Count)
		ImageSize += length;
	else
		PutBytes(buffer, length);
}
