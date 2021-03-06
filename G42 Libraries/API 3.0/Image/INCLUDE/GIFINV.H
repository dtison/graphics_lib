#if ! defined (GIFINV_H)
#define GIFINV_H
#include "gifilter.h"
class G42Invert : public G42ImageFilter
{
public:
						G42Invert (void);
	virtual 			~G42Invert (void) {}
	virtual void 	HandleDescriptor (G42ImageDescriptor * descriptor, 
							G42PathComponent * sender);
	virtual 			G42ImageFilter * MakeCopy(void) const;
protected:
	void				HandleFilter (void);
	virtual void	ProcessFrame (void);
	bool 				PaletteMode;
};
#endif
