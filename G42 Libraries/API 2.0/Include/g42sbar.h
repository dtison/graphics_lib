
#if !defined(G42SBAR_H)              // Sentry, use file only if it's not already included.
#define G42SBAR_H
#include "g42itype.h"
class G42ScrollBar
{
	#ifdef MSWIN
public:
	enum G42BarType {Vertical = SB_VERT, Horizontal = SB_HORZ};
	G42ScrollBar(HWND par_win, G42BarType bar_type);
	~G42ScrollBar(void);
	int  GetCurrentPos(void)
				{return CurrentPos;}
	void SetCurrentPos(int current_pos);
	void SetInitPos(int min_pos, int max_pos, int current_pos,
			int increment, int page_size);
	int  GetMinPos(void)
				{return MinPos;}
	int  GetMaxPos(void)
				{return MaxPos;}
	void EnableScrollBar(bool enable);
	void ShowScrollBar(bool show);
	void ProcessScrollMsg(uint code, uint thumb_pos);
protected:
	G42BarType		BarType;
	HWND				Parent;
	int				CurrentPos;
	int				MinPos;
	int				MaxPos;
	bool				InDrag;
	int				Increment;
	int				PageSize;
	int				NegativeOffset; // 32 bit can't handle negitive numbers
	bool				Visible;
	bool				Active;
	#endif	// MSWIN
};
#endif	// G42SBAR_H sentry.
