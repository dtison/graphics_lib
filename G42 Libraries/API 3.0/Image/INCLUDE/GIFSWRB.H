#if ! defined (GIFSWRB_H)
#define GIFSWRB_H
#include "gifilter.h"
class G42SwapRedBlue : public G42ImageFilter
{
public:
						G42SwapRedBlue (void);
	virtual 			~G42SwapRedBlue (void) {}
	virtual void 	HandleDescriptor (G42ImageDescriptor * descriptor, 
							G42PathComponent * sender);
	virtual 			G42ImageFilter * MakeCopy(void) const;
protected:
	void				HandleFilter (void);
	virtual void	ProcessFrame (void);
	bool 				PaletteMode;
};
#endif
