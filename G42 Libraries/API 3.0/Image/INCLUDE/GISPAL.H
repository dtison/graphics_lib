// gispal.h - screen palette
#if !defined(GISPAL_H)
#define GISPAL_H
#include "giopal.h"
class G42ScreenPalette
{
public:
	enum G42ScreenType { TrueColor, Palette, Grayscale };
	enum G42PaletteMode { UseStandardPalette, OptimizeOnePalette,
		OptimizeMultiplePalettes };
	enum G42StandardPaletteType { Group42, Microsoft, Netscape, External,
		Custom };
#if 0
	class G42ImagePaletteData
	{
	public:
		G42ImagePaletteData(G42ScreenPalette & screen_pal);
		~G42ImagePaletteData(void);
		InvestigateImage(G42Image & image, bool temp_image = false);
		G42ScreenPalette & GetScreenPalette(void);
	private:
	};
#endif
//	friend G42ImagePaletteData;
	G42ScreenPalette(G42PaletteMode mode = OptimizeOnePalette,
		G42Surface surface = 0);
	G42ScreenPalette(G42StandardPaletteType type,
		uintf num_palette = 256, G42Surface surface = 0);
	~G42ScreenPalette(void);
	G42ScreenType GetScreenType(void) const
		{ return ScreenType; }
	byte GetScreenDepth(void) const
		{ return ScreenDepth; }
	const G42OptimizedPalette * GetOptimizedPalette(void) const
		{ return OptimizedPalette; }
	uint16 GetNumColors(void) const
		{ return (OptimizedPalette.NotNull() ?
			(uint16)OptimizedPalette->GetNumPalette() : (uint16)0); }
	const G42Color * GetScreenPalette(void) const
		{ return (OptimizedPalette.NotNull() ? OptimizedPalette->GetPalette() : 0); }
	void SetPaletteMode(G42PaletteMode mode);
	void SetStandardPaletteType(G42StandardPaletteType type,
		uintf num_palette = 256);
	// more work to be done here to investigate each image and build a palette
	// thus, but we'll go with a standard palette for now
private:
	G42ScreenType ScreenType;
	byte ScreenDepth; // for palette or grayscale screens only
	G42Ptr<G42OptimizedPalette> OptimizedPalette;
	G42PaletteMode Mode;
	G42StandardPaletteType Type;
	uintf NumPalette;
	G42Surface Surface;
	void SetOptimizedPalette(void);
	void InvestigateScreen(void);
};
#endif // GISPAL_H
