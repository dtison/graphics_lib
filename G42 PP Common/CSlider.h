#pragma once
#include <LControl.h>
#include <QDOffscreen.h>
#include <UGWorld.h>
//class CSlider : public LStdControl
class CSlider : public LControl
{
public:
	CSlider (void);
	CSlider (short BasePictRes, short SliderPictRest);
	CSlider (ResIDT BasePictRes, ResIDT SliderPictRes, ResIDT SlideSelectPictRes);
 	CSlider (LStream *inStream);
 	virtual ~CSlider (void);
	void SetDefaults (void);
	void SetPicts (ResIDT BasePictRes, ResIDT SliderPictRes);
	void SetPicts (ResIDT BasePictRes, ResIDT SliderPictRes, ResIDT SlideSelectPictRes);
	void OffsetSlider (short Xoffset, short Yoffset);
	virtual void DrawSelf();
	Boolean PointInSlider (Point thePoint);
//	virtual void ClickSelf (const SMouseDownEvent &inMouseDown) =0;  // OverRide
	virtual void TrackSlider (Point oldMouse) =0;					// OverRide
	virtual long GetSliderValue (void) =0;							// OverRide
	virtual void SetSliderValue (long theValue) =0;					// OverRide
	void SetMinMax (long theMin, long theMax);
	void DoAction (void);
	void BroadcastValueMessage();
	MessageT GetValueMessage() const;
	void SetValueMessage(MessageT inValueMessage);
	void SetCursorFlag(Boolean theFlag);
	void SetSliderOnlyFlag(Boolean theFlag);
	void SetupSlider (ResIDT pict1, ResIDT pict2, ResIDT pict3, long min,
	long max, MessageT valueMessage);
	static pascal void CallBack (void);
	
protected:
	ResIDT mBasePictID;							// PICT res#					
	ResIDT mSliderPictID;
	ResIDT mSliderSelectPictID;
	Rect mBasePictRect;							// Rectangles
	Rect mSliderRect;
	LGWorld *mBaseGWorld;						// LGWorlds
	LGWorld *mSliderGWorld;
	LGWorld *mSliderSelectGWorld;
	LGWorld *mWorkGWorld;
	long mBaseLength;							// Pixel size of base picture
	long mBaseHeight;
	long mMinSliderValue;
	long mMaxSliderValue;
	MessageT mValueMessage;						// message to send
	Int32 mValue;								// true value
	Boolean mSelected;							// true if selected/tracking
	Boolean mSliderCursor;						// false = hide cursor when tracking, true is default
	Boolean mSliderOnly;						// true means only the slider itself is active
	Boolean	Valid;
	Point	OldMousePoint;
	virtual void FinishCreateSelf (void);
};
// 
// ===========================================================================
//	 CHorzSlider Class						 
// ===========================================================================
//
class CHorzSlider : public CSlider 
{
public:
	enum {class_ID = 'Hsld'};
	CHorzSlider();
	CHorzSlider(ResIDT BasePictRes, ResIDT SliderPictRes);
    CHorzSlider(ResIDT BasePictRes, ResIDT SliderPictRes, ResIDT SlideSelectPictRes);
	CHorzSlider(LStream *inStream);
	static CHorzSlider *CreateCHorzSliderStream(LStream *inStream);
	virtual ~CHorzSlider();
	void TrackSlider(Point oldMouse);
	long GetSliderValue(void);
	void SetSliderValue(long theValue);
	virtual Boolean		TrackHotSpot(Int16 inHotSpot, Point inPoint);
	virtual void		HotSpotAction(Int16 /* inHotSpot */, Boolean /* inCurrInside */,
										Boolean /* inPrevInside */);
	virtual void		HotSpotResult (Int16 /* inHotSpot */);
};
// 
// ===========================================================================
//	 CVertSlider Class						 
// ===========================================================================
//
class CVertSlider : public CSlider {
public:
	enum {
		class_ID = 'Vsld'
	};
	CVertSlider();
	CVertSlider(ResIDT BasePictRes, ResIDT SliderPictRes);
	CVertSlider(ResIDT BasePictRes, ResIDT SliderPictRes, ResIDT SlideSelectPictRes);
 	CVertSlider(LStream *inStream);
	static CVertSlider *CreateCVertSliderStream(LStream *inStream);
	virtual ~CVertSlider();
	void TrackSlider(Point oldMouse);
	void ClickSelf(const SMouseDownEvent &inMouseDown);
	long GetSliderValue(void);
	void SetSliderValue(long theValue);
};