// g42opal.cpp - optimized palette
/*************************************************************
	File:          g42opal.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Optimize a palette for an image
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "gitype.h"
#include "giopal.h"
#include "gidata.h"
#include "gidesc.h"
#include "gilab.h"
//#include "gesdebug.h"
G42OptimizedPalette::~G42OptimizedPalette()
{
	if (DitherUnique)
		delete[] DitherUnique;
	if (DitherTable)
		delete[] DitherTable;
	if (DitherPalette)
		delete[] DitherPalette;
	if (Avoid)
		delete [] Avoid;
	if (AvoidColor)
		delete [] AvoidColor;
	if (AvoidPixel)
		delete [] AvoidPixel;
}
void
G42OptimizedPalette::G42PaletteBox::ResetBoxLimits(void)
{
	rmin = gmin = bmin = CHANNEL_MASK;
	rmax = gmax = bmax = 0;
	for (uint16 j = 0; j < nh; j++)
	{
		uint32 i = h[j];
		uint16 r = (uint16)((i >> NUM_BITS2) & CHANNEL_MASK);
		uint16 g = (uint16)((i >> NUM_BITS) & CHANNEL_MASK);
		uint16 b = (uint16)(i & CHANNEL_MASK);
		if (r > rmax)
			rmax = r;
		if (r < rmin)
			rmin = r;
		if (g > gmax)
			gmax = g;
		if (g < gmin)
			gmin = g;
		if (b > bmax)
			bmax = b;
		if (b < bmin)
			bmin = b;
	}
	num_force = 0;
	ResetBoxInfo();
}
void
G42OptimizedPalette::G42PaletteBox::ResetForceBoxLimits(uint32 * force_h)
{
	frmin = fgmin = fbmin = CHANNEL_MASK;
	frmax = fgmax = fbmax = 0;
	for (uint16 j = 0; j < num_force; j++)
	{
		uint32 i = force_h[force_index[j]];
		uint16 r = (uint16)((i >> NUM_BITS2) & CHANNEL_MASK);
		uint16 g = (uint16)((i >> NUM_BITS) & CHANNEL_MASK);
		uint16 b = (uint16)(i & CHANNEL_MASK);
		if (r > frmax)
			frmax = r;
		if (r < frmin)
			frmin = r;
		if (g > fgmax)
			fgmax = g;
		if (g < fgmin)
			fgmin = g;
		if (b > fbmax)
			fbmax = b;
		if (b < fbmin)
			fbmin = b;
	}
}
void
G42OptimizedPalette::G42PaletteBox::ResetBoxInfo(void)
{
	int32 dr = (int32)(rmax - rmin);
	int32 dg = (int32)(gmax - gmin);
	int32 db = (int32)(bmax - bmin);
	diag_squared = (dr * dr + dg * dg + db * db);
}
// build a palette through a path
G42OptimizedPalette::G42OptimizedPalette(intf num_palette, bool auto_finish)
:
	Ticks(0), UseLab(true), AutoFinish(auto_finish),
	MaxPalette(num_palette), NumPalette(0),
	DitherTable(0), DitherUnique(0),
	DitherPalette(0), WhitePixel(0), NeedWhite(false),
	FullColorOnly(true),
	NumForceH(0), NumForce(0),
	AddColorList(0), NumAdd(0), NumAvoid(0), AvoidPixel(0),
	Avoid(0), AvoidColor(0), h(0)
{
	h = new uint32 [NUM_COLORS];
	memset(h, 0, NUM_COLORS * sizeof (uint32));
	DitherUnique = new byte [NUM_COLORS];
	DitherTable = new byte [NUM_COLORS];
	NeedOptimize = false;
	DitherPalette = new G42Color[256];
	SetupAvoid();
	MaxPalette -= NumAvoid;
}
void
G42OptimizedPalette::Process(G42PathObject * object, G42PathComponent *)
{
	if (dynamic_cast<G42PathBegin *>(object))
	{
		// should check sender here, etc.
	}
	else if (dynamic_cast<G42ImageDescriptor *>(object))
	{
		G42ImageDescriptor * desc = dynamic_cast<G42ImageDescriptor *>(object);
		if (Receiver)
		{
			Descriptor = new G42ImageDescriptor(*desc);
		}
		else
		{
			Descriptor = desc;
			object = 0;
		}
	}
	else if (dynamic_cast<G42ImageData *>(object))
	{
		G42ImageData * data = dynamic_cast<G42ImageData *>(object);
//		uint32 start = GetTickCount();
		InvestigateData(data);
//		Ticks += GetTickCount() - start;
	}
	else if (dynamic_cast<G42PathEnd *>(object))
	{
		if (AutoFinish)
		{
			FinishOptimization();
		}
		if (Receiver)
		{
			Receiver->Process(object, this);
		}
		else
		{
			delete object;
		}
		Receiver = 0;
		object = 0;
	}
	if (Receiver && object)
	{
		Receiver->Process(object, this);
	}
	else if (object)
	{
		delete object;
	}
}
void
G42OptimizedPalette::FinishOptimization(void)
{
	if (NeedOptimize)
		OptimizeData();
	if (NumPalette)
		PrepareTable();
	if (h)
	{
		delete[] h;
		h = 0;
	}
}
// build a standard palette
G42OptimizedPalette::G42OptimizedPalette(G42PaletteType pal_type,
	intf num_pal)
:
	MaxPalette(num_pal), // DataArray(0),
	NumPalette(num_pal), NumDataArray(0),
	UseLab(false),
	NumForceH(0), NumForce(0),
	DitherTable(0), DitherUnique(0),
	DitherPalette(0), WhitePixel(0), NeedWhite(false),
	FullColorOnly(true), NeedOptimize(true),
	StandardPaletteType(pal_type),
	AddColorList(0), NumAdd(0), NumAvoid(0), AvoidPixel(0),
	Avoid(0), AvoidColor(0)
{
	h = new uint32 [NUM_COLORS];
	memset(h, 0, NUM_COLORS * sizeof (uint32));
	DitherUnique = new byte [NUM_COLORS];
	DitherTable = new byte [NUM_COLORS];
	DitherPalette = new G42Color[256];
	SetupAvoid();
	CreateStandardPalette();
	if (NumPalette)
		PrepareTable();
	delete[] h;
}
// build a standard palette with transparency colors
G42OptimizedPalette::G42OptimizedPalette(G42PaletteType pal_type,
	G42Color * add_color, intf num_add, intf num_avoid)
:
	MaxPalette(16), // DataArray(0),
	UseLab(false),
	NumForceH(0), NumForce(0),
	NumPalette(16), NumDataArray(0),
	DitherTable(0), DitherUnique(0),
	DitherPalette(0), WhitePixel(0), NeedWhite(false),
	FullColorOnly(true), NeedOptimize(true),
	StandardPaletteType(pal_type),
	AddColorList(add_color), NumAdd(num_add), NumAvoid(num_avoid), AvoidPixel(0),
	Avoid(0), AvoidColor(0)
{
	h = new uint32 [NUM_COLORS];
	memset(h, 0, NUM_COLORS * sizeof (uint32));
	DitherUnique = new byte [NUM_COLORS];
	DitherTable = new byte [NUM_COLORS];
	DitherPalette = new G42Color[256];
	SetupAvoid();
	CreateStandardPalette();
	if (NumPalette)
		PrepareTable();
	delete[] h;
}
// these constructors are from the 2.0 libraries.  I'll keep them until
// we are sure we don't need them anymore
#if 0
G42OptimizedPalette::G42OptimizedPalette(
	const G42Color * pal, intf num_pal,
	G42Color * add_color, intf num_add, intf num_avoid)
:
	MaxPalette(num_pal), DataArray(0),
	NumPalette(num_pal), NumDataArray(0),
	DitherTable(0), DitherUnique(0),
	DitherPalette(0), WhitePixel(0), NeedWhite(false),
	NumForceH(0), NumForce(0),
	FullColorOnly(true), NeedOptimize(true),
	AddColorList(add_color), NumAdd(num_add), NumAvoid(num_avoid), AvoidPixel(0),
	Avoid(0), AvoidColor(0)
{
	h = new uint32 [NUM_COLORS];
	memset(h, 0, NUM_COLORS * sizeof (uint32));
	DitherUnique = new byte [NUM_COLORS];
	DitherTable = new byte [NUM_COLORS];
	DitherPalette = new G42Color[256];
	SetupAvoid();
	for (intf i = 0; i < NumPalette; i++)
	{
		DitherPalette[i] = pal[i];
	}
	if (NumAvoid)
		AddAvoid();
	if (NumPalette)
		PrepareTable();
	delete[] h;
}
#endif // end 2.0 libraries constructors
// forces color to be part of palette
void
G42OptimizedPalette::SetForceColor(G42Color c)
{
	AddColor(c);
	uint32 fh;
	if (UseLab)
	{
		G42Lab lab(c);
		fh = RGBTo16(lab.L, lab.A + 128, lab.B + 128);
	}
	else
	{
		fh = ColorTo16(c);
	}
	bool need_add = true;
	for (uintf f = 0; f < NumForceH; f++)
	{
		if (ForceH[f] == fh)
		{
			need_add = false;
			MaxPalette--; // make room for color at end of palette
			ForceInH[NumForce] = false;
			break;
		}
	}
	if (need_add)
	{
		ForceHToColor[NumForceH] = NumForce;
		ForceH[NumForceH++] = fh;
		ForceInH[NumForce] = true;
	}
	ForceColor[NumForce++] = c;
}
byte
G42OptimizedPalette::GetAvoidPixel(G42Color & c)
{
	for (intf i = 0; i < NumAvoid; i++)
	{
		if (c.Red == AvoidColor[i].Red && c.Green == AvoidColor[i].Green &&
			c.Blue == AvoidColor[i].Blue)
		{
			return (byte)AvoidPixel[i];
		}
	}
	return 0;
}
byte
G42OptimizedPalette::GetAnyPixel(G42Color & c)
{
	for (intf i = 0; i < NumAvoid; i++)
	{
		if (c.Red == AvoidColor[i].Red && c.Green == AvoidColor[i].Green &&
			c.Blue == AvoidColor[i].Blue)
		{
			return (byte)AvoidPixel[i];
		}
	}
	return GetPixel(c);
}
void
G42OptimizedPalette::CreateStandardPalette(void)
{
	if (StandardPaletteType == PaletteNetscape && MaxPalette >= 216)
	{
		NumPalette = 216;
		MaxPalette = 216;
		WhitePixel = 215;
		intf pixel = 0;
		for (intf red = 0; red < 256; red += 51)
		{
			for (intf green = 0; green < 256; green += 51)
			{
				for (intf blue = 0; blue < 256; blue += 51)
				{
					DitherPalette[pixel].Red = (byte)red;
					DitherPalette[pixel].Green = (byte)green;
					DitherPalette[pixel].Blue = (byte)blue;
					pixel++;
				}
			}
		}
	}
	else if (MaxPalette < 16)
	{
		DitherPalette[0].Red = 0;
		DitherPalette[0].Green = 0;
		DitherPalette[0].Blue = 0;
		DitherPalette[1].Red = 0xff;
		DitherPalette[1].Green = 0xff;
		DitherPalette[1].Blue = 0xff;
		NumPalette = 2;
		WhitePixel = 1;
	}
	else if (MaxPalette < 216)
	{
		DitherPalette[0].Red =
		DitherPalette[0].Green =
		DitherPalette[0].Blue = 0x0;
		DitherPalette[1].Red = 0x80;
		DitherPalette[1].Green = 0x00;
		DitherPalette[1].Blue = 0x00;
		DitherPalette[2].Red = 0x00;
		DitherPalette[2].Green = 0x80;
		DitherPalette[2].Blue = 0x00;
		DitherPalette[3].Red = 0x80;
		DitherPalette[3].Green = 0x80;
		DitherPalette[3].Blue = 0x00;
		DitherPalette[4].Red = 0x00;
		DitherPalette[4].Green = 0x00;
		DitherPalette[4].Blue = 0x80;
		DitherPalette[5].Red = 0x80;
		DitherPalette[5].Green = 0x00;
		DitherPalette[5].Blue = 0x80;
		DitherPalette[6].Red = 0x00;
		DitherPalette[6].Green = 0x80;
		DitherPalette[6].Blue = 0x80;
		DitherPalette[7].Red = 0x80;
		DitherPalette[7].Green = 0x80;
		DitherPalette[7].Blue = 0x80;
		DitherPalette[8].Red =
		DitherPalette[8].Green =
		DitherPalette[8].Blue = 0xc0;
		DitherPalette[9].Red = 0xff;
		DitherPalette[9].Green = 0x00;
		DitherPalette[9].Blue = 0x00;
		DitherPalette[10].Red = 0x00;
		DitherPalette[10].Green = 0xff;
		DitherPalette[10].Blue = 0x00;
		DitherPalette[11].Red = 0xff;
		DitherPalette[11].Green = 0xff;
		DitherPalette[11].Blue = 0x00;
		DitherPalette[12].Red = 0x00;
		DitherPalette[12].Green = 0x00;
		DitherPalette[12].Blue = 0xff;
		DitherPalette[13].Red = 0xff;
		DitherPalette[13].Green = 0x00;
		DitherPalette[13].Blue = 0xff;
		DitherPalette[14].Red = 0x00;
		DitherPalette[14].Green = 0xff;
		DitherPalette[14].Blue = 0xff;
		DitherPalette[15].Red = 0xff;
		DitherPalette[15].Green = 0xff;
		DitherPalette[15].Blue = 0xff;
		NumPalette = 16;
		WhitePixel = 15;
	}
	else
	{
#if 0
		intf i;
		for (i = 0; i < 16; i++)
		{
			intf gp = i;
			DitherPalette[i].Red =
			DitherPalette[i].Green =
			DitherPalette[i].Blue = (byte)((gp << 4) | gp);
		}
		for (i = 0; i < 192; i++)
		{
			intf rp = (i >> 5);
			DitherPalette[i + 16].Red =
				(byte)(((rp << 5) + (rp << 4) + (rp << 1) + rp));
			intf gp = ((i >> 2) & 7);
			DitherPalette[i + 16].Green =
				(byte)((gp << 5) | (gp << 2) | (gp >> 1));
			intf bp = (i & 3);
			DitherPalette[i + 16].Blue =
				(byte)((bp << 6) | (bp << 4) | (bp << 2) | bp);
		}
#endif
		intf i;
		for (i = 0; i < 192; i++)
		{
			intf rp = (i >> 5);
			DitherPalette[i].Red =
				(byte)(((rp << 5) + (rp << 4) + (rp << 1) + rp));
			intf gp = ((i >> 2) & 7);
			DitherPalette[i].Green =
				(byte)((gp << 5) | (gp << 2) | (gp >> 1));
			intf bp = (i & 3);
			DitherPalette[i].Blue =
				(byte)((bp << 6) | (bp << 4) | (bp << 2) | bp);
		}
		for (i = 192; i < 208; i++)
		{
			intf gp = (i - 192);
			DitherPalette[i].Red =
			DitherPalette[i].Green =
			DitherPalette[i].Blue = (byte)((gp << 4) | gp);
		}
		DitherPalette[208].Red =
		DitherPalette[208].Green =
		DitherPalette[208].Blue = 0xc0; // special one, because black is pixel 0
		DitherPalette[209].Red = 0x00;
		DitherPalette[209].Green = 0x00;
		DitherPalette[209].Blue = 0x80;
		DitherPalette[210].Red = 0x00;
		DitherPalette[210].Green = 0x80;
		DitherPalette[210].Blue = 0x00;
		DitherPalette[211].Red = 0x00;
		DitherPalette[211].Green = 0x80;
		DitherPalette[211].Blue = 0x80;
		DitherPalette[212].Red = 0x80;
		DitherPalette[212].Green = 0x00;
		DitherPalette[212].Blue = 0x00;
		DitherPalette[213].Red = 0x80;
		DitherPalette[213].Green = 0x00;
		DitherPalette[213].Blue = 0x80;
		DitherPalette[214].Red = 0x80;
		DitherPalette[214].Green = 0x80;
		DitherPalette[214].Blue = 0x00;
		DitherPalette[215].Red = 0x80;
		DitherPalette[215].Green = 0x80;
		DitherPalette[215].Blue = 0x80;
		NumPalette = 216;
		WhitePixel = 207;
	}
	if (NumAvoid)
		AddAvoid();
	if (UseLab)
	{
		for (uintf i = 0; i < NumPalette; i++)
		{
			LabPalette[i] = DitherPalette[i];
		}
	}
}
void
G42OptimizedPalette::AddAvoid(void)
{
	if (NumAvoid)
	{
		for (intf i = 0; i < NumAvoid; i++)
		{
			intf dist = 1000;
			intf pixel = 0;
			for (intf j = 0; j < NumPalette; j++)
			{
				intf d = abs((intf)DitherPalette[j].Red - (intf)AvoidColor[i].Red) +
					abs((intf)DitherPalette[j].Green - (intf)AvoidColor[i].Green) +
					abs((intf)DitherPalette[j].Blue - (intf)AvoidColor[i].Blue);
				if (d < dist)
				{
					dist = d;
					pixel = j;
				}
			}
			Avoid[pixel] = true;
			AvoidPixel[i] = pixel;
		}
	}
}
void
G42OptimizedPalette::HandleAvoid(void)
{
	if (NumAdd)
	{
		intf i;
		for (i = 0; i < NumAvoid; i++)
		{
			intf offset = 0;
			for (intf j = 0; j < NumPalette; j++)
			{
				if (DitherPalette[j].Red == AddColorList[i].Red &&
					DitherPalette[j].Green == AddColorList[i].Green &&
					DitherPalette[j].Blue == AddColorList[i].Blue)
				{
					offset++;
					if (DitherPalette[j].Red >= 128)
						DitherPalette[j].Red -= (byte)offset;
					else
						DitherPalette[j].Red += (byte)offset;
					if (DitherPalette[j].Green >= 128)
						DitherPalette[j].Green -= (byte)offset;
					else
						DitherPalette[j].Green += (byte)offset;
					if (DitherPalette[j].Blue >= 128)
						DitherPalette[j].Blue -= (byte)offset;
					else
						DitherPalette[j].Blue += (byte)offset;
					j--; // recheck same color
					continue;
				}
				offset = 0;
			}
		}
		for (i = NumAvoid; i < NumAdd; i++)
		{
			bool need_add = true;
			for (intf j = 0; j < NumPalette; j++)
			{
				if (DitherPalette[j].Red == AvoidColor[i].Red &&
					DitherPalette[j].Green == AvoidColor[i].Green &&
					DitherPalette[j].Blue == AvoidColor[i].Blue)
				{
					need_add = false;
					break;
				}
			}
			if (need_add)
			{
				DitherPalette[NumPalette] = AddColorList[i];
				NumPalette++;
			}
		}
		for (intf j = 0; j < NumAvoid; j++)
		{
			DitherPalette[NumPalette] = AddColorList[j];
			Avoid[NumPalette] = true;
			AvoidPixel[j] = NumPalette;
			NumPalette++;
		}
	}
}
void
G42OptimizedPalette::InvestigateData(G42ImageData * data)
{
	// this first case could probably be simplified greatly, but it should work
	// this way also, so I'm not going to take the chance of introducing
	// bugs - ges
	switch (Descriptor->GetImageType())
	{
		case G42ImageDescriptor::Gray1:
		{
			G42ImageDataGray1 * data1 = dynamic_cast<G42ImageDataGray1 *>(
				data);
			G42ReadImageData lock(*data);
			G42ImageDataGray1::G42ConstIterator i = data1->GetConstBegin(lock);
			for (intf j = 0; j < 2; j++)
			{
				DitherUnique[j] = 0;
			}
			intf count = 0;
			while (i != data1->GetConstEnd(lock))
			{
				G42ImageDataGray1::G42ConstIterator::G42Row r = *i;
				for (G42ImageDataGray1::G42ConstIterator::G42Row::G42Iterator j =
					r.GetBegin(); j != r.GetEnd(); j++)
				{
					intf v = (*j ? 1 : 0);
					if (!DitherUnique[v])
					{
						DitherUnique[v] = 1;
						count++;
						if (count == 2)
						{
							break;
						}
					}
				}
				if (count == 2)
				{
					break;
				}
				i++;
			}
			if (DitherUnique[0])
			{
				AddGray(0);
			}
			if (DitherUnique[1])
			{
				AddGray(0xff);
			}
			break;
		}
		case G42ImageDescriptor::Gray8:
		{
			G42ImageDataGray8 * data8 = dynamic_cast<G42ImageDataGray8 *>(
				data);
			G42ReadImageData lock(*data);
			G42ImageDataGray8::G42ConstIterator i = data8->GetConstBegin(lock);
			for (intf j = 0; j < 256; j++)
			{
				DitherUnique[j] = 0;
			}
			intf count = 0;
			while (i != data8->GetConstEnd(lock))
			{
				G42ImageDataGray8::G42ConstIterator::G42Row r = *i;
				for (G42ImageDataGray8::G42ConstIterator::G42Row::G42Iterator j =
					r.GetBegin(); j != r.GetEnd(); j++)
				{
					intf v = *j;
					if (!DitherUnique[v])
					{
						DitherUnique[v] = 1;
						count++;
						if (count == 256)
						{
							break;
						}
					}
				}
				if (count == 256)
				{
					break;
				}
				i++;
			}
			for (intf k = 0; k < 256; k++)
			{
				if (DitherUnique[k])
				{
					AddGray((byte)k);
				}
			}
			break;
		}
		case G42ImageDescriptor::Color8:
		{
			G42ImageDataColor8 * data8 = dynamic_cast<G42ImageDataColor8 *>(
				data);
			G42ReadImageData lock(*data);
			G42ImageDataColor8::G42ConstIterator i = data8->GetConstBegin(lock);
			for (intf j = 0; j < 256; j++)
			{
				DitherUnique[j] = 0;
			}
			intf count = 0;
			while (i != data8->GetConstEnd(lock))
			{
				G42ImageDataColor8::G42ConstIterator::G42Row r = *i;
				for (G42ImageDataColor8::G42ConstIterator::G42Row::G42Iterator j =
					r.GetBegin(); j != r.GetEnd(); j++)
				{
					intf v = *j;
					if (!DitherUnique[v])
					{
						DitherUnique[v] = 1;
						count++;
						if (count == 256)
						{
							break;
						}
					}
				}
				if (count == 256)
				{
					break;
				}
				i++;
			}
			const G42Color * palette = Descriptor->GetPalette();
			for (intf k = 0; k < 256; k++)
			{
				if (DitherUnique[k])
				{
					AddColor(palette[k]);
				}
			}
			break;
		}
		case G42ImageDescriptor::Color24:
		{
			if (NumPalette && !NeedOptimize)
			{
				for (uintf i = 0; i < NumPalette; i++)
				{
					if (UseLab)
					{
						G42Lab lab(DitherPalette[i]);
						h[RGBTo16(lab.L, lab.A + 128, lab.B + 128)] = 1;
					}
					else
					{
						h[ColorTo16(DitherPalette[i])] = 1;
					}
				}
			}
			NeedOptimize = true;
			G42ImageDataColor24 * data24 = dynamic_cast<G42ImageDataColor24 *>(
				data);
			G42ReadImageData lock(*data);
			G42ImageDataColor24::G42ConstIterator i = data24->GetConstBegin(lock);
			while (i != data24->GetConstEnd(lock))
			{
				G42ImageDataColor24::G42ConstIterator::G42Row r = *i;
				for (G42ImageDataColor24::G42ConstIterator::G42Row::G42Iterator j =
					r.GetBegin(); j != r.GetEnd(); j++)
				{
					if (UseLab)
					{
						G42Color c = (G42ConstColor24)(*j);
						G42Lab lab(c);
						h[RGBTo16(lab.L, lab.A + 128, lab.B + 128)] = 1;
					}
					else
					{
						h[ColorTo16((G42Color)(G42ConstColor24)(*j))] = 1;
					}
				}
				i++;
			}
			break;
		}
		case G42ImageDescriptor::Color32:
		{
			if (NumPalette && !NeedOptimize)
			{
				for (uintf i = 0; i < NumPalette; i++)
				{
					if (UseLab)
					{
						G42Lab lab(DitherPalette[i]);
						h[RGBTo16(lab.L, lab.A + 128, lab.B + 128)] = 1;
					}
					else
					{
						h[ColorTo16(DitherPalette[i])] = 1;
					}
				}
			}
			NeedOptimize = true;
			G42ImageDataColor32 * data32 = dynamic_cast<G42ImageDataColor32 *>(
				data);
			G42ReadImageData lock(*data);
			G42ImageDataColor32::G42ConstIterator i = data32->GetConstBegin(lock);
			while (i != data32->GetConstEnd(lock))
			{
				G42ImageDataColor32::G42ConstIterator::G42Row r = *i;
				for (G42ImageDataColor32::G42ConstIterator::G42Row::G42Iterator j =
					r.GetBegin(); j != r.GetEnd(); j++)
				{
					if (UseLab)
					{
						G42Color c = (G42ConstColor32)(*j);
						G42Lab lab(c);
						h[RGBTo16(lab.L, lab.A + 128, lab.B + 128)] = 1;
					}
					else
					{
						h[ColorTo16((G42Color)(G42ConstColor32)(*j))] = 1;
					}
				}
				i++;
			}
			break;
		}
	}
#if 0
	intf d = data->GetDepth();
	if (d == 1)
	{
		G42Color * palette = data->GetPalette();
		bool has_white = false;
		uint32 i;
		for (i = 0; i < 2; i++)
		{
			DitherUnique[i] = 0;
		}
		intf count = 0;
		for (intf ty = 0; ty < data->GetNumTilesDown(); ty++)
		{
			for (intf tx = 0; tx < data->GetNumTilesAcross(); tx++)
			{
				G42LockedImageTile tile(data->GetTile(ty, tx));
				//if (!tile || !tile.GetTile()->IsValid())
					//continue;
				for (intf y = tile.GetTile()->GetTileOffsetY();
					y < tile.GetTile()->GetTileOffsetY() +
						tile.GetTile()->GetTileHeight(); y++)
				{
					byte * cp1 = *tile.GetTile()->GetRow(y) +
						(tile.GetTile()->GetTileOffsetX() >> 3);
					intf s = 7 - (tile.GetTile()->GetTileOffsetX() & 7);
					for (intf x = tile.GetTile()->GetTileOffsetX();
						x < tile.GetTile()->GetTileOffsetX() +
							tile.GetTile()->GetTileWidth(); x++)
					{
						intf v = ((*cp1) >> s) & 1;
						if (!DitherUnique[v])
						{
							DitherUnique[v] = 1;
							if (palette && palette[v].red == 0xff &&
								palette[v].green == 0xff && palette[v].blue == 0xff)
									has_white = true;
							count++;
							if (count == 2)
								break;
						}
						if (s)
						{
							s--;
						}
						else
						{
							s = 7;
							cp1++;
						}
					}
					if (count == 2)
						break;
				}
				if (count == 2)
					break;
			}
			if (count == 2)
				break;
		}
		if (palette)
		{
			for (i = 0; i < 2; i++)
			{
				if (DitherUnique[i])
					AddColor(palette[i]);
			}
			if (NeedWhite && !has_white)
			{
				AddGray(0xff);
			}
		}
		else
		{
			for (i = 0; i < 2; i++)
			{
				if (DitherUnique[i])
					AddGray((byte)(i * 0xff));
			}
			if (NeedWhite && !DitherUnique[1])
				AddGray(0xff);
		}
	}
	else if (d == 8)
	{
		G42Color * palette = data->GetPalette();
		bool has_white = false;
		for (intf i = 0; i < 256; i++)
		{
			DitherUnique[i] = 0;
		}
		intf count = 0;
		for (intf ty = 0; ty < data->GetNumTilesDown(); ty++)
		{
			for (intf tx = 0; tx < data->GetNumTilesAcross(); tx++)
			{
				G42LockedImageTile tile(data->GetTile(ty, tx));
				//if (!tile || !tile.GetTile()->IsValid())
					//continue;
				for (intf y = tile.GetTile()->GetTileOffsetY();
					y < tile.GetTile()->GetTileOffsetY() + tile.GetTile()->GetTileHeight(); y++)
				{
					byte * cp1 = *tile.GetTile()->GetRow(y) + tile.GetTile()->GetTileOffsetX();
					for (intf x = tile.GetTile()->GetTileOffsetX();
						x < tile.GetTile()->GetTileOffsetX() + tile.GetTile()->GetTileWidth(); x++)
					{
						intf v = *cp1++;
						if (!DitherUnique[v])
						{
							DitherUnique[v] = 1;
							if (palette && palette[v].red == 0xff &&
								palette[v].green == 0xff && palette[v].blue == 0xff)
									has_white = true;
							count++;
							if (count == 256)
								break;
						}
					}
					if (count == 256)
						break;
				}
				if (count == 256)
					break;
			}
			if (count == 256)
				break;
		}
		if (palette)
		{
			for (intf i = 0; i < 256; i++)
			{
				if (DitherUnique[i])
					AddColor(palette[i]);
			}
			if (NeedWhite && !has_white)
			{
				AddGray(0xff);
			}
		}
		else
		{
			for (intf i = 0; i < 256; i++)
			{
				if (DitherUnique[i])
					AddGray((byte)i);
			}
			if (NeedWhite && !DitherUnique[255])
				AddGray(0xff);
		}
	}
	else if (d > 8)
	{
		if (NumPalette && !NeedOptimize)
		{
			for (intf i = 0; i < NumPalette; i++)
			{
				h[ColorTo16(DitherPalette[i])] = 1;
			}
		}
		NeedOptimize = true;
		if (NeedWhite)
		{
			G42Color c(0xff, 0xff, 0xff);
			intf v = ColorTo16(c);
			h[v] = 1;
		}
		for (intf ty = 0; ty < data->GetNumTilesDown(); ty++)
		{
			for (intf tx = 0; tx < data->GetNumTilesAcross(); tx++)
			{
				G42LockedImageTile tile(data->GetTile(ty, tx));
				if (!tile.GetTile() || !tile.GetTile()->IsValid())
					continue;
				for (intf y = tile.GetTile()->GetTileOffsetY();
					y < tile.GetTile()->GetTileOffsetY() + tile.GetTile()->GetTileHeight(); y++)
				{
					byte * cp1 = *tile.GetTile()->GetRow(y) + tile.GetTile()->GetTileOffsetX() * 3;
					for (intf x = tile.GetTile()->GetTileOffsetX();
						x < tile.GetTile()->GetTileOffsetX() + tile.GetTile()->GetTileWidth(); x++)
					{
						G42Color c;
						c.blue = *cp1++;
						c.green = *cp1++;
						c.red = *cp1++;
						intf v = ColorTo16(c);
						h[v] = 1;
					}
				}
			}
		}
	}
#endif
}
void
G42OptimizedPalette::AddColor(G42Color c)
{
	if (NeedOptimize)
	{
		if (UseLab)
		{
			G42Lab lab(c);
			h[RGBTo16(lab.L, lab.A + 128, lab.B + 128)] = 1;
		}
		else
		{
			h[ColorTo16(c)] = 1;
		}
		return;
	}
	bool found = false;
	for (intf i = 0; i < NumPalette; i++)
	{
		if (DitherPalette[i] == c)
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		if (NumPalette < MaxPalette)
		{
			DitherPalette[NumPalette++] = c;
		}
		else
		{
			NeedOptimize = true;
			for (intf i = 0; i < NumPalette; i++)
			{
				if (UseLab)
				{
					G42Color c = DitherPalette[i];
					G42Lab lab(c);
					h[RGBTo16(lab.L, lab.A + 128, lab.B + 128)] = 1;
				}
				else
				{
					h[ColorTo16(DitherPalette[i])] = 1;
				}
			}
			if (UseLab)
			{
				G42Lab lab(c);
				h[RGBTo16(lab.L, lab.A + 128, lab.B + 128)] = 1;
			}
			else
			{
				h[ColorTo16(c)] = 1;
			}
		}
	}
}
void
G42OptimizedPalette::OptimizeData(void)
{
	if (NeedOptimize)
	{
		uint32 i;
		for (i = 0, nh = 0; i < NUM_COLORS; i++)
		{
			if (h[i])
				h[nh++] = i;
			DitherUnique[i] = 2;
			DitherTable[i] = 0;
		}
		if (nh <= MaxPalette)
		{
			for (i = 0; i < nh; i++)
			{
				bool found_force = false;
				for (uintf f = 0; f < NumForceH; f++)
				{
					if (h[i] == ForceH[f])
					{
						DitherPalette[i] = ForceColor[ForceHToColor[f]];
						if (UseLab)
						{
							LabPalette[i].L = DitherPalette[NumPalette].Red;
							LabPalette[i].A =
								(int8)(((intf)DitherPalette[i].Green) - 128);
							LabPalette[i].B =
								(int8)(((intf)DitherPalette[i].Blue) - 128);
							DitherPalette[i] = LabPalette[i];
						}
						found_force = true;
						break;
					}
				}
				if (!found_force)
				{
					if (UseLab)
					{
						G42Lab lab;
						lab.L = (byte)(
							((((h[i] >> NUM_BITS2) & CHANNEL_MASK) << (8 - NUM_BITS)) |
							(((h[i] >> NUM_BITS2) & CHANNEL_MASK) >> ((NUM_BITS - 4) << 1))));
						lab.A = (int8)(-128 + (intf)
							((((h[i] >> NUM_BITS) & CHANNEL_MASK) << (8 - NUM_BITS)) |
							(((h[i] >> NUM_BITS) & CHANNEL_MASK) >> ((NUM_BITS - 4) << 1))));
						lab.B = (int8)(-128 + (intf)
							((((h[i]) & CHANNEL_MASK) << (8 - NUM_BITS)) |
							(((h[i]) & CHANNEL_MASK) >> ((NUM_BITS - 4) << 1))));
						DitherPalette[i] = lab;
						LabPalette[i] = lab;
					}
					else
					{
						DitherPalette[i].Red = (byte)(
							((((h[i] >> NUM_BITS2) & CHANNEL_MASK) << (8 - NUM_BITS)) |
							(((h[i] >> NUM_BITS2) & CHANNEL_MASK) >> ((NUM_BITS - 4) << 1))));
						DitherPalette[i].Green = (byte)(
							((((h[i] >> NUM_BITS) & CHANNEL_MASK) << (8 - NUM_BITS)) |
							(((h[i] >> NUM_BITS) & CHANNEL_MASK) >> ((NUM_BITS - 4) << 1))));
						DitherPalette[i].Blue = (byte)(
							((((h[i]) & CHANNEL_MASK) << (8 - NUM_BITS)) |
							(((h[i]) & CHANNEL_MASK) >> ((NUM_BITS - 4) << 1))));
					}
				}
			}
			NumPalette = nh;
			for (i = 0; i < NumForce; i++)
			{
				if (!ForceInH[i])
				{
					DitherPalette[NumPalette] = ForceColor[i];
					if (UseLab)
					{
						LabPalette[i] = DitherPalette[i];
					}
					NumPalette++;
				}
			}
		}
		else
		{
			intf num_box, cur_box;
			num_box = 1;
			PaletteBox[0].nh = nh;
			PaletteBox[0].h = h;
			PaletteBox[0].ResetBoxLimits();
			PaletteBox[0].force_index = BoxForceIndex;
			PaletteBox[0].num_force = NumForceH;
			for (uintf i = 0; i < NumForceH; i++)
			{
				PaletteBox[0].force_index[i] = (byte)i;
			}
//			intf rmin = PaletteBox[0].rmin;
//			intf rmax = PaletteBox[0].rmax;
//			intf gmin = PaletteBox[0].gmin;
//			intf gmax = PaletteBox[0].gmax;
//			intf bmin = PaletteBox[0].bmin;
//			intf bmax = PaletteBox[0].bmax;
			while (num_box < MaxPalette)
			{
				intf dr, dg, db, ctype;
				uint32 *lptr, *uptr;
				uint32 t;
				uint32 max_nh;
				uint32 max_side;
				intf i, split, start, end;
				intf count[256];
				uintf total_count;
				/* select next PaletteBox to split */
				max_nh = 1;
				max_side = 0;
				for (i = 0; i < num_box; i++)
				{
					if (PaletteBox[i].num_force > 1 && PaletteBox[i].nh > 1)
					{
						cur_box = i;
						max_nh = PaletteBox[i].nh;
						break;
					}
					uint32 cur_nh = PaletteBox[i].nh;
					uint32 cur_side = PaletteBox[i].diag_squared;
#if 0
					if (((PaletteBox[i].rmax - PaletteBox[i].rmin) >> 1) > cur_side)
					{
						cur_side = (uint32)((PaletteBox[i].rmax -
							PaletteBox[i].rmin) >> 1);
					}
					if (PaletteBox[i].gmax - PaletteBox[i].gmin > cur_side)
					{
						cur_side = (uint32)(PaletteBox[i].gmax -
							PaletteBox[i].gmin);
					}
					if (((PaletteBox[i].bmax - PaletteBox[i].bmin) >> 2) > cur_side)
					{
						cur_side = (uint32)((PaletteBox[i].bmax -
							PaletteBox[i].bmin) >> 2);
					}
#endif
					if ((num_box & 1) && num_box > 12)
					{
						if (cur_nh > max_nh ||
							(cur_nh == max_nh && cur_side > max_side))
						{
							cur_box = i;
							max_nh = cur_nh;
							max_side = cur_side;
						}
					}
					else
					{
						if (cur_nh > 1 && (cur_side > max_side ||
							(cur_side == max_side && cur_nh > max_nh)))
						{
							cur_box = i;
							max_nh = cur_nh;
							max_side = cur_side;
						}
					}
				}
				if (max_nh == 1)
					break;
				/* figure out which color to move */
				if (PaletteBox[cur_box].num_force > 1)
				{
					dr = PaletteBox[cur_box].frmax - PaletteBox[cur_box].frmin;
					dg = PaletteBox[cur_box].fgmax - PaletteBox[cur_box].fgmin;
					db = PaletteBox[cur_box].fbmax - PaletteBox[cur_box].fbmin;
				}
				else
				{
					dr = PaletteBox[cur_box].rmax - PaletteBox[cur_box].rmin;
					dg = PaletteBox[cur_box].gmax - PaletteBox[cur_box].gmin;
					db = PaletteBox[cur_box].bmax - PaletteBox[cur_box].bmin;
				}
				intf cmask;
				if (db > dg && db > dr)
				{
					ctype = 0;
					cmask = CHANNEL_MASK;
				}
				else if (dr > dg)
				{
					ctype = NUM_BITS2;
					cmask = CHANNEL_MASK;
				}
				else
				{
					ctype = NUM_BITS;
					cmask = CHANNEL_MASK;
				}
				total_count = 0;
				memset(count, 0, 256 * sizeof (intf));
				if (PaletteBox[cur_box].num_force > 1)
				{
					for (i = 0; i < PaletteBox[cur_box].num_force; i++)
					{
						count[(((ForceH[PaletteBox[cur_box].force_index[i]]) >>
							ctype) & cmask)]++;
						total_count++;
					}
				}
				else
				{
					for (i = 0; i < PaletteBox[cur_box].nh; i++)
					{
						count[((PaletteBox[cur_box].h[i] >> ctype) & cmask)]++;
						total_count++;
					}
				}
				total_count >>= 1;
				for (start = 0; !count[start] && start < NUM_COLORS_IN_CHANNEL - 1;
					start++)
					/* empty loop */ ;
				for (end = NUM_COLORS_IN_CHANNEL - 1; !count[end] && end; end--)
					/* empty loop */ ;
				for (split = 0, i = start;
					split < total_count && i < end;
					i++)
				{
					split += count[i];
				}
				split = i;
				lptr = PaletteBox[cur_box].h;
				uptr = PaletteBox[cur_box].h + PaletteBox[cur_box].nh - 1;
				while (lptr < uptr)
				{
					while ((((*lptr) >> ctype) & cmask) < split &&
						lptr < uptr)
						lptr++;
					while ((((*uptr) >> ctype) & cmask) >= split &&
						uptr > lptr)
						uptr--;
					if (lptr < uptr)
					{
						t = *lptr;
						*lptr = *uptr;
						*uptr = t;
					}
				}
				PaletteBox[num_box].nh = (uint32)(PaletteBox[cur_box].nh -
					(uint32)(lptr - PaletteBox[cur_box].h));
				PaletteBox[num_box].h = lptr;
				PaletteBox[cur_box].nh = (uint32)(lptr - PaletteBox[cur_box].h);
				PaletteBox[cur_box].ResetBoxLimits();
				PaletteBox[num_box].ResetBoxLimits();
				uintf f;
				for (f = 0; f < NumForceH; f++)
				{
					if (PaletteBox[cur_box].IsInBox(ForceH[f]))
					{
						PaletteBox[cur_box].force_index[
							PaletteBox[cur_box].num_force++] = (byte)f;
					}
				}
				PaletteBox[num_box].force_index = PaletteBox[cur_box].force_index +
					PaletteBox[cur_box].num_force;
				for (f = 0; f < NumForceH; f++)
				{
					if (PaletteBox[num_box].IsInBox(ForceH[f]))
					{
						PaletteBox[num_box].force_index[
							PaletteBox[num_box].num_force++] = (byte)f;
					}
				}
				PaletteBox[cur_box].ResetForceBoxLimits(ForceH);
				PaletteBox[num_box].ResetForceBoxLimits(ForceH);
				num_box++;
			}
			for (i = 0; i < num_box; i++)
			{
#if 0
				if (PaletteBox[i].rmin == rmin && PaletteBox[i].rmax == rmax)
				{
					byte v = (byte)((rmin + rmax) >> 1);
					v = (byte)((v << (8 - NUM_BITS)) | (v >> ((NUM_BITS - 4) << 1)));
					DitherPalette[i].Red = v;
				}
				else if (PaletteBox[i].rmin == rmin)
				{
					byte v = (byte)rmin;
					v = (byte)((v << (8 - NUM_BITS)) | (v >> ((NUM_BITS - 4) << 1)));
					DitherPalette[i].Red = v;
				}
				else if (PaletteBox[i].rmax == rmax)
				{
					byte v = (byte)rmax;
					v = (byte)((v << (8 - NUM_BITS)) | (v >> ((NUM_BITS - 4) << 1)));
					DitherPalette[i].Red = v;
				}
				else
#endif
				{
					intf vmax, vmin;
					vmax = PaletteBox[i].rmax;
					vmax = (byte)((vmax << (8 - NUM_BITS)) |
						(vmax >> ((NUM_BITS - 4) << 1)));
					vmin = PaletteBox[i].rmin;
					vmin = (byte)((vmin << (8 - NUM_BITS)) |
						(vmin >> ((NUM_BITS - 4) << 1)));
					DitherPalette[i].Red = (byte)((vmax + vmin) >> 1);
				}
#if 0
				if (PaletteBox[i].gmin == gmin && PaletteBox[i].gmax == gmax)
				{
					byte v = (byte)((gmin + gmax) >> 1);
					v = (byte)((v << (8 - NUM_BITS)) | (v >> ((NUM_BITS - 4) << 1)));
					DitherPalette[i].Green = v;
				}
				else if (PaletteBox[i].gmin == gmin)
				{
					byte v = (byte)(gmin);
					v = (byte)((v << (8 - NUM_BITS)) | (v >> ((NUM_BITS - 4) << 1)));
					DitherPalette[i].Green = v;
				}
				else if (PaletteBox[i].gmax == gmax)
				{
					byte v = (byte)(gmax);
					v = (byte)((v << (8 - NUM_BITS)) | (v >> ((NUM_BITS - 4) << 1)));
					DitherPalette[i].Green = v;
				}
				else
#endif
				{
					intf vmax, vmin;
					vmax = PaletteBox[i].gmax;
					vmax = (byte)((vmax << (8 - NUM_BITS)) |
						(vmax >> ((NUM_BITS - 4) << 1)));
					vmin = PaletteBox[i].gmin;
					vmin = (byte)((vmin << (8 - NUM_BITS)) |
						(vmin >> ((NUM_BITS - 4) << 1)));
					DitherPalette[i].Green = (byte)((vmax + vmin) >> 1);
				}
#if 0				
				if (PaletteBox[i].bmin == bmin && PaletteBox[i].bmax == bmax)
				{
					byte v = (byte)((bmin + bmax) >> 1);
					v = (byte)((v << (8 - NUM_BITS)) | (v >> ((NUM_BITS - 4) << 1)));
					DitherPalette[i].Blue = v;
				}
				else if (PaletteBox[i].bmin == bmin)
				{
					byte v = (byte)(bmin);
					v = (byte)((v << (8 - NUM_BITS)) | (v >> ((NUM_BITS - 4) << 1)));
					DitherPalette[i].Blue = v;
				}
				else if (PaletteBox[i].bmax == bmax)
				{
					byte v = (byte)(bmax);
					v = (byte)((v << (8 - NUM_BITS)) | (v >> ((NUM_BITS - 4) << 1)));
					DitherPalette[i].Blue = v;
				}
				else
#endif
				{
					intf vmax, vmin;
					vmax = PaletteBox[i].bmax;
					vmax = (byte)((vmax << (8 - NUM_BITS)) |
						(vmax >> ((NUM_BITS - 4) << 1)));
					vmin = PaletteBox[i].bmin;
					vmin = (byte)((vmin << (8 - NUM_BITS)) |
						(vmin >> ((NUM_BITS - 4) << 1)));
					DitherPalette[i].Blue = (byte)((vmax + vmin) >> 1);
				}
				if (PaletteBox[i].num_force)
				{
					DitherPalette[i] = ForceColor[ForceHToColor[
						PaletteBox[i].force_index[0]]];
				}
				if (UseLab)
				{
					LabPalette[i].L = DitherPalette[i].Red;
					LabPalette[i].A = (int8)(((intf)DitherPalette[i].Green) - 128);
					LabPalette[i].B = (int8)(((intf)DitherPalette[i].Blue) - 128);
					DitherPalette[i] = LabPalette[i];
				}
			}
			NumPalette = num_box;
			for (i = 0; i < NumForce; i++)
			{
				if (!ForceInH[i])
				{
					DitherPalette[NumPalette] = ForceColor[i];
					if (UseLab)
					{
						LabPalette[i] = DitherPalette[i];
					}
					NumPalette++;
				}
			}
		}
	}
}
#define DISTANCE_STEP 16
void
G42OptimizedPalette::PrepareTable(void)
{
	uintf i;
	// slow way to do this, but I did it this way to determine if it's better
	if (UseLab && NumPalette)
	{
		#ifdef MSWIN
//		uint32 start = GetTickCount();
		#endif
		uint32 d_min_table[256];
		uintf id;
		for (id = 0; id < 256; id++)
		{
			d_min_table[id] = 0x7fffffffL;
		}
		intf next[257];
		intf num_neighbors[256];
		intf neighbors_index[256];
		uint32 total_num_neighbors = 0;
//		G42Vector<byte> neighbors;
		byte * neighbors = new byte [65535];
//		byte neighbors[30000];
		for (uint32 i1 = 0; i1 < NumPalette; i1++)
		{
			for (intf inext = 0; inext < NumPalette; inext++)
			{
				next[inext] = inext;
			}
			next[NumPalette] = -1;
			next[i1] = next[i1 + 1]; // skip original index
			intf p2 = 0;
			for (intf p = next[0]; p >= 0 && p < i1; p = next[p + 1])
			{
				bool found = false;
				uint32 n = (uint32)(neighbors_index[p] + num_neighbors[p]);
				for (uint32 k = (uint32)neighbors_index[p]; k < n; k++)
				{
					if (i1 == neighbors[k])
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					next[p2] = next[p + 1];
				}
				else
				{
					p2 = p + 1;
				}
			}
			intf j2 = 0;
			for (intf j = next[0]; j >= 0; j = next[j + 1])
			{
				int32 dl = (int32)LabPalette[j].L - (int32)LabPalette[i1].L;
				int32 da = (int32)LabPalette[j].A - (int32)LabPalette[i1].A;
				int32 db = (int32)LabPalette[j].B - (int32)LabPalette[i1].B;
				int32 d = (dl * dl + da * da + db * db);
				if (d < d_min_table[i1] && d)
				{
					d_min_table[i1] = d;
				}
				intf k2 = j + 1;
				bool skipped = false;
				for (intf k = next[j + 1]; k >= 0; k = next[k + 1])
				{
					if (!d)
					{
						next[j2] = next[j + 1];
						skipped = true;
						break;
					}
					int32 dl2 = (int32)LabPalette[k].L - (int32)LabPalette[i1].L;
					int32 da2 = (int32)LabPalette[k].A - (int32)LabPalette[i1].A;
					int32 db2 = (int32)LabPalette[k].B - (int32)LabPalette[i1].B;
					int32 d2 = dl2 * dl2 + da2 * da2 + db2 * db2;
					if (!d2 || dl * dl2 + da * da2 + db * db2 >= d)
					{
						next[k2] = next[k + 1];
					}
					else
					{
						k2 = k + 1;
					}
					if (dl * dl2 + da * da2 + db * db2 >= d2)
					{
						next[j2] = next[j + 1];
						skipped = true;
						break;
					}
				}
				if (!skipped)
				{
					j2 = j + 1;
				}
			}
			neighbors_index[i1] = total_num_neighbors;
			for (j2 = next[0]; j2 >= 0; j2 = next[j2 + 1])
			{
				neighbors[total_num_neighbors++] = (byte)j2;
			}
			num_neighbors[i1] = total_num_neighbors - neighbors_index[i1];
		}
		for (id = 0; id < 256; id++)
		{
			d_min_table[id] >>= 1;
		}
		#ifdef MSWIN
//		uint32 after_neighbors = GetTickCount();
		#endif
		byte old_result = 0;
		byte old_red_result = 0;
		byte old_green_result = 0;
		uint32 while_count = 0;
		G42Color c;
		for (uint32 r = 0; r < NUM_COLORS_IN_CHANNEL; r++)
		{
			c.Red = (byte)((r << (8 - NUM_BITS)) | (r >> ((NUM_BITS - 4) << 1)));
			uint32 ir = (uint32)(r << NUM_BITS2);
			old_result = old_red_result;
			for (uint32 g = 0; g < NUM_COLORS_IN_CHANNEL; g++)
			{
				c.Green = (byte)((g << (8 - NUM_BITS)) | (g >> ((NUM_BITS - 4) << 1)));
				uint32 ig = (uint32)(ir | (g << NUM_BITS));
				old_result = old_green_result;
				for (uint32 b = 0; b < NUM_COLORS_IN_CHANNEL; b++)
				{
					c.Blue = (byte)((b << (8 - NUM_BITS)) | (b >> ((NUM_BITS - 4) << 1)));
					uint32 i = ig | b;
					G42Lab lab(c);
					intf drold = (intf)lab.L - (intf)LabPalette[old_result].L;
					intf dgold = (intf)lab.A - (intf)LabPalette[old_result].A;
					intf dbold = (intf)lab.B - (intf)LabPalette[old_result].B;
					uint32 d_min = (drold * drold + dgold * dgold + dbold * dbold);
					byte d_index = old_result;
					while (d_min > d_min_table[old_result])
					{
						while_count++;
						uint32 n = (uint32)(neighbors_index[old_result] +
							num_neighbors[old_result]);
						for (uint32 k = (uint32)neighbors_index[old_result];
							k < n; k++)
						{
							uint32 j = neighbors[k];
							intf dr = (intf)lab.L - (intf)LabPalette[j].L;
							intf dg = (intf)lab.A - (intf)LabPalette[j].A;
							intf db = (intf)lab.B - (intf)LabPalette[j].B;
							uint32 d = (dr * dr + dg * dg + db * db);
							if (d < d_min)
							{
								d_min = d;
								d_index = (byte)j;
							}
						}
						if (d_index == old_result)
						{
							break;
						}
						old_result = d_index;
					}
					DitherTable[i] = old_result;
					if (!b)
					{
						old_green_result = old_result;
						if (!g)
						{
							old_red_result = old_result;
						}
					}
				}
			}
		}
		delete [] neighbors;
/*
		uint32 end = GetTickCount();
		char buf[222];
		sprintf(buf, "time to complete table, neighbors, search: %ld, %ld, %ld\r\nwhile loop percentage: %lf\r\nneighbors per entry: %lf",
			end - start, after_neighbors - start, end - after_neighbors,
			(double)while_count / (double)NUM_COLORS,
			(double)total_num_neighbors / (double)NumPalette);
		::MessageBox(0, buf, "giopal.cpp", MB_OK);
*/
#if 0
#ifdef MSWIN
//		uint32 start = GetTickCount();
#endif
		uint32 d_min_table[256];
		for (uintf id = 0; id < 256; id++)
		{
			d_min_table[id] = 0x7fffffffL;
		}
		#if 0
		intf next [257];	// Minor warning on CW -- Unused variable
		#endif
		for (uint32 i1 = 0; i1 < NumPalette; i1++)
		{
#if 0
			for (intf inext = 0; inext < 257; inext++)
			{
				next[inext] = inext;
			}
			next[256] = -1;
			for (uint32 j = 0; next[j] >= 0; j = next[j] + 1)
			{
				if (j == i1)
				{
					continue;
				}
				int32 dl = LabPalette[i1].L - LabPalette[j].L;
				int32 da = LabPalette[i1].A - LabPalette[j].A;
				int32 db = LabPalette[i1].B - LabPalette[j].B;
				uint32 d = (dl * dl + da * da + db * db) >> 1;
				uint32 k2 = j;
				for (uint32 k = next[j] + 1; next[k] >= 0; k2 = k, k = next[k] + 1)
				{
					if (k == i1)
					{
						continue;
					}
					int32 dl2 = LabPalette[i1].L - LabPalette[k].L;
					int32 da2 = LabPalette[i1].A - LabPalette[k].A;
					int32 db2 = LabPalette[i1].B - LabPalette[k].B;
					if (dl * dl2 + da * da2 + db * db2 > d)
					{
						next[k2] = next[k];
					}
				}
			}
			neighbors_index[i1] = total_num_neighbors;
			for (uint32 j2 = next[0] + 1; j2 >= 0; j2 = next[j2])
			{
				neighbors[total_num_neighbors++] = j2;
			}
			num_neighbors[i1] = total_num_neighbors - neighbors_index[i1];
#endif
			
			for (uint32 i2 = (uint32)(i1 + 1); i2 < NumPalette; i2++)
			{
				int32 dl = LabPalette[i1].L - LabPalette[i2].L;
				int32 da = LabPalette[i1].A - LabPalette[i2].A;
				int32 db = LabPalette[i1].B - LabPalette[i2].B;
				uint32 n = (dl * dl + da * da + db * db);
				uint32 d = n >> 1;
				if (d < d_min_table[i1])
				{
					d_min_table[i1] = d;
				}
				if (d < d_min_table[i2])
				{
					d_min_table[i2] = d;
				}
			}
		}
#ifdef MSWIN
//		uint32 after_dist = GetTickCount();
#endif
		byte old_result = 0;
		intf count = 0;
		for (uint32 i = 0; i < NUM_COLORS; i++)
		{
			uint32 d_min = 0x7fffffffL;
			uint32 d_index = 0;
			G42Color c;
				c.Red = (byte)(
					((((i >> NUM_BITS2) & CHANNEL_MASK) << (8 - NUM_BITS)) |
					(((i >> NUM_BITS2) & CHANNEL_MASK) >> ((NUM_BITS - 4) << 1))));
				c.Green = (byte)(
					((((i >> NUM_BITS) & CHANNEL_MASK) << (8 - NUM_BITS)) |
					(((i >> NUM_BITS) & CHANNEL_MASK) >> ((NUM_BITS - 4) << 1))));
				c.Blue = (byte)(
					((((i) & CHANNEL_MASK) << (8 - NUM_BITS)) |
					(((i) & CHANNEL_MASK) >> ((NUM_BITS - 4) << 1))));
			G42Lab lab(c);
			intf dl = lab.L - LabPalette[old_result].L;
			intf da = lab.A - LabPalette[old_result].A;
			intf db = lab.B - LabPalette[old_result].B;
//			uint32 d = (dl * dl + da * da + db * db);
//			if (d >= d_min_table[old_result])
			{
				count++;
			for (uint32 j = 0; j < NumPalette; j++)
			{
				intf dl = lab.L - LabPalette[j].L;
				intf da = lab.A - LabPalette[j].A;
				intf db = lab.B - LabPalette[j].B;
//				intf d = CalcDistance(dl, da, db);
				uint32 d = (dl * dl + da * da + db * db);
				if (d < d_min)
				{
					d_min = d;
					d_index = j;
				}
			}
			old_result = (byte)d_index;
			}
			DitherTable[i] = old_result;
		}
#ifdef MSWIN
//		uint32 after_table = GetTickCount();
//		char buf[222];
//		sprintf(buf, "dist %ld, table %ld, total %ld, percent %f", after_dist - start,
//			after_table - after_dist, after_table - start, (double)count / 32768.0);
//		::MessageBox(0, buf, "giopal.cpp", MB_OK);
#endif
#if 0
		uint32 start = GetTickCount();
		uint32 d_min_table[256 * 8];
		for (uintf id = 0; id < 256 * 8; id++)
		{
			d_min_table[id] = 0x7fffffffL;
		}
		for (uint32 i1 = 0; i1 < NumPalette; i1++)
		{
			uint32 i18 = (i1 << 3);
			for (uint32 i2 = i1 + 1; i2 < NumPalette; i2++)
			{
				uint32 i28 = (i2 << 3);
				int32 dl = LabPalette[i1].L - LabPalette[i2].L;
				int32 da = LabPalette[i1].A - LabPalette[i2].A;
				int32 db = LabPalette[i1].B - LabPalette[i2].B;
//				intf d = CalcDistance(dl, da, db);
				uint32 n = (dl * dl + da * da + db * db);
				uint32 d = n >> 1;
				byte octant = 0;
				if (LabPalette[i1].L < LabPalette[i2].L)
				{
					octant |= 4;
				}
				if (LabPalette[i1].A < LabPalette[i2].A)
				{
					octant |= 2;
				}
				if (LabPalette[i1].B < LabPalette[i2].B)
				{
					octant |= 1;
				}
				if (d < d_min_table[(i18) | octant])
				{
					d_min_table[(i18) | octant] = d;
				}
				if (d < d_min_table[(i28) | (7 - octant)])
				{
					d_min_table[(i28) | (7 - octant)] = d;
				}
				n = ((n * n) >> 1);
				dl *= dl;
				da *= da;
				db *= db;
				for (byte oct = 0; oct < 8; oct++)
				{
					if (oct == 7 - octant)
					{
						continue;
					}
					d = 0;
					if ((oct & 4) == (octant & 4))
					{
						d += dl;
					}
					if ((oct & 2) == (octant & 2))
					{
						d += da;
					}
					if ((oct & 1) == (octant & 1))
					{
						d += db;
					}
					if (!d)
					{
						continue;
					}
					d = n / d;
					if (d < d_min_table[(i18) | oct])
					{
						d_min_table[(i18) | oct] = d;
					}
					if (d < d_min_table[(i28) | (7 - oct)])
					{
						d_min_table[(i28) | (7 - oct)] = d;
					}
				}
			}
		}
		uint32 after_dist = GetTickCount();
		byte old_result = 0;
		intf count = 0;
		for (uint32 i = 0; i < NUM_COLORS; i++)
		{
			uint32 d_min = 0x7fffffffL;
			uint32 d_index = 0;
			G42Color c;
				c.Red = (byte)(
					((((i >> NUM_BITS2) & CHANNEL_MASK) << (8 - NUM_BITS)) |
					(((i >> NUM_BITS2) & CHANNEL_MASK) >> ((NUM_BITS - 4) << 1))));
				c.Green = (byte)(
					((((i >> NUM_BITS) & CHANNEL_MASK) << (8 - NUM_BITS)) |
					(((i >> NUM_BITS) & CHANNEL_MASK) >> ((NUM_BITS - 4) << 1))));
				c.Blue = (byte)(
					((((i) & CHANNEL_MASK) << (8 - NUM_BITS)) |
					(((i) & CHANNEL_MASK) >> ((NUM_BITS - 4) << 1))));
			G42Lab lab(c);
			intf dl = lab.L - LabPalette[old_result].L;
			intf da = lab.A - LabPalette[old_result].A;
			intf db = lab.B - LabPalette[old_result].B;
			uint32 d = (dl * dl + da * da + db * db);
			byte octant = 0;
			if (dl >= 0)
			{
				octant |= 4;
			}
			if (da >= 0)
			{
				octant |= 2;
			}
			if (db >= 0)
			{
				octant |= 1;
			}
			if (d >= d_min_table[(old_result << 3) | octant])
			{
				count++;
			for (uint32 j = 0; j < NumPalette; j++)
			{
				intf dl = lab.L - LabPalette[j].L;
				intf da = lab.A - LabPalette[j].A;
				intf db = lab.B - LabPalette[j].B;
//				intf d = CalcDistance(dl, da, db);
				uint32 d = (dl * dl + da * da + db * db);
				if (d < d_min)
				{
					d_min = d;
					d_index = j;
				}
			}
			old_result = (byte)d_index;
			}
			DitherTable[i] = old_result;
		}
		uint32 after_table = GetTickCount();
		for (uint32 i = 0; i < NUM_COLORS; i++)
		{
			G42Color c;
				c.Red = (byte)(
					((((i >> NUM_BITS2) & CHANNEL_MASK) << (8 - NUM_BITS)) |
					(((i >> NUM_BITS2) & CHANNEL_MASK) >> ((NUM_BITS - 4) << 1))));
				c.Green = (byte)(
					((((i >> NUM_BITS) & CHANNEL_MASK) << (8 - NUM_BITS)) |
					(((i >> NUM_BITS) & CHANNEL_MASK) >> ((NUM_BITS - 4) << 1))));
				c.Blue = (byte)(
					((((i) & CHANNEL_MASK) << (8 - NUM_BITS)) |
					(((i) & CHANNEL_MASK) >> ((NUM_BITS - 4) << 1))));
			G42Lab lab(c);
		}
		uint32 after_old_table = GetTickCount();
		char buf[222];
		sprintf(buf, "dist %ld, table %ld, total %ld, percent %f, lab %d", after_dist - start,
			after_table - after_dist, after_table - start, (double)count / 32768.0,
			after_old_table - after_table);
		::MessageBox(0, buf, "giopal.cpp", MB_OK);
#endif
#endif // old lab way, probably buggy
	}
	else if (NumPalette)
	{
		#ifdef MSWIN
//		uint32 start = GetTickCount();
		#endif
		uint32 d_min_table[256];
		uintf id;
		for (id = 0; id < 256; id++)
		{
			d_min_table[id] = 0x7fffffffL;
		}
		intf next[257];
		intf num_neighbors[256];
		intf neighbors_index[256];
		uint32 total_num_neighbors = 0;
//		G42Vector<byte> neighbors;
		byte * neighbors = new byte [65535];
//		byte neighbors[30000];
		for (uint32 i1 = 0; i1 < NumPalette; i1++)
		{
			for (intf inext = 0; inext < NumPalette; inext++)
			{
				next[inext] = inext;
			}
			next[NumPalette] = -1;
			next[i1] = next[i1 + 1]; // skip original index
			intf p2 = 0;
			for (intf p = next[0]; p >= 0 && p < i1; p = next[p + 1])
			{
				bool found = false;
				uint32 n = (uint32)(neighbors_index[p] + num_neighbors[p]);
				for (uint32 k = (uint32)neighbors_index[p]; k < n; k++)
				{
					if (i1 == neighbors[k])
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					next[p2] = next[p + 1];
				}
				else
				{
					p2 = p + 1;
				}
			}
			intf j2 = 0;
			for (intf j = next[0]; j >= 0; j = next[j + 1])
			{
				int32 dl = DitherPalette[j].Red - DitherPalette[i1].Red;
				int32 da = DitherPalette[j].Green - DitherPalette[i1].Green;
				int32 db = DitherPalette[j].Blue - DitherPalette[i1].Blue;
				int32 d = (dl * dl + da * da + db * db);
				if (d < d_min_table[i1] && d)
				{
					d_min_table[i1] = d;
				}
				intf k2 = j + 1;
				bool skipped = false;
				for (intf k = next[j + 1]; k >= 0; k = next[k + 1])
				{
					if (!d)
					{
						next[j2] = next[j + 1];
						skipped = true;
						break;
					}
					int32 dl2 = DitherPalette[k].Red - DitherPalette[i1].Red;
					int32 da2 = DitherPalette[k].Green - DitherPalette[i1].Green;
					int32 db2 = DitherPalette[k].Blue - DitherPalette[i1].Blue;
					int32 d2 = dl2 * dl2 + da2 * da2 + db2 * db2;
					if (!d2 || dl * dl2 + da * da2 + db * db2 >= d)
					{
						next[k2] = next[k + 1];
					}
					else
					{
						k2 = k + 1;
					}
					if (dl * dl2 + da * da2 + db * db2 >= d2)
					{
						next[j2] = next[j + 1];
						skipped = true;
						break;
					}
				}
				if (!skipped)
				{
					j2 = j + 1;
				}
			}
			neighbors_index[i1] = total_num_neighbors;
			for (j2 = next[0]; j2 >= 0; j2 = next[j2 + 1])
			{
				neighbors[total_num_neighbors++] = (byte)j2;
			}
			num_neighbors[i1] = total_num_neighbors - neighbors_index[i1];
		}
		for (id = 0; id < 256; id++)
		{
			d_min_table[id] >>= 1;
		}
		#ifdef MSWIN
//		uint32 after_neighbors = GetTickCount();
		#endif
		byte old_result = 0;
		byte old_red_result = 0;
		byte old_green_result = 0;
		uint32 while_count = 0;
		G42Color c;
		for (uint32 r = 0; r < NUM_COLORS_IN_CHANNEL; r++)
		{
			c.Red = (byte)((r << (8 - NUM_BITS)) | (r >> ((NUM_BITS - 4) << 1)));
			uint32 ir = (uint32)(r << NUM_BITS2);
			old_result = old_red_result;
			for (uint32 g = 0; g < NUM_COLORS_IN_CHANNEL; g++)
			{
				c.Green = (byte)((g << (8 - NUM_BITS)) | (g >> ((NUM_BITS - 4) << 1)));
				uint32 ig = (uint32)(ir | (g << NUM_BITS));
				old_result = old_green_result;
				for (uint32 b = 0; b < NUM_COLORS_IN_CHANNEL; b++)
				{
					c.Blue = (byte)((b << (8 - NUM_BITS)) | (b >> ((NUM_BITS - 4) << 1)));
					uint32 i = ig | b;
					intf drold = (intf)c.Red - (intf)DitherPalette[old_result].Red;
					intf dgold = (intf)c.Green - (intf)DitherPalette[old_result].Green;
					intf dbold = (intf)c.Blue - (intf)DitherPalette[old_result].Blue;
					uint32 d_min = (drold * drold + dgold * dgold + dbold * dbold);
					byte d_index = old_result;
					while (d_min > d_min_table[old_result])
					{
						while_count++;
						uint32 n = (uint32)(neighbors_index[old_result] +
							num_neighbors[old_result]);
						for (uint32 k = (uint32)neighbors_index[old_result];
							k < n; k++)
						{
							uint32 j = neighbors[k];
							intf dr = (intf)c.Red - (intf)DitherPalette[j].Red;
							intf dg = (intf)c.Green - (intf)DitherPalette[j].Green;
							intf db = (intf)c.Blue - (intf)DitherPalette[j].Blue;
							uint32 d = (dr * dr + dg * dg + db * db);
							if (d < d_min)
							{
								d_min = d;
								d_index = (byte)j;
							}
						}
						if (d_index == old_result)
						{
							break;
						}
						old_result = d_index;
					}
					DitherTable[i] = old_result;
					if (!b)
					{
						old_green_result = old_result;
						if (!g)
						{
							old_red_result = old_result;
						}
					}
				}
			}
		}
		delete [] neighbors;
/*
		uint32 end = GetTickCount();
		char buf[222];
		sprintf(buf, "time to complete table, neighbors, search: %ld, %ld, %ld\r\nwhile loop percentage: %lf\r\nneighbors per entry: %lf",
			end - start, after_neighbors - start, end - after_neighbors,
			(double)while_count / (double)NUM_COLORS,
			(double)total_num_neighbors / (double)NumPalette);
		::MessageBox(0, buf, "giopal.cpp", MB_OK);
*/		
	}
	else if (0 && NumPalette)
	{
		if (FullColorOnly || NeedOptimize)
		{
//			DWORD start_time = GetTickCount();
			intf min_dist = 0;
			intf max_dist = DISTANCE_STEP - 1;
			for (i = 0; i < NUM_COLORS; i++)
				h[i] = 0x7fff;
			bool * palette_finished = new bool [256];
			bool * side_finished = new bool [256 * 8];
			memset(palette_finished, 0, 256 * sizeof (bool));
			memset(side_finished, 0, 256 * 8 * sizeof (bool));
			while (1)
			{
				bool need_more = false;
				for (i = 0; i < NumPalette; i++)
				{
					if (Avoid[i])
						continue;
					if (palette_finished[i])
						break;
					intf r = (DitherPalette[i].Red >> (8 - NUM_BITS));
					intf dr = ((DitherPalette[i].Red & (0xff >> NUM_BITS)) << 1);
					if (dr > (1 << (8 - NUM_BITS)) - 1)
						dr -= (1 << (8 - NUM_BITS));
					else
						dr -= (1 << (8 - NUM_BITS)) - 2;
					intf g = (DitherPalette[i].Green >> (8 - NUM_BITS));
					intf dg = ((DitherPalette[i].Green & (0xff >> NUM_BITS)) << 1);
					if (dg > (1 << (8 - NUM_BITS)) - 1)
						dg -= (1 << (8 - NUM_BITS));
					else
						dg -= (1 << (8 - NUM_BITS)) - 2;
					intf b = (DitherPalette[i].Blue >> (8 - NUM_BITS));
					intf db = ((DitherPalette[i].Blue & (0xff >> NUM_BITS)) << 1);
					if (db > (1 << (8 - NUM_BITS)) - 1)
						db -= (1 << (8 - NUM_BITS));
					else
						db -= (1 << (8 - NUM_BITS)) - 2;
					intf sfoffset = (i << 3);
					bool is_any_used = false;
					if (!side_finished[sfoffset])
					{
						for (intf dist = min_dist; dist <= max_dist; dist++)
						{
							bool is_used = false;
							intf max_ir = min(r, dist);
							for (intf ir = 0; ir <= max_ir; ir++)
							{
								intf index_r = ((r - ir) << NUM_BITS2);
								intf dist_r = (dr + (ir << (9 - NUM_BITS)));
								intf max_ig = min(g, dist);
								for (intf ig = 0; ig <= max_ig; ig++)
								{
									intf index_g = index_r | ((g - ig) << NUM_BITS);
									intf dist_g = dg + (ig << (9 - NUM_BITS));
									intf ib;
									if (ig == dist || ir == dist)
										ib = 0;
									else
										ib = dist;
									intf max_ib = min(b, dist);
									for (; ib <= max_ib; ib++)
									{
										intf index = index_g | (b - ib);
										intf dist_b = (db + (ib << (9 - NUM_BITS)));
										intf d = CalcDistance(dist_r, dist_g, dist_b);
										if (h[index] >= d)
										{
											h[index] = (uint32)d;
											DitherTable[index] = (byte)i;
											is_used = true;
										}
									} // ib
								} // ig
							} // ir
							if (!is_used)
							{
								side_finished[sfoffset] = true;
								break;
							}
						} // dist
						if (!side_finished[sfoffset])
							is_any_used = true;
					}
					if (!side_finished[sfoffset + 1])
					{
						for (intf dist = min_dist; dist <= max_dist; dist++)
						{
							bool is_used = false;
							intf max_ir = min(r, dist);
							for (intf ir = 0; ir <= max_ir; ir++)
							{
								intf index_r = ((r - ir) << NUM_BITS2);
								intf dist_r = (dr + (ir << (9 - NUM_BITS)));
								intf max_ig = min(g, dist);
								for (intf ig = 0; ig <= max_ig; ig++)
								{
									intf index_g = index_r | ((g - ig) << NUM_BITS);
									intf dist_g = dg + (ig << (9 - NUM_BITS));
									intf ib;
									if (ig == dist || ir == dist)
										ib = 0;
									else
										ib = dist;
									intf max_ib = min((1 << (8 - NUM_BITS)) - 2 - b,
										dist);
									for (; ib <= max_ib; ib++)
									{
										intf index = index_g | (b + ib + 1);
										intf dist_b = ((1 << (NUM_BITS + 1)) - db +
											(ib << (9 - NUM_BITS)));
										intf d = CalcDistance(dist_r, dist_g, dist_b);
										if (h[index] >= d)
										{
											h[index] = (uint32)d;
											DitherTable[index] = (byte)i;
											is_used = true;
										}
									} // ib
								} // ig
							} // ir
							if (!is_used)
							{
								side_finished[sfoffset + 1] = true;
								break;
							}
						} // dist
						if (!side_finished[sfoffset + 1])
							is_any_used = true;
					}
					if (!side_finished[sfoffset + 2])
					{
						for (intf dist = min_dist; dist <= max_dist; dist++)
						{
							bool is_used = false;
							intf max_ir = min(r, dist);
							for (intf ir = 0; ir <= max_ir; ir++)
							{
								intf index_r = ((r - ir) << NUM_BITS2);
								intf dist_r = (dr + (ir << (9 - NUM_BITS)));
								intf max_ig = min((1 << (8 - NUM_BITS)) - 2 - g, dist);
								for (intf ig = 0; ig <= max_ig; ig++)
								{
									intf index_g = index_r | ((g + ig + 1) << NUM_BITS);
									intf dist_g = (1 << (NUM_BITS + 1)) - dg + (ig << (9 - NUM_BITS));
									intf ib;
									if (ig == dist || ir == dist)
										ib = 0;
									else
										ib = dist;
									intf max_ib = min(b, dist);
									for (; ib <= max_ib; ib++)
									{
										intf index = index_g | (b - ib);
										intf dist_b = (db + (ib << (9 - NUM_BITS)));
										intf d = CalcDistance(dist_r, dist_g, dist_b);
										if (h[index] >= d)
										{
											h[index] = (uint32)d;
											DitherTable[index] = (byte)i;
											is_used = true;
										}
									} // ib
								} // ig
							} // ir
							if (!is_used)
							{
								side_finished[sfoffset + 2] = true;
								break;
							}
						} // dist
						if (!side_finished[sfoffset + 2])
							is_any_used = true;
					}
					if (!side_finished[sfoffset + 3])
					{
						for (intf dist = min_dist; dist <= max_dist; dist++)
						{
							bool is_used = false;
							intf max_ir = min(r, dist);
							for (intf ir = 0; ir <= max_ir; ir++)
							{
								intf index_r = ((r - ir) << NUM_BITS2);
								intf dist_r = ((dr + (ir << (9 - NUM_BITS))));
								intf max_ig = min((1 << (8 - NUM_BITS)) - 2 - g, dist);
								for (intf ig = 0; ig <= max_ig; ig++)
								{
									intf index_g = index_r | ((g + ig + 1) << NUM_BITS);
									intf dist_g = (1 << (NUM_BITS + 1)) - dg + (ig << (9 - NUM_BITS));
									intf ib;
									if (ig == dist || ir == dist)
										ib = 0;
									else
										ib = dist;
									intf max_ib = min((1 << (8 - NUM_BITS)) - 2 - b, dist);
									for (; ib <= max_ib; ib++)
									{
										intf index = index_g | (b + ib + 1);
										intf dist_b = (((1 << (NUM_BITS + 1)) - db + (ib << (9 - NUM_BITS))));
										intf d = CalcDistance(dist_r, dist_g, dist_b);
										if (h[index] >= d)
										{
											h[index] = (uint32)d;
											DitherTable[index] = (byte)i;
											is_used = true;
										}
									} // ib
								} // ig
							} // ir
							if (!is_used)
							{
								side_finished[sfoffset + 3] = true;
								break;
							}
						} // dist
						if (!side_finished[sfoffset + 3])
							is_any_used = true;
					}
					if (!side_finished[sfoffset + 4])
					{
						for (intf dist = min_dist; dist <= max_dist; dist++)
						{
							bool is_used = false;
							intf max_ir = min((1 << (8 - NUM_BITS)) - 2 - r, dist);
							for (intf ir = 0; ir <= max_ir; ir++)
							{
								intf index_r = ((r + ir + 1) << NUM_BITS2);
								intf dist_r = ((1 << (NUM_BITS + 1)) - dr + (ir << (9 - NUM_BITS)));
								intf max_ig = min(g, dist);
								for (intf ig = 0; ig <= max_ig; ig++)
								{
									intf index_g = index_r | ((g - ig) << NUM_BITS);
									intf dist_g = dg + (ig << (9 - NUM_BITS));
									intf ib;
									if (ig == dist || ir == dist)
										ib = 0;
									else
										ib = dist;
									intf max_ib = min(b, dist);
									for (; ib <= max_ib; ib++)
									{
										intf index = index_g | (b - ib);
										intf dist_b = (db + (ib << (9 - NUM_BITS)));
										intf d = CalcDistance(dist_r, dist_g, dist_b);
										if (h[index] >= d)
										{
											h[index] = (uint32)d;
											DitherTable[index] = (byte)i;
											is_used = true;
										}
									} // ib
								} // ig
							} // ir
							if (!is_used)
							{
								side_finished[sfoffset + 4] = true;
								break;
							}
						} // dist
						if (!side_finished[sfoffset + 4])
							is_any_used = true;
					}
					if (!side_finished[sfoffset + 5])
					{
						for (intf dist = min_dist; dist <= max_dist; dist++)
						{
							bool is_used = false;
							intf max_ir = min((1 << (8 - NUM_BITS)) - 2 - r, dist);
							for (intf ir = 0; ir <= max_ir; ir++)
							{
								intf index_r = ((r + ir + 1) << NUM_BITS2);
								intf dist_r = ((1 << (NUM_BITS + 1)) - dr + (ir << (9 - NUM_BITS)));
								intf max_ig = min(g, dist);
								for (intf ig = 0; ig <= max_ig; ig++)
								{
									intf index_g = index_r | ((g - ig) << NUM_BITS);
									intf dist_g = dg + (ig << (9 - NUM_BITS));
									intf ib;
									if (ig == dist || ir == dist)
										ib = 0;
									else
										ib = dist;
									intf max_ib = min((1 << (8 - NUM_BITS)) - 2 - b, dist);
									for (; ib <= max_ib; ib++)
									{
										intf index = index_g | (b + ib + 1);
										intf dist_b = (((1 << (NUM_BITS + 1)) - db + (ib << (9 - NUM_BITS))));
										intf d = CalcDistance(dist_r, dist_g, dist_b);
										if (h[index] >= d)
										{
											h[index] = (uint32)d;
											DitherTable[index] = (byte)i;
											is_used = true;
										}
									} // ib
								} // ig
							} // ir
							if (!is_used)
							{
								side_finished[sfoffset + 5] = true;
								break;
							}
						} // dist
						if (!side_finished[sfoffset + 5])
							is_any_used = true;
					}
					if (!side_finished[sfoffset + 6])
					{
						for (intf dist = min_dist; dist <= max_dist; dist++)
						{
							bool is_used = false;
							intf max_ir = min((1 << (8 - NUM_BITS)) - 2 - r, dist);
							for (intf ir = 0; ir <= max_ir; ir++)
							{
								intf index_r = ((r + ir + 1) << NUM_BITS2);
								intf dist_r = ((1 << (NUM_BITS + 1)) - dr + (ir << (9 - NUM_BITS)));
								intf max_ig = min((1 << (8 - NUM_BITS)) - 2 - g, dist);
								for (intf ig = 0; ig <= max_ig; ig++)
								{
									intf index_g = index_r | ((g + ig + 1) << NUM_BITS);
									intf dist_g = (1 << (NUM_BITS + 1)) - dg + (ig << (9 - NUM_BITS));
									intf ib;
									if (ig == dist || ir == dist)
										ib = 0;
									else
										ib = dist;
									intf max_ib = min(b, dist);
									for (; ib <= max_ib; ib++)
									{
										intf index = index_g | (b - ib);
										intf dist_b = (db + (ib << (9 - NUM_BITS)));
										intf d = CalcDistance(dist_r, dist_g, dist_b);
										if (h[index] >= d)
										{
											h[index] = (uint32)d;
											DitherTable[index] = (byte)i;
											is_used = true;
										}
									} // ib
								} // ig
							} // ir
							if (!is_used)
							{
								side_finished[sfoffset + 6] = true;
								break;
							}
						} // dist
						if (!side_finished[sfoffset + 6])
							is_any_used = true;
					}
					if (!side_finished[sfoffset + 7])
					{
						for (intf dist = min_dist; dist <= max_dist; dist++)
						{
							bool is_used = false;
							intf max_ir = min((1 << (8 - NUM_BITS)) - 2 - r, dist);
							for (intf ir = 0; ir <= max_ir; ir++)
							{
								intf index_r = ((r + ir + 1) << NUM_BITS2);
								intf dist_r = ((1 << (NUM_BITS + 1)) - dr + (ir << (9 - NUM_BITS)));
								intf max_ig = min((1 << (8 - NUM_BITS)) - 2 - g, dist);
								for (intf ig = 0; ig <= max_ig; ig++)
								{
									intf index_g = index_r | ((g + ig + 1) << NUM_BITS);
									intf dist_g = (1 << (NUM_BITS + 1)) - dg + (ig << (9 - NUM_BITS));
									intf ib;
									if (ig == dist || ir == dist)
										ib = 0;
									else
										ib = dist;
									intf max_ib = min((1 << (8 - NUM_BITS)) - 2 - b, dist);
									for (; ib <= max_ib; ib++)
									{
										intf index = index_g | (b + ib + 1);
										intf dist_b = (((1 << (NUM_BITS + 1)) - db + (ib << (9 - NUM_BITS))));
										intf d = CalcDistance(dist_r, dist_g, dist_b);
										if (h[index] >= d)
										{
											h[index] = (uint32)d;
											DitherTable[index] = (byte)i;
											is_used = true;
										}
									} // ib
								} // ig
							} // ir
							if (!is_used)
							{
								side_finished[sfoffset + 7] = true;
								break;
							}
						} // dist
						if (!side_finished[sfoffset + 7])
							is_any_used = true;
					}
					if (is_any_used)
					{
						need_more = true;
					}
					else
					{
						palette_finished[i] = true;
					}
				} // for num palette
				if (!need_more)
				{
					break; // from distance loop
				}
				min_dist = max_dist + 1;
				max_dist += DISTANCE_STEP;
				if (min_dist >= 16)
					break;
			}
			WhitePixel = DitherTable[RGBTo16(0xff, 0xff, 0xff)];
			delete [] palette_finished;
			delete [] side_finished;
//			DWORD stop_time = GetTickCount();
//GES((ges, "Elapsed time (milliseconds): %ld\n", stop_time - start_time));
		}
		else
		{
#if 0
			// set up translations for data and sub_data
			memset(DitherTable, 0, 512 * sizeof (uint32));
			if (Data)
			{
				if (Data->GetNumPalette() && Data->GetPalette())
				{
					G42Color * pal = Data->GetPalette();
					for (intf i = 0; i < Data->GetNumPalette(); i++)
					{
						for (intf j = 0; j < NumPalette; j++)
						{
							if (pal[i] == DitherPalette[j])
							{
								DitherTable[i] = (byte)j;
								i = Data->GetNumPalette();
								break;
							}
						}
					}
				}
				else
				{
					for (intf i = 0; i < (1 << Data->GetDepth()); i++)
					{
						byte g = (byte)i;
						if (Data->GetDepth() == 1)
							g *= (byte)0xff;
						else if (Data->GetDepth() == 4)
							g *= (byte)0x11;
						G42Color c(g, g, g);
						for (intf j = 0; j < NumPalette; j++)
						{
							if (c == DitherPalette[j])
							{
								DitherTable[i] = (byte)j;
								i = 0xff;
								break;
							}
						}
					}
				}
			}
			if (SubData)
			{
				if (SubData->GetNumPalette() && SubData->GetPalette())
				{
					G42Color * pal = SubData->GetPalette();
					for (intf i = 0; i < SubData->GetNumPalette(); i++)
					{
						for (intf j = 0; j < NumPalette; j++)
						{
							if (pal[i] == DitherPalette[j])
							{
								DitherTable[i + 256] = (byte)j;
								i = SubData->GetNumPalette();
								break;
							}
						}
					}
				}
				else
				{
					for (intf i = 0; i < (1 << SubData->GetDepth()); i++)
					{
						byte g = (byte)i;
						if (SubData->GetDepth() == 1)
							g *= (byte)0xff;
						else if (SubData->GetDepth() == 4)
							g *= (byte)0x11;
						G42Color c(g, g, g);
						for (intf j = 0; j < NumPalette; j++)
						{
							if (c == DitherPalette[j])
							{
								DitherTable[i + 256] = (byte)j;
								i = 0xff;
								break;
							}
						}
					}
				}
			}
#endif
		}
	}
}
#if 0
// save off old, slow, but accurate version
void
G42OptimizedPalette::PrepareTable(void)
{
	uintf i;
	if (NumPalette)
	{
		if (FullColorOnly || NeedOptimize)
		{
			bool finished = false;
			intf dist = 0;
			for (i = 0; i <= 32767; i++)
				h[i] = 0x7fff;
			bool palette_finished[256];
//			bool palette_used[256];
			for (i = 0; i < 256; i++)
				palette_finished[i] = false;
			while (!finished)
			{
				bool need_more = false;
//				for (intf i = 0; i < 256; i++)
//					palette_used[i] = false;
				for (i = 0; i < NumPalette; i++)
				{
				if (palette_finished[i])
					continue;
				bool is_used = false;
							intf r = (DitherPalette[i].red >> 3);
							intf dr = (DitherPalette[i].red & 7);
							if (dr > 3)
								dr -= 4;
							else
								dr -= 3;
							intf g = (DitherPalette[i].green >> 3);
							intf dg = (DitherPalette[i].green & 7);
							if (dg > 3)
								dg -= 4;
							else
								dg -= 3;
							intf b = (DitherPalette[i].blue >> 3);
							intf db = (DitherPalette[i].blue & 7);
							if (db > 3)
								db -= 4;
							else
								db -= 3;
#if 0
				for (intf ir = 0; ir <= dist; ir++)
				for (intf ig = 0; ig <= dist; ig++)
				{
					intf ib;
					if (ir == dist || ig == dist)
						ib = 0;
					else
						ib = dist;
					for (; ib <= dist; ib++)
					{
#endif
				for (intf ir = 0; ir <= dist; ir++)
				{
					for (intf ig = 0; ig <= dist - ir; ig++)
					{
						intf ib = dist - (ir + ig);
#if 0
						for (i = 0; i < NumPalette; i++)
						{
							if (palette_finished[i])
								continue;
							bool is_used = false;
							intf r = (DitherPalette[i].red >> 3);
							intf dr = (DitherPalette[i].red & 7);
							if (dr > 3)
								dr -= 4;
							else
								dr -= 3;
							intf g = (DitherPalette[i].green >> 3);
							intf dg = (DitherPalette[i].green & 7);
							if (dg > 3)
								dg -= 4;
							else
								dg -= 3;
							intf b = (DitherPalette[i].blue >> 3);
							intf db = (DitherPalette[i].blue & 7);
							if (db > 3)
								db -= 4;
							else
								db -= 3;
						}
#endif
							if (r - ir >= 0 && g - ig >= 0 && b - ib >= 0)
							{
								intf index = ((r - ir) << 10) | ((g - ig) << 5) |
									(b - ib);
								intf dist_b = ((db + (ib << 3)) >> 2);
								intf dist_r = ((dr + (ir << 3)) >> 1);
								intf dist_g = dg + (ig << 3);
								intf d = CalcDistance(dist_r, dist_g, dist_b);
								if (h[index] >= d)
								{
									h[index] = (uint32)d;
									DitherTable[index] = (byte)i;
									is_used = true;
								}
							}
							if (r - ir >= 0 && g - ig >= 0 && b + ib + 1 < 32)
							{
								intf index = ((r - ir) << 10) | ((g - ig) << 5) |
									(b + ib + 1);
								intf dist_b = ((8 - db + (ib << 3)) >> 2);
								intf dist_r = ((dr + (ir << 3)) >> 1);
								intf dist_g = dg + (ig << 3);
								intf d = CalcDistance(dist_r, dist_g, dist_b);
								if (h[index] >= d)
								{
									h[index] = (uint32)d;
									DitherTable[index] = (byte)i;
									is_used = true;
								}
							}
							if (r - ir >= 0 && g + ig + 1 < 32 && b - ib >= 0)
							{
								intf index = ((r - ir) << 10) | ((g + ig + 1) << 5) |
									(b - ib);
								intf dist_b = ((db + (ib << 3)) >> 2);
								intf dist_r = ((dr + (ir << 3)) >> 1);
								intf dist_g = 8 - dg + (ig << 3);
								intf d = CalcDistance(dist_r, dist_g, dist_b);
								if (h[index] >= d)
								{
									h[index] = (uint32)d;
									DitherTable[index] = (byte)i;
									is_used = true;
								}
							}
							if (r - ir >= 0 && g + ig + 1 < 32 && b + ib + 1 < 32)
							{
								intf index = ((r - ir) << 10) | ((g + ig + 1) << 5) |
									(b + ib + 1);
								intf dist_b = ((8 - db + (ib << 3)) >> 2);
								intf dist_r = ((dr + (ir << 3)) >> 1);
								intf dist_g = 8 - dg + (ig << 3);
								intf d = CalcDistance(dist_r, dist_g, dist_b);
								if (h[index] >= d)
								{
									h[index] = (uint32)d;
									DitherTable[index] = (byte)i;
									is_used = true;
								}
							}
							if (r + ir + 1 < 32 && g - ig >= 0 && b - ib >= 0)
							{
								intf index = ((r + ir + 1) << 10) | ((g - ig) << 5) |
									(b - ib);
								intf dist_b = ((db + (ib << 3)) >> 2);
								intf dist_r = ((8 - dr + (ir << 3)) >> 1);
								intf dist_g = dg + (ig << 3);
								intf d = CalcDistance(dist_r, dist_g, dist_b);
								if (h[index] >= d)
								{
									h[index] = (uint32)d;
									DitherTable[index] = (byte)i;
									is_used = true;
								}
							}
							if (r + ir + 1 < 32 && g - ig >= 0 && b + ib + 1 < 32)
							{
								intf index = ((r + ir + 1) << 10) | ((g - ig) << 5) |
									(b + ib + 1);
								intf dist_b = ((8 - db + (ib << 3)) >> 2);
								intf dist_r = ((8 - dr + (ir << 3)) >> 1);
								intf dist_g = dg + (ig << 3);
								intf d = CalcDistance(dist_r, dist_g, dist_b);
								if (h[index] >= d)
								{
									h[index] = (uint32)d;
									DitherTable[index] = (byte)i;
									is_used = true;
								}
							}
							if (r + ir + 1 < 32 && g + ig + 1 < 32 && b - ib >= 0)
							{
								intf index = ((r + ir + 1) << 10) | ((g + ig + 1) << 5) |
									(b - ib);
								intf dist_b = ((db + (ib << 3)) >> 2);
								intf dist_r = ((8 - dr + (ir << 3)) >> 1);
								intf dist_g = 8 - dg + (ig << 3);
								intf d = CalcDistance(dist_r, dist_g, dist_b);
								if (h[index] >= d)
								{
									h[index] = (uint32)d;
									DitherTable[index] = (byte)i;
									is_used = true;
								}
							}
							if (r + ir + 1 < 32 && g + ig + 1 < 32 &&
								b + ib + 1 < 32)
							{
								intf index = ((r + ir + 1) << 10) | ((g + ig + 1) << 5) |
									(b + ib + 1);
								intf dist_b = ((8 - db + (ib << 3)) >> 2);
								intf dist_r = ((8 - dr + (ir << 3)) >> 1);
								intf dist_g = 8 - dg + (ig << 3);
								intf d = CalcDistance(dist_r, dist_g, dist_b);
								if (h[index] >= d)
								{
									h[index] = (uint32)d;
									DitherTable[index] = (byte)i;
									is_used = true;
								}
							}
						}
					}
					if (is_used)
						need_more = true;
					else
						palette_finished[i] = true;
				}
				if (!need_more)
				{
					finished = true;
				}
#if 0
				else
				{
					for (intf i = 0; i < 256; i++)
						if (!palette_used[i])
							palette_finished[i] = true;
				}
#endif				
				dist++;
				if (dist > 96)
					break;
			}
			WhitePixel = DitherTable[RGBTo16(0xff, 0xff, 0xff)];
		}
		else
		{
			// set up translations for data and sub_data
      	memset(DitherTable, 0, 512 * sizeof (uint32));
			if (Data)
			{
				if (Data->GetNumPalette() && Data->GetPalette())
				{
					G42Color * pal = Data->GetPalette();
					for (intf i = 0; i < Data->GetNumPalette(); i++)
					{
						for (intf j = 0; j < NumPalette; j++)
						{
							if (pal[i] == DitherPalette[j])
							{
								DitherTable[i] = (byte)j;
								i = Data->GetNumPalette();
								break;
							}
						}
					}
				}
				else
				{
					for (intf i = 0; i < (1 << Data->GetDepth()); i++)
					{
						byte g = (byte)i;
						if (Data->GetDepth() == 1)
							g *= (byte)0xff;
						else if (Data->GetDepth() == 4)
							g *= (byte)0x11;
						G42Color c(g, g, g);
						for (intf j = 0; j < NumPalette; j++)
						{
							if (c == DitherPalette[j])
							{
								DitherTable[i] = (byte)j;
								i = 0xff;
								break;
							}
						}
					}
				}
			}
			if (SubData)
			{
				if (SubData->GetNumPalette() && SubData->GetPalette())
				{
					G42Color * pal = SubData->GetPalette();
					for (intf i = 0; i < SubData->GetNumPalette(); i++)
					{
						for (intf j = 0; j < NumPalette; j++)
						{
							if (pal[i] == DitherPalette[j])
							{
								DitherTable[i + 256] = (byte)j;
								i = SubData->GetNumPalette();
								break;
							}
						}
					}
				}
				else
				{
					for (intf i = 0; i < (1 << SubData->GetDepth()); i++)
					{
						byte g = (byte)i;
						if (SubData->GetDepth() == 1)
							g *= (byte)0xff;
						else if (SubData->GetDepth() == 4)
							g *= (byte)0x11;
						G42Color c(g, g, g);
						for (intf j = 0; j < NumPalette; j++)
						{
							if (c == DitherPalette[j])
							{
								DitherTable[i + 256] = (byte)j;
								i = 0xff;
								break;
							}
						}
					}
				}
			}
		}
	}
}
}
}
#endif
#pragma option -Od
void
G42OptimizedPalette::SetupAvoid(void)
{
	Avoid = new bool [256];
	for (intf i = 0; i < 256; i++)
		Avoid[i] = false;
	if (NumAvoid)
	{
		AvoidColor = new G42Color [NumAvoid];
		for (intf i = 0; i < NumAvoid; i++)
			AvoidColor[i] = AddColorList[i];
		AvoidPixel = new intf [NumAvoid];
	}
}
