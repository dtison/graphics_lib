// gidither.h - header file for dithering
#if !defined(GIDITHER_H)
#define GIDITHER_H
#include "giopal.h"
#include "gidesc.h"
#include "gidata.h"
#include "gifilter.h"
class G42OldDitherError
{
public:
	G42OldDitherError(int width, int clip = 256)
	:
		Width(width + 2), ErrorRow(0), NextErrorRow(0),
		ErrorPtr(0), NextErrorPtr(0), Clip(clip)
	{
		ErrorRow = new int [Width];
		NextErrorRow = new int [Width];
		memset(ErrorRow, 0, Width * sizeof (int));
		memset(NextErrorRow, 0, Width * sizeof (int));
		ErrorPtr = ErrorRow + 1;
		NextErrorPtr = NextErrorRow;
		Clip2 = Clip << 1;
		Clip3 = Clip + Clip2;
	}
	~G42OldDitherError(void)
	{
		delete [] ErrorRow;
		delete [] NextErrorRow;
	}
	int GetError(void)
	{
		int err = *ErrorPtr++;
		if (err >= Clip3)
			err = Clip2;
		else if (err > Clip)
			err = (Clip + ((err - Clip) >> 1));
		else if (err <= -Clip3)
			err = -Clip2;
		else if (err < -Clip)
			err = (-Clip - ((-err - Clip) >> 1));
		return err;
	}
	void SetError(int err)
	{
		int error1;
		int error3;
		int error5;
		if (err > 0)
		{
			error1 = (err >> 4);
			error3 = (err >> 3) + error1;
			error5 = (err >> 2) + error1;
		}
		else
		{
			error1 = -((-err) >> 4);
			error3 = -((-err) >> 3) + error1;
			error5 = -((-err) >> 2) + error1;
		}
		*(NextErrorPtr++) += error3;
		*(NextErrorPtr++) += error5;
		*(NextErrorPtr--) += error1;
		*ErrorPtr += err - (error1 + error3 + error5);
	}
	void NextRow(void)
	{
		int * temp = ErrorRow;
		ErrorRow = NextErrorRow;
		NextErrorRow = temp;
		memset(NextErrorRow, 0, Width * sizeof (int));
		ErrorPtr = ErrorRow + 1;
		NextErrorPtr = NextErrorRow;
	}
protected:
	int Width;
	int * ErrorRow;
	int * NextErrorRow;
	int * ErrorPtr;
	int * NextErrorPtr;
	int Clip;
	int Clip2;
	int Clip3;
};
// a row converter for color dithering
template <class ConvertToColor, class Pixel>
class G42ColorDitherCopyRowConverter
{
public:
	G42ColorDitherCopyRowConverter(ConvertToColor convert,
		const G42OptimizedPalette * opt_pal, G42OldDitherError & red_error,
		G42OldDitherError & green_error, G42OldDitherError & blue_error,
		Pixel *);
	byte operator () (Pixel pixel)
	{
		G42Color c(Convert(pixel));
		int red = c.Red + RedError.GetError();
		int green = c.Green + GreenError.GetError();
		int blue = c.Blue + BlueError.GetError();
		if (red < 0)
			red = 0;
		if (red > 255)
			red = 255;
		if (green < 0)
			green = 0;
		if (green > 255)
			green = 255;
		if (blue < 0)
			blue = 0;
		if (blue > 255)
			blue = 255;
		byte val = (byte)(OptPal->GetPixel(G42Color(
			(byte)red, (byte)green, (byte)blue)));
		RedError.SetError(red - TruePalette[val].Red);
		GreenError.SetError(green - TruePalette[val].Green);
		BlueError.SetError(blue - TruePalette[val].Blue);
		return val;
	}
	void operator ++ (int)
	{
		RedError.NextRow();
		GreenError.NextRow();
		BlueError.NextRow();
	}
private:
	ConvertToColor Convert;
	const G42OptimizedPalette * OptPal;
	const G42Color * TruePalette;
	G42OldDitherError & RedError;
	G42OldDitherError & GreenError;
	G42OldDitherError & BlueError;
};
template <class ConvertToColor, class Pixel>
G42ColorDitherCopyRowConverter<ConvertToColor, Pixel>::
G42ColorDitherCopyRowConverter(ConvertToColor convert,
	const G42OptimizedPalette * opt_pal, G42OldDitherError & red_error,
	G42OldDitherError & green_error, G42OldDitherError & blue_error,
	Pixel *)
:
	Convert(convert), RedError(red_error), GreenError(green_error),
	BlueError(blue_error), OptPal(opt_pal), TruePalette(opt_pal->GetPalette())
{
}
template <class ConvertToColor, class Pixel>
class G42ColorMatchCopyConverter
{
public:
	G42ColorMatchCopyConverter(ConvertToColor convert,
		const G42OptimizedPalette * opt_pal, Pixel *)
	:
		Convert(convert), OptPal(opt_pal)
	{
	}
	byte operator () (Pixel pixel)
	{
		G42Color c(Convert(pixel));
		byte matched_pixel = (byte)(OptPal->GetPixel(c));
		return matched_pixel;
	}
private:
	ConvertToColor Convert;
	const G42OptimizedPalette * OptPal;
};
template <class ConvertToGray, class Pixel>
class G42GrayDitherCopyRowConverter
{
public:
	G42GrayDitherCopyRowConverter(ConvertToGray convert,
		uintf num_grays, G42OldDitherError & gray_error, Pixel *);
	byte operator () (Pixel pixel)
	{
		intf gray(Convert(pixel));
		gray += GrayError.GetError();
		if (gray < 0)
			gray = 0;
		if (gray > 255)
			gray = 255;
		byte pixel = Map[gray];
		GrayError.SetError(gray - InverseMap[pixel]);
		return pixel;
	}
	void operator ++ (int)
	{
		GrayError.NextRow();
	}
private:
	ConvertToGray Convert;
	G42OldDitherError & GrayError;
	G42Ptr<byte> Map;
	G42Ptr<byte> InverseMap;
};
template <class ConvertToGray, class Pixel>
G42GrayDitherCopyRowConverter<ConvertToGray, Pixel>::
	G42GrayDitherCopyRowConverter(ConvertToGray convert,
	uintf num_grays, G42OldDitherError & gray_error, Pixel *)
:
	Convert(convert), NumGrays(num_grays), GrayError(gray_error)
{
	Map.Set(new byte [256]);
	InverseMap.Set(new byte [num_grays]);
	uint32 max_gray = num_grays - 1;
	uint32 half_gray = (num_grays >> 1);
	for (intf i = 0; i < 256; i++)
	{
		Map[i] = (byte)(((uint32)i * max_gray + (uint32)128) / (uint32)255);
	}
	for (intf j = 0; j < num_grays; j++)
	{
		InverseMap[j] = (byte)(((uint32)i * (uint32)255 + half_gray) /
			max_gray);
	}
}
template <class ConvertToGray, class Pixel>
class G42Gray1DitherCopyRowConverter
{
public:
	G42Gray1DitherCopyRowConverter(ConvertToGray convert,
		G42OldDitherError & gray_error, Pixel *)
	:
		Convert(convert), GrayError(gray_error)
	{
	}
	bool operator () (Pixel pixel)
	{
		int gray(Convert(pixel))
		gray += GrayError.GetError();
		if (gray < 0)
			gray = 0;
		if (gray > 255)
			gray = 255;
		bool pixel = (gray > 80);
		GrayError.SetError(pixel ? gray - 255 : gray);
		return pixel;
	}
	void operator ++ (int)
	{
		GrayError.NextRow();
	}
private:
	ConvertToGray Convert;
	G42OldDitherError & GrayError;
};
template <class ConvertToGray, class Pixel>
class G42GrayMatchCopyConverter
{
public:
	G42GrayMatchCopyConverter(ConvertToGray convert, uintf num_grays, Pixel *);
	byte operator () (Pixel pixel)
	{
		intf gray(Convert(pixel));
		byte pixel = Map[gray];
		return pixel;
	}
private:
	ConvertToGray Convert;
	G42Ptr<byte> Map;
};
template <class ConvertToGray, class Pixel>
G42GrayMatchCopyConverter<ConvertToGray, Pixel>::
G42GrayMatchCopyConverter(ConvertToGray convert, uintf num_grays, Pixel *)
:
	Convert(convert), NumGrays(num_grays)
{
	Map.Set(new byte [256]);
	uint32 max_gray = num_grays - 1;
	for (intf i = 0; i < 256; i++)
	{
		Map[i] = (byte)(((uint32)i * max_gray + (uint32)128) / (uint32)255);
	}
}
template <class ConvertToGray, class Pixel>
class G42Gray1MatchCopyConverter
{
public:
	G42Gray1MatchCopyConverter(ConvertToGray convert, Pixel *)
	:
		Convert(convert)
	{
	}
	bool operator () (Pixel pixel)
	{
		int gray(Convert(pixel))
		bool pixel = (gray > 80);
		return pixel;
	}
private:
	ConvertToGray Convert;
};
class G42ColorReduce : public G42ImageFilter
{
public:
	G42ColorReduce(const G42OptimizedPalette * opt_pal, bool match = false);
	virtual ~G42ColorReduce(void);
	virtual void Process(G42PathObject * object, G42PathComponent * sender);
	void Reset(void);
	G42ImageFilter * MakeCopy(void) const;
private:
	void ResetOutput(G42ImageData * strip);
	void CreateConverter(void);
	bool Match;
	const G42OptimizedPalette * OptPal;
	G42Ptr<G42ImageDescriptor> OldDescriptor;
	G42Ptr<G42ImageDescriptor> NewDescriptor;
	G42Ptr<G42OldDitherError> RedError;
	G42Ptr<G42OldDitherError> GreenError;
	G42Ptr<G42OldDitherError> BlueError;
	uint32 CurrentStripNum;
	G42Ptr<G42ImageDataColor8> CurrentData;
	G42Ptr<G42WriteImageData> CurrentDataLock;
	uint32 OutputRowsLeft;
	G42Ptr<G42ColorDitherCopyRowConverter<
		G42ImageDataColor32::G42ConvertToG42Color, G42ConstColor32> >
			DitherColor32;
	G42Ptr<G42ColorDitherCopyRowConverter<
		G42ImageDataColor24::G42ConvertToG42Color,
		G42ConstColor24> > DitherColor24;
	G42Ptr<G42ColorDitherCopyRowConverter<
		G42ImageDataColor8::G42ConvertToG42Color, byte> > DitherColor8;
	G42Ptr<G42ColorDitherCopyRowConverter<G42ImageDataGray8::G42ConvertToG42Color,
		byte> > DitherGray8;
	G42Ptr<G42ColorDitherCopyRowConverter<G42ImageDataGray1::G42ConvertToG42Color,
		bool> > DitherGray1;
	G42Ptr<G42ColorMatchCopyConverter<
		G42ImageDataColor32::G42ConvertToG42Color, G42ConstColor32> >
			MatchColor32;
	G42Ptr<G42ColorMatchCopyConverter<
		G42ImageDataColor24::G42ConvertToG42Color,
		G42ConstColor24> > MatchColor24;
	G42Ptr<G42ColorMatchCopyConverter<
		G42ImageDataColor8::G42ConvertToG42Color, byte> > MatchColor8;
	G42Ptr<G42ColorMatchCopyConverter<G42ImageDataGray8::G42ConvertToG42Color,
		byte> > MatchGray8;
	G42Ptr<G42ColorMatchCopyConverter<G42ImageDataGray1::G42ConvertToG42Color,
		bool> > MatchGray1;
	G42ImageDataColor8::G42Iterator CurrentOutputIterator;
	G42ImageDataBase::G42TransIterator OutputTransIterator;
};
#endif // GIDITHER_H sentry
