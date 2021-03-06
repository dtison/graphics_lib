// giwgif.cpp - Gif File
/*************************************************************
	File:          g42gif.cpp
   Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of g42ribase that handles GIF files
   Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
      Code   Date   Name and Description
***************************************************************/
#include "giwgif.h"
#include "giadesc.h"
G42ImageWriterGif::G42ImageWriterGif(G42Notifier & notifier,
	G42BaseOptions * options)
:
	G42ImageWriterBase(notifier, options),
	Lzw(0), CurrentPass(0), Progressive(false),
	TempRow(0), MultiImageMode(false)
{
	SetInfo(G42GifTypeId, "gif", "Gif", true);
}
G42ImageWriterGif::G42ImageWriterGif(void)
:
	G42ImageWriterBase(), Lzw(0), CurrentPass(0), Progressive(false),
	TempRow(0), MultiImageMode(false)
{
	SetInfo(G42GifTypeId, "gif", "Gif", true);
}
G42ImageWriterGif::~G42ImageWriterGif()
{
	if (TempRow)
	{
		delete [] TempRow;
		TempRow = 0;
	}
	if (Lzw)
	{
		delete Lzw;
		Lzw = 0;
	}
	Valid = false;
}
bool
G42ImageWriterGif::CanDoMultiple(void)
{
	return true;
}
bool
G42ImageWriterGif::WriteMultiInfo(G42MultipleImageDescriptor * desc)
{
	G42AnimatedImageDescriptor * info =
		dynamic_cast<G42AnimatedImageDescriptor *>(desc);
	if (!info) // can't do other because no width or height
	{
		return false;
	}
	if (!info->GetNumPalette()) // temporary can't handle non-paletted images
	{
		return false;
	}
	MultiImageMode = true;
	uintf num_palette = 0;
	TransparentPixel = 0;
	if (info->GetNumPalette() && info->GetPalette())
	{
		num_palette = info->GetNumPalette();
		if (info->HasBackgroundPixel())
		{
			TransparentPixel = info->GetBackgroundPixel();
		}
	}
#if 0
	else
	{
		num_palette = (1 << info->PixelDepth);
		if (info->HasBackground && info->BackgroundPixel < 0)
		{
			TransparentPixel = (byte)(
				((uint32)info->BackgroundColor.red * (uint32)(num_palette - 1)) /
				(uint32)255);
			if (TransparentPixel >= (num_palette >> 1))
			{
				OpaquePixel = TransparentPixel - 1;
			}
			else
			{
				OpaquePixel = TransparentPixel + 1;
			}
		}
		else
		{
			TransparentPixel = info->BackgroundPixel;
			OpaquePixel = TransparentPixel;
		}
	}
#endif
	uintf code_depth = 2;
	while (code_depth < 8 && num_palette > (1 << code_depth))
		code_depth++;
	IntegerFormat = Intel;
	PutBytes((const byte *)"GIF89a", 6);
	PutUInt16((uint16)info->GetWidth());
	PutUInt16((uint16)info->GetHeight());
	PutByte((byte)(0x80 | (code_depth - 1)));
	PutByte(TransparentPixel);
	PutByte(0);
//	if (1)
	{
		const G42Color * pal = info->GetPalette();
		byte buf[3];
		for (intf i = 0; i < (1 << code_depth); i++)
		{
			if (info->HasBackgroundPixel() && i == TransparentPixel)
			{
				buf[0] = info->GetBackgroundColor().Red;
				buf[1] = info->GetBackgroundColor().Green;
				buf[2] = info->GetBackgroundColor().Blue;
			}
			else if (i < num_palette)
			{
				buf[0] = pal[i].Red;
				buf[1] = pal[i].Green;
				buf[2] = pal[i].Blue;
			}
			else
			{
				buf[0] = buf[1] = buf[2] = 0;
			}
			PutBytes(buf, 3);
		}
	}
#if 0
	else // grayscale palette
	{
		byte buf[3];
		if (info->PixelDepth == 1)
		{
			buf[0] = buf[1] = buf[2] = 0;
			PutBytes(buf, 3);
			buf[0] = buf[1] = buf[2] = 0xff;
			PutBytes(buf, 3);
			buf[0] = buf[1] = buf[2] = 0;
			PutBytes(buf, 3);
			PutBytes(buf, 3);
		}
		else if (info->PixelDepth == 4)
		{
			for (intf i = 0; i < 16; i++)
			{
				buf[0] = buf[1] = buf[2] = (i | (i << 4));
				PutBytes(buf, 3);
			}
		}
		else
		{
			for (intf i = 0; i < 256; i++)
			{
				buf[0] = buf[1] = buf[2] = i;
				PutBytes(buf, 3);
			}
		}
	}
#endif
	if (info->GetRepeat())
	{
		PutByte(0x21);
		PutByte(0xff);
		PutByte(11);
		PutBytes((const byte *)"NETSCAPE2.0", 11);
		PutByte(3);
		PutByte(1);
		PutUInt16(0); // number of times to repeat, unsupported as of yet
		PutByte(0);
	}
	return Valid;
}
bool
G42ImageWriterGif::WriteInfo(G42ImageDescriptor * info)
{
	if (info->GetDepth() > 8) // GIF's must be 8 bits or less
	{
		Valid = false;
		return false;
	}
	G42ImageWriterBase::WriteInfo(info); // set up base stuff
	RowBytes = Width; // always use depth of 8, even if less
	Transparent = info->IsTransparent();
	if (info->IsPaletted())
	{
		if (info->IsTransparent() && !info->HasTransparentPixel())
		{
			TransparentPixel = info->GetNumPalette();
			OpaquePixel = (byte)(TransparentPixel - 1);
		}
		else
		{
			TransparentPixel = info->GetTransparentPixel();
			OpaquePixel = TransparentPixel;
		}
	}
	else
	{
		TransparentPixel = info->GetTransparentColor().Red; // grayscale, better be ok
		TransparentPixel = (byte)(((int32)TransparentPixel *
			(int32)((1 << Depth) - 1)) / (int32)(255));
		if (TransparentPixel >= (1 << (Depth - 1)))
			OpaquePixel = (byte)(TransparentPixel - 1);
		else
			OpaquePixel = (byte)(TransparentPixel + 1);
	}
	if (!Transparent)
		TransparentPixel = 0;
	if (!Options || !dynamic_cast<G42ImageReadWriteOptions *>(Options))
	{
		G42ImageReadWriteOptions * options =
			new G42ImageReadWriteOptions(G42GifTypeId);
		G42ThrowIfNull(options);
		AskForOptions(options);
	}
	G42ImageReadWriteOptions * options;
	if (Options)
	{
		options = dynamic_cast<G42ImageReadWriteOptions *>(Options);
	}
	if (options)
	{
		Progressive = options->IsProgressive();
	}
	else
	{
		Progressive = false;
	}
	if (Height < 5) // just being safe
	{
		Progressive = false;
	}
	uintf num_palette;
	if (!info->IsColor())
	{
		num_palette = (1 << Depth);
	}
	else
	{
		num_palette = info->GetNumPalette();
		if (Transparent && TransparentPixel == num_palette)
			num_palette++;
		if (num_palette > 256)
		{
			Valid = false;
			return false;
		}
	}
	CodeDepth = 2;
	while (CodeDepth < 8 && num_palette > (1 << CodeDepth))
	{
		CodeDepth++;
	}
	TempRow = new byte [Width];
	G42ThrowIfNull(TempRow);
	if (!MultiImageMode)
	{
		IntegerFormat = Intel;
//		if (Transparent) //  || info->NumComments) // put back when we have comments
			PutBytes((const byte *)"GIF89a", 6);
//		else
//			PutBytes((const byte *)"GIF87a", 6);
		PutUInt16((uint16)Width);
		PutUInt16((uint16)Height);
		PutByte((byte)(0x80 | (CodeDepth - 1)));
		PutByte(TransparentPixel);
		PutByte(0);
		if (!info->IsColor())
		{
			byte buf[3];
			if (Depth == 1)
			{
				buf[0] = buf[1] = buf[2] = 0;
				PutBytes(buf, 3);
				buf[0] = buf[1] = buf[2] = 0xff;
				PutBytes(buf, 3);
				buf[0] = buf[1] = buf[2] = 0;
				PutBytes(buf, 3);
				PutBytes(buf, 3);
			}
			else if (Depth == 4)
			{
				for (intf i = 0; i < 16; i++)
				{
					buf[0] = buf[1] = buf[2] = (byte)(i | (i << 4));
					PutBytes(buf, 3);
				}
			}
			else
			{
				for (intf i = 0; i < 256; i++)
				{
					buf[0] = buf[1] = buf[2] = (byte)i;
					PutBytes(buf, 3);
				}
			}
		}
		else
		{
			const G42Color * pal = info->GetPalette();
			byte buf[3];
			for (intf i = 0; i < (1 << CodeDepth); i++)
			{
				if (Transparent && i == TransparentPixel)
				{
					buf[0] = info->GetTransparentColor().Red;
					buf[1] = info->GetTransparentColor().Green;
					buf[2] = info->GetTransparentColor().Blue;
				}
				else if (i < num_palette)
				{
					buf[0] = pal[i].Red;
					buf[1] = pal[i].Green;
					buf[2] = pal[i].Blue;
				}
				else
				{
					buf[0] = buf[1] = buf[2] = 0;
				}
				PutBytes(buf, 3);
			}
		}
	}
#if 0 // put back when we get comments
	if (info->NumComments)
	{
		for (intf i = 0; i < info->NumComments; i++)
		{
			PutByte(0x21);
			PutByte(0xfe);
			G42Comment * comment = info->Comments[i];
			const char * text = comment->GetComment();
			intf len = strlen(text);
			while (len)
			{
				intf size = min(len, 255);
				PutByte((byte)size);
				PutBytes(text, size);
				len -= size;
				comment += size;
			}
			PutByte(0);
		}
	}
#endif
	NeedLocalImage = true;
	return Valid;
}
void
G42ImageWriterGif::WriteLocalImage(void)
{
	if (!NeedLocalImage)
	{
		return;
	}
	if (Descriptor && (Descriptor->IsTransparent() || Frame))
	{
		PutByte(0x21);
		PutByte(0xf9);
		PutByte(4);
		intf packed = 0;
		if (Descriptor->IsTransparent())
			packed = 1;
		if (Frame)
		{
			if (Frame->GetUserInput())
			{
				packed |= 2;
			}
			if (Frame->GetReplace() == G42AnimatedImageFrame::Background)
			{
				packed |= 8;
			}
			else if (Frame->GetReplace() == G42AnimatedImageFrame::Behind)
			{
				packed |= 12;
			}
			else
			{
				packed |= 4;
			}
		}
		PutByte((byte)packed);
		if (Frame && Frame->GetDelay() > 0)
		{
			uint32 delay = Frame->GetDelay() / 10;
			if (delay < 1)
				delay = 1;
			if (delay > 65535)
				delay = 65535;
			PutUInt16((uint16)delay);
		}
		else
		{
			PutByte(0);
			PutByte(0);
		}
		PutByte(TransparentPixel);
		PutByte(0);
	}
	PutByte(',');
	if (Frame)
	{
		PutUInt16((uint16)Frame->GetOffset().X);
		PutUInt16((uint16)Frame->GetOffset().Y);
	}
	else
	{
		PutUInt16((uint16)0);
		PutUInt16((uint16)0);
	}
	PutUInt16((uint16)Width);
	PutUInt16((uint16)Height);
	PutByte((byte)(Progressive ? 0x40 : 0));
	Lzw = new G42LzwCompressor(*this, G42LzwCompressor::LzwGif, CodeDepth);
	PutByte((byte)(Lzw->GetCodeDepth()));
	NeedLocalImage = false;
}
bool
G42ImageWriterGif::WriteRow(const byte * row, const byte * mask)
{
	WriteLocalImage();
	
//	const byte * buffer;
	if (Depth < 8)
	{
//		buffer = TempRow;
		if (Depth == 1)
		{
			const byte * sp = row;
			byte * dp = TempRow;
			intf m = 0x80;
			for (uint32 i = 0; i < Width; i++)
			{
				if (*sp & m)
					*dp++ = 1;
				else
					*dp++ = 0;
				if (m == 1)
				{
					m = 0x80;
               sp++;
				}
				else
					m >>= 1;
			}
		}
		else if (Depth == 4)
		{
			const byte * sp = row;
			byte * dp = TempRow;
			intf s = 4;
			for (uint32 i = 0; i < Width; i++)
			{
				*dp++ = (byte)((*sp >> s) & 0xf);
				if (!s)
				{
					s = 4;
					sp++;
				}
				else
					s = 0;
			}
		}
	}
	else
	{
		memcpy(TempRow, row, Width);
	}
	if (Transparent && mask)
	{
		byte * sp = TempRow;
		const byte * mp = mask;
		intf m = 0x80;
		for (uint32 i = 0; i < Width; i++)
		{
			if (!(*mp & m))
				*sp = TransparentPixel;
			else if (*sp == TransparentPixel)
				*sp = OpaquePixel;
			sp++;
			if (m == 1)
			{
				m = 0x80;
				mp++;
			}
			else
			{
				m >>= 1;
			}
		}
	}
	Lzw->ProcessBuffer(TempRow, Width);  // this calls PutBytes directly
	AdvanceCurrentRow();
	return Valid;
}
bool
G42ImageWriterGif::WriteEnd(void)
{
	Lzw->Flush();
	PutByte(0);  // empty data block
	if (!MultiImageMode)
	{
		PutByte(';'); // end of file
		return G42ImageWriterBase::WriteEnd();
	}
	if (TempRow)
	{
		delete [] TempRow;
		TempRow = 0;
	}
	if (Lzw)
	{
		delete Lzw;
      Lzw = 0;
	}
	CurrentPass = 0;
	CurrentRow = 0;
	
	return Valid;
}
bool
G42ImageWriterGif::WriteMultiEnd(G42MultipleImageDescriptor *)
{
	PutByte(';'); // end of file
	G42ImageWriterBase::WriteEnd();
	return Valid;
}
void
G42ImageWriterGif::AdvanceCurrentRow(void)
{
	if (Progressive)
	{
		switch (CurrentPass)
		{
		case 0:
			CurrentRow += 8;
			if (CurrentRow >= Height)
			{
				CurrentRow = 4;
				CurrentPass++;
			}
			break;
		case 1:
			CurrentRow += 8;
			if (CurrentRow >= Height)
			{
				CurrentRow = 2;
				CurrentPass++;
			}
			break;
		case 2:
			CurrentRow += 4;
			if (CurrentRow >= Height)
			{
				CurrentRow = 1;
				CurrentPass++;
			}
			break;
		case 3:
			CurrentRow += 2;
			break;
		}
	}
	else
	{
		CurrentRow++;
	}
}
void
G42ImageWriterGif::CompressorOutputBuffer(byte * buffer, uintf size)
{
	while (size)
	{
		uintf block_size = min(size, (uintf)255);
		PutByte((byte)block_size);
		PutBytes(buffer, block_size);
		buffer += block_size;
		size -= block_size;
	}
}
