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
#include "girtiff.h"
#include "gidpbits.h"
#include "gidlzw.h"
#include "gidccitt.h"
G42ImageReaderTiffBase::G42ImageReaderTiffBase(void)
:
	ReadMode(ReadHeaderMode),
	CurrentRow(0), StoreBlockArray(0),
	BitsPerSample(1), Compression(NoCompression),
	ColorType(-1), RowsPerStrip(0x7fff), TiffSkip(0),
	SamplesPerPixel(1), StripArray(0), StripSizeArray(0),
	Predictor(PredictionNone), MaxStoreBlock(0), LastStoreBlock(-1),
	StoreBufferLeft(0), TotalFileRead(0), StoreBufferSize(65528U),
	StoreBufferNext(0), WaitOffset(0), T4Options(0), T6Options(0),
	FillOrder(1), TiffRowBuffer(0), InternalRowBytes(0)
{
}
G42ImageReaderTiffBase::G42ImageReaderTiffBase(
	G42HeapManager & heap, G42Notifier & notifier, G42File * file,
	G42BaseOptions * options)
:
	G42ImageReaderBase(heap, notifier, file, options),
	ReadMode(ReadHeaderMode),
	CurrentRow(0), StoreBlockArray(0),
	BitsPerSample(1), Compression(NoCompression),
	ColorType(-1), RowsPerStrip(0x7fff), TiffSkip(0),
	SamplesPerPixel(1), StripArray(0), StripSizeArray(0),
	Predictor(PredictionNone), MaxStoreBlock(0), LastStoreBlock(-1),
	StoreBufferLeft(0), TotalFileRead(0), StoreBufferSize(65528U),
	StoreBufferNext(0), WaitOffset(0), T4Options(0), T6Options(0),
	FillOrder(1), TiffRowBuffer(0), InternalRowBytes(0)
{
	if (File)
	{
		File->SetPosition(0, G42File::FromEnd);
		TotalFileRead = File->GetPosition();
		File->Rewind();
	}
}
G42ImageReaderTiffBase::~G42ImageReaderTiffBase()
{
	SavedBuffer = 0; // don't delete this, as it points to a block
	if (StoreBlockArray)
	{
		for (intf i = 0; i <= LastStoreBlock; i++)
			delete[] StoreBlockArray[i];
		delete[] StoreBlockArray;
	}
	if (StripArray)
		delete[] StripArray;
	if (StripSizeArray)
		delete[] StripSizeArray;
	if (TiffRowBuffer)
	{
		delete [] TiffRowBuffer;
		TiffRowBuffer = 0;
	}
}
// count rows, and mark done when last row is output
void
G42ImageReaderTiffBase::ConvertRow(void)
{
	if (Predictor == PredictionHorizontal)
	{
		switch (TiffDepth)
		{
			case 1:
			{
				byte * dp = RowBuffer + (SamplesPerPixel >> 3);
				intf ds = (7 - (SamplesPerPixel & 7));
				byte * sp = RowBuffer;
				intf ss = 7;
				for (intf i = SamplesPerPixel; i < (RowBytes << 3); i++)
				{
					intf sv = ((*sp) >> ss) & 0x1;
					intf dv = ((*dp) >> ds) & 0x1;
					dv += sv;
					dv &= 0x1;
					if (dv)
						*dp |= (byte)(1 << ds);
					else
						*dp &= (byte)(~(1 << ds));
					if (ds)
						ds--;
					else
					{
						ds = 7;
						dp++;
					}
					if (ss)
						ss--;
					else
					{
						ss = 7;
						sp++;
					}
				}
			}
			break;
			case 2:
			{
				byte * dp = RowBuffer + (SamplesPerPixel >> 2);
				intf ds = ((3 - (SamplesPerPixel & 3)) << 1);
				byte * sp = RowBuffer;
				intf ss = 2;
				for (intf i = SamplesPerPixel; i < (RowBytes << 2); i++)
				{
					intf sv = ((*sp) >> ss) & 0x3;
					intf dv = ((*dp) >> ds) & 0x3;
					dv += sv;
					dv &= 0x3;
					*dp &= (byte)(0x3 << (6 - ds));
					*dp |= (byte)(dv << ds);
					if (ds)
						ds -= 2;
					else
					{
						ds = 6;
						dp++;
					}
					if (ss)
						ss -= 2;
					else
					{
						ss = 6;
						sp++;
					}
				}
			}
			break;
			case 4:
			{
				byte * dp = RowBuffer + (SamplesPerPixel >> 1);
				intf ds = ((1 - (SamplesPerPixel & 1)) << 2);
				byte * sp = RowBuffer;
				intf ss = 4;
				for (intf i = SamplesPerPixel; i < (RowBytes << 1); i++)
				{
					intf sv = ((*sp) >> ss) & 0xf;
					intf dv = ((*dp) >> ds) & 0xf;
					dv += sv;
					dv &= 0xf;
					*dp &= (byte)(0xf << (4 - ds));
					*dp |= (byte)(dv << ds);
					if (ds)
						ds = 0;
					else
					{
						ds = 4;
						dp++;
					}
					if (ss)
						ss = 0;
					else
					{
						ss = 4;
						sp++;
					}
				}
			}
			break;
			default:
			{
				byte * dp = RowBuffer + SamplesPerPixel;
				byte * sp = RowBuffer;
				for (intf i = SamplesPerPixel; i < RowBytes; i++)
				{
					*(dp++) += *(sp++);
				}
			}
			break;
		}
	}
	if (ExtraSamples)
	{
		switch (TiffDepth)
		{
			case 24:
			case 32:
			{
				byte * sp = RowBuffer;
				byte * dp = OutputRowPtr;
				for (intf i = 0; i < Width; i++)
				{
					*(dp++) = *(sp++);
					*(dp++) = *(sp++);
					*(dp++) = *(sp++);
					sp += ExtraSamples;
				}
			}
			break;
			case 8:
			{
				byte * sp = RowBuffer;
				byte * dp = OutputRowPtr;
				for (intf i = 0; i < Width; i++)
				{
					*(dp++) = *(sp++);
					sp += ExtraSamples;
				}
			}
			break;
			case 4:
			{
				byte * sp = RowBuffer + ((ExtraSamples + 1) >> 1);
				byte * dp = OutputRowPtr;
				*dp &= 0xf0; // second nibble is unused anyway
				intf ss = ((ExtraSamples & 1) << 2);
				intf ds = 0;
				for (intf i = 1; i < Width; i++)
				{
					intf v = ((*sp) >> ss) & 0xf;
					*dp |= (byte)(v << ds);
					if (ds)
						ds = 0;
					else
					{
						ds = 4;
						dp++;
						*dp = 0;
					}
					for (intf j = 0; j < ExtraSamples + 1; j++)
					{
						if (ss)
							ss = 0;
						else
						{
							ss = 4;
							sp++;
						}
					}
				}
			}
			break;
			case 2:
			{
				byte * sp = RowBuffer;
				byte * dp = OutputRowPtr;
				intf ss = 6;
				intf ds = 6;
				for (intf i = 0; i < Width; i++)
				{
					intf v = ((*sp) >> ss) & 0x3;
					*dp &= (byte)(0xff & (0xff3f << (6 - ds)));
					*dp |= (byte)(v << ds);
					if (ds)
						ds -= 2;
					else
					{
						ds = 6;
						dp++;
					}
					for (intf j = 0; j < ExtraSamples + 1; j++)
					{
						if (ss)
							ss -= 2;
						else
						{
							ss = 6;
							sp++;
						}
					}
				}
			}
			break;
			case 1:
			{
				byte * sp = RowBuffer;
				byte * dp = OutputRowPtr;
				intf ss = 7;
            intf ds = 7;
				for (intf i = 0; i < Width; i++)
				{
					if (*sp & ss)
						*dp |= (byte)(1 << ds);
					else
						*dp &= (byte)(~(1 << ds));
					if (ds)
						ds--;
					else
					{
						ds = 7;
						dp++;
					}
					for (intf j = 0; j < ExtraSamples + 1; j++)
					{
						if (ss)
							ss--;
						else
						{
							ss = 7;
							sp++;
						}
					}
				}
			}
			break;
		}
	}
#if 0 // taken care of in base
	if (TiffDepth == 2)
	{
		// expand to depth of 4
		byte * sp = OutputRowPtr + ((Width - 1) >> 2);
		byte * dp = OutputRowPtr + ((Width - 1) >> 1);
		intf ss = ((3 - ((Width - 1) & 3)) << 1);
		intf ds = ((1 - ((Width - 1) & 1)) << 2);
		for (intf i = 0; i < Width; i++)
		{
			intf v = ((*sp >> ss) & 3);
			v |= (v << 2);
			if (ds)
			{
				*dp &= 0xf;
				*dp |= (v << 4);
			}
			else
			{
				*dp &= 0xf0;
				*dp |= v;
			}
			if (ss < 6)
			{
				ss += 2;
			}
			else
			{
				ss = 0;
				sp--;
			}
			if (!ds)
			{
				ds = 4;
			}
			else
			{
				ds = 0;
				dp--;
			}
		}
	}
#endif
	if (ColorType == ColorTypeWhiteIsZero)
	{
		byte * sp = OutputRowPtr;
		for (intf i = 0; i < RowBytes; i++)
		{
			*sp = (byte)(~(*sp));
			sp++;
		}
	}
	if (Depth > 8)
	{
		if (GetImageDepth() == 24)
		{
			byte * sp = OutputRowPtr;
			G42Color24Iterator dp(OutputRowPtr);
			for (uint32 i = 0; i < Width; i++)
			{
				*dp = G42Color(*sp, *(sp + 1), *(sp + 2));
				dp++;
				sp += 3;
			}
		}
		else
		{
			byte * sp = OutputRowPtr + (Width - 1) * 3;
			G42Color32Iterator dp(OutputRowPtr);
			dp += Width - 1;
			for (uint32 i = 0; i < Width; i++)
			{
				*dp = G42Color(*sp, *(sp + 1), *(sp + 2));
				dp--;
				sp -= 3;
			}
		}
	}
	G42ImageReaderBase::ConvertRow();
	StripRow--;
	if (!StripRow)
	{
		ReadMode = ReadImageStripMode;
	}
	if (Finished)
	{
		ReadMode = ReadDoneMode;
	}
	CurrentRow++;
}
// override for DecompressorObject
bool
G42ImageReaderTiffBase::DecompressorOutputRow(byte *)
{
	OutputRow();
	return (ReadMode != ReadImageRowsMode || Finished);
}
void
G42ImageReaderTiffBase::ProcessStreamData(void)
{
	AddBuffer(CurrentBuffer, BufferLeft);
	if (TiffSkip)
		return;
	NeedMoreData = false;
	if (ReadMode == ReadWaitMode && WaitOffset > TotalFileRead)
		return;
	if (ReadMode == ReadWaitMode)
		ReadMode = WaitMode;
	while (!NeedMoreData && !TiffSkip)
	{
		switch (ReadMode)
		{
			case ReadHeaderMode:
				ReadHeader();
				break;
			case ReadTiffHeaderMode:
				ReadTiffHeader();
				break;
			case ReadImageRowsMode:
				ReadImageRows();
				break;
			case ReadImageStripMode:
				ReadImageStrip();
				break;
			case ReadIFDMode:
				ReadIFD();
				break;
			case ReadIFDEntryMode:
				ReadIFDEntry();
				break;
			case ReadStripOffsetsMode:
				ReadStripOffsets();
				break;
			case ReadBitsPerSampleMode:
				ReadBitsPerSample();
				break;
			case ReadStripByteCountsMode:
				ReadStripByteCounts();
				break;
			case ReadColorMapMode:
				ReadColorMap();
				break;
			case ReadWaitMode:
				break;
			default:
				NeedMoreData = true;
				break;
		}
		if (ReadMode == ReadWaitMode)
			break;
	}
}
void
G42ImageReaderTiffBase::BeginProcessing(void)
{
	while (!Finished && ReadMode != ReadDoneMode)
	{
		switch (ReadMode)
		{
			case ReadHeaderMode:
				ReadHeader();
				break;
			case ReadTiffHeaderMode:
				ReadTiffHeader();
				break;
			case ReadImageRowsMode:
				ReadImageRows();
				break;
			case ReadImageStripMode:
				ReadImageStrip();
				break;
			case ReadIFDMode:
				ReadIFD();
				break;
			case ReadIFDEntryMode:
				ReadIFDEntry();
				break;
			case ReadStripOffsetsMode:
				ReadStripOffsets();
				break;
			case ReadBitsPerSampleMode:
				ReadBitsPerSample();
				break;
			case ReadStripByteCountsMode:
				ReadStripByteCounts();
				break;
			case ReadColorMapMode:
				ReadColorMap();
				break;
			default:
				ReadMode = ReadDoneMode;
				break;
		}
	}
}
void
G42ImageReaderTiffBase::SendExtendedInfo(void)
{
	while (!Finished && ReadMode != ReadDoneMode)
	{
		switch (ReadMode)
		{
			case ReadHeaderMode:
				ReadHeader();
				break;
			case ReadTiffHeaderMode:
				ReadTiffHeader();
				break;
			case ReadImageRowsMode:
				ReadImageRows();
				break;
			case ReadImageStripMode:
				ReadImageStrip();
				break;
			case ReadIFDMode:
				ReadIFD();
				break;
			case ReadIFDEntryMode:
				ReadIFDEntry();
				break;
			case ReadStripOffsetsMode:
				ReadStripOffsets();
				break;
			case ReadBitsPerSampleMode:
				ReadBitsPerSample();
				break;
			case ReadStripByteCountsMode:
				ReadStripByteCounts();
				break;
			case ReadColorMapMode:
				ReadColorMap();
				break;
			default:
				ReadMode = ReadDoneMode;
				break;
		}
	}
}
void
G42ImageReaderTiffBase::ReadHeader(void)
{
	if (!LoadBuffer(0, 32))
		return;
	byte order = GetByte();
	if (order == 'I' || order == 'M')
	{
		ReadMode = ReadTiffHeaderMode;
		return;
	}
	if (order != 0xc5)
	{
		ReadMode = ReadErrorMode;
		throw G42IRWNotAnImageException();
	}
	if (GetByte() != 0xd0)
	{
		ReadMode = ReadErrorMode;
		throw G42IRWNotAnImageException();
	}
	if (GetByte() != 0xd3)
	{
		ReadMode = ReadErrorMode;
		throw G42IRWNotAnImageException();
	}
	if (GetByte() != 0xc6)
	{
		ReadMode = ReadErrorMode;
		throw G42IRWNotAnImageException();
	}
	SkipData(16);
	TiffSkip = GetUInt32(Intel);
	if (TiffSkip)
	{
		if (TotalFileRead > TiffSkip)
		{
			intf skip = OrigBufferSize - (TotalFileRead - TiffSkip);
			OrigBuffer += skip;
			OrigBufferSize -= skip;
			ClearBuffer();
			AddBuffer(OrigBuffer, OrigBufferSize);
			TiffSkip = 0;
		}
		else
		{
			TiffSkip -= TotalFileRead;
			ClearBuffer();
		}
		ReadMode = ReadTiffHeaderMode;
	}
	else
	{
		ReadMode = ReadErrorMode;
		throw G42IRWInvalidImageException();
	}
}
void
G42ImageReaderTiffBase::ReadTiffHeader(void)
{
	if (!LoadBuffer(0, 8))
		return;
	// check header ('II' or 'MM')
	byte order = GetByte();
	if (order == 'I')
		IntegerFormat = Intel;
	else if (order == 'M')
		IntegerFormat = Motorola;
	else
	{
		ReadMode = ReadErrorMode;
		throw G42IRWNotAnImageException();
	}
	if (GetByte() != order)
	{
		ReadMode = ReadErrorMode;
		throw G42IRWNotAnImageException();
	}
	if (GetUInt16() != 42)
	{
		ReadMode = ReadErrorMode;
		throw G42IRWNotAnImageException();
	}
	IFDOffset = GetUInt32();
	ReadMode = ReadIFDMode;
}
void
G42ImageReaderTiffBase::ReadIFD(void)
{
	if (!LoadBuffer(IFDOffset, 2))
		return;
	NumIFDEntries = GetUInt16();
	CurIFDEntry = 0;
	ReadMode = ReadIFDEntryMode;
	IFDOffset += 2;
}
void
G42ImageReaderTiffBase::ReadIFDEntry(void)
{
	if (!LoadBuffer(IFDOffset, 12))
		return;
	Tag = GetUInt16();
	Type = GetUInt16();
	Count = (uintf)GetUInt32();
	switch (Tag)
	{
		case TagImageWidth:
			Width = GetTiffValue();
			break;
		case TagImageLength:
			Height = GetTiffValue();
			break;
		case TagBitsPerSample:
			if (NeedTiffPtr())
			{
				ReadMode = ReadBitsPerSampleMode;
				ArrayOffset = GetUInt32();
            return; // don't finish
         }
			else
				BitsPerSample = GetTiffValue();
         break;
		case TagCompression:
			Compression = GetTiffValue();
			break;
		case TagPhotometricInterpretation:
			ColorType = GetTiffValue();
			break;
		case TagFillOrder:
			FillOrder = GetTiffValue();
			break;
		case TagStripOffsets:
			StripArray = new uint32 [Count];
			if (NeedTiffPtr())
			{
				ReadMode = ReadStripOffsetsMode;
				ArrayOffset = GetUInt32();
            return; // don't finish
         }
			else
			{
				for (intf i = 0; i < Count; i++)
					StripArray[i] = GetTiffValue();
			}
			break;
		case TagSamplesPerPixel:
			SamplesPerPixel = GetTiffValue();
			break;
		case TagRowsPerStrip:
			RowsPerStrip = GetTiffValue();
			if (RowsPerStrip > 0x7fff)
				RowsPerStrip = 0x7fff;
			break;
		case TagStripByteCounts:
			StripSizeArray = new uint32 [Count];
			if (NeedTiffPtr())
			{
				ReadMode = ReadStripByteCountsMode;
				ArrayOffset = GetUInt32();
				return; // don't finish
			}
			else
			{
				for (intf i = 0; i < Count; i++)
					StripSizeArray[i] = GetTiffValue();
			}
         break;
		case TagT4Options:
			T4Options = GetTiffValue();
			break;
		case TagT6Options:
			T6Options = GetTiffValue();
			break;
		case TagPredictor:
			Predictor = GetTiffValue();
			break;
		case TagColorMap:
			if (NeedTiffPtr()) // if we don't need a pointer, it's a bad tag
			{
				NumPalette = Count / 3;
				if (NumPalette < 1)
				{
					break;
				}
				if (NumPalette > 256)
				{
					NumPalette = 256;
				}
				TiffPalette = new G42Color[NumPalette];
				ReadMode = ReadColorMapMode;
				ArrayOffset = GetUInt32();
				return; // don't finish
			}
			break;
	}
	FinishIFDEntry();
}
void
G42ImageReaderTiffBase::ReadStripOffsets(void)
{
	if (!LoadBuffer(ArrayOffset, Count * GetValueSize()))
		return;
	for (intf i = 0; i < Count; i++)
		StripArray[i] = GetTiffValue();
	ReadMode = ReadIFDEntryMode;
	FinishIFDEntry();
}
void
G42ImageReaderTiffBase::ReadColorMap(void)
{
	if (!LoadBuffer(ArrayOffset, Count * GetValueSize()))
		return;
	intf i;
	for (i = 0; i < NumPalette; i++)
		TiffPalette[i].Red = (byte)(GetTiffValue() >> 8);
	for (i = 0; i < NumPalette; i++)
		TiffPalette[i].Green = (byte)(GetTiffValue() >> 8);
	for (i = 0; i < NumPalette; i++)
		TiffPalette[i].Blue = (byte)(GetTiffValue() >> 8);
	ReadMode = ReadIFDEntryMode;
	FinishIFDEntry();
}
void
G42ImageReaderTiffBase::ReadStripByteCounts(void)
{
	if (!LoadBuffer(ArrayOffset, Count * GetValueSize()))
		return;
	for (intf i = 0; i < Count; i++)
		StripSizeArray[i] = GetTiffValue();
	ReadMode = ReadIFDEntryMode;
	FinishIFDEntry();
}
void
G42ImageReaderTiffBase::ReadBitsPerSample(void)
{
	if (!LoadBuffer(ArrayOffset, GetValueSize()))
		return;
	BitsPerSample = GetTiffValue();
	ReadMode = ReadIFDEntryMode;
	FinishIFDEntry();
}
int32
G42ImageReaderTiffBase::GetTiffValue(void)
{
	int32 ret;
	switch (Type)
	{
		case TypeByte:
			ret = GetByte();
			break;
		case TypeShort:
			ret = GetUInt16();
			break;
		case TypeLong:
			ret = GetUInt32();
			break;
		case TypeSByte:
			ret = GetByte();
			if (ret >= 128)
         	ret = 256 - ret;
			break;
		case TypeSShort:
			ret = GetUInt16();
			if (ret >= 32768L)
         	ret = 65536L - ret;
			break;
		case TypeSLong:
			ret = GetUInt32();
			break;
		default:
			ret = 0;
         break;
	}
	return ret;
}
intf
G42ImageReaderTiffBase::GetValueSize(void)
{
	intf size;
	if (Type == TypeByte || Type == TypeAscii || Type == TypeSByte ||
		Type == TypeUndefined)
		size = 1;
	else if (Type == TypeShort || Type == TypeSShort)
		size = 2;
	else if (Type == TypeLong || Type == TypeSLong || Type == TypeFloat)
		size = 4;
	else
		size = 8;
	return size;
}
bool
G42ImageReaderTiffBase::NeedTiffPtr(void)
{
	intf size = GetValueSize() * Count;
	return (size > 4);
}
void
G42ImageReaderTiffBase::FinishIFDEntry(void)
{
	CurIFDEntry++;
	if (CurIFDEntry >= NumIFDEntries)
	{
		if (!StripArray || !Width || !Height || ColorType < 0)
		{
			ReadMode = ReadErrorMode;
			throw G42IRWInvalidImageException();
		}
		if (BitsPerSample == 1)
			TiffDepth = 1;
		else if (BitsPerSample == 2)
			TiffDepth = 2;
		else if (BitsPerSample == 4)
			TiffDepth = 4;
		else if (BitsPerSample == 8 && ColorType == ColorTypeRGB)
	//		TiffDepth = 8 * TrueColorPixelSize;
			TiffDepth = 24;
		else if (BitsPerSample == 8)
			TiffDepth = 8;
		else
		{
			ReadMode = ReadErrorMode;
			throw G42IRWInvalidImageException();
		}
		if (TiffDepth == 2)
			Depth = 4;
		else
			Depth = TiffDepth;
		if (Depth > 8)
			ExtraSamples = SamplesPerPixel - 3;
		else
			ExtraSamples = SamplesPerPixel - 1;
		if (ExtraSamples < 0)
			ExtraSamples = 0;
		if (ColorType == ColorTypeRGB || ColorType == ColorTypePalette)
			Color = true;
		else
			Color = false;
		InternalRowBytes = (intf)(((int32)Width * (int32)Depth + 7) >> 3);
		if (Depth > 8)
		{
			RowBytes = 3 * Width;
		}
		else
		{
			RowBytes = InternalRowBytes;
		}
		if (ExtraSamples)
		{
			if (TiffDepth > 8)
				TrueRowBytes = SamplesPerPixel * Width;
			else
				TrueRowBytes = (intf)(((int32)SamplesPerPixel * (int32)Width *
					(int32)TiffDepth + 7) >> 3);
		}
		else
		{
			TrueRowBytes = (intf)(((int32)Width * (int32)TiffDepth + 7) >> 3);
		}
		if (Depth > 8)
			Depth = 24;
		uintf buffer_size = max((uintf)RowBytes, (uintf)TrueRowBytes);
		buffer_size = max(buffer_size, (uintf)InternalRowBytes);
		TiffRowBuffer = new byte [buffer_size];
		TopLineFirst = true; // TIFF's are top down files
		if ((Compression < NoCompression || Compression > LZW) &&
			Compression != PackBits)
		{
			ReadMode = ReadErrorMode;
			throw G42IRWInvalidImageException();
		}
		if (Compression == NoCompression)
		{
			Compressed = false;
		}
		else
		{
			Compressed = true;
		}
		WriteOptions = new G42ImageTiffOptions(
			(G42ImageTiffOptions::G42TiffCompressionType)Compression, false);
		if (!Descriptor)
		{
			SetupDecompressor();
			SetupDescriptor();
		}
		if (NumPalette && ImagePalette)
		{
			for (uintf i = 0; i < NumPalette; i++)
			{
				ImagePalette[i] = TiffPalette[i];
			}
		}
		SendDescriptor();
		ReadMode = ReadImageStripMode;
		CurrentStrip = -1;
		StripRow = RowsPerStrip;
	}
	IFDOffset += 12;
}
void
G42ImageReaderTiffBase::ReadImageStrip(void)
{
	CurrentStrip++;
	if (CurrentStrip >= ((Height - 1) / RowsPerStrip) + 1)
	{
		EatBuffer();
		ReadMode = ReadDoneMode;
		ForceFileEnd();
		return;
	}
	StripRow = RowsPerStrip;
	StripOffset = StripArray[CurrentStrip];
	if (StripSizeArray)
		StripBytesLeft = StripSizeArray[CurrentStrip];
	else
		StripBytesLeft = 0x7fffffffL;
	ReadMode = ReadImageRowsMode;
	if (CurrentStrip > 0)
	{
		SetupDecompressor();
	}
}
void
G42ImageReaderTiffBase::SetupDecompressor(void)
{
	switch (Compression)
	{
		case LZW:
			Decompressor = new G42LzwDecompressor(*this, TrueRowBytes);
			break;
		case PackBits:
			Decompressor = new G42PackBitsDecompressor(*this, TrueRowBytes);
			break;
		case CCITT:
		case Group3:
		case Group4:
			Decompressor = new G42CcittDecompressor(*this,
				Width * (1 + ExtraSamples),
				Compression, T4Options, FillOrder, RowsPerStrip);
			break;
	}
	if (RowBuffer)
	{
		Decompressor->SetOutputBuffer(RowBuffer);
	}
}
void
G42ImageReaderTiffBase::ReadImageRows(void)
{
	if (!File && StripOffset >= TotalFileRead)
	{
		NeedMoreData = true;
		return;
	}
	if (!Decompressor) // uncompressed mode
	{
		if (File)
		{
			File->SetPosition(StripOffset);
			while (!Finished && ReadMode == ReadImageRowsMode)
			{
				File->Read(RowPtr, RowBytesLeft, false);
				OutputRow();
			}
		}
		else
		{
			while (!Finished && ReadMode == ReadImageRowsMode)
			{
				uint32 size = RowBytesLeft;
				if (!LoadBuffer(StripOffset, size))
				{
					NeedMoreData = true;
					return;
				}
				if (NextByte && SavedBufferLeft)
				{
					PutSequence(NextByte, SavedBufferLeft);
				}
				if (CurrentBuffer && BufferLeft && ReadMode == ReadImageRowsMode)
				{
					PutSequence(CurrentBuffer, BufferLeft);
				}
				StripBytesLeft -= size;
				StripOffset += size;
			}
		}
		return;
	}
	uint32 size = TotalFileRead - StripOffset;
	//if (size > StripBytesLeft)
		//size = StripBytesLeft;
	if (size > 32767)
		size = 32767;
	if (!LoadBuffer(StripOffset, size))
	{
		NeedMoreData = true;
		return;
	}
	if (NextByte && SavedBufferLeft)
	{
		Decompressor->ProcessBuffer(NextByte, SavedBufferLeft);
	}
	if (CurrentBuffer && BufferLeft && ReadMode == ReadImageRowsMode)
	{
		Decompressor->ProcessBuffer(CurrentBuffer, BufferLeft);
	}
	StripBytesLeft -= size;
	StripOffset += size;
	if (!size)
	{
		ReadMode = ReadImageStripMode;
	}
	//if (!StripBytesLeft)
	//{
		//ReadMode = ReadImageStripMode;
	//}
}
bool
G42ImageReaderTiffBase::LoadBuffer(uint32 offset, uint32 buffer_size)
{
	if (File)
	{
		if (buffer_size > MaxSavedBuffer)
		{
			delete [] SavedBuffer;
			SavedBuffer = new byte [buffer_size];
			G42ThrowIfNull(SavedBuffer);
			MaxSavedBuffer = buffer_size;
		}
		File->SetPosition(offset);
		File->Read(SavedBuffer, buffer_size);
		BufferSize = SavedBufferLeft = buffer_size;
		BufferLeft = 0;
		NextByte = SavedBuffer;
		return true;
	}
	if (offset + buffer_size > TotalFileRead)
	{
		WaitMode = ReadMode;
		ReadMode = ReadWaitMode;
		WaitOffset = offset + buffer_size;
		return false;
	}
	intf block = offset / (uint32)(StoreBufferSize);
	uint32 block_offset = offset % (uint32)(StoreBufferSize);
	if (block_offset + buffer_size > (uint32)StoreBufferSize)
	{
//		NextByte = StoreBlockArray[block]->GetPtr() + block_offset;
		NextByte = StoreBlockArray[block] + block_offset;
		SavedBufferLeft = (uint32)StoreBufferSize - block_offset;
//		CurrentBuffer = StoreBlockArray[block + 1]->GetPtr();
		CurrentBuffer = StoreBlockArray[block + 1];
		BufferLeft = (size_t)(buffer_size - (uint32)SavedBufferLeft);
		BufferSize = buffer_size;
	}
	else
	{
//		NextByte = StoreBlockArray[block]->GetPtr() + block_offset;
		NextByte = StoreBlockArray[block] + block_offset;
		SavedBufferLeft = 0;
		BufferLeft = buffer_size;
		BufferSize = BufferLeft;
		CurrentBuffer = NextByte;
	}
   return true;
}
void
G42ImageReaderTiffBase::ClearBuffer(void)
{
	if (LastStoreBlock > 0)
	{
		for (intf i = 1; i <= LastStoreBlock; i++)
			delete[] StoreBlockArray[i];
		LastStoreBlock = 0;
	}
	StoreBufferNext = 0;
	StoreBufferLeft = StoreBufferSize;
   TotalFileRead = 0;
}
void
G42ImageReaderTiffBase::AddBuffer(const byte * buffer, uint32 size)
{
	OrigBuffer = buffer;
	OrigBufferSize = size;
	if (TiffSkip)
	{
		if (TiffSkip >= size)
		{
			TiffSkip -= size;
			return;
		}
		size -= TiffSkip;
		buffer += TiffSkip;
		TiffSkip = 0;
	}
	while (size)
	{
		if (!StoreBufferLeft)
		{
			LastStoreBlock++;
			StoreBufferNext = 0;
			StoreBufferLeft = StoreBufferSize;
			if (LastStoreBlock >= MaxStoreBlock)
			{
				byte ** temp = StoreBlockArray;
				intf new_max = LastStoreBlock + 16;
//				StoreBlockArray = new G42ImageTile * [new_max];
				StoreBlockArray = new byte * [new_max];
				if (!StoreBlockArray)
					return;
				if (temp)
				{
					memcpy(StoreBlockArray, temp,
						LastStoreBlock * sizeof (byte *));
					delete[] temp;
				}
				MaxStoreBlock = new_max;
			}
			StoreBlockArray[LastStoreBlock] = new byte [StoreBufferSize];
		}
		uintf copy_size;
		if (size < StoreBufferLeft)
			copy_size = size;
		else
			copy_size = StoreBufferLeft;
//		byte * block = StoreBlockArray[LastStoreBlock]->GetPtr();
		byte * block = StoreBlockArray[LastStoreBlock];
//		StoreBlockArray[LastStoreBlock]->MarkModified();
		memcpy(block + StoreBufferNext, buffer, copy_size);
		size -= copy_size;
		buffer += copy_size;
		StoreBufferNext += (uint16)copy_size;
		StoreBufferLeft -= (uint16)copy_size;
		TotalFileRead += copy_size;
	}
}
void
G42ImageReaderTiffBase::ForceFileEnd(void)
{
	if (CurrentRow < Height && Compression >= 2 && Compression <= 4)
	{
		while (CurrentRow < Height)
			PutByteRun(0xff, RowBytes); // Will eventually call OutputRow, which incs CurrentRow
	}
	else
	{
		G42ImageReaderBase::ForceFileEnd();
	}
}
G42ImageReaderTiff::G42ImageReaderTiff(void)
{
	SetInfo(G42TiffTypeId, "tif;tiff", "Tiff", "", 0);
}
G42ImageReaderTiff::G42ImageReaderTiff(
	G42HeapManager & heap, G42Notifier & notifier, G42File * file,
	G42BaseOptions * options)
:
	G42ImageReaderTiffBase(heap, notifier, file, options)
{
	SetInfo(G42TiffTypeId, "tif;tiff", "Tiff", "", 0);
}
G42ImageReaderTiff::~G42ImageReaderTiff()
{
}
bool
G42ImageReaderTiff::TestType(void)
{
	File->Rewind();
	byte buf[4];
	File->Read(buf, 4);
	File->Rewind();
	return TestType(buf, 4);
}
bool
G42ImageReaderTiff::TestType(const byte * buffer, uint32 buffer_size)
{
	if ((buffer[0] == 'I') &&
		(buffer_size < 1 || buffer[1] == 'I') &&
		(buffer_size < 2 || buffer[2] == 42) &&
		(buffer_size < 3 || buffer[3] == 0))
		return true;
	if ((buffer[0] == 'M') &&
		(buffer_size < 1 || buffer[1] == 'M') &&
		(buffer_size < 2 || buffer[2] == 0) &&
		(buffer_size < 3 || buffer[3] == 42))
		return true;
	return false;
}
G42ImageReaderEps::G42ImageReaderEps(void)
{
	SetInfo(G42EpsTypeId, "eps", "Eps", "\305\320\323\306", 4); // 0xc5d0d3c6
}
G42ImageReaderEps::G42ImageReaderEps(
	G42HeapManager & heap, G42Notifier & notifier, G42File * file,
	G42BaseOptions * options)
:
	G42ImageReaderTiffBase(heap, notifier, file, options)
{
	SetInfo(G42EpsTypeId, "eps", "Eps", "\305\320\323\306", 4); // 0xc5d0d3c6
}
G42ImageReaderEps::~G42ImageReaderEps()
{
}
