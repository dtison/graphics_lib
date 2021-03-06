// girxwd.h - xwd version of the raster file base class
#ifndef GIRXWD_H
#define GIRXWD_H
/*************************************************************
	File:          girxwd.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for xwd's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "girbase.h"
#include "gitxwd.h"
class G42ImageReaderXwd : public G42ImageReaderBase
{
	public:
		G42ImageReaderXwd(void);
		G42ImageReaderXwd(G42HeapManager & heap, G42Notifier & notifier,
			G42File * file = 0, G42BaseOptions * options = 0);
		virtual ~G42ImageReaderXwd();
		virtual void ProcessStreamData(void);
		virtual void BeginProcessing(void);
		virtual void SendExtendedInfo(void);
		virtual bool TestType(void);
		virtual bool TestType(const byte * buffer, uint32 buffer_size);
	protected:
		enum G42ReadMode {ReadHeaderMode, ReadImageMode, ReadInfoMode,
			ReadHeaderSizeMode, ReadHasInfoMode, ReadPaletteMode,
			ReadDoneMode, ReadErrorMode};
		G42ReadMode ReadMode;
		uint32 HeaderSize;
		uintf Version;
		uintf VisualType;
		uintf CurrentPalette;
		uint32 CurrentRow;
		void ReadPalette(void);
		void ReadHasInfo(void);
		void ReadHeader(void);
		void ReadImage(void);
		void ReadHeaderSize(void);
		virtual void ConvertRow(void);
};
#endif // GIRXWD_H
