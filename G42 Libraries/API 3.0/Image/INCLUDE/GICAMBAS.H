// gicamult.h - G42AreaMultiController
#if !defined(GICAMBAS_H)
#define GICAMBAS_H
#include "gicasing.h"
// not really intended to be instigated directly, more intended to
// build thumbnails and animated editors from.  Has all the knowledge
// of how to deal with thumbnailed images, none on how to build them
class G42MultiAreaBaseController : public G42AreaSingleController
{
public:
	G42MultiAreaBaseController(G42Notifier * notifier = 0);
	~G42MultiAreaBaseController(void);
	virtual void Draw(G42Rect & rect, G42DrawLocation location = 0,
		bool image_only = false);
	virtual void Rebuild(G42Rect & rect, G42DrawLocation location = 0,
		bool image_only = false);
//	virtual void SetScaleTranslate(G42ScaleTranslate & trans);
//	virtual void SetCumlativeScaleTranslate(G42ScaleTranslate & trans);
	virtual void SetImageScaleTranslate(void);
	virtual void SetArea(void);
	virtual void SetViewport (const G42Rect & viewport);
	virtual void SetMouseKeyController(G42MouseKeyController * mk);
	virtual G42Notifier * SetNotifier(G42Notifier * notifier);
	bool GetThumbnailMode(void) const
		{ return DrawThumbs; }
	virtual void SetThumbnailMode(bool thumbnail_mode = true);
	virtual void ResetImageFilters(void);
	void SetThumbDimensions(uintf width, uintf height,
		uintf space_between, uintf space_below, uintf text_space,
		uintf text_space_above);
	uintf GetThumbWidth(void) const
		{ return ThumbWidth; }
	uintf GetThumbHeight(void) const
		{ return ThumbHeight; }
	uintf GetThumbSpaceBetween(void) const
		{ return SpaceBetween; }
	uintf GetThumbSpaceBelow(void) const
		{ return SpaceBelow; }
	uintf GetThumbTextSpace(void) const
		{ return TextSpace; }
	uintf GetThumbTextSpaceAbove(void) const
		{ return TextSpaceAbove; }
	enum G42LimitDirection { LimitDown, LimitAcross };
	enum G42LimitType { LimitViewport, LimitPixels, LimitThumbs };
	void SetThumbLimit (G42LimitDirection direction, G42LimitType type,
		uintf num);
	G42LimitDirection GetLimitDirection(void) const
		{ return (GrowDown ? LimitAcross : LimitDown); }
	G42LimitType GetLimitType(void) const
		{ return (FitToViewport ? LimitViewport :
			(AutoArrangeThumbs ? LimitPixels : LimitThumbs)); }
	uintf GetNumThumbsAcross(void) const
		{ return NumThumbsAcross; }
	uintf GetNumThumbsDown(void) const
		{ return NumThumbsDown; }
	uintf GetPixelsAcross(void) const
		{ return AreaWidth; }
	uintf GetPixelsDown(void) const
		{ return AreaHeight; }
	uint32 GetThumbAtPoint(const G42Point & p) const;
	uint32 GetInsertIndexAtPoint(const G42Point & p) const;
	void SetSelected(uintf index, bool selected = true);
	bool IsSelected(uintf index);
	void ToggleSelected(uintf index);
	void ClearAllSelected(void);
	void SelectAll(void);
	uintf GetNumSelected(void);
	uint32 GetNumImages(void) const
		{ return NumImages; }
	void DeleteThumb(uint32 thumb, bool copy_strings = true);
	void MoveThumb(uint32 old_index, uint32 new_index, bool copy_strings = true);
protected:
	void ResetThumbsAcross(void);
	void ResetThumbLocation(uint32 index);
	void ResetGrowth(void);
	void DrawText(uintf i, G42DrawLocation location);
	void DrawSelected(uintf i, G42DrawLocation location);
	G42Vector< G42Ptr<G42ImageController> > ImageControllers; // the actual images
	uint32 NumImages;
	uintf ThumbWidth;
	uintf ThumbHeight;
	uintf SpaceBetween;
	uintf SpaceBelow;
	uintf TextSpace;
	uintf TextSpaceAbove;
	bool DrawThumbs;
	bool GrowDown; // if false, grows across
	bool FitToViewport;
	bool AutoArrangeThumbs;
	uintf AreaWidth;
	uintf AreaHeight;
	uint32 NumThumbsAcross;
	uint32 NumThumbsDown;
	G42Vector <G42APtr <char> > StringArray;
	G42Vector <bool> Selected;
	uint32 NumSelected;
};
#endif // GICAMULT_H
