#if ! defined (GICLIPP_H)
#define GICLIPP_H
#include "giclip.h"
class G42ClipboardPaste : public G42Clipboard
{
public:	
									G42ClipboardPaste (const G42HeapManager & heap) 
										: G42Clipboard(), Heap (heap) {}
	virtual 						G42ImageFilter * MakeCopy (void) const;
protected:
	virtual void				ProcessFrame (void);
	const G42HeapManager		& Heap;
};
#endif
