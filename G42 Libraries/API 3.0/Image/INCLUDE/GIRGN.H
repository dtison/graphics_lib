/* G42Region Class
	It may be that this class really belongs in GGEOMTRY.H (and add a .CP) */
#if !defined(GIRGN_H)
#define GIRGN_H
#include "gmem.h"	// For Hack GetHeap() stuff
#include "gidata.h"
#include "gidesc.h"
#include "gicached.h"
/*  Image data for masks  */
typedef G42ImageDataGray1 DataGray1;
typedef G42ImageDataGray8 DataGray8;
/*  Write locked iterators  */
typedef G42ImageDataGray1::G42Iterator::G42Row::G42Iterator DataGray1Iterator;
typedef G42ImageDataGray8::G42Iterator::G42Row::G42Iterator DataGray8Iterator;
/*  Read locked iterators  */
typedef G42ImageDataGray1::G42ConstIterator::G42Row::G42Iterator DataGray1ReadIterator;
typedef G42ImageDataGray8::G42ConstIterator::G42Row::G42Iterator DataGray8ReadIterator;
class G42Region;
#ifdef MSWIN
typedef HRGN G42RegionHandle;
#endif	
#ifdef MACOS
typedef RgnHandle G42RegionHandle;
#endif
/*  Organizer class for G42Region  */
class G42RegionInfo
{
	friend G42Region;
public:
	enum G42RegionType {Rectangle, Ellipse, Polygon, Text};
	/*  Polygon constructor  */
	G42RegionInfo (G42Vector <G42Point> & points, int16 num_points, 
		bool expand = false);
	/*  Rectangle and Ellipse  */
	G42RegionInfo (G42Rect rect, G42RegionInfo::G42RegionType type, 
		bool expand = false);
	/*  Copy constructor  */
	G42RegionInfo (const G42RegionInfo & info);
	virtual				~G42RegionInfo (void);	
protected:
	bool					PointInRegion (const G42Point & point) const
	{
		#ifdef MSWIN
		int32 height_index = RegionBitsHeight - point.Y - 1;
		#endif
		#ifdef MACOS
		int32 height_index = point.Y;
		#endif
		return (bool)(*(RegionBits + RegionBitsRowBytes *
			height_index + (point.X >> 3)) &
				(1 << (7 - (point.X & 7))));
	}
	#ifdef MACOS
	void  					MacSetupRegionBits (CGrafPtr & port, 
									GDHandle & gdh);
	#endif
	#ifdef MSWIN
	void 						WinCleanupRegionBits (HBITMAP & bitmap, 
									HBITMAP & old_bitmap, HDC & dc);
	void 						WinSetupRegionBits (HBITMAP & bitmap, 
									HBITMAP & old_bitmap, HDC & dc);
	#endif
	// Variables
	#ifdef MACOS
	GWorldPtr				GWorld;
	#endif
	int16						NumPoints;
	G42APtr <byte>			RegionBits;				// to hold mono bit mask
	uint32					RegionBitsHeight;		// height of RegionBits
	uint32					RegionBitsRowBytes;	// a row offset in RegionBits
	uint32					RegionBitsWidth;		// width of RegionBits
	G42Rect					RegionBounds;
	G42Rect					RegionRect;
};
class G42Region
{
public:
	enum G42RegionType {Rectangle, Ellipse, Polygon, Text};
	/*  Polygon   */
	G42Region (G42Vector <G42Point> & points, int16 num_points);
	/*  Rectangle and Ellipse  */
	G42Region (G42Rect rect, G42Region::G42RegionType type = 
		G42Region::Rectangle);
	/*  For text regions  */
	G42Region (void);
	/*  Copy constructor  */
	G42Region (const G42Region & info);
	~G42Region (void){}
	bool						PointInRegion (const G42Point & point)
									{return RegionInfo -> PointInRegion (point);}
	byte						PointInRegionAntialias (const G42Point & point);
	/*  Stuff for SetHeap()  Hack  */
	static G42HeapManager * Heap;
	static G42HeapManager *	GetHeap (void) {return Heap;}
	static void				SetHeap (G42HeapManager * heap) {Heap = heap;}
protected:
	void 						AntiMaskFromRegionInfo (G42RegionInfo & info);
	// Variables
	G42Vector <G42Ptr		<G42ImageData> >	AntiMaskData;
	G42Ptr 					<G42ImageDescriptor> AntiMaskDescriptor;
	G42Ptr 					<G42ReadCachedImage <DataGray8, DataGray8ReadIterator> >
									AntiMaskedImage;
	bool						HaveAntiMask;
	int16						NumPoints;
	G42Vector <G42Point> Points;
	G42Ptr 					<G42RegionInfo> RegionInfo;
	G42Rect					RegionRect;
	G42RegionType			RegionType;
};
#endif // GIRGN_H sentry
