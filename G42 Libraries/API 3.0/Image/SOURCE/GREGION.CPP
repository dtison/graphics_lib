
#include "gregion.h"
G42Region::G42Region (G42Vector <G42Point> & points, int16 num_points, 
	bool create_mask) : RegionHandle (0), RegionType (G42Region::Polygon)
{
	for (int16 i = 0; i < num_points; i++) 
	{
		Points [i] = points [i];
	}
	CreateRegion();
	if (create_mask) {
		MaskImage = MaskFromRegionHandle (RegionHandle);
	}
	RegionRect.SetNull();
}
G42Region::G42Region (G42Rect rect, G42Region::G42RegionType type,
	bool create_mask) : RegionHandle (0), RegionType (type)
{
	Points [0] = rect.GetTopLeft();
	Points [1] = rect.GetBottomRight();
	CreateRegion();
	if (create_mask) {
		MaskImage = MaskFromRegionHandle (RegionHandle);
	}
	RegionRect = rect;
}
G42Region::~G42Region (void)
{
	#ifdef MSWIN
	if (RegionHandle)
	{
	}
	#endif
	#ifdef MACOS
	if (RegionHandle)
	{
		::DisposeRgn (RegionHandle);
		RegionHandle = 0;
	}
	#endif
}
void
G42Region::CreateRegion()
{
	switch (RegionType)
	{
		case Rectangle:
			CreateRectRegion();
		break;
		case Ellipse:
			CreateEllipseRegion();
		break;
		case Polygon:
			CreatePolyRegion();
		break;	
	}
}
void
G42Region::CreateRectRegion (void)
{
	/*  Make a rectangle region  */
	#ifdef MSWIN
	
	#endif
	#ifdef MACOS
	/*  Clear the region. (This is the same as delete and re-allocate)  */
	if (RegionHandle) {
		::SetEmptyRgn (RegionHandle);
	}
	G42ThrowIfNull (RegionHandle);
	::RectRgn (RegionHandle, &((Rect) RegionRect));
	/*  Set Region Bounds  */	
	RegionBounds = (**RegionHandle).rgnBBox;
	#endif
}
void
G42Region::CreateEllipseRegion (void)
{
	/*  Make an ellipse region  */
	#ifdef MSWIN
	
	#endif
	#ifdef MACOS
	/*  Clear the region. (This is the same as delete and re-allocate)  */
	if (RegionHandle) {
		::SetEmptyRgn (RegionHandle);
	}
	G42ThrowIfNull (RegionHandle);
	::OpenRgn();
	::FrameOval (&((Rect) RegionRect));
	::CloseRgn (RegionHandle);
	/*  Set Region Bounds  */	
	RegionBounds = (**RegionHandle).rgnBBox;
	#endif
}
void
G42Region::CreatePolyRegion (void)
{
	/*  Make a polygon region  */
	#ifdef MSWIN
	
	#endif
	#ifdef MACOS
	/*  Clear the region. (This is the same as delete and re-allocate)  */
	if (RegionHandle) {
		::SetEmptyRgn (RegionHandle);
	}
	G42ThrowIfNull (RegionHandle);
	::OpenRgn();
	::MoveTo (Points [0].X, Points [0].Y);
	for (int16 i = 1; i < NumPoints; i++)
	{
		::LineTo (Points [i].X, Points [i].Y);
	}
	::LineTo (Points [0].X, Points [0].Y);
	::CloseRgn (RegionHandle);
	/*  Set Region Bounds  */	
	RegionBounds = (**RegionHandle).rgnBBox;
	#endif
}
const G42Image *
G42Region::GetMaskImage (void)
{
	if (! MaskImage) {
		MaskImage = MaskFromRegionHandle (RegionHandle);
	}
	return MaskImage;	
}
G42Image *
G42Region::MaskFromRegionHandle (G42RegionHandle handle)
{
	/*  Go thru points in bounding rectangle and test if point is in region  */
	if (handle) {}
	return 0;
}
bool
G42Region::PointInRegion (G42Point & point)
{
	bool value;
	if (MaskImage)
	{
		/*  Test point in mask image  */
	}
	else
	{
		#ifdef MSWIN
		
		#endif
		#ifdef MACOS
		/*  Call Mac OS point test  */
		Point pointm;
		pointm.h = point.X;
		pointm.v = point.Y;
		value = ::PtInRgn (pointm, RegionHandle);
		#endif	
	}
	return value;
}
//123456789012345678901234567890123456789012345678901234567890123456789012345678
