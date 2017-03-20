// mac only stuff
#include "gtype.h"
#ifdef MACOS
#if ! defined (GMACFL_H)
#define GMACFL_H
#include "gidesc.h"
#include <Files.h>
class G42Display;
class G42MacFile
{
public:
	static void CopyComponent(char *& path, const unsigned char * component, Boolean colon);
	static char *GetFullPath (Str63 name, long parID, short vRefNum);
	static CInfoPBRec lastInfo;
	static GWorldPtr GWorldFromImageData (DescriptorRef descriptor, DataRef data,
				G42Display & display, long flags = 0);
	static GWorldPtr PICT2GWorld (PicHandle pict);
	static PicHandle PICTFromImageData (DescriptorRef descriptor, DataRef data,
				bool is_special);
	#ifdef NOTYET
	/*  Also stick the GWorld <-> G42Image things here for now  */
	static PicHandle PICTFromG42Image (G42Image *image, 
		G42SimpleImageViewer *viewer, Boolean useSpecial = false);
	#endif
	/*  Stuff I don't know where else to put  */
	static void Msg (Str255 msg);
};
#endif	// GMACFL_H
#endif	// MACOS