// giweps.h - ps version of the raster file base class
#ifndef GIWEPS_H
#define GIWEPS_H
/*************************************************************
	File:          giweps.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for postscript derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "giwbase.h"
#include "giteps.h"
class G42ImageWriterEps : public G42ImageWriterBase
{
	public:
		G42ImageWriterEps(G42Notifier & notifier,
			G42BaseOptions * options = 0);
		G42ImageWriterEps(void);
		virtual ~G42ImageWriterEps(void);
		virtual bool WriteInfo(G42ImageDescriptor * info);
		virtual bool WriteRow(const byte * row, const byte * mask);
		virtual uint32 NextRow(void);
		virtual bool NeedRow(void);
		void HandleOutputBuffer(byte * buffer, uintf length);
	private:
		byte * TempRow;
		byte * TempBuf;
		enum G42WriteMode { EpsMode, TiffMode };
		G42WriteMode WriteMode;
		bool IsEps;
		intf Level; // 1 or 2
		G42ImageDescriptor * Info;
		uint32 CurrentEpsRow;
		bool Landscape;
		intf PageX;
		intf PageY;
		intf PageWidth;
		intf PageHeight;
};
#endif // GIWPS_H
