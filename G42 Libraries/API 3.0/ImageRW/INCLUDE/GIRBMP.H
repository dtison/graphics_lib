// girbmp.h - G42ImageReaderBmp
#ifndef GIRBMP_H
#define GIRBMP_H
/*************************************************************
	File:          girbmp.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for bmp's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
***************************************************************/
#include "girbase.h"
#include "gitbmp.h"
class G42ImageReaderBmp : public G42ImageReaderBase
{
	public:
		G42ImageReaderBmp(void);
		G42ImageReaderBmp(G42HeapManager & heap, G42Notifier & notifier,
			G42File * file = 0, G42BaseOptions * options = 0);
		virtual ~G42ImageReaderBmp();
		virtual void ProcessStreamData(void);
		virtual void BeginProcessing(void);
		virtual void SendExtendedInfo(void);
	protected:
		enum G42BmpVersion {Win3x, OS21x, OS22x, Undefined};
		G42BmpVersion osType;
		int CurrentPalette;
		int NeedSkip;
		enum G42ReadMode {ReadHeaderMode, ReadPaletteMode, ReadImageMode,
			ReadDoneMode, ReadErrorMode};
		G42ReadMode ReadMode;
		void ReadHeader(void);
		void ReadPalette(void);
		void ReadImage(void);
		virtual void ConvertRow(void); //allows conversions a row at a time
		uint32 CurrentRow;
};
extern bool G42IsBmp(byte * buffer, size_t buffer_size);
#endif // GIRBMP_H - sentry
