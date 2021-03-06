// gihls.h - hls color class, including converstions to/from G42Color
#if !defined(GIHLS_H)
#define GIHLS_H
#include "gitype.h"
struct G42Hls
{
public:
	G42Hls(void) {}
	G42Hls(const G42Hls & ref)
		: Hue(ref.Hue), Lum(ref.Lum), Sat(ref.Sat) {}
	G42Hls(byte hue, byte lum, byte sat)
		: Hue(hue), Lum(lum), Sat(sat) {}
	G42Hls(const G42Color & color);
	operator G42Color (void);
	G42Hls & operator = (const G42Hls & hls);
	G42Hls & operator = (const G42Color & color);
	byte Hue;
	byte Lum;
	byte Sat;
private:
	void RgbToHls(byte pr, byte pg, byte pb, byte & h, byte & lum, byte & s);
	void HlsToRgb(byte pr, byte pg, byte pb, byte & h, byte & lum, byte & s);
};
// while these are rather big to inline, they tend to be used in time
// critical loops, and they should compile small, so we'll give the
// compiler a chance to inline them if it wants
inline void
G42Hls::RgbToHls(byte pr, byte pg, byte pb, byte & h, byte & lum, byte & s)
{
	intf max = pr;
	if (pg > max)
	{
		max = pg;
	}
	if (pb > max)
	{
		max = pb;
	}
	intf min = pr;
	if (pg < min)
	{
		min = pg;
	}
	if (pb < min)
	{
		min = pb;
	}
	int32 d1 = max + min;
	lum = (byte)(d1 >> 1);
	if (max == min)
	{
		h = s = 0;
	}
	else
	{
		int32 d2 = max - min;
		if (lum <= 127)
		{
			s = (byte)(((d2 << 8) - d2) / d1);
		}
		else
		{
			s = (byte)(((d2 << 8) - d2) / (510 - d1));
		}
		if (pr == max)
		{
			if (pg >= pb)
			{
				int32 tval = pg - pb;
				h = (byte)(((tval * (int32)85) >> 1) / d2);
			}
			else
			{
				int32 tval = pb - pg;
				int32 hue = (byte)(255 - (int)(((tval * (int32)85) >> 1) / d2));
				if (hue < 0)
				{
					hue = 0;
				}
				h = (byte)hue;
			}
		}
		else if (pg == max)
		{
			if (pb >= pr)
			{
				int32 tval = pb - pr;
				h = (byte)(85 + (int)(((tval * (int32)85) >> 1) / d2));
			}
			else
			{
				int32 tval = pr - pb;
				h = (byte)(85 - (int)(((tval * (int32)85) >> 1) / d2));
			}
		}
		else
		{
			if (pr >= pg)
			{
				int32 tval = pr - pg;
				int32 hue = (byte)(170 + (int)(((tval * (int32)85) >> 1) / d2));
				if (hue > 255)
				{
					hue = 255;
				}
				h = (byte)hue;
			}
			else
			{
				int32 tval = pg - pr;
				h = (byte)(170 - (int)(((tval * (int32)85) >> 1) / d2));
			}
		}
	}
/**
***	added 04/25/95 by JRP and GES
***
***	Trying to solve the problem of color artifacts occurring
***/
	intf clip = (lum < 128) ? (lum << 2) : ((255 - lum) << 2);
	if (s > clip)
	{
		s = (byte)clip;
	}
}
inline void
G42Hls::HlsToRgb(byte h, byte lum, byte s, byte & r, byte & g, byte & b)
{
	if (s == 0)
	{
		r = g = b = lum;
	}
	else
	{
		int32 d2;
		if (lum <= 127)
			d2 = ((int32)lum * (int32)(s + 255)) / (int32)254;
		else
			d2 = ((int32)(lum + s) - ((int32)(lum * s) / (int32)255));
		int32 d1 = (int32)(lum << 1) - d2;
		if (d1 < 0)
			d1 = 0;
		int32 t = h + 85;
		if (t > 255)
			t -= 255;
		if (t < 43)
			r = (byte)(d1 + ((d2 - d1) * (int32)t) / 43);
		else if (t < 128)
			r = (byte)(d2);
		else if (t < 170)
			r = (byte)(d1 + ((d2 - d1) * (int32)(170 - t)) / 43);
		else
			r = (byte)(d1);
		t = h;
		if (t < 43)
			g = (byte)(d1 + ((d2 - d1) * (int32)t) / 43);
		else if (t < 128)
			g = (byte)(d2);
		else if (t < 170)
			g = (byte)(d1 + ((d2 - d1) * (int32)(170 - t)) / 43);
		else
			g = (byte)(d1);
		t = h - 85;
		if (t < 0)
			t += 255;
		if (t < 43)
			b = (byte)(d1 + ((d2 - d1) * (int32)t) / 43);
		else if(t < 128)
			b = (byte)(d2);
		else if(t < 170)
			b = (byte)(d1 + ((d2 - d1) * (int32)(170 - t)) / 43);
		else
			b = (byte)(d1);
	}
}
inline
G42Hls::G42Hls(const G42Color & color)
{
	RgbToHls(color.Red, color.Green, color.Blue, Hue, Lum, Sat);
}
inline
G42Hls::operator G42Color (void)
{
	G42Color color;
	HlsToRgb(Hue, Lum, Sat, color.Red, color.Green, color.Blue);
	return color;
}
inline G42Hls &
G42Hls::operator = (const G42Hls & hls)
{
	Hue = hls.Hue;
	Lum = hls.Lum;
	Sat = hls.Sat;
	return *this;
}
inline G42Hls &
G42Hls::operator = (const G42Color & color)
{
	RgbToHls(color.Red, color.Green, color.Blue, Hue, Lum, Sat);
	return *this;
}
#endif // GIHLS_H
