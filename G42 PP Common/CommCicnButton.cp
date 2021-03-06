#include "CommCicnButton.h"
CommCicnButton* CommCicnButton::CreateFromStream (LStream *inStream)
{
	return (new CommCicnButton (inStream));
}
void CommCicnButton::DrawSelf()
{	
	if (! mNormalCicnH) 		// Load 'cicn'
		mNormalCicnH = ::GetCIcon(mNormalID);
	if (mNormalCicnH)
	{
		Rect frame;
		CalcLocalFrameRect (frame);
		short transform = (mEnabled == triState_Off) ? ttDisabled : ttNone;
		::PlotCIconHandle (&frame, atNone, transform, mNormalCicnH);
	}
}