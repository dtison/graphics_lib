#include "girotate.h"
/*  Generalized (low-level) rotation class.  
    Given an angle, it handles mathematics and rotates points  */
G42Rotation::G42Rotation (const G42Size & size, double angle, 
	G42Rotation::G42AngleType angle_type)
: Angle (angle)
{
	/*  Size should remain unchanged  */
	SourceWidth  = size.Width;
	SourceHeight = size.Height;
	/* Change these on every angle change */
	if (angle_type == G42Rotation::Degrees) {
		Angle *= -(3.14159 / 180.0); 
	}
	CosAngle = cos (Angle);
	SinAngle = sin (Angle);
	double tmp_sin_angle = SinAngle;		// (Need to flip negatives)
	if (tmp_sin_angle < 0) {
		tmp_sin_angle *= -1;
	}
	double tmp_cos_angle = CosAngle;
	if (tmp_cos_angle < 0) {
		tmp_cos_angle *= -1;
	}
	/* Calculate the destination image size */
	DestWidth = (uint32) (((double) SourceWidth * tmp_cos_angle) + 
		((double) SourceHeight * tmp_sin_angle));
	DestHeight = (uint32)(((double) SourceHeight * tmp_cos_angle) + 
		((double) SourceWidth * tmp_sin_angle));
	/* Centered rotation points (* axes *) for source and dest images */
	DestAxis = G42Point (DestWidth >> 1, DestHeight >> 1);
	SourceAxis = G42Point (SourceWidth >> 1, SourceHeight >> 1);
	/* And offset value */
	Offset = G42Point (DestAxis.X - SourceAxis.X + 2, DestAxis.Y - SourceAxis.Y + 2);
}
