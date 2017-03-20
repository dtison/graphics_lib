// giwgif.h - bmp version of the raster file base class
#ifndef GIWGIF_H
#define GIWGIF_H
/*************************************************************
	File:          giwgif.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for gif's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "giwbase.h"
#include "giclzw.h"
#include "gitgif.h"
#include "gimdesc.h"
class G42ImageWriterGif : public G42ImageWriterBase, public G42CompressorOutput
{
	public:
		G42ImageWriterGif(void);
		G42ImageWriterGif(G42Notifier & notifier,
			G42BaseOptions * options = 0);
		virtual ~G42ImageWriterGif(void);
		bool WriteInfo(G42ImageDescriptor * info);
		bool WriteRow(const byte * row, const byte * mask);
		bool WriteEnd(void);
		virtual bool CanDoMultiple(void);
		virtual bool WriteMultiInfo(G42MultipleImageDescriptor * info);
		virtual bool WriteMultiEnd(G42MultipleImageDescriptor * info);
		void AdvanceCurrentRow(void);
		virtual void CompressorOutputBuffer(byte * buffer, uintf length);
	private:
		G42LzwCompressor * Lzw;
		intf CurrentPass;
		bool Progressive;
		bool Transparent;
		byte TransparentPixel;
		byte OpaquePixel;
		byte * TempRow;
		bool MultiImageMode;
		bool NeedLocalImage;
		uintf CodeDepth;
		void WriteLocalImage(void);
};
#endif // GIWGIF_H
