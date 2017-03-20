#if ! defined (GICLIPC_H)
#define GICLIPC_H
#include "giclip.h"
/*  Special "filter" for clipboard support  */
class G42ClipboardCopy : public G42Clipboard
{
public:	
	virtual void		HandleEnd (G42PathEnd * end, G42PathComponent *);
	virtual 				G42ImageFilter * MakeCopy (void) const;
protected:
	virtual void		ProcessFrame (void);
};
#endif
