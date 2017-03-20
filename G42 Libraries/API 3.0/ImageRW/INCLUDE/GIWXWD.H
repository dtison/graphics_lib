// g42pcx.h - pcx version of the raster file base class
#ifndef G42WXWD_H
#define G42WXWD_H
/*************************************************************
	File:          g42pcx.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for pcx's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "giwbase.h"
#include "gitxwd.h"
class G42ImageWriterXwd : public G42ImageWriterBase
{
	public:
		G42ImageWriterXwd(void);
		G42ImageWriterXwd(G42Notifier & notifier,
			G42BaseOptions * options = 0);
		virtual ~G42ImageWriterXwd(void);
		virtual bool WriteInfo(G42ImageDescriptor * info);
		virtual bool WriteRow(const byte * row, const byte * mask);
	private:
		byte * TempRow;
};
#endif // G42XWD_H
