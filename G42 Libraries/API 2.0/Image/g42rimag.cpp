// g42rimag.cpp - raster file factory class
/*************************************************************
	File:          g42rimag.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   Determine which class a file is and build that
						raster type.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "g42itype.h"
#include "g42rimag.h"
#include "g42ribas.h"
#include "g42bmp.h"
#include "g42gif.h"
#include "g42jpeg.h"
#include "g42pcx.h"
#ifdef MACOS
#include "g42pict.h"
#endif
#include "g42png.h"
#include "g42pbm.h"
#include "g42ras.h"
#include "g42targa.h"
#include "g42tiff.h"
#include "g42xwd.h"
G42RasterImage::G42RasterImage() :
   Base(0), Mode(NotEnoughData), Type(Unknown)
{
}
G42RasterImage::~G42RasterImage()
{
	if (Base)
   	delete Base;
}
bool G42RasterImage::IsValid(void)
{
	if (Base)
      return Base->IsValid();
	if (Mode == NotEnoughData)
		return true;
   return false;
}
bool G42RasterImage::IsFinished(void)
{
   if (Base)
      return Base->IsFinished();
   return false;
}
void G42RasterImage::ProcessData(byte * buffer, size_t buffer_size)
{
	if (Base)
		Base->ProcessData(buffer, buffer_size);
	else if (Mode == NotEnoughData)
	{
#ifdef G42BMP_H
		if (::G42IsBmp(buffer, buffer_size))
		{
			// Base = static_cast<G42RasterBase *>(new G42BmpImage(*this));
			// not everyone supports static_cast yet
			Base = (G42RasterBase *)(new G42BmpImage(*this));
			Mode = KnownType;
			Type = BmpWin;
			Base->ProcessData(buffer, buffer_size);
		}
#endif
#ifdef G42GIF_H
		if (::G42IsGif(buffer, buffer_size))
		{
			// Base = static_cast<G42RasterBase *>(new G42GifImage(*this));
			// not everyone supports static_cast yet
			Base = (G42RasterBase *)(new G42GifImage(*this));
			Mode = KnownType;
			Type = Gif;
			Base->ProcessData(buffer, buffer_size);
		}
#endif
#ifdef G42JPEG_H
		if (::G42IsJpeg(buffer, buffer_size))
		{
			// Base = static_cast<G42RasterBase *>(new G42JpegImage(*this));
			// not everyone supports static_cast yet
			Base = (G42RasterBase *)(new G42JpegImage(*this));
			Mode = KnownType;
			Type = Jpeg;
			Base->ProcessData(buffer, buffer_size);
		}
#endif
#ifdef G42PNG_H
		if (::G42IsPng(buffer, buffer_size))
		{
			// Base = static_cast<G42RasterBase *>(new G42PngImage(*this));
			// not everyone supports static_cast yet
			Base = (G42RasterBase *)(new G42PngImage(*this));
			Mode = KnownType;
			Type = Png;
			Base->ProcessData(buffer, buffer_size);
		}
#endif
#ifdef G42PBM_H
		if (::G42IsPbm(buffer, buffer_size))
		{
			// Base = static_cast<G42RasterBase *>(new G42PbmImage(*this));
			// not everyone supports static_cast yet
			Base = (G42RasterBase *)(new G42PbmImage(*this));
			Mode = KnownType;
			if (::G42IsPbmAscii(buffer, buffer_size))
				Type = PbmAscii;
			else
				Type = PbmBinary;
			Base->ProcessData(buffer, buffer_size);
		}
#endif
#ifdef G42TIFF_H
		if (::G42IsTiff(buffer, buffer_size))
		{
			// Base = static_cast<G42RasterBase *>(new G42TiffImage(*this));
			// not everyone supports static_cast yet
			Base = (G42RasterBase *)(new G42TiffImage(*this));
			Mode = KnownType;
			if (::G42IsEps(buffer, buffer_size))
				Type = EpsL2;
			else
				Type = Tiff;
			Base->ProcessData(buffer, buffer_size);
		}
#endif
/*  Needed to put PICT before Targa because Targa was returning true for PICT images  */
#ifdef G42PICT_H
		if (Mode != KnownType)
			if (::G42IsPICT (buffer, buffer_size))
			{
				// Base = static_cast<G42RasterBase *>(new G42XwdImage(*this));
				// not everyone supports static_cast yet
				Base = (G42RasterBase *)(new G42PICTImage(*this));
				Mode = KnownType;
				Type = Pict;
				Base->ProcessData(buffer, buffer_size);
			}
#endif
#ifdef G42TARGA_H
		if (Mode != KnownType)
			if (::G42IsTarga(buffer, buffer_size))
			{
				// Base = static_cast<G42RasterBase *>(new G42TargaImage(*this));
				// not everyone supports static_cast yet
				Base = (G42RasterBase *)(new G42TargaImage(*this));
				Mode = KnownType;
				Type = Tga;
				Base->ProcessData(buffer, buffer_size);
			}
#endif
#ifdef G42PCX_H
		if (::G42IsPcx(buffer, buffer_size))
		{
			// Base = static_cast<G42RasterBase *>(new G42PcxImage(*this));
			// not everyone supports static_cast yet
			Base = (G42RasterBase *)(new G42PcxImage(*this));
			Mode = KnownType;
			Type = Pcx;
			Base->ProcessData(buffer, buffer_size);
		}
#endif
#ifdef G42RAS_H
		if (::G42IsRas(buffer, buffer_size))
		{
			// Base = static_cast<G42RasterBase *>(new G42RasImage(*this));
			// not everyone supports static_cast yet
			Base = (G42RasterBase *)(new G42RasImage(*this));
			Mode = KnownType;
			Type = Ras;
			Base->ProcessData(buffer, buffer_size);
		}
#endif
#ifdef G42XWD_H
		if (::G42IsXwd(buffer, buffer_size))
		{
			// Base = static_cast<G42RasterBase *>(new G42XwdImage(*this));
			// not everyone supports static_cast yet
			Base = (G42RasterBase *)(new G42XwdImage(*this));
			Mode = KnownType;
			Type = Xwd;
			Base->ProcessData(buffer, buffer_size);
		}
#endif
		if (!Base)
			Mode = UnknownType;
	}
}
void G42RasterImage::ForceFileEnd(void)
{
	if (Base)
   	Base->ForceFileEnd();
}
uint32 G42RasterImage::GetWidth(void)
{
	if (Base)
      return Base->GetWidth();
   return 0;
}
uint32 G42RasterImage::GetHeight(void)
{
   if (Base)
      return Base->GetHeight();
   return 0;
}
uint G42RasterImage::GetDepth(void)
{
   if (Base)
      return Base->GetDepth();
	return 0;
}
uint G42RasterImage::GetNumPalette(void)
{
   if (Base)
		return Base->GetNumPalette();
   return 0;
}
G42Color * G42RasterImage::GetPalette(void)
{
   if (Base)
		return Base->GetPalette();
   return 0;
}
bool G42RasterImage::IsColor(void)
{
	if (Base)
		return Base->IsColor();
	return true;
}
bool G42RasterImage::IsTransparent(void)
{
	if (Base)
		return Base->IsTransparent();
	return false;
}
bool G42RasterImage::TopToBottom(void)
{
	if (Base)
		return Base->TopToBottom();
	return true;
}
void
G42RasterImage::CombineRow(byte * old_row, byte * new_row, int32 rowbytes)
{
	if (Base)
   	Base->CombineRow(old_row, new_row, rowbytes);
}
void
G42RasterImage::CombineMask(byte * old_row, byte * new_row, int32 rowbytes)
{
	if (Base)
   	Base->CombineMask(old_row, new_row, rowbytes);
}
void
G42RasterImage::HasRow(byte *, byte *)
{
}
void
G42RasterImage::HasInfo(void)
{
	if (Base)
	{
		Base->FillInfo();
      HasInfo(Base->GetInfo());
	}
}
