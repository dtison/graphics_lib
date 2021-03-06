// mac only stuff
#include "g42type.h"
#include "g42image.h"
#include "g42iview.h"
//#include "g42sview.h"
#ifdef MACOS
#if ! defined (G42MACFL_H)
#define G42MACFL_H
#include <Files.h>
class G42MacFile
{
public:
	static void CopyComponent(char *& path, const unsigned char * component, Boolean colon);
	static char *GetFullPath (Str63 name, long parID, short vRefNum);
	static CInfoPBRec lastInfo;
	/*  Also stick the GWorld <-> G42Image things here for now  */
	static GWorldPtr GWorldFromG42Image (G42Image *image, 
		G42SimpleImageViewer *viewer, long flags = nil);
	static PicHandle PICTFromG42Image (G42Image *image, 
		G42SimpleImageViewer *viewer, Boolean useSpecial = false);
	/*  Stuff I don't know where else to put  */
	static void Msg (Str255 msg);
};
#endif	// G42MACFL_H
#endif	// MACOS