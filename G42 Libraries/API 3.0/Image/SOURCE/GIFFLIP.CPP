#include "gifflip.h"
/* -------------------------- 
  	Templated Filter Functions 
   -------------------------- */
//  Flip horizontal function <almost done--width >> 1>
template <class Data, class Iterator, class Color, class Pixel>
void FilterHorizontal (DescriptorRef desc, DataRef data, 
	Data *, Iterator *, Color *, Pixel *)
{
	uint32 width 	= desc -> GetWidth();
	uint32 height 	= desc -> GetHeight();
	G42WriteCachedImage <Data, Iterator> image (desc, data);
	for (uint32 i = 0; i < height; i++)
	{
		Iterator ptr1 = image [i];
		Iterator ptr2 = ptr1;
		ptr2 += width - 1;
		for (uint32 j = 0; j < (width >> 1); j++)
		{
			/*  Save off colors and then swap positions  */
			Pixel color1 = (Color) *ptr1;
			Pixel color2 = (Color) *ptr2;
			*ptr1++ = color2;
			*ptr2-- = color1;
		}	
	}
}
// Flip vertical function <almost done--height >> 1>
template <class Data, class Iterator, class Color, class Pixel>
void FilterVertical (DescriptorRef desc, DataRef data, 
	Data *, Iterator *, Color *, Pixel *)
{
	uint32 width 	= desc -> GetWidth();
	uint32 height 	= desc -> GetHeight();
	G42WriteCachedImage <Data, Iterator> image1 (desc, data);
	for (uint32 i = 0; i < (height >> 1); i++)
	{
		Iterator ptr1 = image1 [i];
		Iterator ptr2 = image1 [height - i - 1];
		for (uint32 j = 0; j < width; j++)
		{
			/*  Save off colors and then swap rows  */
			Pixel color1 = (Color) *ptr1;
			Pixel color2 = (Color) *ptr2;
			*ptr1++ = color2;
			*ptr2++ = color1;
		}	
	}
}
G42Flip::G42Flip (G42Flip::G42FlipType type)
:
	FlipType (type)
{
}
void
G42Flip::HandleFilter (void)
{
	G42ImageDescriptor::G42ImageType type = InputDescriptor -> GetImageType();
	if (type == G42ImageDescriptor::Color8)
	{
		if (FlipType == FlipHorizontal)
		{		
			FilterHorizontal (InputDescriptor, InputData, DC8, IC8, PB8, PB8);
		}
		else
		{
			FilterVertical (InputDescriptor, InputData, DC8, IC8, PB8, PB8);
		}	
	}
	else if (type == G42ImageDescriptor::Color24) 
	{
		if (FlipType == FlipHorizontal)
		{		
			FilterHorizontal (InputDescriptor, InputData, DC24, IC24, PC24, 
				(G42Color *) 0);	// Q: Why is this needed [warnings if color class is deleted]
		}
		else
		{
			FilterVertical (InputDescriptor, InputData, DC24, IC24, PC24,
				(G42Color *) 0);
		}	
	}
	else if (type == G42ImageDescriptor::Color32)
	{
		if (FlipType == FlipHorizontal)
		{		
			FilterHorizontal (InputDescriptor, InputData, DC32, IC32, PC32,
				(G42Color *) 0);
		}
		else
		{
			FilterVertical (InputDescriptor, InputData, DC32, IC32, PC32, 
				(G42Color *) 0);
		}	
	}
	else if (type == G42ImageDescriptor::Gray8)
	{
		if (FlipType == FlipHorizontal)
		{		
			FilterHorizontal (InputDescriptor, InputData, DG8, IG8, PB8, PB8);
		}
		else
		{
			FilterVertical (InputDescriptor, InputData, DG8, IG8, PB8, PB8);
		}
	}
	else if (type == G42ImageDescriptor::Gray1)
	{
		if (FlipType == FlipHorizontal)
		{		
			FilterHorizontal (InputDescriptor, InputData, DG1, IG1, PB8, PB8);
		}
		else
		{
			FilterVertical (InputDescriptor, InputData, DG1, IG1, PB8, PB8);
		}
	}
	/*  Handle transparency if applicable  */
	if (InputDescriptor -> IsTransparent())
	{
		if (FlipType == FlipHorizontal)
		{		
			FilterHorizontal (InputDescriptor, InputData, DT1, IT1, PB8, PB8);
		}
		else
		{
			FilterVertical (InputDescriptor, InputData, DT1, IT1, PB8, PB8);
		}
	}
}
G42ImageFilter *
G42Flip::MakeCopy (void) const
{
	return new G42Flip (FlipType);
}
/*  Do processing here  */
void
G42Flip::ProcessFrame (void)
{
	HandleFilter();
	Receiver -> Process (new G42ImageDescriptor (*InputDescriptor), this);
	/*  Now send off all the strips  */
	for (uint32 i = 0; i < InputDescriptor -> GetNumStrips(); i++)
	{
		Receiver -> Process (InputData [i] -> MakeCopy(), this);
	}	
	/*  Clear everything  */
	ClearDataAndDescriptor();
}
