#include "g42type.h"
// mac only file
#ifdef MACOS
#include <string.h>
#include "g42macfl.h"
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
		lastInfo.dirInfo.ioFDirIndex 	= -1;
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
G42MacFile::GWorldFromG42Image (G42Image *image, G42SimpleImageViewer *viewer, long flags)
{
	GWorldPtr gWorld = nil;
	Rect rect;
	SetRect (&rect, 0, 0, image -> GetInfo().Width, image -> GetInfo().Height);
	QDErr qdErr = ::NewGWorld (&gWorld, 0, &rect, NULL, NULL, flags);
	ThrowIfError_ (qdErr);
	::LockPixels (::GetGWorldPixMap (gWorld));
	viewer -> DrawImageAt (gWorld, image, 0, 0);
	::UnlockPixels (::GetGWorldPixMap (gWorld));
	return gWorld;
}
/*  Make a PICT from a G42Image TODO:  Try to use iview instead of sview */
PicHandle 
G42MacFile::PICTFromG42Image (G42Image *image, G42SimpleImageViewer *viewer,
	Boolean useSpecial)
{
	Rect rect;
	PicHandle pict = nil;
	const G42ImageInfo info = image -> GetInfo();
	::SetRect (&rect, 0, 0, info.Width, info.Height);
	RgnHandle savRgn = ::NewRgn();
	::GetClip (savRgn);
	::ClipRect (&rect);
 	if (useSpecial || info.PixelDepth < 24)	// 8 Bit and printing case
	{
		/*  Make a GWorld and draw image into it  */
		GWorldPtr gWorld = G42MacFile::GWorldFromG42Image (image, 
			viewer, useTempMem);
		GrafPtr	savePort;
		::GetPort (&savePort);
		/*  Get another port  */
		GWorldPtr tempPort = (GWorldPtr) new CGrafPort;
		::OpenCPort (tempPort);
		::SetPort ((GrafPtr) tempPort);
		/*  Draw the gWorld into the temp port during OpenCPicture  */
		pict = ::OpenPicture (&rect);
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
		pict = ::OpenPicture (&rect);
//		viewer -> DrawImageOnly ((GWorldPtr) port, image);
		viewer -> DrawImageAt ((GWorldPtr) port, image, 0, 0);
		::ClosePicture();
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
/*	Display an Alert with the string passed. */
void 
G42MacFile::Msg(Str255 msg)
{
	::ParamText (msg, nil, nil, nil);
	::Alert (130, nil);
}
#endif
