// girgif.cpp - Gif File
/*************************************************************
	File:          girgif.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of g42ribase that handles GIF files
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "gtype.h"
#include "girgif.h"
G42ImageReaderGif::G42ImageReaderGif(void)
:
	G42ImageReaderBase(),
	ReadMode(ReadHeaderMode), GlobalPalette(0), NumImages(0),
	CodeSize(0)
{
	SetInfo(G42GifTypeId, "gif", "Gif", "GIF", 3);
}
G42ImageReaderGif::G42ImageReaderGif(G42HeapManager & heap,
	G42Notifier & notifier, G42File * file, G42BaseOptions * options)
:
	G42ImageReaderBase(heap, notifier, file, options),
	ReadMode(ReadHeaderMode), GlobalPalette(0), NumImages(0),
	CodeSize(0)
{
	SetInfo(G42GifTypeId, "gif", "Gif", "GIF", 3);
}
G42ImageReaderGif::~G42ImageReaderGif()
{
	if (GlobalPalette)
		delete [] GlobalPalette;
}
// count rows, and mark done when last row is output
bool
G42ImageReaderGif::DecompressorOutputRow(byte *)
{
	if (Transparent && OutputTransRowPtr)
	{
		byte * sp = RowBuffer;
		G42BitIterator dp(OutputTransRowPtr);
		for (intf i = 0; i < Width; i++)
		{
			*dp++ = (*sp++ != TransparentPixel);
		}
	}
	if (Progressive)
	{
		switch (CurPass)
		{
			case 0:
			{
				// if (ShowProgressive) make copy into rows 1 - 7
				for (intf i = 0; i < 8 && CurPass == 0; i++)
				{
					OutputRow();
				}
				break;
			}
			case 1:
			{
				// if (ShowProgressive) make copy into rows 1 - 3 only
				for (intf i = 0; i < 8 && CurPass == 1; i++)
				{
					OutputRow();
				}
				break;
			}
			case 2:
			{
				// if (ShowProgressive) make copy into row 1 only
				for (intf i = 0; i < 4 && CurPass == 2; i++)
				{
					OutputRow();
				}
				break;
			}
			case 3:
			{
				OutputRow();
				if (CurPass == 3)
				{
					OutputRow();
				}
				break;
			}
		}
	}
	else
	{
		OutputRow();
	}
	return (ReadMode != ReadImageMode);
}
void
G42ImageReaderGif::EndPass(void)
{
	G42ImageReaderBase::EndPass();
	// skip blank rows.  Note that this function is recursive on small images.
	if (CurPass < NumPasses)
	{
		switch (CurPass)
		{
			case 1:
			{
				intf i;
				intf n = min(4, (intf)Height);
				for (i = 0; i < n; i++)
				{
					NextRow();
				}
				break;
			}
			case 2:
			{
				intf i;
				intf n = min(2, (intf)Height);
				for (i = 0; i < n; i++)
				{
					NextRow();
				}
				break;
			}
			case 3:
			{
				NextRow();
				break;
			}
		}
	}
}
void
G42ImageReaderGif::ProcessStreamData()
{
	while (BufferSize)
	{
		switch (ReadMode)
		{
			case ReadHeaderMode:
				ReadHeader();
				break;
			case ReadGlobalPaletteMode:
				ReadGlobalPalette();
				break;
			case ReadChunkMode:
				ReadChunk();
				break;
			case ReadExtensionChunkMode:
				ReadExtensionChunk();
				break;
			case ReadCommentMode:
				ReadComment();
				break;
			case ReadUnknownSubBlockMode:
				ReadUnknownSubBlock();
				break;
			case ReadLocalImageMode:
				ReadLocalImage();
				break;
			case ReadLocalPaletteMode:
				ReadLocalPalette();
				break;
			case ReadImageMode:
				ReadImage();
				break;
			case ReadImageEndMode:
				ReadImageEnd();
				break;
			case ReadImageClearMode:
				ReadImageClear();
				break;
			default:
				EatBuffer();
				break;
		}
	}
}
void
G42ImageReaderGif::BeginProcessing()
{
	while (ReadMode != ReadDoneMode && !Finished)
	{
		switch (ReadMode)
		{
			case ReadHeaderMode:
				ReadHeader();
				break;
			case ReadGlobalPaletteMode:
				ReadGlobalPalette();
				break;
			case ReadChunkMode:
				ReadChunk();
				break;
			case ReadExtensionChunkMode:
				ReadExtensionChunk();
				break;
			case ReadCommentMode:
				ReadComment();
				break;
			case ReadUnknownSubBlockMode:
				ReadUnknownSubBlock();
				break;
			case ReadLocalImageMode:
				ReadLocalImage();
				break;
			case ReadLocalPaletteMode:
				ReadLocalPalette();
				break;
			case ReadImageMode:
				ReadImage();
				break;
			case ReadImageEndMode:
				ReadImageEnd();
				break;
			case ReadImageClearMode:
				ReadImageClear();
				break;
			default:
				ReadMode = ReadDoneMode;
				break;
		}
	}
}
void
G42ImageReaderGif::SendExtendedInfo()
{
	while (ReadMode != ReadDoneMode && !Finished)
	{
		switch (ReadMode)
		{
			case ReadHeaderMode:
				ReadHeader();
				break;
			case ReadGlobalPaletteMode:
				ReadGlobalPalette();
				break;
			case ReadChunkMode:
				ReadChunk();
				break;
			case ReadExtensionChunkMode:
				ReadExtensionChunk();
				break;
			case ReadCommentMode:
				ReadComment();
				break;
			case ReadUnknownSubBlockMode:
				ReadUnknownSubBlock();
				break;
			case ReadLocalImageMode:
				ReadLocalImage();
				break;
			case ReadLocalPaletteMode:
				ReadLocalPalette();
				break;
/* note: do we want to skip past the image to count number of images?
			case ReadImageMode:
				ReadImage();
				break;
			case ReadImageEndMode:
				ReadImageEnd();
				break;
*/
			default:
				ReadMode = ReadDoneMode;
				break;
		}
	}
}
void
G42ImageReaderGif::ReadHeader(void)
{
	if (!CheckBuffer(13, true))
	{
		SaveBuffer();
		return;
	}
	IntegerFormat = Intel;
	// check header ('GIF')
	if (GetByte() != 'G')
		throw G42IRWNotAnImageException();
	if (GetByte() != 'I')
		throw G42IRWNotAnImageException();
	if (GetByte() != 'F')
		throw G42IRWNotAnImageException();
	// skip version info
	SkipData(3);
	Width = GetUInt16();
	Height = GetUInt16();
	AnimatedWidth = Width;
	AnimatedHeight = Height;
	intf packed = GetByte();
	MultiBackgroundPixel = GetByte();
	MultiBackground = true;
	(void)GetByte();
	Depth = (packed & 7) + 1;
	NumPalette = (1 << Depth);
	if (packed & 0x80)
	{
		NumGlobalPalette = NumPalette;
	}
	else
	{
		NumGlobalPalette = 0;
	}
	GlobalDepth = Depth;
	RowBytes = Width;
	GlobalRowBytes = RowBytes;
	Color = true; // GIF's are always paletted files
	Compressed = true;
/*
	Lzw = new G42LzwDecompressor(*this, Width,
		G42LzwDecompressor::LzwGif, Depth);
	CodeSize = 0;
	Decompressor = Lzw;
*/
/*
	Info.Width = Width;
	Info.Height = Height;
	Info.PixelDepth = Depth;
	Info.HasBackground = true;
	Info.BackgroundPixel = BackgroundPixel;
	MultiInfo.BackgroundPixel = BackgroundPixel;
	Info.ImageType = Gif;
	Info.ColorType = G42ImageInfo::ColorTypePalette;
	Info.Compressed = true;
*/
	if (NumGlobalPalette)
	{
		NumMultiPalette = (uint16)NumPalette;
		GlobalPalette = new G42Color[NumGlobalPalette]; // need to read in palette
		G42ThrowIfNull(GlobalPalette);
	}
	CurrentPalette = 0;
	if (packed & 0x80)
	{
		ReadMode = ReadGlobalPaletteMode;
	}
	else
	{
		ReadMode = ReadChunkMode;
//		Parent.HasMultiInfo(MultiInfo);
	}
}
void
G42ImageReaderGif::ReadChunk(void)
{
	intf code = GetByte();
	if (code == 0x2c)
	{
		ReadMode = ReadLocalImageMode;
	}
	else if (code == 0x21)
	{
		ReadMode = ReadExtensionChunkMode;
	}
	else if (code == 0x3b)
	{
		ReadMode = ReadDoneMode;
		G42ImageReaderBase::ForceFileEnd();
	}
	else
	{
		ReadMode = ReadErrorMode;
		throw G42IRWInvalidImageException();
	}
}
void
G42ImageReaderGif::ForceFileEnd()
{
	ReadMode = ReadImageEndMode;
	CleanupData();
}
void
G42ImageReaderGif::ReadImageClear(void)
{
	if (Decompressor)
	{
//		Decompressor->Flush();
		Decompressor = 0;
		Lzw = 0;
	}
	NextImage();
	ReadMode = ReadChunkMode;
}
void
G42ImageReaderGif::TerminateStream(void)
{
	ReadImageClear();
	ReadMode = ReadDoneMode;
	G42ImageReaderBase::ForceFileEnd();
}
void
G42ImageReaderGif::ReadExtensionChunk(void)
{
	if (!CheckBuffer(13, true))
	{
		SaveBuffer();
		return;
	}
	intf label = GetByte();
	if (label == 0xf9)
	{
		SkipData();
		intf flags = GetByte();
		FrameUserInput = ((flags & 2) ? true : false);
		if ((flags & 0x1c) == 8)
			FrameReplace = G42AnimatedImageFrame::Background;
		else if ((flags & 0x1c) == 0xc)
			FrameReplace = G42AnimatedImageFrame::Behind;
		else
			FrameReplace = G42AnimatedImageFrame::None;
		FrameDelay = GetUInt16() * 10;
		if (!FrameDelay)
			FrameDelay = -1;
//		SkipData(2);
		if (flags & 1)
		{
			TransparentPixel = GetByte();
			SkipData();
			Transparent = true;
		}
		else
		{
			SkipData(2);
		}
		ReadMode = ReadChunkMode;
	}
	else if (label == 0xff) // look for Netscape repeat extension
	{
		byte length = GetByte();
		if (length != 11)
		{
			ReadMode = ReadUnknownSubBlockMode;
			SubBlockSize = length;
			return;
		}
		byte name[12];
		for (intf i = 0; i < 11; i++)
		{
			name[i] = GetByte();
		}
		name[11] = '\0';
		if (strcmp((char *)name, "NETSCAPE2.0"))
		{
			ReadMode = ReadUnknownSubBlockMode;
			SubBlockSize = 0;
			return;
		}
		length = GetByte();
		if (length != 3)
		{
			ReadMode = ReadUnknownSubBlockMode;
			SubBlockSize = length;
			return;
		}
		byte mode = GetByte();
		if (mode == 1)
		{
			AnimatedRepeat = true;
		}
		SkipData(2);
		length = GetByte();
		if (length)
		{
			ReadMode = ReadUnknownSubBlockMode;
			SubBlockSize = length;
		}
		else
		{
			ReadMode = ReadChunkMode;
		}
	}
	else if (label == 0xfe)
	{
		ReadMode = ReadCommentMode;
		CommentBlockSize = 0;
		CommentTextOffset = 0;
	}
	else
	{
		ReadMode = ReadUnknownSubBlockMode;
		SubBlockSize = 0;
	}
}
void
G42ImageReaderGif::ReadComment(void)
{
	while (BufferSize || File)
	{
		if (!CommentBlockSize)
			CommentBlockSize = GetByte();
		if (!CommentBlockSize)
		{
			#ifdef MSWIN
			// expand to 0xd 0xa pairs
			intf i = CommentTextOffset - 1;
			while (i >= 0)
			{
				if (CommentText[i] == '\n')
				{
					if (i == 0 || CommentText[i - 1] != '\r')
					{
						for (intf j = CommentTextOffset; j > i; j--)
							CommentText[j] = CommentText[j - 1];
						CommentText[i] = '\r';
						i--;
						CommentTextOffset++;
					}
					else
					{
						i -= 2;
					}
				}
				else if (CommentText[i] == '\r')
				{
					for (intf j = CommentTextOffset; j > i + 1; j--)
						CommentText[j] = CommentText[j - 1];
					CommentText[i + 1] = '\n';
					i--;
					CommentTextOffset++;
				}
				else
				{
					i--;
				}
			}
			#endif
			#ifdef MACOS
			for (intf i = 0; i < CommentTextOffset; i++)
			{
				if (CommentText[i] == '\n')
				{
					if (!i || CommentText[i - 1] != '\r')
					{
						CommentText[i] = '\r';
					}
					else
					{
						CommentTextOffset--;
						for (intf j = i - 1; j < CommentTextOffset; j++)
							CommentText[j] = CommentText[j + 1];
					}
				}
			}
			#endif
			#ifdef UNIX
			for (intf i = 0; i < CommentTextOffset; i++)
			{
				if (CommentText[i] == '\r')
				{
					if (i == CommentTextOffset - 1 || CommentText[i + 1] != '\r')
					{
						CommentText[i] = '\n';
					}
					else
					{
						CommentTextOffset--;
						for (intf j = i; j < CommentTextOffset; j++)
							CommentText[j] = CommentText[j + 1];
					}
				}
			}
			#endif
			// add comment to info
/*
			CommentText[CommentTextOffset] = '\0';
			const char * text = CommentText;
			G42Comment * comment = new G42Comment();
			comment->SetComment(text);
			Info.Comments[Info.NumComments++].Set(comment);
*/
			ReadMode = ReadChunkMode;
			break;
		}
		else
		{
			intf size = CommentBlockSize;
			if (!File && BufferSize < size)
			{
				size = BufferSize;
			}
			CommentBlockSize -= size;
			while (size)
			{
				CommentText[CommentTextOffset++] = GetByte();
				size--;
			}
		}
	}
}
// skip the sub blocks of an unknown chunk
void
G42ImageReaderGif::ReadUnknownSubBlock(void)
{
	while (BufferSize || File)
	{
		if (!SubBlockSize)
			SubBlockSize = GetByte();
		if (!SubBlockSize)
		{
			ReadMode = ReadChunkMode;
			break;
		}
		else
		{
			if (BufferSize >= SubBlockSize || File)
			{
				SkipData(SubBlockSize);
				SubBlockSize = 0;
			}
			else
			{
				SubBlockSize -= BufferSize;
				SkipData(BufferSize);
			}
		}
	}
}
void
G42ImageReaderGif::ReadLocalImage(void)
{
	if (!CheckBuffer(9, true))
	{
		SaveBuffer();
		return;
	}
	if (!MultiDescriptorSent)
	{
		SetupAnimatedDescriptor();
		if (NumGlobalPalette)
		{
			for (uintf i = 0; i < NumGlobalPalette; i++)
			{
				MultiPalette[i] = GlobalPalette[i];
			}
		}
		SendAnimatedDescriptor();
	}
	Depth = 8;
	Color = true; // GIF's are always paletted files
	Compressed = true;
	FrameOffset.X = GetUInt16();
	FrameOffset.Y = GetUInt16();
	Width = GetUInt16();
	Height = GetUInt16();
	RowBytes = Width;
	intf packed = GetByte();
	Lzw = new G42LzwDecompressor(*this, Width,
		G42LzwDecompressor::LzwGif, Depth);
	Decompressor = Lzw;
	CodeSize = 0;
	if (packed & 0x40)
	{
		Progressive = true;
		NumPasses = 4;
	}
	else
	{
		Progressive = false;
		NumPasses = 1;
	}
	if (packed & 0x80)
	{
		intf num_local = (packed & 7) + 1;
		NumPalette = num_local;
		NumPalette = (1 << NumPalette);
		SetupDescriptor();
		ReadMode = ReadLocalPaletteMode;
		CurrentPalette = 0;
	}
	else
	{
		ReadMode = ReadImageMode; // no palette, so info is complete
		NumPalette = NumGlobalPalette;
		SetupDescriptor();
		for (intf i = 0; i < NumGlobalPalette; i++)
		{
			ImagePalette[i] = GlobalPalette[i];
		}
		SendDescriptor();
		SendAnimatedFrame();
	}
}
void
G42ImageReaderGif::ReadLocalPalette(void)
{
	// read until palette is complete or buffer is empty
	CheckBuffer(NumPalette * 3, true);
	while (BufferSize >= 3 && CurrentPalette < NumPalette)
	{
		ImagePalette[CurrentPalette].Red = GetByte();
		ImagePalette[CurrentPalette].Green = GetByte();
		ImagePalette[CurrentPalette].Blue = GetByte();
		CurrentPalette++;
	}
	if (CurrentPalette == NumPalette)
	{
		ReadMode = ReadImageMode;
		SendDescriptor();
		SendAnimatedFrame();
	}
	else if (BufferSize)
	{
		SaveBuffer(); // save for rest of data
	}
}
void
G42ImageReaderGif::ReadGlobalPalette(void)
{
	// read until palette is complete or buffer is empty
	CheckBuffer(NumGlobalPalette * 3, true);
	while (BufferSize >= 3 && CurrentPalette < NumGlobalPalette)
	{
		GlobalPalette[CurrentPalette].Red = GetByte();
		GlobalPalette[CurrentPalette].Green = GetByte();
		GlobalPalette[CurrentPalette].Blue = GetByte();
		CurrentPalette++;
	}
	if (CurrentPalette == NumGlobalPalette)
	{
		ReadMode = ReadChunkMode;
	}
	else if (BufferSize)
	{
		SaveBuffer(); // save for rest of data
	}
}
void
G42ImageReaderGif::ReadImage(void)
{
	if (!BufferSize && !File)
		return;
	if (!CodeSize)
	{
		CodeSize = GetByte();
		Lzw->SetCodeSize(CodeSize);
		BlockSize = 0;
		if (!BufferSize && !File)
			return;
	}
	if (!BlockSize)
	{
		BlockSize = GetByte();
		if (!BlockSize)
		{
			SubBlockSize = 0;
			ReadMode = ReadImageClearMode;
			return;
		}
	}
	if (BlockSize && (BufferSize || File))
	{
		intf size = BlockSize;
		if (!File && BufferSize < BlockSize)
			 size = BufferSize;
		FillBuffer(InputBuffer, size);
		Decompressor->ProcessBuffer(InputBuffer, size);
		BlockSize -= size;
	}
}
void
G42ImageReaderGif::ReadImageEnd(void)
{
	if (!BufferSize && !File)
		return;
	if (!BlockSize)
	{
		BlockSize = GetByte();
		if (!BlockSize)
		{
			SubBlockSize = 0;
			ReadMode = ReadImageClearMode;
			return;
		}
	}
	if (BlockSize && (BufferSize || File))
	{
		intf size;
		if (File)
		{
			size = BlockSize;
		}
		else
		{
			size = min(BlockSize, (intf)BufferSize);
		}
		BlockSize -= size;
		SkipData(size);
	}
}
void
G42ImageReaderGif::ConvertRow()
{
#if 0
	// really, unless the image is mono, we don't need to do this
	if (Depth == 1)
	{
		byte * sp = RowBuffer;
		byte * dp = OutputRowPtr;
		intf i;
		intf m = 0x80;
		for (i = 0; i < Width; i++)
		{
			if (*sp)
				*dp |= (byte)m;
			else
				*dp &= (byte)~m;
			sp++;
			if (m == 1)
			{
				m = 0x80;
				dp++;
			}
			else
				m >>= 1;
		}
	}
	else if (Depth <= 4)
	{
		byte * sp = RowBuffer + 1;
		byte * dp = OutputRowPtr;
		(*row) <<= 4;
		intf i;
		intf s = 0;
		for (i = 1; i < Width; i++)
		{
			*dp |= (byte)(*sp << s);
			sp++;
			if (s == 0)
			{
				s = 4;
				dp++;
				*dp = 0;
			}
			else
				s = 0;
		}
	}
	G42ImageReaderBase::ConvertRow();
#endif	
}
