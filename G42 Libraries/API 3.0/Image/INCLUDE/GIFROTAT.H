#if ! defined (GIFROTATE_H)
#define GIFROTATE_H
#include "gifilter.h"
#include "girotate.h"
class G42Rotate : public G42ImageFilter
{
public:
	enum  G42RotateType {Rotate90, Rotate180, Rotate270, RotateAngle, 
									RotateAngleAntialias};
						G42Rotate (G42RotateType type = Rotate90, double angle = 90.0, 
							G42Color background = G42Color (0, 0, 0));
	virtual 			~G42Rotate (void) {}
	virtual 			G42ImageFilter * MakeCopy(void) const;
protected:
	void				HandleFilter (void);
	virtual void	ProcessFrame (void);
	double			Angle;
	G42Color			BackgroundColor;
	G42RotateType 	RotateType;
	G42Ptr <G42Rotation> Rotation;
};
#endif
