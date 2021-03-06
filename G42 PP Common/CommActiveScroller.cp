#ifdef PowerPlant_PCH
#include PowerPlant_PCH
#endif
#include "CommActiveScroller.h"
CommActiveScroller *CommActiveScroller::CreateCommActiveScrollerStream (LStream *inStream)
{
	return (new CommActiveScroller (inStream));
}
CommActiveScroller::CommActiveScroller()
{
}
CommActiveScroller::CommActiveScroller (LStream *inStream) : LActiveScroller (inStream)
{
}
CommActiveScroller::~CommActiveScroller()
{
}
/*  This code "overrides" the default behavior of PowerPlant scrollbars
	The panX and Y parameters are calculated and retrieved from the
	G42 Image Library.  D. Ison					*/
void CommActiveScroller::SetScrollBars (Int32 panX, Int32 panY)
{
	if (mScrollingView == nil) 
		return;
	SPoint32		scrollUnit;
	SPoint32		scrollPosition;
	SDimension16	scrollFrameSize;
	SDimension32	scrollImageSize;
	mScrollingView -> GetScrollUnit (scrollUnit);
	mScrollingView -> GetFrameSize (scrollFrameSize);
	mScrollingView -> GetImageSize (scrollImageSize);
	mScrollingView -> GetScrollPosition (scrollPosition);
	
	if (mVerticalBar) 
	{
		Int32 vertDiff = scrollImageSize.height - scrollFrameSize.height;
		if (scrollPosition.v > vertDiff) 
			vertDiff = scrollPosition.v;
		
		Int32 vertMax = 0;
		if (vertDiff > 0) 
			vertMax = (vertDiff + scrollUnit.v - 1) / scrollUnit.v;
		mVerticalBar -> SetMaxValue (vertMax);
	//	mVerticalBar -> SetValue ((scrollPosition.v + scrollUnit.v - 1) / scrollUnit.v);
		mVerticalBar -> SetValue (panY);
	}
	
	if (mHorizontalBar != nil) 
	{
		Int32	horizDiff = scrollImageSize.width - scrollFrameSize.width;
		if (scrollPosition.h > horizDiff) 
			horizDiff = scrollPosition.h;
		
		Int32	horizMax = 0;
		if (horizDiff > 0) 
			horizMax = (horizDiff + scrollUnit.h - 1) / scrollUnit.h;
		
		mHorizontalBar->SetMaxValue(horizMax);
//		mHorizontalBar->SetValue((scrollPosition.h + scrollUnit.h - 1) / scrollUnit.h);
		mHorizontalBar -> SetValue (panX);
	}
		mScrollingView->ScrollImageTo (panX, panY, false);
}
/*  For our implementation, we want to bypass the default PowerPlant scrollbars 
	setup because we want our "image-centering" algorithm above  */
void CommActiveScroller::AdjustScrollBars()
{
}
