#if ! defined (GIFNOISE_H)
#define GIFNOISE_H
#include "gifilter.h"
class G42RemoveNoise : public G42ImageFilter
{
public:	
							G42RemoveNoise (int16 value = 0);
	virtual 				~G42RemoveNoise (void) {}
	virtual 				G42ImageFilter * MakeCopy (void) const;
protected:
	void					HandleFilter (void);
	virtual void		ProcessFrame (void);
	int16					RemoveNoiseValue;
};
#endif
