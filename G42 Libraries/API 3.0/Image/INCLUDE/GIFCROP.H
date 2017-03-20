#if ! defined (GIFCROP_H)
#define GIFCROP_H
#include "gifilter.h"
class G42CropOutsideImageException : public G42Exception
{
public:
	G42CropOutsideImageException(int32 error_id = 0)
		: G42Exception(error_id) {}
};
class G42Crop : public G42ImageFilter
{
public:	
							G42Crop (const G42Rect & crop_rect);
	virtual 				~G42Crop (void) {}
	virtual 				G42ImageFilter * MakeCopy(void) const;
protected:
	void					HandleFilter (void);
	virtual void		ProcessFrame (void);
	G42Rect				CropRect;
};
#endif
