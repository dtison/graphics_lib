// girpcx.cpp - Pcx File
/*************************************************************
	File:          girpcx.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of g42ribase that handles Pbm files
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "girpcx.h"
G42ImageReaderPcx::G42ImageReaderPcx(void) :
	G42ImageReaderBase(),
	ReadMode(ReadHeaderMode), CurrentPalette(0)
{
	SetInfo(G42PcxTypeId, "pcx", "Pcx", "\12", 1); // 12 is in octal (!) 0xa hex
}
G42ImageReaderPcx::G42ImageReaderPcx(
	G42HeapManager & heap, G42Notifier & notifier, G42File * file,
	G42BaseOptions * options)
:
	G42ImageReaderBase(heap, notifier, file, options),
	ReadMode(ReadHeaderMode), CurrentPalette(0)
{
	SetInfo(G42PcxTypeId, "pcx", "Pcx", "\12", 1); // 12 is in octal (!) 0xa hex
}
G42ImageReaderPcx::~G42ImageReaderPcx()
{
}
void
G42ImageReaderPcx::ConvertRow(void)
{
	/*  This code pulled from the Targa reader & modified to suit PCX  */
	if (Depth == 24)
	{
		if (GetImageDepth() == 24)
		{
			byte * sp = RowBuffer;
			G42Color24Iterator dp(OutputRowPtr);
			uintf plane_bytes_2 = PlaneBytes << 1;
			for (uint32 i = 0; i < Width; i++)
			{
				*dp = G42Color(*sp, *(sp + PlaneBytes), *(sp + plane_bytes_2));
				dp++;
				sp++;
			}
		}
		else
		{
			byte * sp = RowBuffer;
			G42Color32Iterator dp(OutputRowPtr);
			uintf plane_bytes_2 = PlaneBytes << 1;
			for (uint32 i = 0; i < Width; i++)
			{
				*dp = G42Color(*sp, *(sp + PlaneBytes), *(sp + plane_bytes_2));
				dp++;
				sp++;
			}
		}
	}
	else if (Depth == 4)
	{
		byte * sp1 = RowBuffer;
		byte * sp2 = RowBuffer + PlaneBytes;
		byte * sp3 = RowBuffer + (PlaneBytes << 1);
		byte * sp4 = RowBuffer + (PlaneBytes << 1) + PlaneBytes;
		intf sm = 0x80;
		byte * dp = OutputRowPtr;
//		memset(dp, 0, ((Width + 1) >> 1));
//		intf ds = 4;
		for (uint32 i = 0; i < Width; i++)
		{
			intf v = 0;
			if (*sp1 & sm)
				v |= 1;
			if (*sp2 & sm)
				v |= 2;
			if (*sp3 & sm)
				v |= 4;
			if (*sp4 & sm)
				v |= 8;
//			*dp |= (byte)(v << ds);
			*dp++ = (byte)v;
/*			if (ds == 4)
			{
				ds = 0;
			}
			else
			{
				ds = 4;
				dp++;
			}
*/
			if (sm == 1)
			{
				sp1++;
				sp2++;
				sp3++;
				sp4++;
				sm = 0x80;
			}
			else
			{
				sm >>= 1;
			}
		}
	}
//	G42ImageReaderBase::ConvertRow();
}
void
G42ImageReaderPcx::ForceFileEnd(void)
{
	Finished = true; // to get the decompressor to stop, we'll change it
						// back if necessary in ReadImage
}
void
G42ImageReaderPcx::ProcessStreamData(void)
{
	while (BufferSize)
	{
		switch (ReadMode)
		{
			case ReadHeaderMode:
				ReadHeader();
				break;
			case ReadPaletteMode:
				ReadPalette();
				break;
			case ReadImageMode:
				ReadImage();
				break;
			case CheckTrailerMode:
				CheckTrailer();
				break;
			default:
				EatBuffer();
				break;
		}
	}
}
void
G42ImageReaderPcx::BeginProcessing(void)
{
	while (ReadMode != ReadDoneMode && !Finished)
	{
		switch (ReadMode)
		{
			case ReadHeaderMode:
				ReadHeader();
				break;
			case ReadPaletteMode:
				ReadPalette();
				break;
			case ReadImageMode:
				ReadImage();
				break;
			case CheckTrailerMode:
				CheckTrailer();
				break;
			default:
				ReadMode = ReadDoneMode;
				break;
		}
	}
}
void
G42ImageReaderPcx::SendExtendedInfo(void)
{
	while (ReadMode != ReadDoneMode && !Finished)
	{
		switch (ReadMode)
		{
			case ReadHeaderMode:
				ReadHeader();
				break;
			case ReadPaletteMode:
				ReadPalette();
				break;
			case ReadImageMode:
				ReadImage(); // we need to skip the image here
				break;
			case CheckTrailerMode:
				CheckTrailer();
				break;
			default:
				ReadMode = ReadDoneMode;
				break;
		}
	}
}
void
G42ImageReaderPcx::ReadHeader(void)
{
	if (!CheckBuffer(128, true))
	{
		SaveBuffer();
		return;
	}
  	IntegerFormat = Intel;
	if (GetByte() != 0x0a)
		throw G42IRWNotAnImageException();
	SkipData(1); // skip version for now
	if (GetByte() != 1) // RLE compression only
		throw G42IRWNotAnImageException();
	intf bits_per_pixel = GetByte();
	uint16 x_start = GetUInt16();
	uint16 y_start = GetUInt16();
	Width = GetUInt16() - x_start  + 1;
	Height = GetUInt16() - y_start + 1;
	SkipData(4); // resolutions aren't useful to us
	G42Color * ega_palette = new G42Color[16];
	G42ThrowIfNull(ega_palette);
	// this is kludgy, but we don't know yet if we need this, so we'd better
	// just store it for now
	for (intf i = 0; i < 16; i++)
	{
		ega_palette[i].Red = GetByte();
		ega_palette[i].Green = GetByte();
		ega_palette[i].Blue = GetByte();
	}
	SkipData(1);
	intf planes = GetByte();
	PlaneBytes = GetUInt16();
	RowBytes = planes * PlaneBytes;		// D.I.
	SkipData(60); // move to the image
	if (planes == 1 && bits_per_pixel == 1)
	{
		Depth = 1;
		NumPalette = 0;
		delete [] ega_palette;
		ega_palette = 0;
		Color = false;
	}
	else if (planes == 4 && bits_per_pixel == 1)
	{
		Depth = 4;
		NumPalette = 16;
		Color = true;
	}
	else if (planes == 1 && bits_per_pixel == 8)
	{
		Depth = 8;
		NumPalette = 256;
		Color = true;
	}
	else if (planes == 3 && bits_per_pixel == 8)
	{
		Color = true;
		Depth = 24;	// D.I.
		NumPalette = 0;
		delete [] ega_palette;
		ega_palette = 0;
	}
	else
	{
		delete [] ega_palette;
		throw G42IRWInvalidImageException();
	}
	CurrentPalette = 0;
	Compressed = true;
	if (planes > 1)
	{
		ReadRowMode = UseTemporaryBuffer; // force temporary buffer
	}
	Decompressor = new G42PcxRLEDecompressor(*this, RowBytes);
	SetupDescriptor();
	if (ega_palette)
	{
		for (intf i = 0; i < 16; i++)
		{
			ImagePalette[i] = ega_palette[i];
		}
		delete [] ega_palette;
	}
	if (NumPalette == 256)
	{
		if (File)
		{
			ImagePosition = File->GetPosition() - BufferSize;
			File->SetPosition(769, G42File::FromEnd); // go to end of file to read palette
			EatBuffer(); // clear out any extra memory
			ReadMode = CheckTrailerMode; // go read palette
			return;
		}
		else
		{
			BlockDataSending = true; // don't send data until we read palette
		}
	}
	else
	{
		SendDescriptor();
	}
	ReadMode = ReadImageMode; // 8 bit palette is after image
}
void
G42ImageReaderPcx::CheckTrailer(void)
{
	CheckBuffer(1, true);
	if (GetByte() == 0x0C)
	{
		ReadMode = ReadPaletteMode;
	}
	else // we hope ega palette was good enough
	{
		if (File)
		{
			File->SetPosition(ImagePosition);
			EatBuffer();
			ReadMode = ReadImageMode;
		}
		else
		{
			ReadMode = ReadDoneMode;
			G42ImageReaderBase::ForceFileEnd();
		}
	}
}
void
G42ImageReaderPcx::ReadPalette(void)
{
	// read until palette is complete or buffer is empty
	CheckBuffer(3 * NumPalette, true);
	while (BufferSize >= 3 && CurrentPalette < NumPalette)
	{
		ImagePalette[CurrentPalette].Red = GetByte();
		ImagePalette[CurrentPalette].Green = GetByte();
		ImagePalette[CurrentPalette].Blue = GetByte();
		CurrentPalette++;
	}
	if (CurrentPalette == NumPalette)
	{
		SendDescriptor();
		if (File)
		{
			File->SetPosition(ImagePosition);
			EatBuffer();
			ReadMode = ReadImageMode;
		}
		else
		{
			ReadMode = ReadDoneMode;
			G42ImageReaderBase::ForceFileEnd();
		}
	}
	else if (BufferSize)
	{
		SaveBuffer(); // save for rest of data
	}
}
void
G42ImageReaderPcx::ReadImage(void)
{
	ProcessImageData();
	if (Finished)
	{
		if (Depth == 8 && !File) // still need to read in end palette
		{
			Finished = false;
			ReadMode = CheckTrailerMode;
		}
		else
		{
			ReadMode = ReadDoneMode;
			G42ImageReaderBase::ForceFileEnd();
		}
	}
}
G42PcxRLEDecompressor::G42PcxRLEDecompressor(
	G42DecompressorOutput & output, uintf row_bytes)
:
	G42Decompressor(output, row_bytes),
	DecompressionMode(StartDecompression)
{
}
G42PcxRLEDecompressor::~G42PcxRLEDecompressor(void)
{
}
void
G42PcxRLEDecompressor::ProcessBuffer(void)
{
	while (InputBufferLeft() && !Done)
	{
		switch (DecompressionMode)
		{
			case StartDecompression:
			{
				byte active_byte = GetByte();
				if ((0xC0 & active_byte) == 0xC0)
				{
					RunCount = (0x3F & active_byte);
					DecompressionMode = GetRunValue;
				}
				else
				{
					PutByte(active_byte);
				}
				break;
			}
			case GetRunValue:
			{
				byte active_byte = GetByte();
				PutRun(active_byte, RunCount);
				DecompressionMode = StartDecompression;
				break;
			}
		}
	}
}
