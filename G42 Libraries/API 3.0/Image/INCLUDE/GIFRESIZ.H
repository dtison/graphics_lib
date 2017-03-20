#if ! defined (GIFRESIZ_H)
#define GIFRESIZ_H
#include "gifilter.h"
class G42Resize : public G42ImageFilter
{
public:	
	enum  				G42ResizeType {Truncate, Average, Interpolate};
							G42Resize (uint32 new_width, uint32 new_height, 
								G42ResizeType type = Truncate);
	virtual 				~G42Resize (void) {}
	virtual 				G42ImageFilter * MakeCopy (void) const;
protected:
	void					HandleFilter (void);
	virtual void		ProcessFrame (void);
	uint32				NewHeight;
	uint32				NewWidth;
	G42ResizeType		ResizeType;
};
#endif
