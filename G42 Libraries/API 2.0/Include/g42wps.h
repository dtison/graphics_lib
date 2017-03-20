// g42wps.h - ps version of the raster file base class
#ifndef G42WPS_H
#define G42WPS_H
/*************************************************************
	File:          g42wps.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for tiff's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "g42wibas.h"
#include "g42idata.h"
class G42PsWriter : G42RasterWriterBase
{
	public:
		G42PsWriter(G42RasterImageWriter & parent);
		virtual ~G42PsWriter();
		virtual bool WriteInfo(G42ImageInfo * info);
		virtual bool WriteRow(const byte * row, const byte * mask);
		virtual uint32 NextRow(void);
		virtual bool NeedRow(void);
		void HandleOutputBuffer(byte * buffer, uint length);
	private:
		byte * TempRow;
		byte * TempBuf;
		enum G42WriteMode { PsMode, TiffMode };
		G42WriteMode WriteMode;
		bool IsEps;
		int Level; // 1 or 2
		G42ImageInfo * Info;
		uint32 CurrentPsRow;
		bool Landscape;
		int PageX;
		int PageY;
		int PageWidth;
		int PageHeight;
};
#endif // G42WPS_H
