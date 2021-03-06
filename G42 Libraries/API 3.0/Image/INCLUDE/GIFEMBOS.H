#if ! defined (GIFEMBOS_H)
#define GIFEMBOS_H
#include "gifilter.h"
class G42Emboss : public G42ImageFilter
{
public:	
							G42Emboss (int16 emboss_angle = 90);
	virtual 				~G42Emboss (void) {}
	virtual 				G42ImageFilter * MakeCopy (void) const;
protected:
	void					HandleFilter (void);
	virtual void		ProcessFrame (void);
	int16					EmbossAngle;
};
#endif
