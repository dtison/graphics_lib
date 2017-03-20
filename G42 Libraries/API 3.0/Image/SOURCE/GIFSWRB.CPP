#include "gifswrb.h"
/* -------------------------- 
  	Templated Filter Functions 
   -------------------------- */
//  SwapRedBlue 
template <class Data, class Iterator, 
	class SourcePixelConv, class DestPixelConv>
void FilterSwapRB (DescriptorRef desc, DataRef data, 
	SourcePixelConv source_converter, 
	DestPixelConv dest_converter,
	Data *, Iterator *)
{
	uint32 width 	= desc -> GetWidth();
	uint32 height 	= desc -> GetHeight();
	G42WriteCachedImage <Data, Iterator> image (desc, data);
	for (uint32 i = 0; i < height; i++)
	{
		Iterator ptr = image [i];
		for (uint32 j = 0; j < width; j++)
		{
			G42Color color = source_converter (*ptr);
			swap (color.Red, color.Blue);
			*ptr++ = dest_converter (color);
		}	
	}
}
G42SwapRedBlue::G42SwapRedBlue (void) 
: 
	PaletteMode (false)
{ }
void
G42SwapRedBlue::HandleDescriptor (G42ImageDescriptor * desc, G42PathComponent * sender)
{
	/*  Invert processes palette images differently  */
	if (desc -> IsPaletted())
	{
		/*  For Color8, we SwapRedBlue the palette  */
		G42Color * pal = desc -> GetPalette();
		for (intf i = 0; i < desc -> GetNumPalette(); i++)
		{
			pal[i].Red 		= (byte) (~(pal[i].Red));
			pal[i].Green 	= (byte) (~(pal[i].Green));
			pal[i].Blue 	= (byte) (~(pal[i].Blue));
		}
		PaletteMode = true;
	}
	/*  Call parent function for default handler  */
	G42ImageFilter::HandleDescriptor (desc, sender);
}
/*  Do processing here  */
/*  Only process color images  */
void
G42SwapRedBlue::HandleFilter (void)
{
	G42ImageDescriptor::G42ImageType type = InputDescriptor -> GetImageType();
	if (type == G42ImageDescriptor::Color24) 
	{
		FilterSwapRB (InputDescriptor, InputData, 
			G42ImageDataColor24::G42ConvertToG42Color(),
			G42ImageDataColor24::G42ConvertFromG42Color(),
			DC24, IC24);
	}
	else if (type == G42ImageDescriptor::Color32)
	{
		FilterSwapRB (InputDescriptor, InputData, 
			G42ImageDataColor32::G42ConvertToG42Color(),
			G42ImageDataColor32::G42ConvertFromG42Color(),
			DC32, IC32);
	}
	/*  (Transparency stays within InputDescriptor)  */
}
G42ImageFilter *
G42SwapRedBlue::MakeCopy (void) const
{
	return new G42SwapRedBlue();
}
void
G42SwapRedBlue::ProcessFrame (void)
{
	if (! InputDescriptor -> IsPaletted())
	{
		HandleFilter();
	}
	Receiver -> Process (new G42ImageDescriptor (*InputDescriptor), this);
	/*  Now send off all the strips  */
	for (uint32 i = 0; i < InputDescriptor -> GetNumStrips(); i++)
	{
		Receiver -> Process (InputData [i] -> MakeCopy(), this);
	}	
	/*  Clear everything  */
	ClearDataAndDescriptor();
}