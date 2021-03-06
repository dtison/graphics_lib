
#include "CommToggleButton.h"
CommToggleButton*
CommToggleButton::CreateCommToggleButtonStream (LStream	*inStream)
{
	return (new CommToggleButton(inStream));
}
CommToggleButton::CommToggleButton (LStream *inStream) 
	: IsPushed (false), CommCicnButton (inStream)
{
}
void 
CommToggleButton::DrawSelf()
{
	Rect frame;
	CalcLocalFrameRect(frame);
	CIconHandle & iconHandle 	= IsPushed ? mPushedCicnH : mNormalCicnH;
	if (! mNormalCicnH) 
		mNormalCicnH = ::GetCIcon (mNormalID);
	else
		if (! mPushedCicnH) 
			mPushedCicnH = ::GetCIcon (mPushedID);
	if (iconHandle)		// Loaded successfully?
	{
		short transform = (mEnabled == triState_On) ? ttNone : ttDisabled;
		::PlotCIconHandle (&frame, atNone, transform, iconHandle);
	}
}
void
CommToggleButton::ClickSelf (const SMouseDownEvent &inMouseDown)
{
//	DontRefresh();
	inherited::ClickSelf (inMouseDown);
	IsPushed = ! IsPushed;
	Draw (nil);
}
void
CommToggleButton::SetPushed (Boolean pushed, Boolean refresh)
{
	IsPushed = pushed;
	if (refresh)
		Draw (nil);
}
