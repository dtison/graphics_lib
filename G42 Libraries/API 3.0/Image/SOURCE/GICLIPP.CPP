#include "giclipp.h"
#include "gmacfl.h"
#include "gtmpmem.h"
/*  Get the image from the clipboard  */
void
G42ClipboardPaste::ProcessFrame (void)
{
	/*  Grab PICT if it's out there  */
	bool mem_error = false;
	long scrap_offset;
	PicHandle pict = (PicHandle) ::NewHandleSys (0);
	if (pict) {
		scrap_size = ::GetScrap ((Handle) pict, PICTClipboardType, &scrap_offset);
	}
	else {
		throw (G42MemoryException (0));
	}
	
	if (scrap_size < 0)
	{
		mem_error = true;
		DisposeHandle ((Handle) pict);
		throw (G42MemoryException (0));
	}
	
	/*  (Kills Pict Handle)  */
	GWorldPtr gworld = G42MacFile::Pict2GWorld (pict);	
	
	::DisposeGWorld (gworld);
	/*  Clear everything  */
	OutputData.Clear(); 
	OutputDescriptor.Clear();
}
G42ImageFilter *
G42ClipboardPaste::MakeCopy (void) const
{
	return new G42ClipboardPaste (Heap);
}
#if 0
PicHandle
CImageDoc::PICTFromClipboard (long &scrapSize)
{
	long scrapOffset;
	Boolean memError = false;
	/*  Step 1: Try to pull in PICT thru SYSTEM heap  */
	PicHandle pict = (PicHandle) ::NewHandleSys (0);
	if (pict)
		scrapSize = ::GetScrap ((Handle) pict, PICTClipboardType, &scrapOffset);
	if (scrapSize < 0)
	{
		memError = true;
		DisposeHandle ((Handle) pict);
	}
	if (memError)
	{
		Str255 msg;
		::GetIndString (msg, STRx_Miscellaneous, str_ErrorPastingImage);
		G42MacFile::Msg (msg);
		pict = nil;
	}
	return pict;
}
#endif