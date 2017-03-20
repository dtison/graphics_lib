#include "gifbord.h"
/* -------------------------- 
  	Templated Filter Functions 
   -------------------------- */
//  Border function
/*  Most efficient for true color data, but will work for all data types  */
	template <class SourceData, class DestData, 
	class SourceIterator, class DestIterator, 
	class SourcePixelConv, class DestPixelConv>
void FilterBorder (DescriptorRef source_desc, DataRef source_data, 
	DescriptorRef dest_desc, DataRef dest_data, 
	SourcePixelConv source_converter, DestPixelConv dest_converter, 
	int16 size, G42Color & color, 
	SourceData *, DestData *, 
	SourceIterator *, DestIterator *)
{
	uint32 width 	= dest_desc -> GetWidth();
	uint32 height 	= dest_desc -> GetHeight();
	uint32 source_width  = source_desc -> GetWidth();
	uint32 source_height = source_desc -> GetHeight();
	G42ReadCachedImage <SourceData, SourceIterator> 
		source_image (source_desc, source_data);
	G42WriteCachedImage <DestData, DestIterator> 
		dest_image (dest_desc, dest_data);
	#if 0
	/*  Calculate a shadow color  */
	RGBColor colorm;
	colorm.red = color.Red << 8;
	colorm.green = color.Green << 8;
	colorm.blue = color.Blue << 8;
	
	HSVColor color_hsv;
	::RGB2HSV (&colorm, &color_hsv);
	color_hsv.value >>= 1;
	::HSV2RGB (&color_hsv, &colorm);
	G42Color color2 (colorm.red >> 8, colorm.green >> 8, colorm.blue >> 8);
	#endif
	
	G42Color color2 = color;
	DestIterator dest_ptr;
	/*  Copy image over  */
	int i;
   for (i = 0; i < source_height; i++)
   {
   	SourceIterator source_ptr = source_image [i];
		DestIterator dest_ptr = dest_image [i + size];
		dest_ptr += size;
		CopyPixels (source_ptr, dest_ptr, source_converter, dest_converter, 
			source_width);	
   }
	
	/*  Top border  */
   for (i = 0; i < size; i++)
   {
		dest_ptr = dest_image [i];
		for (int j = 0; j < width; j++)
		{
			*dest_ptr++ = dest_converter (color);
		} 
   }
	/*  Two sides  */
   for (i = size; i < (height - size); i++)
   {
   	/*  Left side  */
		dest_ptr = dest_image [i];
		int j;
		for (j = 0; j < size; j++)
		{
			*dest_ptr++ = dest_converter (color);
		} 
   	/*  Right side  */
		dest_ptr = dest_image [i];
		dest_ptr += (width - size);		
		for (j = 0; j < size; j++)
		{
			*dest_ptr++ = dest_converter (color2);
		} 	
   }
	/*  Bottom border  */
   for (i = height - size; i < height; i++)
   {
		dest_ptr = dest_image [i];
		for (int j = 0; j < width; j++)
		{
			*dest_ptr++ = dest_converter (color2);
		} 
   }
   
	#ifndef MSWIN	//  Borland warnings
   
   /*  Put on the black lines  */
   G42Color black (0, 0, 0);
   for (i = 0; i < size; i++)
	{
		dest_ptr = dest_image [i];
		dest_ptr += i;
		*dest_ptr = dest_converter (black);		
		dest_ptr += (width - (i << 1) - 1);
		*dest_ptr++ = dest_converter (black);		
		#if 0
		/*  Where pixels are to the right of black line.. */
		if (i > 0)
		{
			for (uint32 j = 0; j < (i - 1); j++)
			{
				*dest_ptr++ = dest_converter (color2);		
			}
		}
		#else
		for (uint32 j = 0; j < i; j++)
		{
			*dest_ptr++ = dest_converter (color2);		
		}
		#endif
		/*  Bottom border  */
		dest_ptr = dest_image [i + height - size];
		uint32 dest = (size - i - 1);
		for (uint32 j = 0; j < dest; j++)
		{
			*dest_ptr++ = dest_converter (color);
		}		
		*dest_ptr = dest_converter (black);		
		dest_ptr += (source_width + 1 + (i << 1));
		*dest_ptr = dest_converter (black);		
	}
   #endif
}
G42Border::G42Border (int16 size, G42Color color)
:	BorderColor (color), BorderSize (size)
{ }
/*  Border Filter:
	 Color8 promotes to Color24/Color32 
	 Gray8 promotes to Color24/Color32 
	 Gray1 promotes to Color24/Color32  */
//123456789012345678901234567890123456789012345678901234567890123456789012345678
void
G42Border::HandleFilter (void)
{
	G42ImageDescriptor::G42ImageType type = InputDescriptor -> GetImageType();
	if (type == G42ImageDescriptor::Color8)
	{
		/*  Color8 promotes to Color24/Color32  */
		if (OutputDescriptor -> GetImageType() == G42ImageDescriptor::Color24)
		{
			FilterBorder (InputDescriptor, InputData, OutputDescriptor, OutputData, 
				G42ImageDataColor8::G42ConvertToG42Color (InputDescriptor -> GetPalette()),
				G42ImageDataColor24::G42ConvertFromG42Color(), 
				BorderSize, BorderColor, DC8, DC24, IC8R, IC24);
		}
		else
		{
			FilterBorder (InputDescriptor, InputData, OutputDescriptor, OutputData, 
				G42ImageDataColor8::G42ConvertToG42Color (InputDescriptor -> GetPalette()),
				G42ImageDataColor32::G42ConvertFromG42Color(), 
				BorderSize, BorderColor, DC8, DC32, IC8R, IC32);
		} 
	}
	else if (type == G42ImageDescriptor::Color24)
	{
		FilterBorder (InputDescriptor, InputData, OutputDescriptor, OutputData, 
			G42ImageDataColor24::G42ConvertToG42Color(),
			G42ImageDataColor24::G42ConvertFromG42Color(), 
			BorderSize, BorderColor, DC24, DC24, IC24R, IC24);
	}
	else if (type == G42ImageDescriptor::Color32)
	{
		FilterBorder (InputDescriptor, InputData, OutputDescriptor, OutputData, 
			G42ImageDataColor32::G42ConvertToG42Color(),
			G42ImageDataColor32::G42ConvertFromG42Color(),
			BorderSize, BorderColor, DC32, DC32, IC32R, IC32);
	}
	else if (type == G42ImageDescriptor::Gray8)
	{
		/*  Gray8 promotes to Color24/Color32  */
		if (OutputDescriptor -> GetImageType() == G42ImageDescriptor::Color24)
		{
			FilterBorder (InputDescriptor, InputData, OutputDescriptor, OutputData, 
				G42ImageDataGray8::G42ConvertToG42Color(),
				G42ImageDataColor24::G42ConvertFromG42Color(),
				BorderSize, BorderColor, DG8, DC24, IG8R, IC24);
		}
		else
		{
			FilterBorder (InputDescriptor, InputData, OutputDescriptor, OutputData, 
				G42ImageDataGray8::G42ConvertToG42Color(),
				G42ImageDataColor32::G42ConvertFromG42Color(),
				BorderSize, BorderColor, DG8, DC32, IG8R, IC32);
		}
	}
	else if (type == G42ImageDescriptor::Gray1)
	{
		/*  Gray1 promotes to Color24/Color32  */
		if (OutputDescriptor -> GetImageType() == G42ImageDescriptor::Color24)
		{
			FilterBorder (InputDescriptor, InputData, OutputDescriptor, OutputData, 
				G42ImageDataGray1::G42ConvertToG42Color(),
				G42ImageDataColor24::G42ConvertFromG42Color(),
				BorderSize, BorderColor, DG1, DC24, IG1R, IC24);
		}
		else
		{
			FilterBorder (InputDescriptor, InputData, OutputDescriptor, OutputData, 
				G42ImageDataGray1::G42ConvertToG42Color(),
				G42ImageDataColor32::G42ConvertFromG42Color(),
				BorderSize, BorderColor, DG1, DC32, IG1R, IC32);
		}
	}
	/*  Handle transparency if applicable  */
	if (OutputDescriptor -> IsTransparent())
	{
		FilterBorder (InputDescriptor, InputData, OutputDescriptor, OutputData, 
			G42ImageDataGray1::G42ConvertToG42Color(),
			G42ImageDataGray1::G42ConvertFromG42Color(),
			BorderSize, G42Color (255, 255, 255), DT1, DT1, IT1R, IT1);	
	}
}
G42ImageFilter *
G42Border::MakeCopy (void) const
{
	return new G42Border (BorderSize, BorderColor);
}
/*  Do processing here  */
void
G42Border::ProcessFrame (void)
{
	/*  Make a new descriptor that describes the output data  */
	uint32 width 	= (InputDescriptor -> GetWidth() + (BorderSize << 1));
	uint32 height 	= (InputDescriptor -> GetHeight() + (BorderSize << 1));
	G42ImageDescriptor::G42ImageType type = InputDescriptor -> GetImageType();
	switch (type)
	{
		case G42ImageDescriptor::Color8:
		case G42ImageDescriptor::Gray8:
		case G42ImageDescriptor::Gray1:
		{
			G42ImageDescriptor::G42ImageType 
				type = G42GetImageType (24, true);
			OutputDescriptor = new G42ImageDescriptor 
				(*InputDescriptor, width, height, type);
		}
		break;
		default:
			OutputDescriptor = new G42ImageDescriptor (*InputDescriptor, 
				width, height);
		break;
	}
	CreateOutputData();
	HandleFilter();
	Receiver -> Process (new G42ImageDescriptor (*OutputDescriptor), this);
	SendOutputData();
	/*  Clear everything  */
	ClearDataAndDescriptor (true);
}
