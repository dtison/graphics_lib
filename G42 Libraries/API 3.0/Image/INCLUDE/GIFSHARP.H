#if ! defined (GIFSHARP_H)
#define GIFSHARP_H
#include "gifilter.h"
#define PREV		0	
#define CURR 		1
#define NEXT		2
class G42Sharpen : public G42ImageFilter
{
public:	
	enum  				G42SharpenType {Highpass_Filter, Unsharp_Mask};
							G42Sharpen (G42SharpenType type = Unsharp_Mask, 
								int16 sharpness = 1);
	virtual 				~G42Sharpen (void) {}
	virtual 				G42ImageFilter * MakeCopy(void) const;
protected:
	void					HandleFilter (void);
	virtual void		ProcessFrame (void);
	G42SharpenType 	SharpenType;
	int16 				Sharpness;
};
#endif
