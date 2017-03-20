// gippscrl.h -- G42PPScroller 
/*-----------------------------------------------------------------------------
	Modifications:
-----------------------------------------------------------------------------*/
#include "gtype.h"
#ifdef MACOS	// Mac-only file
#ifdef PowerPlant_PCH
#include PowerPlant_PCH
#endif
#include "gippscrl.h"
/*-----------------------------------------------------------------------------
	G42PPScroller member functions:
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
	Function - G42PPScroller constructor
	Description:
		Stream constructor; constructs from PP LStream from parent class.
-----------------------------------------------------------------------------*/
G42PPScroller::G42PPScroller (LStream *inStream) : LActiveScroller (inStream)
{}
/*-----------------------------------------------------------------------------
	Function - G42PPScroller creator <static>
	Description:
		Creates and returns instance of this class from a stream.
-----------------------------------------------------------------------------*/
G42PPScroller *G42PPScroller::CreateG42PPScrollerStream (LStream *inStream)
{
	return (new G42PPScroller (inStream));
}
/*-----------------------------------------------------------------------------
	Function - GetScrollingViewPos
	Description:
		Returns scroll position of mScrollingView.
-----------------------------------------------------------------------------*/
void
G42PPScroller::GetScrollingViewPos (G42Point &point)
{
	SPoint32 pos;
	mScrollingView -> GetScrollPosition (pos);
	point = G42Point (pos.h, pos.v);
}
/*-----------------------------------------------------------------------------
	Function - SetScrollBars
	Parameters:
		xpos - Position to set horizontal scrollbar.
		ypos - Position to set vertical scrollbar.
	Description:
		Modifies the default behavior of PowerPlant scrollbars.  Allows 
		library to set specific scroll position.
-----------------------------------------------------------------------------*/
void 
G42PPScroller::SetScrollBars (int32 xpos, int32 ypos)
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
		#if 0
		mVerticalBar -> SetValue ((scrollPosition.v + scrollUnit.v - 1) / scrollUnit.v);
		#endif
		mVerticalBar -> SetValue (ypos);
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
		#if 0
		mHorizontalBar->SetValue((scrollPosition.h + scrollUnit.h - 1) / scrollUnit.h);
		#endif
		mHorizontalBar -> SetValue (xpos);
	}
	mScrollingView->ScrollImageTo (xpos, ypos, false);
}
/*-----------------------------------------------------------------------------
	Function - AdjustScrollBars
	Description:
 		For our implementation, we want to bypass the default PowerPlant 
 		scrollbars setup because we want the algorithm above. 
-----------------------------------------------------------------------------*/
void G42PPScroller::AdjustScrollBars()
{ }
#endif // MACOS
