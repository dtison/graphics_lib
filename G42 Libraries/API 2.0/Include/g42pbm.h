// g42pbm.h - pbm version of the raster file base class
#ifndef G42PBM_H
#define G42PBM_H
/*************************************************************
	File:          g42pbm.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for bmp's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "g42ribas.h"
class G42PbmImage : G42RasterBase
{
	public:
		void ProcessData(byte * buffer, size_t buffer_size);
		G42PbmImage(G42RasterImage & parent);
		virtual ~G42PbmImage();
	protected:
		enum G42PbmVersion {GrayscaleAscii = 2, MonochromeAscii = 1,
			ColorAscii = 3, MonochromeBinary = 4, GrayscaleBinary = 5,
			ColorBinary = 6};
		int Version;
		uint32 CurrentValue;
		uint32 MaxValue;
      int MaxDepth;
		int CurrentShift;
      byte CurrentByte;
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
		virtual void OutputRow(void);
		uint32 CurrentRow;
};
extern bool G42IsPbm(byte * buffer, size_t buffer_size);
extern bool G42IsPbmAscii(byte * buffer, size_t buffer_size);
#endif // G42PBM_H
