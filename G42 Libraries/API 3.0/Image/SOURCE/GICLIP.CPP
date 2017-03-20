#include "giclip.h"
void
G42Clipboard::HandleDescriptor(G42ImageDescriptor * descriptor, 
	G42PathComponent *)
{
	if (HaveFirstDescriptor) 
	{
		if (! HaveFirstFrame)
		{
			/*  It's time to run the filter on the frame */
			ProcessFrame();
			HaveFirstFrame = true;
		}
	}
	else 
	{
		HaveFirstDescriptor = true;
	}
	SetInputDescriptor (descriptor);
}
