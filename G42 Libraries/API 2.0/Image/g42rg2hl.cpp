#include "g42idata.h"
#include "g42iter.h"
void
RGBtoHLS(G42ImageData * data)
{
	if ((data->GetNumPalette() != 0) || (data->GetDepth() <= 8))
		return;
	G42ImageData24BitIterator image_it(data);
	do
	{
		int32 max, min;
		int32 lum, s, h, pr, pg, pb, clip;
		register int32 tval, d1, d2;
		pb = image_it.GetBlue32();
		pg = image_it.GetGreen32();
		pr = image_it.GetRed32();
		max = pr;
		if(pg > max)
			max = pg;
		if(pb > max)
			max = pb;
		min = pr;
		if(pg < min)
			min = pg;
		if(pb < min)
			min = pb;
		d1 = max + min;
		lum = d1 >> 1;
		if(max == min)
		{
			h = s = 0;
		}
		else
		{
			d2 = max - min;
			if(lum <= 127)
				s = ((d2 << 8) - d2)/d1;
			else
				s = ((d2 << 8) - d2)/(510 - d1);
			if(pr == max)
			{
				if(pg >= pb)
				{
					tval = pg - pb;
					h = ((tval * (int32)85)>>1)/d2;
				}
				else
				{
					tval = pb - pg;
					h = 255 - ((tval * (int32)85)>>1)/d2;
					if (h < 0) h = 0;
				}
			}
			else if(pg == max)
			{
				if(pb >= pr)
				{
					tval = pb - pr;
					h = 85 + ((tval * (int32)85)>>1)/d2;
				}
				else
				{
					tval = pr - pb;
					h = 85 - ((tval * (int32)85)>>1)/d2;
				}
			}
			else
			{
				if(pr >= pg)
				{
					tval = pr - pg;
					h = 170 + ((tval * (int32)85)>>1)/d2;
					if (h > 255) h = 255;
				}
				else
				{
					tval = pg - pr;
					h = 170 - ((tval * (int32)85)>>1)/d2;
				}
			}
		}
/**
***	added 04/25/95 by JRP and GES
***
***	Trying to solve the problem of color artifacts occurring
***/
		clip = (lum < 128) ? (lum<<2) : ((255 - lum)<<2);
		if (s > clip) s = clip;
		image_it.Set((unsigned char)h, (unsigned char)s, (unsigned char)lum);
		if (!image_it.Increment())
		{
			if (!image_it.NextRow())
				break;
		}
	}
	while (1);
}
void
HLStoRGB(G42ImageData * data)
{
	if ((data->GetNumPalette() != 0) || (data->GetDepth() <= 8))
		return;
	G42ImageData24BitIterator image_it(data);
	do
	{
		int32 d1, d2;
		int32 h;
		int32 r, g, b;
		int32 pr, pg, pb;
		pb = image_it.GetBlue32();
		pg = image_it.GetGreen32();
		pr = image_it.GetRed32();
		if(pg == 0)
		{
			r = g = b = pr;
		}
		else
		{
			if(pr <= 127)
				d2 = ((int32)pr * (int32)(pg + 255))/(int32)254;
			else
				d2 = ((int32)(pr + pg) - ((int32)(pr * pg)/(int32)255));
			d1 = (int32)(pr << 1) - d2;
			if(d1 < 0) d1 = 0;
			h = pb + 85;
			if(h > 255) h -= 255;
			if(h < 43)
				r = d1 + ((d2 - d1)*(int32)h)/43;
			else if(h < 128)
				r = d2;
			else if(h < 170)
				r = d1 + ((d2 - d1)*(int32)(170 - h))/43;
			else
				r = d1;
			h = pb;
			if(h < 43)
				g = d1 + ((d2 - d1)*(int32)h)/43;
			else if(h < 128)
				g = d2;
			else if(h < 170)
				g = d1 + ((d2 - d1)*(int32)(170 -h))/43;
			else
				g = d1;
			h = pb - 85;
			if(h < 0) h += 255;
			if(h < 43)
				b = d1 + ((d2 - d1)*(int32)h)/43;
			else if(h < 128)
				b = d2;
			else if(h < 170)
				b = d1 + ((d2 - d1)*(int32)(170 -h))/43;
			else
				b = d1;
		}
		image_it.Set((unsigned char)b, (unsigned char)g, (unsigned char)r);
		if (!image_it.Increment())
		{
			if (!image_it.NextRow())
				break;
		}
	}
	while (1);
}
G42Color *
RGBtoHLSPalette(G42Color * palette, int num_palette)
{
	G42Color * new_palette = new G42Color[num_palette];
	if (!new_palette)
   	return 0;
	for (uint y = 0; y < num_palette; y++)
	{
		int32 max, min;
		int32 lum, s, h, pr, pg, pb, clip;
		register int32 tval, d1, d2;
		pb = palette[y].blue;
		pg = palette[y].green;
		pr = palette[y].red;
		max = pr;
		if(pg > max)
			max = pg;
		if(pb > max)
			max = pb;
		min = pr;
		if(pg < min)
			min = pg;
		if(pb < min)
			min = pb;
		d1 = max + min;
		lum = d1 >> 1;
		if(max == min)
		{
			h = s = 0;
		}
		else
		{
			d2 = max - min;
			if(lum <= 127)
				s = ((d2 << 8) - d2)/d1;
			else
				s = ((d2 << 8) - d2)/(510 - d1);
			if(pr == max)
			{
				if(pg >= pb)
				{
					tval = pg - pb;
					h = ((tval * (int32)85)>>1)/d2;
				}
				else
				{
					tval = pb - pg;
					h = 255 - ((tval * (int32)85)>>1)/d2;
					if (h < 0) h = 0;
				}
			}
			else if(pg == max)
			{
				if(pb >= pr)
				{
					tval = pb - pr;
					h = 85 + ((tval * (int32)85)>>1)/d2;
				}
				else
				{
					tval = pr - pb;
					h = 85 - ((tval * (int32)85)>>1)/d2;
				}
			}
			else
			{
				if(pr >= pg)
				{
					tval = pr - pg;
					h = 170 + ((tval * (int32)85)>>1)/d2;
					if (h > 255) h = 255;
				}
				else
				{
					tval = pg - pr;
					h = 170 - ((tval * (int32)85)>>1)/d2;
				}
			}
		}
/**
***	added 04/25/95 by JRP and GES
***
***	Trying to solve the problem of color artifacts occurring
***/
		clip = (lum < 128) ? (lum<<2) : ((255 - lum)<<2);
		if (s > clip) s = clip;
		new_palette[y].blue = h;
		new_palette[y].green = s;
		new_palette[y].red = lum;
	}
   return new_palette;
}
G42Color *
HLStoRGBPalette(G42Color * palette, int num_palette)
{
	G42Color * new_palette = new G42Color[num_palette];
	if (!new_palette)
		return 0;
	for (uint y = 0; y < num_palette; y++)
	{
		int32 d1, d2;
		int32 h;
		int32 r, g, b;
		int32 pr, pg, pb;
		pb = palette[y].blue;
		pg = palette[y].green;
		pr = palette[y].red;
		if(pg == 0)
		{
			r = g = b = pr;
		}
		else
		{
			if(pr <= 127)
				d2 = ((int32)pr * (int32)(pg + 255))/(int32)254;
			else
				d2 = ((int32)(pr + pg) - ((int32)(pr * pg)/(int32)255));
			d1 = (int32)(pr << 1) - d2;
			if(d1 < 0) d1 = 0;
			h = pb + 85;
			if(h > 255) h -= 255;
			if(h < 43)
				r = d1 + ((d2 - d1)*(int32)h)/43;
			else if(h < 128)
				r = d2;
			else if(h < 170)
				r = d1 + ((d2 - d1)*(int32)(170 - h))/43;
			else
				r = d1;
			h = pb;
			if(h < 43)
				g = d1 + ((d2 - d1)*(int32)h)/43;
			else if(h < 128)
				g = d2;
			else if(h < 170)
				g = d1 + ((d2 - d1)*(int32)(170 -h))/43;
			else
				g = d1;
			h = pb - 85;
			if(h < 0) h += 255;
			if(h < 43)
				b = d1 + ((d2 - d1)*(int32)h)/43;
			else if(h < 128)
				b = d2;
			else if(h < 170)
				b = d1 + ((d2 - d1)*(int32)(170 -h))/43;
			else
				b = d1;
		}
		new_palette[y].blue = b;
		new_palette[y].green = g;
		new_palette[y].red = r;
	}
   return new_palette;
}
