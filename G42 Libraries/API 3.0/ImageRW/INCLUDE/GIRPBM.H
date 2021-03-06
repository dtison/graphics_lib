// girpbm.h - pbm version of the raster file base class
#if !defined(GIRPBM_H)
#define GIRPBM_H
/*************************************************************
	File:          girpbm.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for pbm's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "girbase.h"
#include "gidcomp.h" // for ascii versions
#include "gitpbm.h"
class G42ImageReaderPbm : public G42ImageReaderBase
{
	public:
		G42ImageReaderPbm(void);
		G42ImageReaderPbm(G42HeapManager & heap, G42Notifier & notifier,
			G42File * file = 0, G42BaseOptions * options = 0);
		virtual ~G42ImageReaderPbm();
		virtual void ProcessStreamData(void);
		virtual void BeginProcessing(void);
		virtual void SendExtendedInfo(void);
	protected:
		enum G42PbmVersion {GrayscaleAscii = 2, MonochromeAscii = 1,
			ColorAscii = 3, MonochromeBinary = 4, GrayscaleBinary = 5,
			ColorBinary = 6};
		intf Version;
		uint32 MaxValue;
		uint32 MaxDepth;
		uint32 CurrentValue;
		enum G42ReadMode {ReadHeaderMode, ReadWidthMode, ReadHeightMode,
			ReadDepthMode, ReadImageMode, ReadInfoMode,
			ReadDoneMode, ReadErrorMode};
		G42ReadMode ReadMode;
		void ReadHeader(void);
		void ReadWidth(void);
		void ReadHeight(void);
		void ReadDepth(void);
		void ReadImage(void);
		void ReadInfo(void);
		virtual void ConvertRow(void);
		uint32 CurrentRow;
};
// calling this a decompressor is really an awful use of the word, but that's
// the pattern I'm using, and I don't want to break it just because this
// decompressor makes things significantly smaller.
class G42PbmDecompressor : public G42Decompressor
{
public:
	G42PbmDecompressor(G42DecompressorOutput & output, uintf row_bytes,
		 uintf depth, uintf max_value, uint32 width);
	virtual ~G42PbmDecompressor(void);
	virtual void ProcessBuffer(void);
private:
	intf CurrentShift;
	byte CurrentByte;
	uint32 CurrentValue;
	uint32 MaxValue;
	uint32 MaxDepth;
	uint32 Width;
	intf Depth;
	uint32 CurrentColumn;
};
#endif // GIRPBM_H
