#if ! defined (GIFEQUAL_H)
#define GIFEQUAL_H
#include "gifilter.h"
class G42Equalize : public G42ImageFilter
{
public:	
							G42Equalize (void) {}
	virtual 				~G42Equalize (void) {}
	virtual 				G42ImageFilter * MakeCopy (void) const;
protected:
	void					HandleFilter (void);
	virtual void		ProcessFrame (void);
};
#endif
