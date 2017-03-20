// gimdesc.cpp - multiple image descriptor
#include "gimdesc.h"
G42MultipleImageDescriptor::G42MultipleImageDescriptor(void)
:
	NumImages(0), NumPalette(0), BackgroundColorSet(false), BackgroundPixel(-1),
	NumGrays(0)
{
}
G42MultipleImageDescriptor::G42MultipleImageDescriptor(uint32 num_images)
:
	NumImages(0), NumPalette(0), BackgroundColorSet(false), BackgroundPixel(-1),
	NumGrays(0)
{
	Set(num_images);
}
G42MultipleImageDescriptor::G42MultipleImageDescriptor(
	G42MultipleImageDescriptor & ref)
:
	NumImages(0), NumPalette(0), BackgroundColorSet(false), BackgroundPixel(-1),
	NumGrays(0)
{
	Copy(ref);
}
void
G42MultipleImageDescriptor::Copy(G42MultipleImageDescriptor & ref)
{
	if (&ref == this)
		return;
	NumImages = ref.NumImages;
	BackgroundColorSet = ref.BackgroundColorSet;
	BackgroundColor = ref.BackgroundColor;
	BackgroundPixel = ref.BackgroundPixel;
	NumGrays = ref.NumGrays;
	if (ref.GlobalPalette && ref.NumPalette)
	{
		NumPalette = 0; // in case new throws
		GlobalPalette = new G42Color [ref.NumPalette];
		G42ThrowIfNull(GlobalPalette);
		NumPalette = ref.NumPalette;
		for (intf i = 0; i < NumPalette; i++)
		{
			GlobalPalette[i] = ref.GlobalPalette[i];
		}
	}
	else
	{
		GlobalPalette = 0;
		NumPalette = 0;
	}
}
G42MultipleImageDescriptor::~G42MultipleImageDescriptor(void)
{
}
void
G42MultipleImageDescriptor::Set(uint32 num_images)
{
	NumImages = num_images;
}
void
G42MultipleImageDescriptor::SetNumPalette (uint16 num_palette)
{
	NumPalette = num_palette;
	if (NumPalette)
	{
		GlobalPalette.Set(new G42Color [NumPalette]);
	}
	else
	{
		GlobalPalette.Clear();
	}
}
void
G42MultipleImageDescriptor::SetNumGrays (uint16 num_grays)
{
	NumGrays = num_grays;
	if (NumGrays)
	{
		GlobalPalette.Clear();
		NumPalette = 0;
	}
}
void
G42MultipleImageDescriptor::ClearBackgroundColor(void)
{
	BackgroundColorSet = false;
}
void
G42MultipleImageDescriptor::SetBackgroundPixel (int16 back_pixel)
{
	BackgroundPixel = back_pixel;
	BackgroundColorSet = true;
	if (back_pixel >= 0 && back_pixel < NumPalette)
	{
		BackgroundColor = GlobalPalette[back_pixel];
	}
}
void
G42MultipleImageDescriptor::SetBackgroundColor(G42Color & back_color)
{
	BackgroundColor = back_color;
	BackgroundColorSet = true;
	BackgroundPixel = -1;
	if (NumPalette)
	{
		for (uint16 i = 0; i < NumPalette; i++)
		{
			if (GlobalPalette[i] == BackgroundColor)
			{
				BackgroundPixel = i;
				break;
			}
		}
	}
	else
	{
		BackgroundPixel = -1;
	}
}
G42MultipleImageDescriptor *
G42MultipleImageDescriptor::MakeCopy(void)
{
	return new G42MultipleImageDescriptor(*this);
}
