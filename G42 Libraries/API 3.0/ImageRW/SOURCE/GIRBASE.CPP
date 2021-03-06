// girbase.cpp - raster image base class
/*************************************************************
	File:          girbase.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Base class for reading raster files
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code	Date				Name	Description
				18 Dec 1996		Guy	Modified for 3.0 libraries
***************************************************************/
#include "girbase.h"
// this initializes everything to a standard default value
G42ImageReaderBase::G42ImageReaderBase(G42HeapManager & heap,
	G42Notifier & notifier, G42File * file, G42BaseOptions * options)
:
	G42BaseReader(heap, notifier, file, options),
	Finished(false), Width(0), Height(0), Depth(0), NumPalette(0),
	NumSavedBuffer(0), MaxSavedBuffer(0), BufferSize(0), BufferLeft(0),
	SavedBufferLeft(0), SavedBuffer(0), CurrentBuffer(0), NextByte(0),
	IntegerFormat(Intel), RowBuffer(0), RowPtr(0), RowBytesLeft(0),
	RowSize(0), TopLineFirst(true), Compressed(false),
	Transparent(false), BeginSent(false), BlockDataSending(false),
	ReadRowMode(AutoDetect), NumPasses(1), DeleteRowBuffer(false),
	Progressive(false), OutputTransRowPtr(0), AlphaChannel(false),
	Background(false), TransparentPixel(-1), BackgroundPixel(-1),
	TrueColorDepth(24), ReadOptions(0), Color(0), MultiNumImages(0),
	MultiDescriptorSent(false), WriteOptionsSent(false)
{
}
G42ImageReaderBase::G42ImageReaderBase(void)
:
	G42BaseReader(),
	Finished(false), Width(0), Height(0), Depth(0), NumPalette(0),
	NumSavedBuffer(0), MaxSavedBuffer(0), BufferSize(0), BufferLeft(0),
	SavedBufferLeft(0), SavedBuffer(0), CurrentBuffer(0), NextByte(0),
	IntegerFormat(Intel), RowBuffer(0), RowPtr(0), RowBytesLeft(0),
	RowSize(0), TopLineFirst(true), Compressed(false),
	Transparent(false), BeginSent(false), BlockDataSending(false),
	ReadRowMode(AutoDetect), NumPasses(1), DeleteRowBuffer(false),
	Progressive(false), OutputTransRowPtr(0), AlphaChannel(false),
	Background(false), TransparentPixel(-1), BackgroundPixel(-1),
	TrueColorDepth(24), ReadOptions(0), Color(0), MultiNumImages(0),
	MultiDescriptorSent(false), WriteOptionsSent(false)
{
}
G42ImageReaderBase::~G42ImageReaderBase()
{
	if (SavedBuffer)
		delete[] SavedBuffer;
	if (RowBuffer && DeleteRowBuffer)
		delete[] RowBuffer;
	CurLock = 0;
	CurData = 0;
}
void
G42ImageReaderBase::SetInfo(intf type,
	const char * extension, const char * name,
	const char * pattern, intf pattern_length, bool last)
{
	Info.Set(new G42ReaderInfo(type, extension, name,
		#if defined(G42_IN_DLL)
		true, dll name,
		#else
		false, 0,
		#endif
		3, 0, G42RWInfo::Image, pattern, pattern_length, last));
}
bool G42ImageReaderBase::IsFinished(void)
{
	return Finished;
}
void
G42ImageReaderBase::ForceFileEnd(void)
{
	if (!Finished)
	{
		Finished = true;
		CleanupData();
		SendEnd();
	}
}
bool
G42ImageReaderBase::CheckBuffer(uint32 size, bool force)
{
	if (size <= BufferSize)
	{
		return true;
	}
	if (!force || !File)
	{
		return false;
	}
	if (size + SavedBufferLeft > MaxSavedBuffer)
	{
		byte * temp = SavedBuffer;
		SavedBuffer = new byte [size + SavedBufferLeft];
		MaxSavedBuffer = size + SavedBufferLeft;
		if (SavedBufferLeft && NextByte)
		{
			memcpy(SavedBuffer, NextByte, SavedBufferLeft);
		}
		if (temp)
		{
			delete[] temp;
		}
	}
	else if (SavedBufferLeft)
	{
		const byte * sp = NextByte;
		byte * dp = SavedBuffer;
		for (uint32 i = 0; i < SavedBufferLeft; i++)
		{
			*dp++ = *sp++;
		}
	}
	uint32 read = File->Read(SavedBuffer + SavedBufferLeft, size, false);
	NumSavedBuffer = SavedBufferLeft = size + SavedBufferLeft;
	NextByte = SavedBuffer;
	BufferSize = SavedBufferLeft + BufferLeft; // since File, BufferLeft == 0
	if (NumSavedBuffer < size)
	{
		return false;
	}
	return true;
}
// old way to setup input buffers
void
G42ImageReaderBase::RestoreBuffer(const byte * /* buffer */,
	size_t /* buffer_size */ )
{
/*
	CurrentBuffer = buffer;
	BufferLeft = buffer_size;
	BufferSize = BufferLeft + SavedBufferLeft;
	if (SavedBufferLeft)
		NextByte = SavedBuffer;
	else
		NextByte = CurrentBuffer;
*/
}
// old way to save any remaining bytes in the input buffer for next time
void
G42ImageReaderBase::SaveBuffer(void)
{
	BufferSize = 0; // ProcessData uses BufferLeft to determine how much of
						// the buffer has been used
/*
	if (SavedBufferLeft)
	{
		const byte * sp;
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
*/
}
uint32
G42ImageReaderBase::ProcessData(const byte * buffer, uint32 buffer_size)
{
	CurrentBuffer = buffer;
	BufferLeft = buffer_size;
	BufferSize = BufferLeft;
	NextByte = CurrentBuffer;
	ProcessStreamData();
	BytesUsed = buffer_size - BufferLeft; // if we used it all, BufferLeft == 0
	return BytesUsed;
}
// get a 32 bit integer in requested format.  Note that this
// works no matter what the native processor format is
uint32
G42ImageReaderBase::GetUInt32(G42IntType type)
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
uint16 G42ImageReaderBase::GetUInt16(G42IntType type)
{
   if (type == Default)
      type = IntegerFormat;
   uint16 ret;
	if (type == Motorola)
	{
      ret = (uint16)(((uint16)GetByte()) << 8);
		ret |= ((uint16)GetByte());
   }
	else
	{
		ret = ((uint16)GetByte());
		ret |= (uint16)(((uint16)GetByte()) << 8);
	}
	return ret;
}
// chew up the remaining bytes in the input buffer
void G42ImageReaderBase::EatBuffer(void)
{
	BufferLeft = 0;
	BufferSize = 0;
	SavedBufferLeft = 0;
}
// initialize the row data
void G42ImageReaderBase::InitRow(void)
{
	if (ReadRowMode == AutoDetect)
	{
		if (RowBytes == Descriptor->GetRowBytes() &&
#ifdef MSWIN
			!TopLineFirst)
#else
			TopLineFirst)
#endif
		{
			ReadRowMode = ReadStrip;
		}
		else if (RowBytes <= Descriptor->GetRowBytes())
		{
			ReadRowMode = ReadRow;
		}
		else
		{
			ReadRowMode = UseTemporaryBuffer;
		}
	}
	CurPass = 0;
	SetupData();
	if (ReadRowMode == UseTemporaryBuffer)
	{
		RowBuffer = new byte [RowBytes];
		DeleteRowBuffer = true;
		if (Decompressor)
		{
			Decompressor->SetOutputBuffer(RowBuffer);
		}
	}
	else
	{
		RowBuffer = OutputRowPtr;
		DeleteRowBuffer = false;
		if (Decompressor)
		{
			Decompressor->SetOutputBuffer(RowBuffer);
		}
	}
	RowSize = RowBytes;
	RowPtr = RowBuffer;
	RowBytesLeft = RowBytes;
}
// this outputs a run of bytes into the output buffer and
// outputs rows when necessary
void G42ImageReaderBase::PutByteRun(byte value, int number)
{
	if (!RowPtr)
	{
		InitRow();
	}
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
void G42ImageReaderBase::PutSequence(const byte *buffer, int buffer_size)
{
	if (!RowPtr)
	{
		InitRow();
	}
	const byte * bptr = buffer;
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
// override for DecompressorObject
bool
G42ImageReaderBase::DecompressorOutputRow(byte *)
{
	OutputRow();
	return Finished;
}
// output a row to the data object
void
G42ImageReaderBase::OutputRow(void)
{
	ConvertRow();
	NextRow();
}
void G42ImageReaderBase::EndPass(void) // for simple files
{
	CurPass++;
	if (NumPasses && CurPass >= NumPasses)
	{
		ForceFileEnd();
	}
	else
	{
		SetupData();
	}
}
void G42ImageReaderBase::FillBuffer(byte * buffer, int size)
{
	if (SavedBufferLeft)
	{
		uintf copy_size = min((uintf)size, SavedBufferLeft);
		memcpy(buffer, NextByte, copy_size);
		size -= copy_size;
		buffer += copy_size;
		SavedBufferLeft -= copy_size;
		BufferSize -= copy_size;
		if (SavedBufferLeft)
		{
			NextByte += copy_size;
		}
		else
		{
			NextByte = CurrentBuffer;
		}
	}
	if (size && BufferSize)
	{
		uintf copy_size = min((uintf)size, BufferSize);
		memcpy(buffer, NextByte, copy_size);
		BufferSize -= copy_size;
		NextByte += copy_size;
		BufferLeft -= copy_size;
		buffer += copy_size;
		size -= copy_size;
	}
	if (size && File)
	{
		File->Read(buffer, size, false);
//		uintf copy_size = File->Read(buffer, size, false);
		// really should do something with the return value
	}
}
// first do any saved data, then do the new data
void G42ImageReaderBase::ProcessUncompressedData()
{
	if (File)
	{
		if (SavedBufferLeft)
		{
			PutSequence(NextByte, SavedBufferLeft);
			NextByte = CurrentBuffer;
		}
		if (BufferLeft)
		{
			PutSequence(NextByte, BufferLeft);
		}
		EatBuffer(); // mark all the buffers as used
		if (ReadRowMode == ReadStrip)
		{
			while (!Finished)
			{
#ifdef MSWIN // upside down images
				File->Read(RowPtr, RowBytesLeft + RowBytes * CurStripRow, false);
				int num_rows = CurStripRow + 1;
				for (intf i = 0; i < num_rows; i++)
				{
					RowBuffer = OutputRowPtr;
					DeleteRowBuffer = false;
					OutputRow();
				}
#else // right side up images
#endif
			}
		}
		else // row at a time
		{
			while (!Finished)
			{
				File->Read(RowPtr, RowBytesLeft, false);
				OutputRow();
			}
		}
	}
	else
	{
		if (SavedBufferLeft)
		{
			PutSequence(NextByte, SavedBufferLeft);
			NextByte = CurrentBuffer;
		}
		PutSequence(NextByte, BufferLeft);
		EatBuffer(); // mark all the buffers as used
	}
}
// first do any saved data, then do the new data
// note: Must have a Decompressor set
void
G42ImageReaderBase::ProcessCompressedData(void)
{
	if (SavedBufferLeft)
	{
		uintf handled = Decompressor->ProcessBuffer(NextByte, SavedBufferLeft);
		if (handled == SavedBufferLeft)
		{
			BufferSize -= handled;
			SavedBufferLeft = 0;
			NextByte = CurrentBuffer;
		}
		else
		{
			NextByte += handled;
			SavedBufferLeft -= handled;
			BufferSize -= handled;
			return;
		}
	}
	if (BufferLeft)
	{
		uintf handled = Decompressor->ProcessBuffer(NextByte, BufferLeft);
		if (handled == BufferLeft)
		{
			BufferSize = 0;
			BufferLeft = 0;
		}
		else
		{
			NextByte += handled;
			BufferLeft -= handled;
			BufferSize -= handled;
			return;
		}
	}
	if (File)
	{
		uintf buffer_size = 32767;
		G42APtr<byte> buffer(new byte [buffer_size]);
		while (!Finished)
		{
			uintf size = File->Read(buffer, buffer_size, false);
			if (!size)
			{
				break;
			}
			uintf handled = Decompressor->ProcessBuffer(buffer, size);
			if (handled < buffer_size)
			{
				SavedBufferLeft = buffer_size - handled;
				if (SavedBufferLeft > MaxSavedBuffer)
				{
					delete [] SavedBuffer;
					SavedBuffer = new byte [SavedBufferLeft];
					G42ThrowIfNull(SavedBuffer);
					MaxSavedBuffer = SavedBufferLeft;
				}
				memcpy(SavedBuffer, buffer + handled, SavedBufferLeft);
				BufferSize = SavedBufferLeft + BufferLeft;
				NextByte = SavedBuffer;
				break;
			}
		}
		if (!Finished)
		{
			ForceFileEnd();
		}
	}
}
void
G42ImageReaderBase::ProcessImageData(void)
{
	if (Decompressor)
	{
		ProcessCompressedData();
	}
	else
	{
		ProcessUncompressedData();
	}
}
void
G42ImageReaderBase::SetupDescriptor(void) // creates and sends info
{
	G42ImageDescriptor::G42ImageType type =
		G42GetImageType((byte)Depth, Color);
	Descriptor = new G42ImageDescriptor(
		Width, Height, type, *Heap, Transparent);
	G42ThrowIfNull(Descriptor);
	if (NumPalette)
	{
		Descriptor->SetNumPalette((uint16)NumPalette);
	}
	ImagePalette = Descriptor->GetPalette();
	// add transparency, etc. here
	if (Transparent)
	{
		Descriptor->SetTransparency();
	}
	InitRow();
}
void
G42ImageReaderBase::SendDescriptor(void)
{
	if (!Descriptor)
	{
		throw G42InvalidParameterException();
	}
	if (Transparent)
	{
		if (TransparentPixel >= 0)
		{
			Descriptor->SetTransparentPixel((byte)TransparentPixel);
		}
		else if (AlphaChannel)
		{
			Descriptor->SetAlphaChannel();
		}
		else
		{
			Descriptor->SetTransparentColor(TransparentColor);
		}
	}
	if (Background)
	{
		if (BackgroundPixel >= 0)
		{
			Descriptor->SetBackgroundPixel((byte)BackgroundPixel);
		}
		else
		{
			Descriptor->SetBackgroundColor(BackgroundColor);
		}
	}
	if (!WriteOptionsSent)
	{
		Notifier->ProcessMessage(Info);
		Notifier->ProcessMessage(Descriptor);
		if (!WriteOptions)
		{
			WriteOptions = new G42ImageReadWriteOptions(Info->GetTypeId(),
				Compressed, TrueColorDepth, Progressive, false);
			G42ThrowIfNull(WriteOptions);
		}
		Notifier->ProcessMessage(WriteOptions);
	}
	G42ImageDescriptor * desc_copy = new G42ImageDescriptor(*Descriptor);
	G42ThrowIfNull(desc_copy);
	SendBegin();
	if (Receiver)
	{
		Receiver->Process(desc_copy, this);
	}
	if (!WriteOptionsSent)
	{
		if (Receiver)
		{
			Receiver->Process(WriteOptions.Release(), this);
		}
		WriteOptionsSent = true;
	}
}
void
G42ImageReaderBase::SendBegin(void)
{
	if (!BeginSent && Receiver)
	{
		G42PathBegin * begin = new G42PathBegin;
		G42ThrowIfNull(begin);
		Receiver->Process(begin, this);
		BeginSent = true;
	}
}
void
G42ImageReaderBase::SendEnd(void)
{
	if (Receiver)
	{
		G42PathEnd * end = new G42PathEnd;
		G42ThrowIfNull(end);
		Receiver->Process(end, this);
	}
}
void
G42ImageReaderBase::SetupMultiDescriptor(void) // creates the info and sets the palette
{
	if (!MultiDescriptorSent && !MultiDescriptor)
	{
		MultiDescriptor = new G42MultipleImageDescriptor(MultiNumImages);
		G42ThrowIfNull(MultiDescriptor);
		if (NumMultiPalette)
		{
			MultiDescriptor->SetNumPalette((uint16)NumMultiPalette);
			MultiPalette = MultiDescriptor->GetPalette();
		}
	}
}
void
G42ImageReaderBase::SendMultiDescriptor(void) // sends the info to the user;
{
	if (!MultiDescriptorSent)
	{
		SendBegin();
		if (MultiBackground)
		{
			if (MultiBackgroundPixel >= 0 && NumMultiPalette)
			{
				MultiDescriptor->SetBackgroundPixel((byte)MultiBackgroundPixel);
			}
			else
			{
				MultiDescriptor->SetBackgroundColor(MultiBackgroundColor);
			}
		}
		if (Receiver)
		{
			Receiver->Process(MultiDescriptor.Release(), this);
		}
		MultiDescriptorSent = true;
	}
}
void
G42ImageReaderBase::SetupAnimatedDescriptor(void) // creates the info and sets the palette
{
	MultiDescriptor = new G42AnimatedImageDescriptor(AnimatedWidth,
		AnimatedHeight, AnimatedRepeat, MultiNumImages);
	G42ThrowIfNull(MultiDescriptor);
	if (NumMultiPalette)
	{
		MultiDescriptor->SetNumPalette(NumMultiPalette);
		MultiPalette = MultiDescriptor->GetPalette();
	}
}
void
G42ImageReaderBase::SendAnimatedDescriptor(void) // sends the info to the user;
{
	SendMultiDescriptor();
}
void
G42ImageReaderBase::SendAnimatedFrame(void) // sends the info to the user;
{
	if (Receiver)
	{
		G42AnimatedImageFrame * frame = new G42AnimatedImageFrame(
			FrameOffset, FrameDelay, FrameUserInput, FrameReplace);
		G42ThrowIfNull(frame);
		Receiver->Process(frame, this);
	}
}
// prepare for next image
void
G42ImageReaderBase::NextImage(void)
{
	Width = 0;
	Height = 0;
	Depth = 0;
	NumPalette = 0;
	if (RowBuffer && DeleteRowBuffer)
	{
		delete[] RowBuffer;
	}
	RowBuffer = 0;
	RowPtr = 0;
	RowBytesLeft = 0;
	RowSize = 0;
	TopLineFirst = true;
	Compressed = false;
	Transparent = false;
	Color = true;
	BlockDataSending = false;
	ReadRowMode = AutoDetect;
	NumPasses = 1;
	DeleteRowBuffer = false;
	Progressive = false;
	OutputTransRowPtr = 0;
	AlphaChannel = false;
	Background = false;
	TransparentPixel = -1;
	BackgroundPixel = -1;
	TrueColorDepth = 24;
	CurRowNum = 0;
	CurPass = 0;
	OutputRowPtr = 0;
	OutputTransRowPtr = 0;
	CurStripRow = 0;
	CurRow = 0;
	CurStrip = 0;
	CurLock = 0;
	CurData = 0;
	NumStrips = 0;
	DataStorage.Clear();
	Decompressor = 0;
	Descriptor = 0;
	Data = 0;
}
void
G42ImageReaderBase::SetupData()
{
	if (TopLineFirst)
	{
		CurStrip = 0;
		GetData(0);
		CurStripRow = 0;
	}
	else
	{
		CurStrip = Descriptor->GetNumStrips() - 1;
		GetData(CurStrip);
		CurStripRow = CurData->GetNumRows() - 1;
	}
	NumStrips = Descriptor->GetNumStrips();
	OutputRowPtr = GetRow(false);
	if (Transparent)
	{
		OutputTransRowPtr = GetRow(true);
	}
}
byte *
G42ImageReaderBase::GetRow(bool trans)
{
	if (CurLock)
		return CurLock->GetRow(CurStripRow, trans);
	return 0;
}
void
G42ImageReaderBase::NextRow(void)
{
	if (TopLineFirst)
	{
		CurStripRow++;
		if (CurStripRow >= CurData->GetNumRows())
		{
			CurLock = 0;
			SendCurData();
			CurStrip++;
			if (CurStrip < NumStrips)
			{
				GetData(CurStrip);
				CurStripRow = 0;
			}
			else
			{
				EndPass();
			}
		}
	}
	else
	{
		if (CurStripRow == 0)
		{
			CurLock = 0;
			SendCurData();
			if (CurStrip > 0)
			{
				CurStrip--;
				GetData(CurStrip);
				CurStripRow = CurData->GetNumRows() - 1;
			}
			else
			{
				EndPass();
			}
		}
		else
		{
			CurStripRow--;
		}
	}
	if (!Finished && (CurPass < NumPasses || !NumPasses))
	{
		OutputRowPtr = GetRow(false);
		if (Transparent)
		{
			OutputTransRowPtr = GetRow(true);
		}
		if (ReadRowMode != UseTemporaryBuffer)
		{
			RowBuffer = OutputRowPtr;
			DeleteRowBuffer = false;
			if (Decompressor)
			{
				Decompressor->SetOutputBuffer(RowBuffer);
			}
		}
		RowPtr = RowBuffer;
		RowBytesLeft = RowBytes;
	}
}
void
G42ImageReaderBase::GetData(uint32 strip)
{
	if (!CurPass)
	{
		CurData = GetImageData(Descriptor->GetImageType(),
			*Heap, strip, Descriptor->GetRowsInStrip(strip),
			Descriptor->GetRowBytes(), Descriptor->GetStripStartInImage(strip),
			Descriptor->GetWidth(), (Descriptor->IsTransparent() ?
				Descriptor->GetTransRowBytes() : 0));
	}
	else
	{
		CurData = DataStorage[strip].Release();
	}
	CurLock.Set(new G42LockPtr(*this, *CurData));
}
void
G42ImageReaderBase::SendCurData(void)
{
	CurLock = 0;
	if (BlockDataSending || CurPass + 1 != NumPasses) // careful, NumPasses could == 0
	{
		uintf num = CurData->GetStripNum();
		DataStorage[num] = CurData.Release();
		GetReadOptions();
		if (!BlockDataSending && ReadOptions->IsDisplayingProgressively() &&
			Receiver)
		{
			Receiver->Process(DataStorage[num]->MakeCopy(), this);
		}
	}
	else if (Receiver)
	{
		Receiver->Process(CurData.Release(), this);
	}
}
void
G42ImageReaderBase::SendDataStorage(void)
{
	CurLock = 0;
	if (CurData)
	{
		uintf num = CurData->GetStripNum();
		DataStorage[num] = CurData.Release();
	}
	for (uint32 i = 0; i < Descriptor->GetNumStrips(); i++)
	{
		if (DataStorage[i] && Receiver)
		{
			Receiver->Process(DataStorage[i].Release(), this);
		}
	}
}
void
G42ImageReaderBase::CleanupData(void)
{
	if (CurData)
	{
		CurLock = 0;
		SendCurData();
	}
	if (BlockDataSending || !NumPasses)
	{
		SendDataStorage();
	}
}
// the base class does conversions from 1, 2, and 4 bit to 8 bit
void
G42ImageReaderBase::ConvertRow(void)
{
	if (Descriptor->GetDepth() == 8 && Descriptor->IsPaletted())
	{
		G42ByteIterator dp(RowBuffer, Width - 1);
		if (Depth == 4)
		{
			G42Bit4Iterator sp(RowBuffer, Width - 1);
			for (uint32 i = 0; i < Width ; i++)
			{
				*dp-- = *sp--;
			}
		}
		else if (Depth == 2)
		{
			G42Bit2Iterator sp(RowBuffer, Width - 1);
			for (uint32 i = 0; i < Width ; i++)
			{
				*dp-- = *sp--;
			}
		}
		else if (Depth == 1)
		{
			G42BitIterator sp(RowBuffer, Width - 1);
			for (uint32 i = 0; i < Width ; i++)
			{
				*dp-- = *sp--;
			}
		}
	}
	else if (Descriptor->GetDepth() == 8) // grayscale
	{
		G42ByteIterator dp(RowBuffer, Width - 1);
		if (Depth == 4)
		{
			G42Bit4Iterator sp(RowBuffer, Width - 1);
			byte map[16];
			for (intf m = 0; m < 16; m++)
			{
				map[m] = (byte)(m | (m << 4));
			}
			for (uint32 i = 0; i < Width ; i++)
			{
				intf index = *sp--;
				*dp-- = map[index];
			}
		}
		else if (Depth == 2)
		{
			G42Bit2Iterator sp(RowBuffer, Width - 1);
			byte map[4];
			map[0] = 0;
			map[1] = 0x55;
			map[2] = 0xaa;
			map[3] = 0xff;
			for (uint32 i = 0; i < Width ; i++)
			{
				intf index = *sp--;
				*dp-- = map [index];
			}
		}
	}
}
#if defined(G42_IN_RW_DLL)
#else // not in dll
G42ImageData *
G42ImageReaderBase::GetImageData(
	G42ImageDescriptor::G42ImageType type,
	G42HeapManager & heap, uint32 strip_num,
	uint32 num_rows, uint32 row_bytes, uint32 strip_start_in_image,
	uint32 width, uint32 trans_row_bytes)
{
	G42ImageData * data = 0;
	switch (type)
	{
		case G42ImageDescriptor::Color32:
			data = new G42ImageDataColor32(heap, strip_num, num_rows,
				row_bytes, strip_start_in_image, width, trans_row_bytes);
			break;
		case G42ImageDescriptor::Color24:
			data = new G42ImageDataColor24(heap, strip_num, num_rows,
				row_bytes, strip_start_in_image, width, trans_row_bytes);
			break;
		case G42ImageDescriptor::Color8:
			data = new G42ImageDataColor8(heap, strip_num, num_rows,
				row_bytes, strip_start_in_image, width, trans_row_bytes);
			break;
		case G42ImageDescriptor::Gray8:
			data = new G42ImageDataGray8(heap, strip_num, num_rows,
				row_bytes, strip_start_in_image, width, trans_row_bytes);
			break;
		case G42ImageDescriptor::Gray1:
			data = new G42ImageDataGray1(heap, strip_num, num_rows,
				row_bytes, strip_start_in_image, width, trans_row_bytes);
			break;
	}
	return data;
}
void
G42ImageReaderBase::ClearImageData(G42ImageData * data)
{
	if (data)
	{
		delete data;
	}
}
G42WriteImageData *
G42ImageReaderBase::GetWriteImageLock(G42ImageData * data)
{
	if (!data)
	{
		return 0;
	}
	G42WriteImageData * lock = new G42WriteImageData(*data);
	G42ThrowIfNull(lock);
	return lock;
}
void
G42ImageReaderBase::ClearWriteImageLock(G42WriteImageData * lock)
{
	if (lock)
	{
		delete lock;
	}
}
byte *
G42ImageReaderBase::GetRowFromLock(G42WriteImageData * data, uint32 row,
	bool trans)
{
	return data->GetRow(row, trans);
}
void
G42ImageReaderBase::GetReadOptions(void)
{
	if (ReadOptions)
	{
		return;
	}
	if (Options && dynamic_cast<G42ImageReadOptions *>(Options))
	{
		ReadOptions = dynamic_cast<G42ImageReadOptions *>(Options);
		return;
	}
	ReadOptions = new G42ImageReadOptions();
	G42ThrowIfNull(ReadOptions);
	Notifier->ProcessMessage(ReadOptions);
	Options = ReadOptions; // so it deletes properly
}
#endif // in dll
