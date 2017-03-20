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
#include "g42wimag.h"
#include "g42wibas.h"
#include "g42wbmp.h"
#include "g42wgif.h"
#include "g42wjpeg.h"
#include "g42wpcx.h"
#ifdef MACOS
#include "g42pict.h"
#endif
#include "g42wpng.h"
#include "g42wpbm.h"
#include "g42wras.h"
#include "g42wtga.h"
#include "g42wtiff.h"
#include "g42wxwd.h"
#include "g42wps.h"
G42RasterImageWriter::G42RasterImageWriter() :
	Base(0), Valid(true)
{
}
G42RasterImageWriter::~G42RasterImageWriter()
{
	if (Base)
	{
		delete Base;
      Base = 0;
	}
	Valid = false;
}
bool
G42RasterImageWriter::IsValid(void) const
{
	return (Valid && (!Base || Base->IsValid()));
}
bool
G42RasterImageWriter::WriteInfo(G42ImageInfo * info)
{
	if (!Valid || Base)
		return false;
	switch (info->ImageType)
	{
#if defined(G42WBMP_H)
		case BmpWin:
		case BmpOS2:
			// Base = static_cast<G42RasterWriterBase *>(new G42GifWriter(*this));
			// not everyone supports static_cast yet
			Base = (G42RasterWriterBase *)(new G42BmpWriter(*this));
			break;
#endif
#if defined(G42WPS_H)
		case EpsL1:
		case EpsL2:
		case PsL1:
		case PsL2:
			// Base = static_cast<G42RasterWriterBase *>(new G42PsWriter(*this));
			// not everyone supports static_cast yet
			Base = (G42RasterWriterBase *)(new G42PsWriter(*this));
			break;
#endif
#if defined(G42WGIF_H)
		case Gif:
			// Base = static_cast<G42RasterWriterBase *>(new G42GifWriter(*this));
			// not everyone supports static_cast yet
			Base = (G42RasterWriterBase *)(new G42GifWriter(*this));
			break;
#endif
#if defined(G42WJPEG_H)
		case Jpeg:
			// Base = static_cast<G42RasterWriterBase *>(new G42JpegWriter(*this));
			// not everyone supports static_cast yet
			Base = (G42RasterWriterBase *)(new G42JpegWriter(*this));
			break;
#endif
#if defined(G42WPBM_H)
		case PbmAscii:
		case PbmBinary:
			// Base = static_cast<G42RasterWriterBase *>(new G42PbmWriter(*this));
			// not everyone supports static_cast yet
			Base = (G42RasterWriterBase *)(new G42PbmWriter(*this));
			break;
#endif
#if defined(G42WPCX_H)
		case Pcx:
			// Base = static_cast<G42RasterWriterBase *>(new G42PcxWriter(*this));
			// not everyone supports static_cast yet
			Base = (G42RasterWriterBase *)(new G42PcxWriter(*this));
			break;
#endif
#if defined(G42WPNG_H)
		case Png:
			// Base = static_cast<G42RasterWriterBase *>(new G42PngWriter(*this));
			// not everyone supports static_cast yet
			Base = (G42RasterWriterBase *)(new G42PngWriter(*this));
			break;
#endif
#if defined(G42WRAS_H)
		case Ras:
			// Base = static_cast<G42RasterWriterBase *>(new G42RasWriter(*this));
			// not everyone supports static_cast yet
			Base = (G42RasterWriterBase *)(new G42RasWriter(*this));
			break;
#endif
#if defined(G42WTGA_H)
		case Tga:
			// Base = static_cast<G42RasterWriterBase *>(new G42TargaWriter(*this));
			// not everyone supports static_cast yet
			Base = (G42RasterWriterBase *)(new G42TargaWriter(*this));
			break;
#endif
#if defined(G42WTIFF_H)
		case Tiff:
			// Base = static_cast<G42RasterWriterBase *>(new G42TiffWriter(*this));
			// not everyone supports static_cast yet
			Base = (G42RasterWriterBase *)(new G42TiffWriter(*this));
			break;
#endif
#if defined(G42WXWD_H)
		case Xwd:
			// Base = static_cast<G42RasterWriterBase *>(new G42XwdWriter(*this));
			// not everyone supports static_cast yet
			Base = (G42RasterWriterBase *)(new G42XwdWriter(*this));
			break;
#endif
	}
	if (Base && Base->IsValid())
		return Base->WriteInfo(info);
	return false;
}
bool
G42RasterImageWriter::WriteRow(const byte * row, const byte * mask)
{
	if (Valid && Base && Base->IsValid())
		return Base->WriteRow(row, mask);
	return false;
}
uint32
G42RasterImageWriter::NextRow(void)
{
	if (Valid && Base && Base->IsValid())
		return Base->NextRow();
	return 0;
}
bool
G42RasterImageWriter::NeedRow(void)
{
	if (Valid && Base && Base->IsValid())
		return Base->NeedRow();
	return false;
}
bool
G42RasterImageWriter::WriteEnd(G42ImageInfo * info)
{
	if (Valid && Base && Base->IsValid())
		return Base->WriteEnd(info);
	return false;
}
int
G42RasterImageWriter::GetDepth(void)
{
	if (Valid && Base && Base->IsValid())
		return Base->GetDepth();
	return 0;
}
