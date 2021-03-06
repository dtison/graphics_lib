// ribase.cpp - raster image base class
/*************************************************************
	File:          g42ribas.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Base class for reading raster files
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "g42itype.h"
#include "g42ribas.h"
// this initializes everything to a standard default value
G42RasterBase::G42RasterBase(G42RasterImage & parent) :
	Parent(parent), Valid(false), Finished(false), Width(0), Height(0), Depth(0),
   NumSavedBuffer(0), MaxSavedBuffer(0), BufferSize(0), BufferLeft(0),
   SavedBufferLeft(0), SavedBuffer(0), CurrentBuffer(0), NextByte(0),
   IntegerFormat(Intel), RowBuffer(0), RowPtr(0), RowBytesLeft(0),
	RowSize(0), ImagePalette(0), NumPalette(0), Color(false), TopLineFirst(true),
	Transparent(false)
{
}
G42RasterBase::~G42RasterBase()
{
   if (SavedBuffer)
      delete[] SavedBuffer;
   if (RowBuffer)
		delete[] RowBuffer;
	if (ImagePalette)
		delete[] ImagePalette;
}
bool G42RasterBase::IsValid(void)
{
   return Valid;
}
bool G42RasterBase::IsFinished(void)
{
	return Finished;
}
uint32 G42RasterBase::GetWidth(void)
{
   return Width;
}
uint32 G42RasterBase::GetHeight(void)
{
   return Height;
}
G42ImageInfo & G42RasterBase::GetInfo(void)
{
	return Info;
}
void
G42RasterBase::ForceFileEnd(void)
{
	if (!Finished)
	{
		Finished = true;
		Parent.HasEnd(Info);
	}
}
void G42RasterBase::FillInfo(void) // for those who don't support yet
{
	Info.Width = Width;
	Info.Height = Height;
	Info.PixelDepth = Depth;
	if (Depth > 8)
		Info.ColorType = G42ImageInfo::ColorTypeTrueColor;
	else if (Color)
		Info.ColorType = G42ImageInfo::ColorTypePalette;
	else
		Info.ColorType = G42ImageInfo::ColorTypeGrayscale;
	Info.NumPalette = NumPalette;
	if (NumPalette)
	{
		G42Color * pal = new G42Color [NumPalette];
		if (pal)
		{
			for (int i = 0; i < NumPalette; i++)
			{
				pal[i] = ImagePalette[i];
			}
			Info.ImagePalette.Set(pal);
		}
	}
}
uint G42RasterBase::GetDepth(void)
{
	return Depth;
}
uint G42RasterBase::GetNumPalette(void)
{
	return NumPalette;
}
G42Color * G42RasterBase::GetPalette(void)
{
   return ImagePalette;
}
bool G42RasterBase::IsColor(void)
{
	return Color;
}
bool G42RasterBase::IsTransparent(void)
{
	return Transparent;
}
bool G42RasterBase::TopToBottom(void)
{
   return TopLineFirst;
}
// setup input buffers
void
G42RasterBase::RestoreBuffer(byte * buffer, size_t buffer_size)
{
	CurrentBuffer = buffer;
	BufferLeft = buffer_size;
	BufferSize = BufferLeft + SavedBufferLeft;
   if (SavedBufferLeft)
      NextByte = SavedBuffer;
   else
      NextByte = CurrentBuffer;
}
// save any remaining bytes in the input buffer for next time
void
G42RasterBase::SaveBuffer(void)
{
	if (SavedBufferLeft)
	{
		byte * sp;
		byte * dp;
		int i;
		for (i = 0, sp = NextByte, dp = SavedBuffer;
			i < SavedBufferLeft; i++, sp++, dp++)
			*dp = *sp;
      NumSavedBuffer = SavedBufferLeft;
   }
   else
	{
      NumSavedBuffer = 0;
	}
   if (BufferSize)
   {
      if (BufferSize > MaxSavedBuffer)
      {
         byte * temp = SavedBuffer;
         SavedBuffer = new byte [BufferSize];
         if (NumSavedBuffer && temp)
            memcpy(SavedBuffer, temp, NumSavedBuffer);
         if (temp)
            delete[] temp;
      }
      if (NumSavedBuffer)
         memcpy(SavedBuffer + NumSavedBuffer, CurrentBuffer,
            BufferLeft);
		else
         memcpy(SavedBuffer, NextByte, BufferLeft);
		NumSavedBuffer = SavedBufferLeft = BufferSize;
	}
	else
	{
		SavedBufferLeft = 0;
		NumSavedBuffer = 0;
	}
	BufferLeft = BufferSize = 0;
}
// get a 32 bit integer in requested format.  Note that this
// works no matter what the native processor format is
uint32 G42RasterBase::GetUInt32(G42IntType type)
{
	if (type == Default)
		type = IntegerFormat;
	uint32 ret;
	if (type == Motorola)
	{
		ret = ((uint32)GetByte()) << 24;
		ret |= ((uint32)GetByte()) << 16;
		ret |= ((uint32)GetByte()) << 8;
		ret |= ((uint32)GetByte());
	}
	else
	{
		ret = ((uint32)GetByte());
		ret |= ((uint32)GetByte()) << 8;
		ret |= ((uint32)GetByte()) << 16;
		ret |= ((uint32)GetByte()) << 24;
	}
	return ret;
}
// get a 16 bit integer in requested format.  Note that this
// works no matter what the native processor format is
uint16 G42RasterBase::GetUInt16(G42IntType type)
{
   if (type == Default)
      type = IntegerFormat;
   uint16 ret;
   if (type == Motorola)
   {
      ret = ((uint16)GetByte()) << 8;
		ret |= ((uint16)GetByte());
   }
   else
   {
      ret = ((uint16)GetByte());
      ret |= ((uint16)GetByte()) << 8;
   }
	return ret;
}
// chew up the remaining bytes in the input buffer
void G42RasterBase::EatBuffer(void)
{
   BufferLeft = 0;
   BufferSize = 0;
   SavedBufferLeft = 0;
}
// initialize the row data
void G42RasterBase::InitRow(void)
{
	RowSize = Width * 4 + 4; // will have to take into account 48 bit stuff
	RowBuffer = new byte [RowSize];
	RowPtr = RowBuffer;
	RowBytesLeft = RowBytes;
	if (TopLineFirst)
		CurRowNum = 0;
	else
		CurRowNum = Height - 1;
}
// this outputs a run of bytes into the output buffer and
// outputs rows when necessary
void G42RasterBase::PutByteRun(byte value, int number)
{
	if (!RowPtr) InitRow();
   while (number)
   {
      int size;
      if (number > RowBytesLeft)
         size = RowBytesLeft;
      else
         size = number;
      memset(RowPtr, value, size);
		RowPtr += size;
		number -= size;
      RowBytesLeft -= size;
      if (!RowBytesLeft)
         OutputRow();
	}
}
// this puts a number of bytes into the output buffer and
// outputs rows when necessary
void G42RasterBase::PutSequence(byte *buffer, int buffer_size)
{
   if (!RowPtr) InitRow();
   byte *bptr = buffer;
   while (buffer_size)
   {
      int size;
		if (buffer_size > RowBytesLeft)
         size = RowBytesLeft;
      else
         size = buffer_size;
		memcpy(RowPtr, bptr, size);
      RowPtr += size;
      RowBytesLeft -= size;
		bptr += size;
      buffer_size -= size;
		if (!RowBytesLeft)
         OutputRow();
	}
}
// output a row to the parent class
void G42RasterBase::OutputRow(void)
{
	Parent.HasRow(RowBuffer, CurRowNum, 0, Width);
	RowPtr = RowBuffer;
	RowBytesLeft = RowBytes;
	if (TopLineFirst)
	{
		CurRowNum++;
		if (CurRowNum >= Height)
      	EndPass();
	}
	else
	{
		if (CurRowNum)
			CurRowNum--;
		else
			EndPass();
	}
}
void G42RasterBase::EndPass(void) // for simple files
{
	ForceFileEnd();
}
void G42RasterBase::FillBuffer(byte * buffer, int size)
{
	if (SavedBufferLeft)
	{
		uint copy_size = min((uint)size, SavedBufferLeft);
		memcpy(buffer, SavedBuffer, copy_size);
		size -= copy_size;
		SavedBufferLeft -= copy_size;
		BufferSize -= copy_size;
		if (SavedBufferLeft)
			NextByte += copy_size;
		else
			NextByte = CurrentBuffer;
	}
	if (size)
	{
		uint copy_size = min((uint)size, BufferSize);
		memcpy(buffer, NextByte, copy_size);
		BufferSize -= copy_size;
		NextByte += copy_size;
		BufferLeft -= copy_size;
	}
}
// first do any saved data, then do the new data
void G42RasterBase::ProcessUncompressedData()
{
	if (SavedBufferLeft)
	{
		PutSequence(NextByte, SavedBufferLeft);
		NextByte = CurrentBuffer;
	}
	PutSequence(NextByte, BufferLeft);
	EatBuffer(); // mark all the buffers as used
}
void
G42RasterBase::CombineRow(byte * old_row, byte * new_row, int32 rowbytes)
{
	memcpy(old_row, new_row, rowbytes);
}
void
G42RasterBase::CombineMask(byte * old_row, byte * new_row, int32 rowbytes)
{
	memcpy(old_row, new_row, rowbytes);
}
