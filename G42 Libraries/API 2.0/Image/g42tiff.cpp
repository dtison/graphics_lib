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
#include "g42tiff.h"
#include "g42iter.h"
G42TiffImage::G42TiffImage(G42RasterImage & parent) :
	G42RasterBase(parent), ReadMode(ReadHeaderMode),
	CurrentRow(0), Uncompress(0), StoreBlockArray(0),
	BitsPerSample(1), Compression(NoCompression),
	ColorType(-1), RowsPerStrip(0x7fff), TiffSkip(0),
	SamplesPerPixel(1), StripArray(0), StripSizeArray(0),
	Predictor(PredictionNone), MaxStoreBlock(0), LastStoreBlock(-1),
	StoreBufferLeft(0), TotalFileRead(0), StoreBufferSize(65528), // memory_block_size),
	StoreBufferNext(0), WaitOffset(0), T4Options(0), T6Options(0),
	FillOrder(1), TiffRowBuffer(0), InternalRowBytes(0)
{
	Valid = true;
}
G42TiffImage::~G42TiffImage()
{
	SavedBuffer = 0; // don't delete this, as it points to a block
	if (Uncompress)
		delete Uncompress;
	if (StoreBlockArray)
	{
		for (int i = 0; i <= LastStoreBlock; i++)
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
G42TiffImage::OutputRow(void)
{
	CurrentRow++;
	G42RasterBase::OutputRow();
	StripRow--;
	if (!StripRow)
	{
		ReadMode = ReadImageStripMode;
	}
	if (CurrentRow >= Height)
	{
		EatBuffer();
		ReadMode = ReadDoneMode;
		ForceFileEnd();
	}
}
void
G42TiffImage::ProcessData(byte * buffer, size_t buffer_size)
{
	AddBuffer(buffer, buffer_size);
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
G42TiffImage::ReadHeader(void)
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
		Valid = false;
		ReadMode = ReadErrorMode;
		return;
	}
	if (GetByte() != 0xd0)
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		return;
	}
	if (GetByte() != 0xd3)
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		return;
	}
	if (GetByte() != 0xc6)
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		return;
	}
	SkipData(16);
	TiffSkip = GetUInt32(Intel);
	if (TiffSkip)
	{
		if (TotalFileRead > TiffSkip)
		{
			int skip = OrigBufferSize - (TotalFileRead - TiffSkip);
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
		Valid = false;
		ReadMode = ReadErrorMode;
	}
}
void
G42TiffImage::ReadTiffHeader(void)
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
		Valid = false;
		ReadMode = ReadErrorMode;
		return;
	}
	if (GetByte() != order)
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		return;
	}
	if (GetUInt16() != 42)
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		return;
	}
	IFDOffset = GetUInt32();
	ReadMode = ReadIFDMode;
}
void
G42TiffImage::ReadIFD(void)
{
	if (!LoadBuffer(IFDOffset, 2))
		return;
	NumIFDEntries = GetUInt16();
	CurIFDEntry = 0;
	ReadMode = ReadIFDEntryMode;
	IFDOffset += 2;
}
void
G42TiffImage::ReadIFDEntry(void)
{
	if (!LoadBuffer(IFDOffset, 12))
		return;
	Tag = GetUInt16();
	Type = GetUInt16();
	Count = (uint)GetUInt32();
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
				for (int i = 0; i < Count; i++)
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
				for (int i = 0; i < Count; i++)
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
				ImagePalette = new G42Color [NumPalette];
				G42Color * pal = new G42Color[NumPalette]; // need to read in palette
				if (!pal)
				{
					Valid = false;
					ReadMode = ReadErrorMode;
					return;
				}
				Info.ImagePalette.Set(pal);
				ReadMode = ReadColorMapMode;
				ArrayOffset = GetUInt32();
				return; // don't finish
			}
			break;
	}
	FinishIFDEntry();
}
void
G42TiffImage::ReadStripOffsets(void)
{
	if (!LoadBuffer(ArrayOffset, Count * GetValueSize()))
		return;
	for (int i = 0; i < Count; i++)
		StripArray[i] = GetTiffValue();
	ReadMode = ReadIFDEntryMode;
	FinishIFDEntry();
}
void
G42TiffImage::ReadColorMap(void)
{
	if (!LoadBuffer(ArrayOffset, Count * GetValueSize()))
		return;
	int i;
	for (i = 0; i < NumPalette; i++)
		ImagePalette[i].red = (GetTiffValue() >> 8);
	for (i = 0; i < NumPalette; i++)
		ImagePalette[i].green = (GetTiffValue() >> 8);
	for (i = 0; i < NumPalette; i++)
		ImagePalette[i].blue = (GetTiffValue() >> 8);
	for (i = 0; i < NumPalette; i++)
		((G42Color *)(Info.ImagePalette))[i] = ImagePalette[i];
	ReadMode = ReadIFDEntryMode;
	FinishIFDEntry();
}
void
G42TiffImage::ReadStripByteCounts(void)
{
	if (!LoadBuffer(ArrayOffset, Count * GetValueSize()))
		return;
	for (int i = 0; i < Count; i++)
		StripSizeArray[i] = GetTiffValue();
	ReadMode = ReadIFDEntryMode;
	FinishIFDEntry();
}
void
G42TiffImage::ReadBitsPerSample(void)
{
	if (!LoadBuffer(ArrayOffset, GetValueSize()))
		return;
	BitsPerSample = GetTiffValue();
	ReadMode = ReadIFDEntryMode;
	FinishIFDEntry();
}
int32
G42TiffImage::GetTiffValue(void)
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
int
G42TiffImage::GetValueSize(void)
{
	int size;
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
G42TiffImage::NeedTiffPtr(void)
{
	int size = GetValueSize() * Count;
	return (size > 4);
}
void
G42TiffImage::FinishIFDEntry(void)
{
	CurIFDEntry++;
	if (CurIFDEntry >= NumIFDEntries)
	{
		if (!StripArray || !Width || !Height || ColorType < 0)
		{
			Valid = false;
			ReadMode = ReadErrorMode;
			return;
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
			Valid = false;
			ReadMode = ReadErrorMode;
			return;
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
		InternalRowBytes = (int)(((int32)Width * (int32)Depth + 7) >> 3);
		if (Depth > 8)
		{
			RowBytes = TrueColorPixelSize * Width;
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
				TrueRowBytes = (int)(((int32)SamplesPerPixel * (int32)Width *
					(int32)TiffDepth + 7) >> 3);
		}
		else
		{
			TrueRowBytes = (int)(((int32)Width * (int32)TiffDepth + 7) >> 3);
		}
		if (Depth > 8)
			Depth = TrueColorPixelSize * 8;
		
		uint buffer_size = max((uint)RowBytes, (uint)TrueRowBytes);
		buffer_size = max(buffer_size, (uint)InternalRowBytes);
		
		TiffRowBuffer = new byte [buffer_size];
		TopLineFirst = true; // TIFF's are top down files
		if ((Compression < NoCompression || Compression > LZW) &&
			Compression != PackBits)
		{
			Valid = false;
			ReadMode = ReadErrorMode;
			return;
		}
		Info.Width = Width;
		Info.Height = Height;
		Info.PixelDepth = (Depth == 24) ? TrueColorPixelSize * 8 : Depth;
		Info.ImageType = Tiff;
		if (Compression == NoCompression)
			Info.Compressed = false;
		else
      {
      	// Changed 06/07/96 by JRP
      	// All Compressions seemed to be Packbits, so I changed this hoping
         // it would solve that problem.
#if 0
			Info.Compressed = true;
#else
			if (Compression == PackBits)
         	Info.Compressed = 1;
         else
				Info.Compressed = Compression;
#endif
      }
		if (ColorType == ColorTypeRGB)
			Info.ColorType = G42ImageInfo::ColorTypeTrueColor;
		else if (ColorType == ColorTypePalette)
			Info.ColorType = G42ImageInfo::ColorTypePalette;
		else
			Info.ColorType = G42ImageInfo::ColorTypeGrayscale;
		if (NumPalette)
		{
			G42Color * pal = new G42Color[NumPalette]; // need to read in palette
			if (!pal)
			{
				Valid = false;
				ReadMode = ReadErrorMode;
				return;
			}
			for (int i = 0; i < NumPalette; i++)
			{
				pal[i] = ImagePalette[i];
			}
			Info.ImagePalette.Set(pal);
         Info.NumPalette = NumPalette;
		}
		Parent.HasInfo(Info);         // give user the file dimensions
		ReadMode = ReadImageStripMode;
		CurrentStrip = -1;
		StripRow = RowsPerStrip;
	}
	IFDOffset += 12;
}
void
G42TiffImage::ReadImageStrip(void)
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
	if (Uncompress)
		delete Uncompress;
	switch (Compression)
	{
   	default:
		case NoCompression:
			Uncompress = new G42TiffNoCompression(this, TrueRowBytes);
			break;
		case LZW:
			Uncompress = new G42TiffLzw(this, TrueRowBytes);
			break;
		case PackBits:
			Uncompress = new G42TiffPackBits(this, TrueRowBytes);
			break;
		case CCITT:
		case Group3:
		case Group4:
			Uncompress = new G42TiffCcitt(this, Width * (1 + ExtraSamples),
				Compression, T4Options, FillOrder, RowsPerStrip);
			break;
	}
}
void
G42TiffImage::ReadImageRows(void)
{
	if (StripOffset >= TotalFileRead)
	{
		NeedMoreData = true;
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
		Uncompress->ProcessBuffer(NextByte, SavedBufferLeft);
	}
	if (CurrentBuffer && BufferLeft && ReadMode == ReadImageRowsMode)
	{
		Uncompress->ProcessBuffer(CurrentBuffer, BufferLeft);
	}
	StripBytesLeft -= size;
	StripOffset += size;
	//if (!StripBytesLeft)
	//{
		//ReadMode = ReadImageStripMode;
	//}
}
bool
G42TiffImage::LoadBuffer(uint32 offset, uint32 buffer_size)
{
	if (offset + buffer_size > TotalFileRead)
	{
		WaitMode = ReadMode;
		ReadMode = ReadWaitMode;
		WaitOffset = offset + buffer_size;
		return false;
	}
	int block = offset / (uint32)(StoreBufferSize);
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
G42TiffImage::ClearBuffer(void)
{
	if (LastStoreBlock > 0)
	{
		for (int i = 1; i <= LastStoreBlock; i++)
			delete[] StoreBlockArray[i];
		LastStoreBlock = 0;
	}
	StoreBufferNext = 0;
	StoreBufferLeft = StoreBufferSize;
   TotalFileRead = 0;
}
void
G42TiffImage::AddBuffer(byte * buffer, uint32 size)
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
				int new_max = LastStoreBlock + 16;
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
		uint copy_size;
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
		StoreBufferNext += copy_size;
		StoreBufferLeft -= copy_size;
		TotalFileRead += copy_size;
	}
}
void
G42TiffImage::ForceFileEnd(void)
{
	if (CurrentRow < Height && Compression >= 2 && Compression <= 4)
   {
		while (CurrentRow < Height)
	      PutByteRun(0xff, RowBytes); // Will eventually call OutputRow, which incs CurrentRow
   }
   else
		G42RasterBase::ForceFileEnd();
}
void
G42TiffImage::ProcessRow(byte * row)
{
	if (ReadMode != ReadImageRowsMode)
		return;
	memcpy(TiffRowBuffer, row, TrueRowBytes);
	if (Predictor == PredictionHorizontal)
	{
		switch (TiffDepth)
		{
			case 1:
			{
				byte * dp = TiffRowBuffer + (SamplesPerPixel >> 3);
				int ds = (7 - (SamplesPerPixel & 7));
				byte * sp = TiffRowBuffer;
				int ss = 7;
				for (int i = SamplesPerPixel; i < (TrueRowBytes << 3); i++)
				{
					int sv = ((*sp) >> ss) & 0x1;
					int dv = ((*dp) >> ds) & 0x1;
					dv += sv;
					dv &= 0x1;
					if (dv)
						*dp |= (1 << ds);
					else
						*dp &= ~(1 << ds);
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
				byte * dp = TiffRowBuffer + (SamplesPerPixel >> 2);
				int ds = ((3 - (SamplesPerPixel & 3)) << 1);
				byte * sp = TiffRowBuffer;
				int ss = 2;
				for (int i = SamplesPerPixel; i < (TrueRowBytes << 1); i++)
				{
					int sv = ((*sp) >> ss) & 0x3;
					int dv = ((*dp) >> ds) & 0x3;
					dv += sv;
					dv &= 0x3;
					*dp &= (0x3 << (6 - ds));
					*dp |= (dv << ds);
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
				byte * dp = TiffRowBuffer + (SamplesPerPixel >> 1);
				int ds = ((1 - (SamplesPerPixel & 1)) << 2);
				byte * sp = TiffRowBuffer;
				int ss = 4;
				for (int i = SamplesPerPixel; i < (TrueRowBytes << 1); i++)
				{
					int sv = ((*sp) >> ss) & 0xf;
					int dv = ((*dp) >> ds) & 0xf;
					dv += sv;
					dv &= 0xf;
					*dp &= (0xf << (4 - ds));
					*dp |= (dv << ds);
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
				byte * dp = TiffRowBuffer + SamplesPerPixel;
				byte * sp = TiffRowBuffer;
				for (int i = SamplesPerPixel; i < TrueRowBytes; i++)
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
				byte * sp = TiffRowBuffer;
				byte * dp = TiffRowBuffer;
				for (int i = 0; i < Width; i++)
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
				byte * sp = TiffRowBuffer;
				byte * dp = TiffRowBuffer;
				for (int i = 0; i < Width; i++)
				{
					*(dp++) = *(sp++);
					sp += ExtraSamples;
				}
			}
			break;
			case 4:
			{
				byte * sp = TiffRowBuffer + ((ExtraSamples + 1) >> 1);
				byte * dp = TiffRowBuffer;
            *dp &= 0xf0; // second nibble is unused anyway
				int ss = ((ExtraSamples & 1) << 2);
				int ds = 0;
				for (int i = 1; i < Width; i++)
				{
					int v = ((*sp) >> ss) & 0xf;
					*dp |= (v << ds);
					if (ds)
						ds = 0;
					else
					{
						ds = 4;
						dp++;
						*dp = 0;
					}
					for (int j = 0; j < ExtraSamples + 1; j++)
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
				byte * sp = TiffRowBuffer;
				byte * dp = TiffRowBuffer;
				int ss = 6;
				int ds = 6;
				for (int i = 0; i < Width; i++)
				{
					int v = ((*sp) >> ss) & 0x3;
					*dp &= (0xff & (0xff3f << (6 - ds)));
					*dp |= (v << ds);
					if (ds)
						ds -= 2;
					else
					{
						ds = 6;
						dp++;
					}
					for (int j = 0; j < ExtraSamples + 1; j++)
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
				byte * sp = TiffRowBuffer;
				byte * dp = TiffRowBuffer;
				int ss = 7;
            int ds = 7;
				for (int i = 0; i < Width; i++)
				{
					if (*sp & ss)
						*dp |= (1 << ds);
					else
						*dp &= ~(1 << ds);
					if (ds)
						ds--;
					else
					{
						ds = 7;
						dp++;
					}
					for (int j = 0; j < ExtraSamples + 1; j++)
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
	if (TiffDepth == 2)
	{
		// expand to depth of 4
		byte * sp = TiffRowBuffer + ((Width - 1) >> 2);
		byte * dp = TiffRowBuffer + ((Width - 1) >> 1);
		int ss = ((3 - ((Width - 1) & 3)) << 1);
		int ds = ((1 - ((Width - 1) & 1)) << 2);
		for (int i = 0; i < Width; i++)
		{
			int v = ((*sp >> ss) & 3);
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
	if (ColorType == ColorTypeWhiteIsZero)
	{
		byte * sp = TiffRowBuffer;
		for (int i = 0; i < RowBytes; i++)
		{
			*sp = ~(*sp);
			sp++;
		}
	}
	if (Depth > 8)
	{
		byte * sp = TiffRowBuffer + (Width - 1) * 3;
		G42Byte24BitIterator dp(TiffRowBuffer);
		dp.Forward(Width - 1);
		for (uint i = 0; i < Width; i++)
		{
			dp.Set(*(sp + 2), *(sp + 1), *sp);
			dp.Decrement();
			sp -= 3;
		}
	}
	PutSequence(TiffRowBuffer, RowBytes);
}
// key for Tiff files is II or MM, followed by a 42 as an integer
bool G42IsTiff(byte * buffer, size_t buffer_size)
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
	if ((buffer[0] == 0xc5) &&
		(buffer_size < 1 || buffer[1] == 0xd0) &&
		(buffer_size < 2 || buffer[2] == 0xd3) &&
		(buffer_size < 3 || buffer[3] == 0xc6))
		return true;
	return false;
}
// key for Eps files is 0xc5d0d3c6
bool G42IsEps(byte * buffer, size_t buffer_size)
{
	if ((buffer[0] == 0xc5) &&
		(buffer_size < 1 || buffer[1] == 0xd0) &&
		(buffer_size < 2 || buffer[2] == 0xd3) &&
		(buffer_size < 3 || buffer[3] == 0xc6))
		return true;
	return false;
}
