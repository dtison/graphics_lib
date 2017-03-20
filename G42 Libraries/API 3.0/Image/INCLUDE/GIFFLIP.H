#if ! defined (GIFFLIP_H)
#define GIFFLIP_H
#include "gifilter.h"
class G42Flip : public G42ImageFilter
{
public:
	enum  G42FlipType {FlipHorizontal, FlipVertical};
						G42Flip (G42FlipType type = FlipVertical);
	virtual 			~G42Flip (void) {}
	virtual 			G42ImageFilter * MakeCopy(void) const;
protected:
	void				HandleFilter (void);
	void				ProcessFrame (void);
	G42FlipType 	FlipType;
};
#endif
