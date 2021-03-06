#if ! defined (GIROTATE_H)
#define GIROTATE_H
#include "gifilter.h"
#include <math.h>
class G42Rotation
{
public:
	enum  G42AngleType {Degrees, Radians};
						G42Rotation (const G42Size & size, double angle = 90.0, 
							G42AngleType angle_type = Degrees);
	virtual 			~G42Rotation (void) {}
	G42Size			GetDestSize (void) 
							{return G42Size (DestWidth, DestHeight);}
	G42Point			GetDestAxis (void) {return DestAxis;}
	G42Size			GetSourceSize (void) 
							{return G42Size (SourceWidth, SourceHeight);}
	G42Point			GetSourceAxis (void) {return SourceAxis;}
	void				RotatePoint (G42Point & point);
	void				RotatePoint (G42Point & point, double & row_delta, 
							double & col_delta);
protected:
	// Variables
	double			Angle;
	double			CosAngle;
	int32				DestHeight;
	int32				DestWidth;
	double			SinAngle;
	int32				SourceHeight;
	int32				SourceWidth;
	G42Point			DestAxis;
	G42Point			SourceAxis;
	G42Point			Offset;
};
/*  Point comes back rotated  */
inline void
G42Rotation::RotatePoint (G42Point & point, double & row_delta, 
	double & col_delta)
{
 	double x = (double) point.X;
	double y = (double) point.Y;
  	double rotated_x = (((x * CosAngle) - (y * SinAngle)) +
				(double) DestAxis.X - (double) Offset.X) + 0.5;
	double rotated_y = (((y * CosAngle) + (x * SinAngle)) +
				(double) DestAxis.Y - (double) Offset.Y) + 0.5;
	point.X = (int32) rotated_x;
	point.Y = (int32) rotated_y;	
	/*  Find deltas  */
	row_delta = rotated_y - (double) point.Y;
	col_delta = rotated_x - (double) point.X;
	if (row_delta < 0) {
		row_delta += 1.0;
	}
	if (col_delta < 0) {
		col_delta += 1.0;
	}
}	
// Want to reduce duplication if possible
inline void
G42Rotation::RotatePoint (G42Point & point)
{
 	double x = (double) point.X;
	double y = (double) point.Y;
  	double rotated_x = (((x * CosAngle) - (y * SinAngle)) +
				(double) DestAxis.X - (double) Offset.X) + 0.5;
	double rotated_y = (((y * CosAngle) + (x * SinAngle)) +
				(double) DestAxis.Y - (double) Offset.Y) + 0.5;
	point.X = (int32) rotated_x;
	point.Y = (int32) rotated_y;		
}	
#endif
