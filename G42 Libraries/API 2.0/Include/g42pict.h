// g42pict.h - Mac PICT version of the raster file base class
#ifndef G42PICT_H
#define G42PICT_H
/*************************************************************
	File:          g42pict.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for PICT Files inheriting from the raster base class.
	Author:        David Ison
	Creation Date: 22 April 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "g42ribas.h"
#ifdef MACOS
#include <Files.h>
#include <PictUtil.h>
class G42PICTImage : G42RasterBase
{
public:
				G42PICTImage(G42RasterImage & parent);
	virtual 	~G42PICTImage();
	void 		ProcessData(byte * buffer, size_t buffer_size);
protected:
	int			CurrentPalette;
	enum 		G42ReadMode {ReadHeaderMode, ReadImageMode, ReadDoneMode,
		ReadErrorMode};
	G42ReadMode ReadMode;
	void 		ReadHeader(void);
	void 		ReadImage(void);
	uint32 		CurrentRow;
	virtual 	void OutputRow(void);
	 virtual 	void ForceFileEnd(void);
    short		TempFile;
    char		*TempFilePath;
    bool		TempFileOpen;
 	byte 		*CurrBuffer;
 	char		ScratchPath[512];
	size_t 		CurrBufferSize;
   	static 		void CopyComponent(char *& path, const unsigned char * component, Boolean colon);
	static 		char *GetFullPath (Str63 name, long parID, short vRefNum);
	static 		CInfoPBRec lastInfo; 
private:
	void 		SetupInfo (PixMapHandle pixMapHandle, PictInfo *pictInfo);
	Boolean		ForcedFileEnd;
 };
/*  In order to save PICT's we will probably need to change a few things in the 
	library because of Mac OS file system differences */
#if 0
class G42PICTWriter : G42RasterWriterBase
{
	public:
		G42PICTWriter(G42RasterImageWriter & parent);
		virtual ~G42PICTWriter();
		virtual bool WriteInfo(G42ImageInfo * info);
		virtual bool WriteRow(const byte * row);
		virtual uint32 NextRow(void);
		virtual bool NeedRow(void);
	private:
		byte * TempRow;
      int32 CurrentPICTRow;
};
#endif
#endif // MACOS
extern bool G42IsPICT (byte * buffer, size_t buffer_size);
#endif // G42PICT_H
