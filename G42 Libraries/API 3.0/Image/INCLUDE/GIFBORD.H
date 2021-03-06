#if ! defined (GIFBORD_H)
#define GIFBORD_H
#include "gifilter.h"
	
class G42Border : public G42ImageFilter
{
public:	
							G42Border (int16 size, G42Color color);
	virtual 				~G42Border (void) {}
	virtual 				G42ImageFilter * MakeCopy (void) const;
protected:
	void					HandleFilter (void);
	virtual void		ProcessFrame (void);
	G42Color				BorderColor;
	int16 				BorderSize;
};
#endif
