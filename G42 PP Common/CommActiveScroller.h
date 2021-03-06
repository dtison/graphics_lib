#pragma once
#include <LActiveScroller.h>
class CommActiveScroller : public LActiveScroller
{
	public:
		enum {class_ID = 'cscr'};
		static CommActiveScroller*	CreateCommActiveScrollerStream (LStream *inStream);
		CommActiveScroller ();
		CommActiveScroller (LStream *inStream);
		CommActiveScroller (const LScroller &inOriginal);
		virtual			~CommActiveScroller();
		virtual void	AdjustScrollBars();
		void			SetScrollBars (Int32 panX, Int32 panY);
};
