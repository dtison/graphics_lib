// gimdesc.h - multiple image descriptor
#if !defined(GIAMDESC_H)
#define GIAMDESC_H
#include "gpobj.h"
#include "gitype.h"
class G42MultipleImageDescriptor : public G42PathObject
{
public:
	G42MultipleImageDescriptor(void);
	G42MultipleImageDescriptor(uint32 num_images);
	virtual ~G42MultipleImageDescriptor(void);
	void Set(uint32 num_images = 0);
	uint16 GetNumPalette(void) const
		{ return NumPalette; }
	const G42Color * GetPalette (void) const
		{ return GlobalPalette; }
	G42Color * GetPalette (void)
		{ return GlobalPalette; }
	void SetNumPalette (uint16 num_palette);
	uint16 GetNumGrays (void) const
		{ return NumGrays; }
	void SetNumGrays (uint16 num_grays);
	const G42Color & GetBackgroundColor(void) const
		{ return BackgroundColor; }
	bool HasBackgroundColor(void) const
		{ return BackgroundColorSet; }
	bool HasBackgroundPixel (void) const
		{ return (BackgroundPixel >= 0); }
	byte GetBackgroundPixel (void) const
		{ return (byte)BackgroundPixel; }
	void ClearBackgroundColor(void);
	void SetBackgroundPixel (int16 back_pixel);
	void SetBackgroundColor(G42Color & back_color);
	uint32 GetNumImages(void) const
		{ return NumImages; }
	void SetNumImages(uint32 num)
		{ NumImages = num; }
	virtual G42MultipleImageDescriptor * MakeCopy(void);
protected:
	G42MultipleImageDescriptor(G42MultipleImageDescriptor & ref);
	void Copy(G42MultipleImageDescriptor & ref);
private:
	G42MultipleImageDescriptor & operator = (G42MultipleImageDescriptor &)
		{ return *this; }
	uint32				NumImages; // 0 if unknown
	uint16				NumPalette; // 0 if gray or true color
	uint16				NumGrays; // 0 if paletted or true color
	G42APtr<G42Color>	GlobalPalette;
	bool					BackgroundColorSet;
	G42Color				BackgroundColor;
	int16					BackgroundPixel;
};
#endif // GIAMDESC_H
