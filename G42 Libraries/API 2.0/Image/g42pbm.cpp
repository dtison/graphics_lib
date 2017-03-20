// g42pbm.cpp - Pbm File
/*************************************************************
	File:          g42pbm.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of g42ribase that handles Pbm files
   Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
      Code   Date   Name and Description
***************************************************************/
#include "g42itype.h"
#include "g42pbm.h"
#include "g42idata.h"
#include "g42iter.h"
G42PbmImage::G42PbmImage(G42RasterImage & parent) :
	G42RasterBase(parent), ReadMode(ReadHeaderMode),
	CurrentRow(0), CurrentValue(0)
{
}
G42PbmImage::~G42PbmImage()
{
}
// count rows, and mark done when last row is output
void
G42PbmImage::OutputRow(void)
{
	if (Depth > 8)
	{
		byte * sp = RowBuffer + (size_t)(Width - 1) * 3;
		G42Byte24BitIterator dp(RowBuffer);
		dp.Forward((size_t)Width - 1);
		for (uint i = 0; i < Width; i++)
		{
			dp.Set(*(sp + 2), *(sp + 1), *sp);
			dp.Decrement();
			sp -= 3;
		}
	}
	G42RasterBase::OutputRow();
	CurrentRow++;
	if (CurrentRow >= Height)
	{
		EatBuffer();
		ReadMode = ReadDoneMode;
		ForceFileEnd();
	}
}
void
G42PbmImage::ProcessData(byte * buffer, size_t buffer_size)
{
	RestoreBuffer(buffer, buffer_size);
	while (BufferSize)
	{
		switch (ReadMode)
		{
			case ReadHeaderMode:
				ReadHeader();
				break;
			case ReadWidthMode:
				ReadWidth();
				break;
			case ReadHeightMode:
				ReadHeight();
				break;
			case ReadDepthMode:
				ReadDepth();
				break;
			case ReadInfoMode:
				ReadInfo();
				break;
			case ReadImageMode:
				ReadImage();
				break;
			default:
				EatBuffer();
				break;
		}
   }
}
void
G42PbmImage::ReadHeader(void)
{
   Valid = true;
	if (BufferSize < 2) // 2 is worst case behavior
	{
		SaveBuffer();
		return;
	}
	IntegerFormat = Motorola;
	// check header ('P')
	if (GetByte() != 'P')
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		return;
	}
	int version_num = GetByte();
	if (version_num < '1' || version_num > '6')
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		return;
	}
	Version = version_num - '0';
	if (Version == ColorAscii ||
			Version == ColorBinary)
		Color = true;
	else
		Color = false;
	CurrentValue = 0;
	ReadMode = ReadWidthMode;
   Valid = true;
}
void
G42PbmImage::ReadWidth(void)
{
	while (BufferSize)
	{
		char c = GetByte();
		if (c >= '0' && c <= '9')
		{
			CurrentValue *= 10;
			CurrentValue += c - '0';
		}
		else
		{
			if (CurrentValue)
			{
				Width = (int)CurrentValue;
				ReadMode = ReadHeightMode;
				CurrentValue = 0;
				break;
			}
		}
	}
}
void
G42PbmImage::ReadHeight(void)
{
	while (BufferSize)
	{
		char c = GetByte();
		if (c >= '0' && c <= '9')
		{
			CurrentValue *= 10;
			CurrentValue += c - '0';
		}
		else
		{
			if (CurrentValue)
			{
				Height = (int)CurrentValue;
				if (Version == MonochromeAscii ||
					Version == MonochromeBinary)
				{
					Depth = 1;
               MaxValue = 1;
					ReadMode = ReadInfoMode;
				}
				else
				{
					ReadMode = ReadDepthMode;
					CurrentValue = 0;
				}
				break;
			}
		}
	}
}
void
G42PbmImage::ReadDepth(void)
{
	while (BufferSize)
	{
		char c = GetByte();
		if (c >= '0' && c <= '9')
		{
			CurrentValue *= 10;
			CurrentValue += c - '0';
		}
		else
		{
			if (CurrentValue)
			{
				MaxValue = CurrentValue;
				if (MaxValue >= 16)
					Depth = 8;
				else if (MaxValue >= 4)
					Depth = 4;
//				else if (MaxValue >= 2)
//					Depth = 2;
				else
					Depth = 1;
				MaxDepth = ((1 << Depth) - 1);
				if (Version == ColorAscii || Version == ColorBinary)
				{
					Depth = 24;
					MaxDepth = 255;
				}
				ReadMode = ReadInfoMode;
				break;
			}
		}
	}
}
void
G42PbmImage::ReadInfo(void)
{
	NumPalette = 0;
	RowBytes = ((Width * Depth + 7) >> 3);
	TopLineFirst = true; // pbm's are top down files
	ImagePalette = 0;
	Valid = true;
	Info.Width = Width;
	Info.Height = Height;
	if (Depth > 8)
	{
		Info.ColorType = G42ImageInfo::ColorTypeTrueColor;
		Info.PixelDepth = 8 * TrueColorPixelSize;
	}
	else
	{
		Info.ColorType = G42ImageInfo::ColorTypeGrayscale;
		Info.PixelDepth = Depth;
	}
	if (Version < MonochromeBinary)
		Info.ImageType = PbmAscii;
	else
		Info.ImageType = PbmBinary;
	Info.Compressed = false;
	ReadMode = ReadImageMode; // no palette, so info is complete
	Parent.HasInfo(Info);         // give user the file dimensions
	CurrentShift = (8 - Depth);
	CurrentByte = 0;
	CurrentValue = 0;
}
void
G42PbmImage::ReadImage(void)
{
	if (Version < MonochromeBinary)
	{
		while (BufferSize)
		{
			char c = GetByte();
			if (c >= '0' && c <= '9')
			{
				CurrentValue *= 10;
				CurrentValue += c - '0';
			}
			else
			{
				if (MaxValue != MaxDepth)
				{
					CurrentValue = (CurrentValue *
						(uint32)(MaxDepth)) /
						(uint32)(MaxValue);
				}
				switch (Depth)
				{
					case 1:
					{
						CurrentByte |= (byte)(CurrentValue << CurrentShift);
						if (!CurrentShift)
						{
							CurrentShift = 7;
							PutByte(CurrentByte);
							  CurrentByte = 0;
						}
						else
							CurrentShift--;
						break;
					}
					case 2:
					{
						CurrentByte |= (byte)(CurrentValue << CurrentShift);
						if (!CurrentShift)
						{
							CurrentShift = 6;
							PutByte(CurrentByte);
							CurrentByte = 0;
						}
						else
							CurrentShift -= 2;
						break;
					}
					case 4:
					{
						CurrentByte |= (byte)(CurrentValue << CurrentShift);
						if (!CurrentShift)
						{
							CurrentShift = 4;
							PutByte(CurrentByte);
							CurrentByte = 0;
						}
						else
							CurrentShift -= 4;
						break;
					}
					case 8:
					case 24:
					case 32:
					{
						PutByte(CurrentValue);
						break;
					}
				}
				CurrentValue = 0;
			}
		}
	}
	else
	{
		while (BufferSize)
		{
			CurrentValue = GetByte();
			if (MaxValue != MaxDepth)
			{
				CurrentValue = (CurrentValue *
					(uint32)(MaxDepth)) /
					(uint32)(MaxValue);
			}
			switch (Depth)
			{
				case 1:
				{
					CurrentByte |= (byte)(CurrentValue << CurrentShift);
					if (!CurrentShift)
					{
						CurrentShift = 7;
						PutByte(CurrentByte);
						CurrentByte = 0;
					}
					else
						CurrentShift--;
					break;
				}
				case 2:
				{
					CurrentByte |= (byte)(CurrentValue << CurrentShift);
					if (!CurrentShift)
					{
						CurrentShift = 6;
						PutByte(CurrentByte);
						CurrentByte = 0;
					}
					else
						CurrentShift -= 2;
					break;
				}
				case 4:
				{
					CurrentByte |= (byte)(CurrentValue << CurrentShift);
					if (!CurrentShift)
					{
						CurrentShift = 4;
						PutByte(CurrentByte);
						CurrentByte = 0;
					}
					else
						CurrentShift -= 4;
					break;
				}
				case 8:
				case 24:
				case 32:
				{
					PutByte((byte)CurrentValue);
					break;
				}
			}
		}
	}
}
// key for bmp files is the letters 'P1' - 'P6' in the first two characters
bool G42IsPbm(byte * buffer, size_t buffer_size)
{
	if ((buffer[0] == 'P') &&
		(buffer_size < 1 ||
		(buffer[1] >= '1' && buffer[1] <= '6')))
		return true;
	return false;
}
bool G42IsPbmAscii(byte * buffer, size_t buffer_size)
{
	if ((buffer[0] == 'P') &&
		(buffer_size < 1 ||
		(buffer[1] >= '1' && buffer[1] <= '3')))
		return true;
	return false;
}
