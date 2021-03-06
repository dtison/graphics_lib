#include "gifinv.h"
#ifdef MACOS
#define USE_PROFILER
#ifdef USE_PROFILER
#include <profiler.h>
#endif
#endif
#define USE_COLOR_CONVERTERS
/* -------------------------- 
  	Templated Filter Functions 
   -------------------------- */
//  Invert 
template <class Data, class Iterator, 
	class SourcePixelConv, class DestPixelConv>
void FilterInvert (DescriptorRef desc, DataRef data, 
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
#ifdef USE_COLOR_CONVERTERS
			G42Color color = source_converter (*ptr);
			color.Red 	= (byte) ~color.Red;
			color.Green = (byte) ~color.Green;
			color.Blue 	= (byte) ~color.Blue;
			*ptr++ = dest_converter (color);
#else
			*ptr = *ptr++;	// Just a copy
#endif
		}	
	}
}
G42Invert::G42Invert (void) 
: 
	PaletteMode (false)
{ }
void
G42Invert::HandleDescriptor (G42ImageDescriptor * desc, G42PathComponent * sender)
{
	/*  Invert processes palette images differently  */
	if (desc -> IsPaletted())
	{
		/*  For Color8, we invert the palette  */
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
void
G42Invert::HandleFilter (void)
{
	G42ImageDescriptor::G42ImageType type = InputDescriptor -> GetImageType();
	if (type == G42ImageDescriptor::Color24) 
	{
		FilterInvert (InputDescriptor, InputData, 
			G42ImageDataColor24::G42ConvertToG42Color(),
			G42ImageDataColor24::G42ConvertFromG42Color(),
			DC24, IC24);
	}
	else if (type == G42ImageDescriptor::Color32)
	{
	#ifdef USE_PROFILER
	ProfilerSetStatus (true);
	#endif
		FilterInvert (InputDescriptor, InputData, 
			G42ImageDataColor32::G42ConvertToG42Color(),
			G42ImageDataColor32::G42ConvertFromG42Color(),
			DC32, IC32);
	#ifdef USE_PROFILER
	ProfilerSetStatus (false);
	ProfilerDump ("\pTestApp.prof");
	ProfilerClear();
	#endif
	}
	else if (type == G42ImageDescriptor::Gray8)
	{
		FilterInvert (InputDescriptor, InputData, 
			G42ImageDataGray8::G42ConvertToG42Color(),
			G42ImageDataGray8::G42ConvertFromG42Color(),
			DG8, IG8); 
	}
	else if (type == G42ImageDescriptor::Gray1)
	{
		FilterInvert (InputDescriptor, InputData, 
			G42ImageDataGray1::G42ConvertToG42Color(),
			G42ImageDataGray1::G42ConvertFromG42Color(),
			DG1, IG1); 
	}
	/*  (Transparency stays within InputDescriptor)  */
}
G42ImageFilter *
G42Invert::MakeCopy (void) const
{
	return new G42Invert();
}
void
G42Invert::ProcessFrame (void)
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
