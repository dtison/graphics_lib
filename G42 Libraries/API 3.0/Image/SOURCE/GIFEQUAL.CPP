#include "gifequal.h"
#include "gilab.h"
#include "gtmpmem.h"
/* -------------------------- 
  	Templated Filter Functions 
   -------------------------- */
//  Equalize function
/*  Most efficient for true color data, but will work for all data types  */
template <class SourceData, class DestData, 
	class SourceIterator, class DestIterator, 
	class SourcePixelConv, class DestPixelConv>
void FilterEq (DescriptorRef source_desc, DataRef source_data, 
	DescriptorRef dest_desc, DataRef dest_data, 
	SourcePixelConv source_converter, DestPixelConv dest_converter,  
	SourceData *, DestData *, 
	SourceIterator *, DestIterator *)
{
	uint32 width  = source_desc -> GetWidth();
	uint32 height = source_desc -> GetHeight();
	G42ReadCachedImage <SourceData, SourceIterator> 
		source_image (source_desc, source_data);
	G42WriteCachedImage <DestData, DestIterator> 
		dest_image (dest_desc, dest_data);
//	#define USE_FLOAT
	/*  Need a histogram and cdf (cumulative distribution function)  */
	G42TempMem <uint32> histogram (256, true);
	#ifdef USE_FLOAT
	G42TempMem <float> cdf (256, true);
	#else
	G42TempMem <uint32> cdf (256, true);
	#endif
	
	G42CIEColor::SetScreenGamma (1.8);	// 1.0 works well for equalize
	G42LABColor::SetUseTable (true);
	/*  Calculate a histogram on the L * plane  */
	uint32 i;
	for (i = 0; i < height; i++)
	{
		SourceIterator source_ptr = source_image [i];
		DestIterator dest_ptr = dest_image [i];
		for (uint32 j = 0; j < width; j++)
		{
			G42LABColor lab (source_converter (*source_ptr++));
			histogram [lab.L]++;
		}	
	}
	uint32 num_pixels = width * height;
	uint32 gray_total = 0;
	#ifdef SPECIAL_MODE
	gray_total += histogram [0];
	for (i = 1; i < 256; i++)
	{
		#ifdef USE_FLOAT
		cdf [i] = ((float) gray_total / (float) num_pixels);;
		{
			char buffer [80];
			sprintf (buffer, "CDF [%d] = %5.2f\n", i, cdf [i]);
			LogFile -> Write ((const byte *) buffer, strlen (buffer));
		}
		#else
		cdf [i] = ((gray_total << 8) / num_pixels);;
		#endif
		gray_total += histogram [i];
	}
	#endif
	for (i = 0; i < 256; i++)
	{
		gray_total += histogram [i];
		#ifdef USE_FLOAT
		cdf [i] = ((float) gray_total / (float) num_pixels);;
		#else
		cdf [i] = ((gray_total << 8) / num_pixels);;
		#endif
	}
	
	/*  Now go back and apply the equalization table  */
	for (i = 0; i < height; i++)
   {
		SourceIterator source_ptr = source_image [i];
		DestIterator dest_ptr = dest_image [i];
  		for (uint32 j = 0; j < width; j++)
  		{
			G42Color color = source_converter (*source_ptr++);
			G42LABColor lab2 (color);
			#ifdef USE_FLOAT
			int16 gray =  (byte) (cdf [lab2.L] * 255.0);
			#else
			int16 gray = (int16) cdf [lab2.L];
			#endif
			lab2.L = max (min (gray, (int16) 255), (int16) 0);
			*dest_ptr++ = dest_converter (G42Color (lab2));
		}
	}
}
/*  Equalize Filter: 
	 Color8 promotes to Color24/Color32 
	 Gray1 promotes to Gray8  */
//123456789012345678901234567890123456789012345678901234567890123456789012345678
void
G42Equalize::HandleFilter (void)
{
	G42ImageDescriptor::G42ImageType type = InputDescriptor -> GetImageType();
	if (type == G42ImageDescriptor::Color8)
	{
		/*  Color8 promotes to Color24/Color32  */
		if (OutputDescriptor -> GetImageType() == G42ImageDescriptor::Color24)
		{
			FilterEq (InputDescriptor, InputData, 
				OutputDescriptor, OutputData, 
				G42ImageDataColor8::G42ConvertToG42Color 
					(InputDescriptor -> GetPalette()),
				G42ImageDataColor24::G42ConvertFromG42Color(), 
				DC8, DC24, IC8R, IC24);
		}
		else 
		{
			FilterEq (InputDescriptor, InputData, 
				OutputDescriptor, OutputData, 
				G42ImageDataColor8::G42ConvertToG42Color 
					(InputDescriptor -> GetPalette()),
				G42ImageDataColor32::G42ConvertFromG42Color(), 
				DC8, DC32, IC8R, IC32);
		} 
	}
	else if (type == G42ImageDescriptor::Color24)
	{
		FilterEq (InputDescriptor, InputData, OutputDescriptor, OutputData, 
			G42ImageDataColor24::G42ConvertToG42Color(),
			G42ImageDataColor24::G42ConvertFromG42Color(), 
			DC24, DC24, IC24R, IC24);
	}
	else if (type == G42ImageDescriptor::Color32)
	{
		FilterEq (InputDescriptor, InputData, OutputDescriptor, OutputData, 
			G42ImageDataColor32::G42ConvertToG42Color(),
			G42ImageDataColor32::G42ConvertFromG42Color(), 
			DC32, DC32, IC32R, IC32);
	}
	else if (type == G42ImageDescriptor::Gray8)
	{
		FilterEq (InputDescriptor, InputData, 
			OutputDescriptor, OutputData, 
			G42ImageDataGray8::G42ConvertToG42Color(),
			G42ImageDataGray8::G42ConvertFromG42Color(), 
			DG8, DG8, IG8R, IG8);
	}
	else if (type == G42ImageDescriptor::Gray1)
	{	
		/*  Gray1 promotes to Gray8  */
		FilterEq (InputDescriptor, InputData, 
			OutputDescriptor, OutputData, 
			G42ImageDataGray1::G42ConvertToG42Color(),
			G42ImageDataGray8::G42ConvertFromG42Color(),
			DG1, DG8, IG1R, IG8);
	}
	/*  Copy transparency over  */
	CopyTransparency();	
}
G42ImageFilter *
G42Equalize::MakeCopy (void) const
{
	return new G42Equalize;
}
/*  Do processing here  */
void
G42Equalize::ProcessFrame (void)
{
	/*  Make a new descriptor that describes the output data  */
	uint32 width 	= InputDescriptor -> GetWidth();
	uint32 height 	= InputDescriptor -> GetHeight();
	G42ImageDescriptor::G42ImageType type = InputDescriptor -> GetImageType();
	switch (type)
	{
		case G42ImageDescriptor::Color8:
		{
			G42ImageDescriptor::G42ImageType 
				type = G42GetImageType (24, true);
			OutputDescriptor = new G42ImageDescriptor 
				(*InputDescriptor, width, height, type);
		}
		break;
		case G42ImageDescriptor::Gray1:
		{
			G42ImageDescriptor::G42ImageType 
				type = G42GetImageType (8, false);
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