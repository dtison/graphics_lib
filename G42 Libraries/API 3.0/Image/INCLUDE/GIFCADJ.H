// gicoladj.h - color adjust: gamma, brightness, contrast filters
#if !defined(GICOLADJ_H)
#define GICOLADJ_H
/*
	This filter does brightness, contrast, and gamma control.  Brightness
	and contrast numbers do nothing at 0, and do the most at 50 or -50,
	where 50 increases the brightness/contrast by 50%, and -50 decreases
	the brightness/contrast by 50%.  Numbers above/below 50/-50 tend to
	give odd results.
	Gamma adjustment of 1.0 does not normally adjust anything.  A gamma
	of 2.0 brightens the middle of the range, a gamma of 0.5 darkens the
	middle.  Valid gamma values run between 0.1 and 5.0.
*/
#include "gitype.h"
#include "gifilter.h"
#include "gtools.h"
#include "gihls.h"
#include "gidesc.h"
#include "gidata.h"
	template <class Pixel>
	class G42ColorAdjustFunction
	{
	public:
		G42ColorAdjustFunction(byte * map, byte * sat_map, Pixel *) : Map(map),
			SatMap(sat_map) {}
		G42Color operator () (const Pixel & c)
		{
			G42Hls hls((G42Color)c);
			hls.Lum = Map[hls.Lum];
			hls.Sat = SatMap[hls.Sat];
			return ((G42Color)hls);
		}
	private:
		byte * Map;
		byte * SatMap;
	};
	// class for template converter
	class G42GrayAdjustFunction
	{
	public:
		G42GrayAdjustFunction(byte * map) : Map(map) {}
		byte operator () (byte b)
			{ return Map[b]; }
	private:
		byte * Map;
	};
	template <class Pixel>
	class G42RedAdjustFunction
	{
	public:
		G42RedAdjustFunction(byte * map) : Map(map) {}
		G42Color operator () (const Pixel & p)
		{
			G42Color c(p);
			c.Red = Map[c.Red];
			return c;
		}
	private:
		byte * Map;
	};
	template <class Pixel>
	class G42GreenAdjustFunction
	{
	public:
		G42GreenAdjustFunction(byte * map) : Map(map) {}
		G42Color operator () (const Pixel & p)
		{
			G42Color c(p);
			c.Green = Map[c.Green];
			return c;
		}
	private:
		byte * Map;
	};
	template <class Pixel>
	class G42BlueAdjustFunction
	{
	public:
		G42BlueAdjustFunction(byte * map) : Map(map) {}
		G42Color operator () (const Pixel & p)
		{
			G42Color c(p);
			c.Blue = Map[c.Blue];
			return c;
		}
	private:
		byte * Map;
	};
	class G42ColorAdjustConvertMono
	{
	public:
		G42ColorAdjustConvertMono(byte * map) : Map(map) {}
		byte operator () (bool p)
		{
			return Map[p ? 1 : 0];
		}
	private:
		byte * Map;
	};
class G42ImageColorAdjust : public G42ImageFilter
{
public:
	enum G42Channel { All, Red, Green, Blue };
	G42ImageColorAdjust(intf brightness, intf contrast, double gamma,
		G42Channel channel = All);
	virtual ~G42ImageColorAdjust(void);
//	virtual void Process(G42PathObject * object, G42PathComponent * sender);
	virtual void HandleDescriptor(G42ImageDescriptor * descriptor);
	virtual void HandleData(G42ImageData * data);
	virtual void HandleEnd(G42PathEnd * end);
	virtual G42ImageFilter * MakeCopy(void) const;
protected:
	bool PaletteMode;
	bool NeedNewData;
	intf Brightness;
	intf Contrast;
	double Gamma;
	G42Channel Channel;
	G42APtr<byte> Map;
	G42APtr<byte> SatMap;
	byte MonoMap[2];
	G42ImageDataColor8::G42Iterator CurrentOutputIteratorColor8;
	G42ImageDataGray8::G42Iterator CurrentOutputIteratorGray8;
	uint32 OutputRowsLeft;
	G42ImageDataBase::G42TransIterator OutputTransIterator;
	uint32 CurrentStripNum;
	G42Color AdjustColor(const G42Color & c);
};
#endif // GICOLADJ_H
