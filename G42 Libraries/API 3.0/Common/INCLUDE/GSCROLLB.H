
#if !defined(GSCROLLB_H)              // Sentry, use file only if it's not already included.
#define GSCROLLB_H
#include "gtype.h"
// we only use this on MSWIN for now
#ifdef MSWIN
#define STRICT
#include <windows.h>
class G42ScrollBar
{
public:
	enum G42BarType {Vertical = SB_VERT, Horizontal = SB_HORZ};
	G42ScrollBar(HWND par_win, G42BarType bar_type);
	~G42ScrollBar(void);
	intf  GetCurrentPos(void)
				{return CurrentPos;}
	void SetCurrentPos(intf current_pos);
	void SetInitPos(intf min_pos, intf max_pos, intf current_pos,
			intf increment, intf page_size);
	intf  GetMinPos(void)
				{return MinPos;}
	intf  GetMaxPos(void)
				{return MaxPos;}
	void EnableScrollBar(bool enable);
	void ShowScrollBar(bool show);
	void ProcessScrollMsg(uintf code, uintf thumb_pos);
protected:
	G42BarType		BarType;
	HWND				Parent;
	intf				CurrentPos;
	intf				MinPos;
	intf				MaxPos;
	bool				InDrag;
	intf				Increment;
	intf				PageSize;
	intf				NegativeOffset; // 32 bit can't handle negitive numbers
	bool				Visible;
	bool				Active;
};
#endif	// MSWIN
#endif	// GSCROLLB_H sentry.
