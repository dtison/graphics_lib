#if ! defined (GICLIP_H)
#define GICLIP_H
#include "gifilter.h"
const PICTClipboardType   = 'PICT';
const RegionClipboardType = 'G42r';
const TransClipboardType  = 'G42t';
/*  Special "filter" for clipboard support  */
class G42Clipboard : public G42ImageFilter
{
public:	
							G42Clipboard (void) : HaveFirstFrame (false) {}
	virtual void 		HandleBegin (G42PathBegin *, G42PathComponent *) {}
	virtual void 		HandleDescriptor (G42ImageDescriptor * descriptor, 
								G42PathComponent *);
	virtual void 		HandleMultiDescriptor(G42MultipleImageDescriptor *, 
								G42PathComponent *) {}
protected:
	bool					HaveFirstFrame;
private:
};
#endif
