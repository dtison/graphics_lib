// gicimage.h - G42ImageController
#if !defined(GICIMAGE_H)
#define GICIMAGE_H
#include "giimage.h"
#include "gidisp.h"
#include "ggeomtry.h"
#include "gtools.h"
#include "gifilter.h"
class G42AreaController;
class G42ImageController
{
public:
	G42ImageController();
	~G42ImageController();
	void SetAreaController(G42AreaController * area_controller);
	void SetSurface(G42Surface surface);
	void SetSurfaceController(G42SurfaceController * surface);
	const G42ScaleTranslate & GetScaleTranslate(void) { return ScaleTranslate; }
	void SetScaleTranslate(const G42ScaleTranslate & trans);
	G42ScaleTranslate & GetCumlativeScaleTranslate(void) { return CumlativeScaleTranslate; }
	void SetCumlativeScaleTranslate(G42ScaleTranslate & trans);
	void SetDisplayScaleTranslate(void);
	G42Image * GetImage() { return Image; }
	const G42Image * GetImage() const { return Image; }
	G42Display * GetDisplay() { return Display; }
	const G42Display * GetDisplay() const { return Display; }
	const G42Rect & GetArea(void) const
		{ return Area; }
	uintf GetNumFilters(void) { return NumFilters; }
	void AppendFilter(const G42ImageFilter & filter)
		{ InsertFilter(filter, NumFilters); }
	void InsertFilter(const G42ImageFilter & filter, uintf index = 0);
	G42ImageFilter * GetFilter(uintf index)
		{ return Filters[index]; }
	void RemoveFilter(uintf index);
	void ClearFilters(void);
	bool GetNeedRebuild(void)
		{ return NeedRebuild; }
	void SetNeedRebuild(bool rebuild = true)
		{ NeedRebuild = rebuild; }
private:
	void SetArea(void);
	G42AreaController * AreaController;
	G42Ptr<G42Image> Image;
	G42Ptr<G42Display> Display;
	G42ScaleTranslate ScaleTranslate;
	G42ScaleTranslate CumlativeScaleTranslate;
	G42ExpandableArray< G42Ptr <G42ImageFilter> > Filters;
	uintf NumFilters;
	G42Rect Area;
	G42Margin Margin;
	bool NeedRebuild;
};
#endif // GICIMAGE_H
