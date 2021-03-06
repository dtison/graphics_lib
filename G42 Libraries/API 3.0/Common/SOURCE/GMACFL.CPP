#include "gtype.h"
//  Mac only file
#ifdef MACOS
#include "gmacfl.h"
#include "gidisp.h"
#include <string.h>
#include <PictUtil.h>
#define	maxPathLen 512
CInfoPBRec G42MacFile::lastInfo;
char 
*G42MacFile::GetFullPath (Str63 name, long parID, short vRefNum)
{
	char		fullPath [maxPathLen + 16];
	char 		*curPath	= fullPath + maxPathLen - 1;
	Str255		scratch;
	OSErr		error;
	*curPath = 0;
	CopyComponent (curPath, name, parID == fsRtParID);
	lastInfo.dirInfo.ioNamePtr = scratch;
	for (lastInfo.dirInfo.ioDrParID = parID; lastInfo.dirInfo.ioDrParID != fsRtParID;) 
	{
		lastInfo.dirInfo.ioVRefNum 		= vRefNum;
		lastInfo.dirInfo.ioFDirIndex 		= -1;
		lastInfo.dirInfo.ioDrDirID 		= lastInfo.dirInfo.ioDrParID;
		error = PBGetCatInfoSync (&lastInfo);
		if (error)
		 	return "";
		CopyComponent (curPath, scratch, true);
	}
	char *str = new char [(strlen (curPath) << 1) + 1];
	strcpy (str, curPath);
	return str;
}
/* Prefix a path name component to path and update path */
void 
G42MacFile::CopyComponent(char *& path, const unsigned char * component, Boolean colon)
{
	if (colon)
		*--path = ':';
	path -= *component;
	memmove (path, component+1, *component);
}
GWorldPtr
G42MacFile::GWorldFromImageData (DescriptorRef descriptor, DataRef data, 
	G42Display & display, long flags)
{
	GWorldPtr gWorld = 0;
	G42Rect rect (0, 0, descriptor -> GetWidth(), descriptor -> GetHeight());
	QDErr err = ::NewGWorld (&gWorld, 0, &((Rect) rect), NULL, NULL, flags);
	if (err) {
		throw (G42MemoryException (0));
	}
	::LockPixels (::GetGWorldPixMap (gWorld));
	CGrafPtr port;
	GDHandle gdh;
	::GetGWorld (&port, &gdh);
	::SetGWorld (gWorld, 0);
	for (uint32 i = 0; i < descriptor -> GetNumStrips(); i++)
	{
		display.MacDrawData (data [i], descriptor, rect); 
	}
	::SetGWorld (port, gdh);
	::UnlockPixels (::GetGWorldPixMap (gWorld));
	return gWorld;
}
GWorldPtr
G42MacFile::PICT2GWorld (PicHandle pict)
{
	GWorldPtr gworld = 0;
	PictInfo pict_info;
	memset (&pict_info, 0, sizeof (pict_info));
	::HLock ((Handle) pict);
	OSErr err = ::GetPictInfo (pict, &pict_info, returnColorTable, 256, systemMethod, 0);
	::HUnlock ((Handle) pict);
									
	if (! ::EmptyRect (&pict_info.sourceRect))
	{
		/*  Draw PICT handle into the GWorld, then discard PICT handle  */
		short depth = pict_info.depth;
		if (depth == 2 || depth == 4) {
			depth = 8;
		}
		err = ::NewGWorld (&gworld, depth, &(**pict).picFrame, 
			pict_info.theColorTable, 0, useTempMem);
		if (err) {
			throw (G42MemoryException (0));
		}
		
		CGrafPtr currGrafPtr;
		GDHandle currGDevice;
		::GetGWorld (&currGrafPtr, &currGDevice);
		::SetGWorld (gworld, nil);
		PixMapHandle pixmap_handle = ::GetGWorldPixMap (gworld);
		::LockPixels (pixmap_handle);
		::EraseRect (&(**pict).picFrame);
		::DrawPicture (pict, &(**pict).picFrame);
		::SetGWorld (currGrafPtr, currGDevice);			
		::DisposeHandle ((Handle) pict);
		pict = 0;
		::UnlockPixels (pixmap_handle);
	}
	else {
		throw G42InternalException (0);
	}
	
	return gworld;
}
PicHandle 
G42MacFile::PICTFromImageData (DescriptorRef descriptor, DataRef data,
	bool is_special)
{
	G42Display display;
	G42Rect rect (0, 0, descriptor -> GetWidth() - 1,
		descriptor -> GetHeight() - 1);
	RgnHandle savRgn = ::NewRgn();
	::GetClip (savRgn);
	::ClipRect (&((Rect) rect));
	PicHandle pict = 0;
	try
	{
		 if (is_special || descriptor -> GetDepth() < 24)	// 8 Bit and printing case
		{
			/*  Make a GWorld and draw image into it  */
			GWorldPtr gWorld = G42MacFile::GWorldFromImageData (descriptor, 
				data, display, useTempMem);
			GrafPtr	savePort;
			::GetPort (&savePort);
			/*  Get another port  */
			GWorldPtr tempPort = (GWorldPtr) new CGrafPort;
			::OpenCPort (tempPort);
			::SetPort ((GrafPtr) tempPort);
			/*  Draw the gWorld into the temp port during OpenCPicture  */
			pict = ::OpenPicture (&((Rect) rect));
			::LockPixels (::GetGWorldPixMap (gWorld));
			::CopyBits (&WINBITMAP (gWorld), &WINBITMAP (tempPort),
				&WINPORTRECT (gWorld),  &WINPORTRECT (gWorld), srcCopy, NULL);
			::UnlockPixels (::GetGWorldPixMap (gWorld));
			::ClosePicture();
			::DisposeGWorld (gWorld);
			::SetPort ((GrafPtr) savePort);
			::CloseCPort (tempPort);
			delete tempPort;
		}
		else	// 24 bit case
		{
			GrafPtr port;
			::GetPort (&port);
			pict = ::OpenPicture (&((Rect) rect));
			for (uint32 i = 0; i < descriptor -> GetNumStrips(); i++)
			{
				display.MacDrawData (data [i], descriptor, rect); 
			}
			::ClosePicture();
		}
	}
	
	/*  Catch memory allocation errors  */
	catch (...)
	{
//		SysBeep (0);
		
	}
	::SetClip (savRgn);
	::DisposeRgn (savRgn);
	/*  See if the pict actually was made  */
	if (::EmptyRect (&(**pict).picFrame))
	{
		::KillPicture (pict);
		pict = 0;
	}	
	return pict;
}
#if 0
/*	Display an Alert with the string passed. */
void 
G42MacFile::Msg(Str255 msg)
{
	::ParamText (msg, nil, nil, nil);
	::Alert (130, nil);
}
#endif
#endif // MACOS