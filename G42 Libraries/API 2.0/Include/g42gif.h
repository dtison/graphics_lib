// g42bmp.h - bmp version of the raster file base class
#ifndef G42GIF_H
#define G42GIF_H
/*************************************************************
	File:          g42gif.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for gif's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
      Code   Date   Name and Description
***************************************************************/
#include "g42ribas.h"
#include "g42lzw.h"
class G42GifLzw;
class G42GifImage : G42RasterBase
{
	friend class G42GifLzw;
	public:
		void ProcessData(byte * buffer, size_t buffer_size);
		G42GifImage(G42RasterImage & parent);
		virtual ~G42GifImage();
	protected:
		int CurrentPalette;
		int SubBlockSize;
		bool Interlace;
		G42GifLzw * Lzw;
		int BlockSize;
		int Background;
		byte InputBuffer[256];
		int Pass;
		int NumPasses;
		byte * MaskBuffer;
		int TransparentPixel;
		enum G42ReadMode {ReadHeaderMode, ReadGlobalPaletteMode,
			ReadChunkMode, ReadExtensionChunkMode, ReadUnknownSubBlockMode,
			ReadLocalImageMode, ReadLocalPaletteMode, ReadImageMode,
			ReadDoneMode, ReadErrorMode};
		G42ReadMode ReadMode;
		void ReadHeader(void);
		void ReadGlobalPalette(void);
		void ReadChunk(void);
		void ReadExtensionChunk(void);
		void ReadUnknownSubBlock(void);
		void ReadLocalImage(void);
		void ReadLocalPalette(void);
		void ReadImage(void);
      void ProcessRow(byte * row);
		virtual void OutputRow(void);
		uint32 CurrentRow;
};
class G42GifLzw : public G42Lzw
{
	protected:
		G42GifImage * Parent;
	public:
		G42GifLzw(G42GifImage * parent, int width, int code_size) :
			Parent(parent), G42Lzw(width, LzwGif, code_size)
		{
		}
		~G42GifLzw(){}
		bool OutputRow(byte * line)
		{
			Parent->ProcessRow(line);
			return false;
		}
};
extern bool G42IsGif(byte * buffer, size_t buffer_size);
#endif // G42BMP_H
