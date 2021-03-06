// g42sbar.cpp - controls a windows scrollbar
#include "g42sbar.h"
/*--------------------------------------------------------------------------
	G42ScrollBar Constructor
	Creates a G42ScrollBar object
	Parms -
	par_win	- Handle of parent window
	bar_type	- Enum of scroll bar type.  Valid values are:
						G42BarType::Horizontal
						G42BarType::Vertical
						G42BarType::Control
--------------------------------------------------------------------------*/
G42ScrollBar::G42ScrollBar(HWND par_win, G42BarType bar_type) :
	NegativeOffset(0), Visible(true), Active(true)
{
	InDrag = false;
	Parent = par_win;
	BarType = bar_type;
}
/*--------------------------------------------------------------------------
	G42ScrollBar Destructor
	Destroys the object and any child objects
--------------------------------------------------------------------------*/
G42ScrollBar::~G42ScrollBar(void)
{
}
/*--------------------------------------------------------------------------
	G42ScrollBar SetCurrentPos
	Allows the G42ScrollBar object owner to set the scroll bar thumb at any
	position and updates the scroll bar to reflect the change.
	Parms -
		current_pos - Must be >= MinPos, <= MaxPos
--------------------------------------------------------------------------*/
void
G42ScrollBar::SetCurrentPos(int current_pos)
{
	CurrentPos = current_pos;
	SetScrollPos(Parent, BarType, CurrentPos + NegativeOffset, true);
}
/*--------------------------------------------------------------------------
	G42ScrollBar SetInitPos
	Allows the G42ScrollBar object owner to set the inital defaults for the
	scroll bar object.  Sets the HWINDOW ScrollBar range and thumb position
	Parms -
		min_pos		- Any value
		max_pos		- Must be > min_pos
		current_pos - Must be >= min_pos, <= max_pos
		increment	- Must be > 0
		page_size	- Must be > 0
--------------------------------------------------------------------------*/
void
G42ScrollBar::SetInitPos(int min_pos, int max_pos, int current_pos,
	int increment, int page_size)
{
	if (min_pos < 0)
		NegativeOffset = -min_pos;
	else
		NegativeOffset = 0;
	MinPos = min_pos;
	MaxPos = max_pos;
	CurrentPos = current_pos;
	Increment = increment;
	PageSize = page_size;
	SetScrollRange(Parent, BarType, MinPos + NegativeOffset,
		MaxPos + NegativeOffset, false);
	SetScrollPos(Parent, BarType, CurrentPos + NegativeOffset, true);
}
/*--------------------------------------------------------------------------
	G42ScrollBar ProcessScrollMsg
	Processes the Scroll messages from the owner object.  Moves the thumb
	position according to the message.
	Parms -
		code			- Must be valid Windows SB_XXX code
		thumb_pos	- Generally 0, except on SB_THUMBTRACK messages where it
						  contains the new thumb position
--------------------------------------------------------------------------*/
void
G42ScrollBar::ProcessScrollMsg(uint code, uint true_thumb_pos)
{
	if (!Visible || !Active)
		return;
		
	int thumb_pos = (int)true_thumb_pos - NegativeOffset;
	switch (code)
	{
		case SB_BOTTOM:
			CurrentPos = MaxPos - PageSize;
			InDrag = false;
			break;
		case SB_TOP:
			CurrentPos = MinPos;
			InDrag = false;
			break;
		case SB_LINEDOWN:
			CurrentPos += Increment;
			if (CurrentPos > MaxPos)
				CurrentPos = MaxPos;
			InDrag = false;
			break;
		case SB_LINEUP:
			CurrentPos -= Increment;
			if (CurrentPos < MinPos)
				CurrentPos = MinPos;
			InDrag = false;
			break;
		case SB_PAGEDOWN:
			CurrentPos += PageSize;
			if (CurrentPos > MaxPos)
				CurrentPos = MaxPos;
			InDrag = false;
			break;
		case SB_PAGEUP:
			CurrentPos -= PageSize;
			if (CurrentPos < MinPos)
				CurrentPos = MinPos;
			InDrag = false;
			break;
		case SB_THUMBTRACK:
			CurrentPos = thumb_pos;
			InDrag = true;
			break;
		case SB_THUMBPOSITION:
			CurrentPos = thumb_pos;
			InDrag = false;
			break;
		case SB_ENDSCROLL:
			break;
	}
	SetScrollPos(Parent, BarType, CurrentPos + NegativeOffset, true);
}
void
G42ScrollBar::EnableScrollBar(bool enable)
{
	if (enable)
		::EnableScrollBar(Parent, BarType, ESB_ENABLE_BOTH);
	else
		::EnableScrollBar(Parent, BarType, ESB_DISABLE_BOTH);
	Active = enable;
}
void
G42ScrollBar::ShowScrollBar(bool show)
{
	::ShowScrollBar(Parent, BarType, show);
	Visible = show;
}
