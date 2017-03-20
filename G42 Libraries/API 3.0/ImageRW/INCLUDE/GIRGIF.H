// girgif.h - gif version of the raster file base class
#ifndef GIRGIF_H
#define GIRGIF_H
/*************************************************************
	File:          girgif.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for gif's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
      Code   Date   Name and Description
***************************************************************/
#include "girbase.h"
#include "gidlzw.h"
#include "gitgif.h"
class G42ImageReaderGif : public G42ImageReaderBase
{
	public:
		G42ImageReaderGif(G42HeapManager & heap, G42Notifier & notifier,
			G42File * file = 0, G42BaseOptions * options = 0);
		G42ImageReaderGif(void);
		virtual ~G42ImageReaderGif();
		virtual void ProcessStreamData(void);
		virtual void BeginProcessing(void);
		virtual void SendExtendedInfo(void);
		virtual void ForceFileEnd(void); // force image to be finished
		virtual void TerminateStream(void); // user terminated stream
		virtual bool DecompressorOutputRow(byte * row);
	protected:
		intf CurrentPalette;
		intf SubBlockSize;
		uintf GlobalDepth;
		uint32 GlobalRowBytes;
		intf BlockSize;
		byte InputBuffer[256];
		byte * MaskBuffer;
		G42Color * GlobalPalette;
		intf NumGlobalPalette;
//		G42MultiImageInfo MultiInfo;
		bool NumImages;
		uintf CommentBlockSize;
		uintf CommentTextOffset;
		G42Vector<char> CommentText;
		byte CodeSize;
		G42LzwDecompressor * Lzw;
		enum G42ReadMode {ReadHeaderMode, ReadGlobalPaletteMode,
			ReadChunkMode, ReadExtensionChunkMode, ReadUnknownSubBlockMode,
			ReadLocalImageMode, ReadLocalPaletteMode, ReadImageMode,
			ReadImageEndMode, ReadImageClearMode, ReadCommentMode,
			ReadDoneMode, ReadErrorMode};
		G42ReadMode ReadMode;
		void ReadHeader(void);
		void ReadGlobalPalette(void);
		void ReadChunk(void);
		void ReadExtensionChunk(void);
		void ReadUnknownSubBlock(void);
		void ReadComment(void);
		void ReadLocalImage(void);
		void ReadLocalPalette(void);
		void ReadImage(void);
		void ReadImageEnd(void);
		void ReadImageClear(void);
		void ConvertRow(void);
		virtual void EndPass(void); // finishes a one pass file
};
#endif // GIRGIF_H
