#include "gicached.h"
/// TODO:  Reset LastStripNum somewhere........
G42CachedImage::G42CachedImage (G42ImageDescriptor & desc, G42Vector <G42ImageData> & data)
: CurrentLock (0), Data (data), Descriptor (desc), LastStripNum (-1)
{
}
G42CachedImage::~G42CachedImage (void)
{
	#if 1
	/*  Maybe this will be replaced by using G42Ptr */
	if (CurrentLock)
	{
		delete CurrentLock;
		CurrentLock = 0;
	}
	#endif
}