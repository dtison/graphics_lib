// g42pict.cpp - PICT File
/*************************************************************
	File:          g42pict.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for PICT Files inheriting from the raster base class.
	Author:        David Ison
	Creation Date: 22 April 1996
	Modification History:
      Code   Date   Name and Description
***************************************************************/
#include "g42itype.h"
#ifdef MACOS
#include "g42pict.h"
#include "g42idata.h"
#include "g42iter.h"
#include "g42macfl.h"
#include "unistd.h"
#include "CommMac.h"
G42PICTImage::G42PICTImage(G42RasterImage & parent) :
	G42RasterBase(parent), ReadMode(ReadHeaderMode),
	CurrentRow(0), TempFilePath (0), TempFileOpen (false), TempFile (0),
	ForcedFileEnd (false)
{
	ScratchPath [0] = 0; 
	Valid = true;
}
G42PICTImage::~G42PICTImage()
{
	if (TempFilePath)
	{
		delete [] TempFilePath;
		TempFilePath = 0;
	}
	if (TempFile)
	{
		::FSClose (TempFile);
		TempFile = 0;
	}
	
	if (TempFileOpen)
	{
		unlink (ScratchPath);
	}	
}
void
G42PICTImage::ProcessData (byte * buffer, size_t buffer_size)
{
	if (!Valid)
		return;
		
	/*  Just write all the data out to the temp file */
	OSErr err = noErr;
	if (! TempFileOpen)	// Make the temp file if needed 
	{
	    short 	vRefNum;
	    long 	dirID;
		FSSpec 	dirFSSpec;
		::FindFolder (kOnSystemDisk, kTemporaryFolderType, kCreateFolder, &vRefNum, &dirID);
		::FSMakeFSSpec (vRefNum, dirID, nil, &dirFSSpec);
		TempFilePath = G42MacFile::GetFullPath (dirFSSpec.name, dirFSSpec.parID, 
			dirFSSpec.vRefNum);
		strcat (TempFilePath, ":G42Temp");
//#define TESTING
#ifdef TESTING
		strcpy (TempFilePath, "G42TempPICT");
#endif
		strcpy (ScratchPath, TempFilePath);
		StringPtr tempFilename = c2pstr (TempFilePath);
		err = FSOpen (tempFilename, 0, &TempFile);
		if (err)
		{
			err = ::Create (tempFilename, dirFSSpec.vRefNum, 'Gr42', 'Temp');
		#ifdef TESTING
			err = ::Create (tempFilename, 0, 'Gr42', 'Temp');
		#endif
			if (! err)
				err = ::FSOpen (tempFilename, 0, &TempFile);
		}
	}
	if (err)
		Valid = false;
	else
	{
		TempFileOpen = true;
		long count = buffer_size;
		err = ::FSWrite (TempFile,  &count, buffer);
	}
}
/*  Here we pull in the temp file, play the pict into a Gworld, get the
	pixmap out and send it off to.  This fn needs better error handling */
void
G42PICTImage::ForceFileEnd (void)
{
	if (!Valid)
		return;
		
	/*  Sentinal for this function to be run only once */
	if (ForcedFileEnd)
		return;
	ForcedFileEnd = true;
#ifdef TESTING
//::FSClose (TempFile);
#endif
	
	OSErr err;
	if (TempFileOpen)
	{
		long	fileSize;
		::SetFPos (TempFile, fsFromStart, 0L);
		err = ::GetEOF (TempFile, &fileSize);	
		if (fileSize > 527)
		{
			PicHandle picHandle = (PicHandle) ::NewHandle (fileSize);
			ThrowIfNil_(picHandle);
	
			/*  Step 1:  Read in the PICT data from the temp file */
			PictInfo pictInfo;
			memset (&pictInfo, 0, sizeof (pictInfo));
			::HLock ((Handle) picHandle);
			::SetFPos (TempFile, fsFromStart, 512L);
			err = ::FSRead (TempFile, &fileSize, *picHandle);																		
			err = ::GetPictInfo (picHandle, &pictInfo, returnColorTable, 256, systemMethod, 0);
			::HUnlock ((Handle) picHandle);								
			if (! ::EmptyRect (&pictInfo.sourceRect))
			{
				#ifdef OLDPICT
				(**picHandle).picFrame = pictInfo.sourceRect;
				#endif
				/*  Step 2:  Draw PICT handle into the GWorld then discard PICT handle  */
				GWorldPtr gWorld;
				short depth = (pictInfo.depth == 2) ? 4 : pictInfo.depth;
				#ifdef OLDPICT
				err = ::NewGWorld (&gWorld, depth, &pictInfo.sourceRect, 
				#endif
				err = ::NewGWorld (&gWorld, depth, &(**picHandle).picFrame, 
					pictInfo.theColorTable, 0, useTempMem);
				ThrowIfError_(err);
				
				CGrafPtr currGrafPtr;
				GDHandle currGDevice;
				::GetGWorld (&currGrafPtr, &currGDevice);
				::SetGWorld (gWorld, nil);
				PixMapHandle pixMapHandle = ::GetGWorldPixMap (gWorld);
				::LockPixels (pixMapHandle);
				::EraseRect (&(**picHandle).picFrame);
				::DrawPicture (picHandle, &(**picHandle).picFrame);
				::SetGWorld (currGrafPtr, currGDevice);			
				::DisposeHandle ((Handle) picHandle);
				picHandle = 0;
				/*  Step 3: Get all the info out of the PixMap  */
				SetupInfo (pixMapHandle, &pictInfo);
				if (Valid)
				{
				 	int rowBytes = (**pixMapHandle).rowBytes & 0x7FFF;
						
					if (Depth > 8)
						RowBytes = Width * TrueColorPixelSize;
					else
						RowBytes = ((Width * Depth + 7) >> 3);
					/*  Step 4:  Copy the bytes out  */
					byte *data = (byte *) ::GetPixBaseAddr (pixMapHandle);
					for (int i = 0; i < Height; i++)
					{
						PutSequence (data, RowBytes);
						data += rowBytes;
					}
				}
				/*  Step 5:  Unlock & free up everything  */
				::UnlockPixels (pixMapHandle);
				::DisposeGWorld (gWorld);
			}
			else
				Valid = false;	// Something's wrong in pictInfo
			if (picHandle)		// Must have missed the prev. KillPicture()
				::KillPicture (picHandle);
		}
		else
			Valid = false;		// File size is too small
	
	}
	
	Parent.HasEnd(Info);
	
	inherited::ForceFileEnd();
}
void
G42PICTImage::SetupInfo (PixMapHandle pixMapHandle, PictInfo *pictInfo)
{	
	Rect pixRect	= (**pixMapHandle).bounds;
	Width 			= pixRect.right  - pixRect.left;
	Height 			= pixRect.bottom - pixRect.top;
	Depth 			= (**pixMapHandle).cmpCount * (**pixMapHandle).cmpSize;
	Color 			= true;
	TopLineFirst 	= true;
  	IntegerFormat 	= Motorola;
	/*  Wierd because mac palette stuff can be wierd [256 colors in a 4bit pixmap] */
	if (Depth > 8)
		NumPalette = 0;
	else
		if (Depth == 4)
			NumPalette = 16;
		else
			if (Depth == 1)
				NumPalette = 2;
			else
				NumPalette = (**(**pixMapHandle).pmTable).ctSize + 1;
	
/*  Muck for palette  */
	#if 1
	CTabHandle ctabHandle = (**pixMapHandle).pmTable;
	ColorSpecPtr ctable = &(**ctabHandle).ctTable;
	if (NumPalette)
	{
		G42Color * pal = new G42Color [NumPalette];
		if (! pal)
		{
			Valid = false;
			ReadMode = ReadErrorMode;
			return;
		}
		for (short i = 0; i < NumPalette; i++)
		{
			byte red, green, blue;
			red 	= (ctable [i].rgb.red >> 8);
			green 	= (ctable [i].rgb.green >> 8);
			blue 	= (ctable [i].rgb.blue >> 8);
			pal [i].red 	= (ctable [i].rgb.red >> 8);
			pal [i].green 	= (ctable [i].rgb.green >> 8);
			pal	[i].blue 	= (ctable [i].rgb.blue >> 8);
		}
		Info.ImagePalette.Set(pal);
	}
	#endif
	CurrentPalette 	= 0;
	Info.Width 		= Width;
	Info.Height 	= Height;
	Info.PixelDepth = (Depth > 8) ? (8 * TrueColorPixelSize) : Depth;
	Info.NumPalette = NumPalette;
	Info.ColorType  = (Depth > 8) ?  G42ImageInfo::ColorTypeTrueColor : G42ImageInfo::ColorTypePalette;
	Info.Compressed = false;
	if (Info.ColorType == G42ImageInfo::ColorTypeTrueColor)
		Info.NumPalette = 0;
	Info.ImageType = Pict;
	Parent.HasInfo(Info);
}
void
G42PICTImage::OutputRow(void)
{
	/*  Take care of 15/16 bit, 4 bit (1 bit)  */
	switch (Depth)
	{
		case 15:
		{
			short *sp = (short *) (RowBuffer + ((Width - 1) << 1));
			G42Byte24BitIterator dp (RowBuffer);
			dp.Forward (Width - 1);
			/*  Do the Big-Endian 16-bit unpack  */
			for (uint32 i = 0; i < Width; i++)
			{
				short v2 = *sp;
				byte b = (((byte) v2 & 0x1F) << 3);
				v2 >>= 5;
				byte g = (((byte) v2 & 0x1F) << 3);
				v2 >>= 5;
				byte r = (((byte) v2 & 0x1F) << 3);
				dp.Set (b, g, r);
				dp.Decrement ();
				sp--;		
			}		
		}
		break;
		#if 0
		case 4:
		{
			byte *sp = RowBuffer + ((Width - 1) >> 1);
			byte *op = RowBuffer + ((Width - 1) >> 1);
			for (uint32 i = 0; i < (Width >> 1); i++)
			{
				byte b1, b2;
				byte v2 = *sp;
				b1 = (v2 >> 4);
				b2 = (v2 & 0x0F);
				b2 <<= 4;
				
				*op-- = b1 | b2;
				sp--;		
			}		
		}
		break;
		#endif			
	}
	G42RasterBase::OutputRow();
	CurrentRow++;
	#if 0
	if (CurrentRow >= Height)
	{
		EatBuffer();
		ReadMode = ReadDoneMode;
		Finished = true;
	}
	#endif
}
#endif	// MACOS
/*  PICT Header verification complements of Karl..  */
bool G42IsPICT(byte * buffer, size_t buffer_size)
{
#ifdef MACOS
 
        if (buffer_size < 527) // Not enough to tell with certainty yet..
        	return true;
        byte* ptr = buffer + 512 + 10; // move the ptr to version info.
        if (*ptr == 0x00) {
                ptr++;
                if (*ptr != 0x11)
                        return false;
                ptr++;
                if (*ptr != 0x02)
                        return false;
                ptr++;
                if (*ptr != 0xff)
                        return false;
                ptr++;
                if (*ptr != 0x0C)
                        return false;
                ptr++;
                if (*ptr != 0x00)
                        return false;
                // looks like a version 2 PICT;
                return true;
        }
        else if (*ptr == 0x11) { // may be version 1 PICT
                ptr++;
                if (*ptr == 0x01)
                        return true;
                else
                        return false;
        }
        return false;
#else
        return false;
#endif
}
