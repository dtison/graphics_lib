// gitweak.h - image tweaker
#if !defined (GITWEAK_H)
#define GITWEAK_H

#include "giimage.h"

class G42ImageTweaker
{
public:
	G42ImageTweaker(G42Image & image, uint32 frame_index = 0);
	~G42ImageTweaker(void);

	G42Color GetColorAtPoint(const G42Point & point);
//	void SetColorAtPoint(const G42Point & point, const G42Color & color);

private:
	G42Image & Image;
	uint32 FrameIndex;
//	G42ReadCashedImage<DC24, IC24L> ReadColor24;
};

#endif // GITWEAK_H
