/*---------------------------------------------------------------------------------
	CSlider.cp	- Class for horizontal or vertical sliders [aka the Mac Sound CP]
	This original inspiration for this class was found on the Metrowerks web site.  
	The class was then converted to be an LControl [from LView originally] which 
	required a Listener View.  The only misgiving I have with this class now is the 
	"funkiness" of all the rectangles floating around.  Sometime it could use a 
	cleanup because some rects refer to Pane coordinates; others to port coordinates.  D. Ison 2-96
--------------------------------------------------------------------------------*/
#include "CSlider.h"
#include <UGWorld.h>
#include <PP_Messages.h>
CSlider *GCSlider = 0;
short BCount = 0;
CSlider::CSlider()
{
	SetDefaults();
}
//	 Create CSlider and assign Pict resource numbers
CSlider::CSlider (ResIDT BasePictRes, ResIDT SliderPictRes)
{
	SetDefaults();
	mBasePictID = BasePictRes;
	mSliderPictID = SliderPictRes;
}
//	 Create CSlider including selected slider and assign Pict resource numbers
CSlider::CSlider(ResIDT BasePictRes, ResIDT SliderPictRes, ResIDT SlideSelectPictRes)
{
	SetDefaults();
	mBasePictID = BasePictRes;
	mSliderPictID = SliderPictRes;
	mSliderSelectPictID = SlideSelectPictRes;
}
void CSlider::FinishCreateSelf (void)
{
	GCSlider = this;
}
void CSlider::SetDefaults (void)
{
	mBasePictID 		= resID_Undefined;
	mSliderPictID 		= resID_Undefined;
	mSliderSelectPictID = resID_Undefined;
	mMinSliderValue 	= 0;
	mMaxSliderValue 	= 100;
	mSelected 			= false;
	mSliderCursor 		= true;
	mBaseGWorld 		= nil;
	mSliderGWorld 		= nil;
	mSliderSelectGWorld = nil;
	mWorkGWorld 		= nil;
	mSliderOnly 		= false;
	SetRect (&mBasePictRect, 0, 0, 100, 25);
	SetRect (&mSliderRect, 0, 0, 25, 25);
}
CSlider::CSlider(LStream *inStream) : LControl (inStream)
//CSlider::CSlider(LStream *inStream) : LStdControl (inStream)
{
	SetDefaults();
}
CSlider::~CSlider(void)
{
	if (mBaseGWorld)
		delete mBaseGWorld;
	if (mWorkGWorld)
		delete mWorkGWorld;
	if (mSliderGWorld)
		delete mSliderGWorld;
	if (mSliderSelectGWorld)
		delete mSliderSelectGWorld;
}
/*  Does all the things necessary to get the Slider active */
void CSlider::SetupSlider (ResIDT pict1, ResIDT pict2, ResIDT pict3, long min,
	long max, MessageT valueMessage)
{
	SetPicts (pict1, pict2, pict3);
	SetMinMax (min, max);
	SetValueMessage (valueMessage);
	PicHandle macPictureH = ::GetPicture (mBasePictID);
	if (macPictureH != nil) 	
	{
		// Make GWorld for Bar pict
		mBasePictRect = (*macPictureH) -> picFrame;
		OffsetRect (&mBasePictRect, -mBasePictRect.left, -mBasePictRect.top);
		mBaseLength = mBasePictRect.right;
		mBaseHeight = mBasePictRect.bottom;
	
		ResizeFrameTo (mBasePictRect.right, mBasePictRect.bottom, false);
		mBaseGWorld = new LGWorld (mBasePictRect, 0, 0, 0, 0);
 		if (mBaseGWorld) 
		{	
			mBaseGWorld -> BeginDrawing();
			::DrawPicture(macPictureH, &mBasePictRect);
			mBaseGWorld -> EndDrawing();
			mWorkGWorld = new LGWorld (mBasePictRect, 0, 0, 0, 0);
 			if (mWorkGWorld) 
			{
				mWorkGWorld -> BeginDrawing();
				::DrawPicture (macPictureH, &mBasePictRect);
				mWorkGWorld -> EndDrawing();
			}
  		}
		ReleaseResource ((Handle) macPictureH); 
	}
 
	// Make GWorld for Slider pict
	macPictureH = ::GetPicture (mSliderPictID);
	if (macPictureH != nil)
	{
 		mSliderRect = (*macPictureH) -> picFrame;
		OffsetRect(&mSliderRect, -mSliderRect.left, -mSliderRect.top);
		
		mBaseLength = mBaseLength - (mSliderRect.right - mSliderRect.left);
		mBaseHeight = mBaseHeight - (mSliderRect.bottom - mSliderRect.top);
		mSliderGWorld = new LGWorld (mSliderRect, 0, 0, 0, 0);
 		if (mSliderGWorld) 
 		{
			mSliderGWorld -> BeginDrawing();
			::DrawPicture (macPictureH, &mSliderRect);
			mSliderGWorld -> EndDrawing();
 		}
		ReleaseResource ((Handle) macPictureH); 
	}
 
	if (mSliderSelectPictID != resID_Undefined)
	{
		macPictureH = ::GetPicture (mSliderSelectPictID);
		if (macPictureH != nil) 
		{
 			mSliderSelectGWorld = new LGWorld (mSliderRect, 0, 0, 0, 0);
 			if (mSliderSelectGWorld) 
 			{
				mSliderSelectGWorld -> BeginDrawing();
				::DrawPicture (macPictureH, &mSliderRect);
				mSliderSelectGWorld -> EndDrawing();
			}
 			ReleaseResource ((Handle) macPictureH); 
		}
 	} 
 	
// 	ThumbActionUPP	mHorzThumbAction;
//	mHorzThumbAction = NewThumbActionProc (CallBack);		
//	SetThumbFunc(mHorzThumbAction);
	Valid = ((mBaseGWorld && mSliderGWorld && mWorkGWorld)
	  && ((mSliderSelectPictID == resID_Undefined) || mSliderSelectGWorld));
}
//	Defines PICT resource numbers for base and slider, no Selected slider
void CSlider::SetPicts (ResIDT BasePictRes, ResIDT SliderPictRes)
{
	mBasePictID = BasePictRes;
	mSliderPictID = SliderPictRes;
}
//	Defines PICT resource numbers for base, slider, and Selected slider
void CSlider::SetPicts (ResIDT BasePictRes, ResIDT SliderPictRes, ResIDT SlideSelectPictRes)
{
	mBasePictID = BasePictRes;
	mSliderPictID = SliderPictRes;
	mSliderSelectPictID = SlideSelectPictRes;
}
//	Move Slider relative to base picture.  Used when slider is smaller than base picture
void CSlider::OffsetSlider (short Xoffset, short Yoffset)
{
	OffsetRect (&mSliderRect, Xoffset, Yoffset);
}
void CSlider::DrawSelf()
{
	GrafPtr savePort;
	GetPort (&savePort);
	::PenNormal();
	Rect portPictRect = mBasePictRect;
	::OffsetRect (&portPictRect, mFrameLocation.h, mFrameLocation.v);
	if (mBaseGWorld && mWorkGWorld) 		// Copy base GWorld to temp GWorld
	{
		mWorkGWorld -> BeginDrawing();
		mBaseGWorld -> CopyImage ((GrafPtr) mWorkGWorld -> GetMacGWorld(), mBasePictRect, srcCopy, 0);
		mWorkGWorld -> EndDrawing();
	}
	if (mSelected && mSliderSelectGWorld)	// Is selected & does a Select LGWorld exist?
	{
		if (mSliderSelectGWorld && mWorkGWorld)	// Copy Select slider GWorld to temp GWorld
		{
			mWorkGWorld -> BeginDrawing();
			mSliderSelectGWorld -> CopyImage 
				((GrafPtr) mWorkGWorld -> GetMacGWorld(), mSliderRect, srcCopy, 0);
			mWorkGWorld -> EndDrawing();
		}
	} 
	else 
	{
		if (mSliderGWorld && mWorkGWorld) 		// Copy slider GWorld to temp GWorld
		{
			mWorkGWorld -> BeginDrawing();
			mSliderGWorld -> CopyImage
				((GrafPtr)mWorkGWorld->GetMacGWorld(), mSliderRect, srcCopy, 0);
			mWorkGWorld -> EndDrawing();
		}
	}
	if (mWorkGWorld) 
		mWorkGWorld -> CopyImage (savePort, portPictRect, srcCopy, 0);
	else
		FrameRect(&portPictRect);
}
//	Test whether a point is within the slider rect
Boolean CSlider::PointInSlider (Point thePoint)
{
	Rect portSliderRect = mSliderRect;
	OffsetRect (&portSliderRect, mFrameLocation.h, mFrameLocation.v);
	return (PtInRect(thePoint, &portSliderRect));
}
//	Set min and max values for slider, long values
void CSlider::SetMinMax(long theMin, long theMax)
{
	mMinSliderValue = theMin;
	mMaxSliderValue = theMax;
	if (IsVisible())
		SetSliderValue(mValue);		//  Clamp existing value and redraw
}
pascal void CSlider::CallBack (void)
{
	GCSlider -> BroadcastValueMessage();
}
/*  Handles Broadcasting  */
void CSlider::DoAction (void)
{
	long tempValue;
	tempValue = GetSliderValue();
	if (mValue != tempValue) 		// (Only send message if value changed)
	{
		mValue = tempValue;
		BroadcastValueMessage();
	}
}
void CSlider::BroadcastValueMessage()
{
	if (mValueMessage != cmd_Nothing) 
	{
		Int32 value = mValue;
		BCount++;
		if (BCount == 10)
		{
	//		SysBeep (0);
			BCount = 0;
		}
		BroadcastMessage (mValueMessage, (void *) & value);
	}
}
//	Assign Message number
void CSlider::SetValueMessage (MessageT inValueMessage)
{
	mValueMessage = inValueMessage;
}
//	Return Message number
MessageT CSlider::GetValueMessage() const
{
	return mValueMessage;
}
//	If true then display cursor, false hides cursor when tracking
void CSlider::SetCursorFlag(Boolean theFlag)
{
	mSliderCursor = theFlag;
}
//	If true then activate only when slider hit, false makes the whole base pciture active
void CSlider::SetSliderOnlyFlag(Boolean theFlag)
{
	mSliderOnly = theFlag;
}
/* *****************  HORZSLIDER  *****************/
CHorzSlider::CHorzSlider() : CSlider()
{ }
/*	 Create Horizontal Slider and assign Pict resource numbers	*/
CHorzSlider::CHorzSlider(ResIDT BasePictRes, ResIDT SliderPictRes)
	: CSlider(BasePictRes, SliderPictRes)
{ }
/*	 Create Horizontal Slider and assign Pict resource numbers  */
CHorzSlider::CHorzSlider(ResIDT BasePictRes, ResIDT SliderPictRes, ResIDT SlideSelectPictRes)
	: CSlider(BasePictRes, SliderPictRes, SlideSelectPictRes)
{ }
CHorzSlider::CHorzSlider (LStream *inStream) : CSlider (inStream)
{ }
CHorzSlider *CHorzSlider::CreateCHorzSliderStream (LStream *inStream)
{
	return (new CHorzSlider(inStream));
}
CHorzSlider::~CHorzSlider()
{ }
Boolean
CHorzSlider::TrackHotSpot (Int16 inHotSpot, Point inPoint)
{
	if (! mSliderCursor)
		HideCursor();	
	Boolean returnVal = LControl::TrackHotSpot (inHotSpot, inPoint);
	ShowCursor();
	return (returnVal);
}
void CHorzSlider::HotSpotAction (Int16 /* inHotSpot */, Boolean /* inCurrInside */, 
	Boolean /* inPrevInside */)
{
	/*  Step 1:  See if the mouse moved & bail out if so [stops cursor flicker] */
	Point mousePoint;
	GetMouse (&mousePoint);
	if (EqualPt (mousePoint, OldMousePoint)) 
		return;
	
	/*  Step 2:  Update the UI  */	
	if (PointInSlider (mousePoint))
	{
		mSelected = true;
		DrawSelf();	
		FocusDraw();  // Call to TrackSlider (mousePoint) was after this
		TrackSlider (mousePoint);
	} 
	else 
	{
		Rect portPictRect 	= mBasePictRect;
		Rect portSliderRect = mSliderRect;
		OffsetRect (&portPictRect, mFrameLocation.h, mFrameLocation.v);
		OffsetRect (&portSliderRect, mFrameLocation.h, mFrameLocation.v);
		if (PtInRect (mousePoint, &portPictRect) && (! mSliderOnly)) 
		{
			// Move Slider to center on mouse
			OffsetRect (&mSliderRect, (mousePoint.h - portSliderRect.left - 
				((mSliderRect.right - mSliderRect.left)  / 2)), 0);
			// Keep slider in base pict
			if (mSliderRect.right > mBasePictRect.right)
				OffsetRect (&mSliderRect, mBasePictRect.right - mSliderRect.right, 0);
			if (mSliderRect.left < mBasePictRect.left)
				OffsetRect (&mSliderRect, mBasePictRect.left - mSliderRect.left, 0);
			mSelected = true;
			DrawSelf();
			FocusDraw();	// Call to TrackSlider (mousePoint) was after this
		}
	}
	/*  Step 3:  Update the Listeners  */	
	DoAction();
	OldMousePoint = mousePoint;
}
void CHorzSlider::HotSpotResult (Int16 /* inHotSpot */)
{
	mSelected = false;
	DrawSelf();
}
			
/*	Move slider while mouse button is down  Calls DoAction() when moving  */
void CHorzSlider::TrackSlider (Point oldMouse)
{
	Point newMouse;
	mSelected = true;
	if (! mSliderCursor)	// Hide cursor if mSliderCursor is false
		HideCursor();	
	while (Button()) 
	{
		GetMouse (&newMouse);
		if (! EqualPt (newMouse, oldMouse)) 
		{
			Rect portPictRect 	= mBasePictRect;
			Rect portSliderRect = mSliderRect;
			OffsetRect (&portPictRect, mFrameLocation.h, mFrameLocation.v);
			OffsetRect (&portSliderRect, mFrameLocation.h, mFrameLocation.v);
			/*  Test for mouse move  */
			#ifdef OLDWAY
			if (((newMouse.h > mBasePictRect.left) || (mSliderRect.left > mBasePictRect.left))
				&& ((newMouse.h < mBasePictRect.right) || (mSliderRect.right < mBasePictRect.right))) {
				OffsetRect(&mSliderRect, newMouse.h - oldMouse.h, 0); // Move if mouse within base picture area
			#endif
			if (((newMouse.h > portPictRect.left) || (mSliderRect.left > portPictRect.left))
				&& ((newMouse.h < portPictRect.right) || (mSliderRect.right < portPictRect.right))) 
			{
				OffsetRect (&mSliderRect, newMouse.h - oldMouse.h, 0); // Move if mouse within base picture area
				// Keep Slider from going too far
				if (mSliderRect.right > mBasePictRect.right)
					OffsetRect (&mSliderRect, mBasePictRect.right - mSliderRect.right, 0);
				if (mSliderRect.left < mBasePictRect.left)
					OffsetRect (&mSliderRect, mBasePictRect.left - mSliderRect.left, 0);
				oldMouse = newMouse;
				DrawSelf();
				DoAction();	
				FocusDraw();
			}
		}
		oldMouse = newMouse;
	}
	mSelected = false;
	DrawSelf();									// Display again without selection
	ShowCursor();								// Display cursor in case it was hidden
}
//	Return value of slider
long CHorzSlider::GetSliderValue (void)
{
	long total;
  	float tempvalue;
	total = (mMaxSliderValue - mMinSliderValue);
	tempvalue = ((float)total / (float)(mBaseLength)) * (mSliderRect.left - mBasePictRect.left);
   	return (tempvalue + mMinSliderValue);
}
//	Set value of slider, update display and send message
void CHorzSlider::SetSliderValue (long theValue)
{
	long total;
  	float tempvalue;
	total = (mMaxSliderValue - mMinSliderValue);
	if (theValue < mMinSliderValue)  
		theValue = mMinSliderValue;
	if (theValue > mMaxSliderValue)  
		theValue = mMaxSliderValue;
	tempvalue =	((float)(theValue- mMinSliderValue)/(float)total)  * (mBaseLength);
 	OffsetRect(&mSliderRect,-mSliderRect.left,0);
 	OffsetRect(&mSliderRect,tempvalue,0);
  	FocusDraw();
	DrawSelf();
	DoAction();							// Send message
}
/* *****************  VERTSLIDER  *****************/
CVertSlider::CVertSlider() : CSlider()
{ }
CVertSlider::CVertSlider (ResIDT BasePictRes, ResIDT SliderPictRes)
	: CSlider(BasePictRes, SliderPictRes)
{ }
CVertSlider::CVertSlider (ResIDT BasePictRes, ResIDT SliderPictRes, ResIDT SlideSelectPictRes)
	: CSlider(BasePictRes, SliderPictRes, SlideSelectPictRes)
{ }
CVertSlider::CVertSlider (LStream *inStream) : CSlider(inStream)
{ }
CVertSlider *CVertSlider::CreateCVertSliderStream (LStream *inStream)
{
	return (new CVertSlider(inStream));
}
CVertSlider::~CVertSlider()
{ }
/*	Move slider while mouse button is down Calls DoAction() when moving	*/
void CVertSlider::TrackSlider (Point oldMouse)
{
	Point newMouse;
	mSelected = true;
	if (! mSliderCursor)
		HideCursor();
	while (Button()) 
	{
		GetMouse (&newMouse);
		if (!EqualPt (newMouse, oldMouse)) 
		{
			// Mouse moved?
			if (((newMouse.v > mBasePictRect.top) || (mSliderRect.top > mBasePictRect.top)) && 
			((newMouse.v < mBasePictRect.bottom) || (mSliderRect.bottom < mBasePictRect.bottom))) {
			
				// Move Slider if mouse in base picture area
				OffsetRect(&mSliderRect, 0, newMouse.v - oldMouse.v);
				if (mSliderRect.bottom > mBasePictRect.bottom)
					OffsetRect(&mSliderRect, 0, mBasePictRect.bottom - mSliderRect.bottom);
				if (mSliderRect.top < mBasePictRect.top)
					OffsetRect(&mSliderRect, 0, mBasePictRect.top - mSliderRect.top);
				oldMouse = newMouse;
				DrawSelf();
				DoAction();						//  Send message
				FocusDraw();
			}
		}
		oldMouse = newMouse;
	}
	mSelected = false;
	DrawSelf();									// update without selection
	ShowCursor();								// Show cursor in case it was hidden
}
void CVertSlider::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	if (PointInSlider(inMouseDown.whereLocal)) 
	{
		FocusDraw();
		mSelected = true;
		DrawSelf();
		TrackSlider(inMouseDown.whereLocal);
	} 
	else 
	{
		if (PtInRect(inMouseDown.whereLocal, &mBasePictRect)&&(!mSliderOnly)) {
			// click in base?
			FocusDraw();
			mSelected = true;
			OffsetRect(&mSliderRect, 0, (inMouseDown.whereLocal.v - mSliderRect.top - 
				((mSliderRect.bottom - mSliderRect.top) / 2)));
			if (mSliderRect.bottom > mBasePictRect.bottom)
				OffsetRect(&mSliderRect, 0, mBasePictRect.bottom - mSliderRect.bottom);
			if (mSliderRect.top < mBasePictRect.top)
				OffsetRect(&mSliderRect, 0, mBasePictRect.top - mSliderRect.top);
			DrawSelf();
			DoAction();
			FocusDraw();
			TrackSlider (inMouseDown.whereLocal);
		}
	}
}
long CVertSlider::GetSliderValue(void)
{
	long total;
 	float tempvalue;
	total = (mMaxSliderValue - mMinSliderValue);
	tempvalue = ((float)total / (float)(mBaseHeight)) *  (mSliderRect.top - mBasePictRect.top);
 	return (tempvalue + mMinSliderValue);
}
void CVertSlider::SetSliderValue (long theValue)
{
	long total;
  	float tempvalue;
	total = (mMaxSliderValue - mMinSliderValue);
	if (theValue < mMinSliderValue)  
		theValue = mMinSliderValue;
	if (theValue > mMaxSliderValue)  
		theValue = mMaxSliderValue;
	tempvalue =	((float) (theValue - mMinSliderValue) / (float) total) * mBaseHeight;
 	OffsetRect (&mSliderRect,0,-mSliderRect.top);
 	OffsetRect (&mSliderRect,0,tempvalue);
  	FocusDraw();
	DrawSelf();
	DoAction();	
}
/*  OLD MUCK  ***********************************************************/
#if 0
void CHorzSlider::ClickSelf (const SMouseDownEvent &inMouseDown)
{
	if (PointInSlider (inMouseDown.whereLocal)) 
	{
		FocusDraw();
		mSelected = true;
		DrawSelf();	
		TrackSlider (inMouseDown.whereLocal);
	} 
	else 
	{
		Rect tempRect 		= mBasePictRect;
		Rect portPictRect 	= mBasePictRect;
		Rect portSliderRect = mSliderRect;
		OffsetRect (&portPictRect, mFrameLocation.h, mFrameLocation.v);
		OffsetRect (&portSliderRect, mFrameLocation.h, mFrameLocation.v);
		if (PtInRect (inMouseDown.whereLocal, &portPictRect)&&(!mSliderOnly)) // Click in base?
		{
			#ifdef OLDWAY
			// Move Slider to center on mouse
			OffsetRect (&mSliderRect, (inMouseDown.whereLocal.h - mSliderRect.left - 
				((mSliderRect.right - mSliderRect.left)  / 2)), 0);
			
			// Keep slider in base pict
			if (mSliderRect.right > mBasePictRect.right)
				OffsetRect (&mSliderRect, mBasePictRect.right - mSliderRect.right, 0);
			if (mSliderRect.left < mBasePictRect.left)
				OffsetRect (&mSliderRect, mBasePictRect.left - mSliderRect.left, 0);
			#endif
			// Move Slider to center on mouse
			OffsetRect (&mSliderRect, (inMouseDown.whereLocal.h - portSliderRect.left - 
				((mSliderRect.right - mSliderRect.left)  / 2)), 0);
			
			// Keep slider in base pict
			if (mSliderRect.right > mBasePictRect.right)
				OffsetRect (&mSliderRect, mBasePictRect.right - mSliderRect.right, 0);
			if (mSliderRect.left < mBasePictRect.left)
				OffsetRect (&mSliderRect, mBasePictRect.left - mSliderRect.left, 0);
			mSelected = true;
			FocusDraw();
			DrawSelf();
			DoAction();
			FocusDraw();
			TrackSlider (inMouseDown.whereLocal);
		}
	}
}
#endif
