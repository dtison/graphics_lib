#include "gifrms.h"
#ifdef MACOS
#include "gmacfl.h"
#endif
#include <math.h>
//#define ALLTYPES
/* -------------------------- 
  	Templated Filter Functions 
   -------------------------- */
template <class SourceData, class SecondData, class DestData, class TransData,
	class SourceIterator, class SecondIterator, class DestIterator, 
	class SourceTransIterator, class DestTransIterator,  
	class SourcePixelConv, class SecondPixelConv, class DestPixelConv>
void FilterRMS (DescriptorRef source_desc, DataRef source_data, 
	DescriptorRef second_desc, DataRef second_data, 
	DescriptorRef dest_desc, DataRef dest_data, 
	SourcePixelConv source_converter, 
	SecondPixelConv second_converter, 
	DestPixelConv dest_converter, 
	double & average_rms,
	SourceData *, SecondData *, DestData *, TransData *,
	SourceIterator *, SecondIterator *, DestIterator *, 
	SourceTransIterator *, DestTransIterator *)
{
	uint32 source_width 		= source_desc -> GetWidth();
	uint32 source_height 	= source_desc -> GetHeight();
	uint32 second_width 		= second_desc -> GetWidth();
	uint32 second_height		= second_desc -> GetHeight();
	uint32 dest_width 		= dest_desc -> GetWidth();
	G42ReadCachedImage <SourceData, SourceIterator> 
		source_image (source_desc, source_data);
	G42ReadCachedImage <SecondData, SecondIterator> 
		second_image (second_desc, second_data);
	G42WriteCachedImage <DestData, DestIterator> 
		dest_image (dest_desc, dest_data);
	for (uint32 i = 0; i < source_height; i++)
	{
		SourceIterator source_ptr = source_image [i];
		SecondIterator second_ptr = second_image [i];
		DestIterator dest_ptr = dest_image [i];
		for (uint32 j = 0; j < source_width; j++)
		{
			G42Color source_color = source_converter (*source_ptr++);
			G42Color second_color = second_converter (*second_ptr++);
			// NO LAB yet
			intf red 	= (intf) source_color.Red - (intf) second_color.Red;
			intf green  = (intf) source_color.Green - (intf) second_color.Green;
			intf blue  	= (intf) source_color.Blue - (intf) second_color.Blue;
			// Variance
			red *= red;
			green *= green;
			blue *= blue;
			// Distance
			double distance = sqrt ((double) (red + green + blue));
//			distance = pow (distance, 1.5);
			distance *= 10;
			distance = min (distance, 255.0);
			*dest_ptr++ = (byte) distance;
			average_rms += distance;
		}	
	}
	average_rms /= ((double) (source_width * source_height));
}
/*  Do processing here  */
void
G42RMS::HandleEnd (G42PathEnd * end, G42PathComponent * sender)
{
	/*  Process on overlay sender call  */
	if (sender == SecondSender) 
	{
		/*  Setup output descriptor   */
		uint32 width 	= InputDescriptor -> GetWidth();
		uint32 height 	= InputDescriptor -> GetHeight();
		/*  Output Descriptor is always Gray8  */
		G42ImageDescriptor::G42ImageType type = G42GetImageType (8, false);
		OutputDescriptor = new G42ImageDescriptor (*InputDescriptor, width, 
			height, type);
		CreateOutputData();
		HandleFilter();
		Receiver -> Process (new G42ImageDescriptor (*OutputDescriptor), this);
		SendOutputData();
		Receiver -> Process (end, this);
	}
}
//123456789012345678901234567890123456789012345678901234567890123456789012345678
void
G42RMS::HandleFilter (void)
{
	/*  Merge two images, InputData, SecondImageData, into OutputData  */
	G42ImageDescriptor::G42ImageType type = InputDescriptor -> GetImageType();
	G42ImageDescriptor::G42ImageType overlay_type = 
		SecondImageDescriptor -> GetImageType();
	if (type == G42ImageDescriptor::Color8)
	{
		HandleColor8 (overlay_type);
	}
	else if (type == G42ImageDescriptor::Color24)
	{
		HandleColor24 (overlay_type);
	}
	else if (type == G42ImageDescriptor::Color32)
	{
		HandleColor32 (overlay_type);
	}
	else if (type == G42ImageDescriptor::Gray8)
	{
		HandleGray8 (overlay_type);
	}
	else if (type == G42ImageDescriptor::Gray1)
	{
		HandleGray1 (overlay_type);
	}
}
void
G42RMS::HandleColor8 (G42ImageDescriptor::G42ImageType overlay_type)
{
#ifdef ALLTYPES
	if (overlay_type == G42ImageDescriptor::Color8)
	{
		/*  Color8 + Color8 promotes Color8 to Color24/Color32 (if palettes differ)  */
		if (OutputDescriptor -> GetImageType() == G42ImageDescriptor::Color24)
		{
			FilterOverlay (InputDescriptor, InputData, 
				SecondImageDescriptor, SecondImageData, 
				OutputDescriptor, OutputData, 
				G42ImageDataColor8::G42ConvertToG42Color 
					(InputDescriptor -> GetPalette()),
				G42ImageDataColor8::G42ConvertToG42Color 
					(SecondImageDescriptor -> GetPalette()),
				G42ImageDataColor24::G42ConvertFromG42Color(), 
				Point, DrawColor, DrawColorMode, OverlayType, TransType, 
				DC8, DC8, DC24, DTS1, IC8R, IC8R, IC24, ITS1R, ITS1);
		}
		else
		{
			FilterOverlay (InputDescriptor, InputData, 
				SecondImageDescriptor, SecondImageData, 
				OutputDescriptor, OutputData, 
				G42ImageDataColor8::G42ConvertToG42Color 
					(InputDescriptor -> GetPalette()),
				G42ImageDataColor8::G42ConvertToG42Color 
					(SecondImageDescriptor -> GetPalette()),
				G42ImageDataColor32::G42ConvertFromG42Color(), 
				Point, DrawColor, DrawColorMode, OverlayType, TransType, 
				DC8, DC8, DC32, DTS1, IC8R, IC8R, IC32, ITS1R, ITS1);
		}
	}	
#endif
#ifdef ALLTYPES
	else if (overlay_type == G42ImageDescriptor::Color24)
	{
		/*  Color8 + Color24 promotes Color8 to Color24/Color32  */
		/*  Only filter 24 case because output could only be 24  */
		FilterOverlay (InputDescriptor, InputData, 
			SecondImageDescriptor, SecondImageData, 
			OutputDescriptor, OutputData, 
			G42ImageDataColor8::G42ConvertToG42Color 
				(InputDescriptor -> GetPalette()),
			G42ImageDataColor24::G42ConvertToG42Color(),
			G42ImageDataColor24::G42ConvertFromG42Color(), 
			Point, DrawColor, DrawColorMode, OverlayType, TransType, 
			DC8, DC24, DC24, DTS1, IC8R, IC24R, IC24, ITS1R, ITS1);
		
	}	
	else if (overlay_type == G42ImageDescriptor::Color32)
	{
		/*  Color8 + Color32 promotes Color8 to Color24/Color32  */
		/*  Only filter 32 case because output could only be 32  */
		FilterOverlay (InputDescriptor, InputData, 
			SecondImageDescriptor, SecondImageData, 
			OutputDescriptor, OutputData, 
			G42ImageDataColor8::G42ConvertToG42Color 
				(InputDescriptor -> GetPalette()),
			G42ImageDataColor32::G42ConvertToG42Color(),
			G42ImageDataColor32::G42ConvertFromG42Color(), 
			Point, DrawColor, DrawColorMode, OverlayType, TransType, 
			DC8, DC32, DC32, DTS1, IC8R, IC32R, IC32, ITS1R, ITS1);
	}
	else 
	if (overlay_type == G42ImageDescriptor::Gray8)
	{
		/*  Color8 + Gray8 promotes Color8 and Gray8 to Color24/Color32  */
		if (OutputDescriptor -> GetImageType() == G42ImageDescriptor::Color24)
		{
			FilterOverlay (InputDescriptor, InputData, 
				SecondImageDescriptor, SecondImageData, 
				OutputDescriptor, OutputData, 
				G42ImageDataColor8::G42ConvertToG42Color 
					(InputDescriptor -> GetPalette()),
				G42ImageDataGray8::G42ConvertToG42Color(),
				G42ImageDataColor24::G42ConvertFromG42Color(), 
				Point, DrawColor, DrawColorMode, OverlayType, TransType, 
				DC8, DG8, DC24, DTS1, IC8R, IG8R, IC24, ITS1R, ITS1);
		}
		else
		{
			FilterOverlay (InputDescriptor, InputData, 
				SecondImageDescriptor, SecondImageData, 
				OutputDescriptor, OutputData, 
				G42ImageDataColor8::G42ConvertToG42Color 
					(InputDescriptor -> GetPalette()),
				G42ImageDataGray8::G42ConvertToG42Color(),
				G42ImageDataColor32::G42ConvertFromG42Color(), 
				Point, DrawColor, DrawColorMode, OverlayType, TransType, 
				DC8, DG8, DC32, DTS1, IC8R, IG8R, IC32, ITS1R, ITS1);
		}
	}
	else if (overlay_type == G42ImageDescriptor::Gray1)
	{
		/*  Color8 + Gray1 promotes Color8 and Gray1 to Color24/Color32  */
		if (OutputDescriptor -> GetImageType() == G42ImageDescriptor::Color24)
		{
			FilterOverlay (InputDescriptor, InputData, 
				SecondImageDescriptor, SecondImageData, 
				OutputDescriptor, OutputData, 
				G42ImageDataColor8::G42ConvertToG42Color 
					(InputDescriptor -> GetPalette()),
				G42ImageDataGray1::G42ConvertToG42Color(),
				G42ImageDataColor24::G42ConvertFromG42Color(), 
				Point, DrawColor, DrawColorMode, OverlayType, TransType, 
				DC8, DG1, DC24, DTS1, IC8R, IG1R, IC24, ITS1R, ITS1);
		}
		else
		{
			FilterOverlay (InputDescriptor, InputData, 
				SecondImageDescriptor, SecondImageData, 
				OutputDescriptor, OutputData, 
				G42ImageDataColor8::G42ConvertToG42Color 
					(InputDescriptor -> GetPalette()),
				G42ImageDataGray1::G42ConvertToG42Color(),
				G42ImageDataColor32::G42ConvertFromG42Color(), 
				Point, DrawColor, DrawColorMode, OverlayType, TransType, 
				DC8, DG1, DC32, DTS1, IC8R, IG1R, IC32, ITS1R, ITS1);
		}
	}
#endif	// ALLTYPES
}
void
G42RMS::HandleColor24 (G42ImageDescriptor::G42ImageType overlay_type)
{
	if (overlay_type == G42ImageDescriptor::Color8)
	{
		/*  Don't need transparency DTS1, ITS1R and ITS1  */
		FilterRMS (InputDescriptor, InputData, 
			SecondImageDescriptor, SecondImageData, 
			OutputDescriptor, OutputData, 
			G42ImageDataColor24::G42ConvertToG42Color(),
			G42ImageDataColor8::G42ConvertToG42Color 
				(SecondImageDescriptor -> GetPalette()),
			G42ImageDataGray8::G42ConvertFromG42Color(), 
			AverageRMS,
			DC24, DC8, DG8, DTS1, IC24R, IC8R, IG8, ITS1R, ITS1);
	}	
#ifdef ALLTYPES
	else if (overlay_type == G42ImageDescriptor::Color24)
	{
		FilterOverlay (InputDescriptor, InputData, 
			SecondImageDescriptor, SecondImageData, 
			OutputDescriptor, OutputData, 
			G42ImageDataColor24::G42ConvertToG42Color(), 
			G42ImageDataColor24::G42ConvertToG42Color(), 
			G42ImageDataColor24::G42ConvertFromG42Color(), 
			Point, DrawColor, DrawColorMode, OverlayType, TransType, 
			DC24, DC24, DC24, DTS1, IC24R, IC24R, IC24, ITS1R, ITS1);
	}
	else if (overlay_type == G42ImageDescriptor::Gray8)
	{
		FilterOverlay (InputDescriptor, InputData, 
			SecondImageDescriptor, SecondImageData, 
			OutputDescriptor, OutputData, 
			G42ImageDataColor24::G42ConvertToG42Color(),
			G42ImageDataGray8::G42ConvertToG42Color(), 
			G42ImageDataColor24::G42ConvertFromG42Color(), 
			Point, DrawColor, DrawColorMode, OverlayType, TransType, 
			DC24, DG8, DC24, DTS1, IC24R, IG8R, IC24, ITS1R, ITS1);		
	}
	else if (overlay_type == G42ImageDescriptor::Gray1)
	{
		FilterOverlay (InputDescriptor, InputData, 
			SecondImageDescriptor, SecondImageData, 
			OutputDescriptor, OutputData, 
			G42ImageDataColor24::G42ConvertToG42Color(),
			G42ImageDataGray1::G42ConvertToG42Color(), 
			G42ImageDataColor24::G42ConvertFromG42Color(), 
			Point, DrawColor, DrawColorMode, OverlayType, TransType, 
			DC24, DG1, DC24, DTS1, IC24R, IG1R, IC24, ITS1R, ITS1);		
	}
#else
if (overlay_type) {}
#endif
}
void
G42RMS::HandleColor32 (G42ImageDescriptor::G42ImageType overlay_type)
{
	if (overlay_type == G42ImageDescriptor::Color8)
	{
		/*  Don't need transparency DTS1, ITS1R and ITS1  */
		FilterRMS (InputDescriptor, InputData, 
			SecondImageDescriptor, SecondImageData, 
			OutputDescriptor, OutputData, 
			G42ImageDataColor32::G42ConvertToG42Color(),
			G42ImageDataColor8::G42ConvertToG42Color 
				(SecondImageDescriptor -> GetPalette()),
			G42ImageDataGray8::G42ConvertFromG42Color(), 
			AverageRMS,
			DC32, DC8, DG8, DTS1, IC32R, IC8R, IG8, ITS1R, ITS1);
	}	
#ifdef ALLTYPES
	else if (overlay_type == G42ImageDescriptor::Color32)
	{
		FilterOverlay (InputDescriptor, InputData, 
			SecondImageDescriptor, SecondImageData, 
			OutputDescriptor, OutputData, 
			G42ImageDataColor32::G42ConvertToG42Color(), 
			G42ImageDataColor32::G42ConvertToG42Color(), 
			G42ImageDataColor32::G42ConvertFromG42Color(), 
			Point, DrawColor, DrawColorMode, OverlayType, TransType, 
			DC32, DC32, DC32, DTS1, IC32R, IC32R, IC32, ITS1R, ITS1);		
	}
	else if (overlay_type == G42ImageDescriptor::Gray8)
	{
		FilterOverlay (InputDescriptor, InputData, 
			SecondImageDescriptor, SecondImageData, 
			OutputDescriptor, OutputData, 
			G42ImageDataColor32::G42ConvertToG42Color(),
			G42ImageDataGray8::G42ConvertToG42Color(), 
			G42ImageDataColor32::G42ConvertFromG42Color(), 
			Point, DrawColor, DrawColorMode, OverlayType, TransType, 
			DC32, DG8, DC32, DTS1, IC32R, IG8R, IC32, ITS1R, ITS1);		
	}
	else if (overlay_type == G42ImageDescriptor::Gray1)
	{
		FilterOverlay (InputDescriptor, InputData, 
			SecondImageDescriptor, SecondImageData, 
			OutputDescriptor, OutputData, 
			G42ImageDataColor32::G42ConvertToG42Color(),
			G42ImageDataGray1::G42ConvertToG42Color(), 
			G42ImageDataColor32::G42ConvertFromG42Color(), 
			Point, DrawColor, DrawColorMode, OverlayType, TransType, 
			DC32, DG1, DC32, DTS1, IC32R, IG1R, IC32, ITS1R, ITS1);		
	}
#else
if (overlay_type) {}
#endif
}
void
G42RMS::HandleGray8 (G42ImageDescriptor::G42ImageType overlay_type)
{
#ifdef ALLTYPES
	if (overlay_type == G42ImageDescriptor::Color8)
	{
		/*  Gray8 + Color8 promotes Gray8 and Color8 to Color24/32 */
		if (OutputDescriptor -> GetImageType() == G42ImageDescriptor::Color24)
		{
			FilterOverlay (InputDescriptor, InputData, 
				SecondImageDescriptor, SecondImageData, 
				OutputDescriptor, OutputData, 
				G42ImageDataGray8::G42ConvertToG42Color(),
				G42ImageDataColor8::G42ConvertToG42Color 
					(SecondImageDescriptor -> GetPalette()),
				G42ImageDataColor24::G42ConvertFromG42Color(), 
				Point, DrawColor, DrawColorMode, OverlayType, TransType, 
				DG8, DC8, DC24, DTS1, IG8R, IC8R, IC24, ITS1R, ITS1);
		}
		else
		{
			FilterOverlay (InputDescriptor, InputData, 
				SecondImageDescriptor, SecondImageData, 
				OutputDescriptor, OutputData, 
				G42ImageDataGray8::G42ConvertToG42Color(),
				G42ImageDataColor8::G42ConvertToG42Color 
					(SecondImageDescriptor -> GetPalette()),
				G42ImageDataColor32::G42ConvertFromG42Color(), 
				Point, DrawColor, DrawColorMode, OverlayType, TransType, 
				DG8, DC8, DC32, DTS1, IG8R, IC8R, IC32, ITS1R, ITS1);
		}
	}	
	else if (overlay_type == G42ImageDescriptor::Color24)
	{
		/*  Gray8 + Color24 promotes Gray8 to Color24  */
		FilterOverlay (InputDescriptor, InputData, 
			SecondImageDescriptor, SecondImageData, 
			OutputDescriptor, OutputData, 
			G42ImageDataGray8::G42ConvertToG42Color(),
			G42ImageDataColor24::G42ConvertToG42Color(),
			G42ImageDataColor24::G42ConvertFromG42Color(), 
				Point, DrawColor, DrawColorMode, OverlayType, TransType, 
			DG8, DC24, DC24, DTS1, IG8R, IC24R, IC24, ITS1R, ITS1);
	}
	else 
	if (overlay_type == G42ImageDescriptor::Color32)
	{
		/*  Gray8 + Color32 promotes Gray8 to Color32  */
		FilterOverlay (InputDescriptor, InputData, 
			SecondImageDescriptor, SecondImageData, 
			OutputDescriptor, OutputData, 
			G42ImageDataGray8::G42ConvertToG42Color(),
			G42ImageDataColor32::G42ConvertToG42Color(),
			G42ImageDataColor32::G42ConvertFromG42Color(), 
			Point, DrawColor, DrawColorMode, OverlayType, TransType, 
			DG8, DC32, DC32, DTS1, IG8R, IC32R, IC32, ITS1R, ITS1);
	}
	#endif
	#ifdef ALLTYPES
	else if (overlay_type == G42ImageDescriptor::Gray8)
	{
		/*  Gray8 + Gray8 does not promote  */
		FilterOverlay (InputDescriptor, InputData, 
			SecondImageDescriptor, SecondImageData, 
			OutputDescriptor, OutputData, 
			G42ImageDataGray8::G42ConvertToG42Color(),
			G42ImageDataGray8::G42ConvertToG42Color(),
			G42ImageDataGray8::G42ConvertFromG42Color(), 
				Point, DrawColor, DrawColorMode, OverlayType, TransType, 
			DG8, DG8, DG8, DTS1, IG8R, IG8R, IG8, ITS1R, ITS1);
	}
	else if (overlay_type == G42ImageDescriptor::Gray1)
	{
		/*  Gray8 + Gray1 produces Gray8  */
		FilterOverlay (InputDescriptor, InputData, 
			SecondImageDescriptor, SecondImageData, 
			OutputDescriptor, OutputData, 
			G42ImageDataGray8::G42ConvertToG42Color(),
			G42ImageDataGray1::G42ConvertToG42Color(), 
			G42ImageDataGray8::G42ConvertFromG42Color(), 
			Point, DrawColor, DrawColorMode, OverlayType, TransType, 
			DG8, DG1, DG8, DTS1, IG8R, IG1R, IG8, ITS1R, ITS1);		
	}
#else
if (overlay_type) {}
#endif
}
void
G42RMS::HandleGray1 (G42ImageDescriptor::G42ImageType overlay_type)
{
#ifdef ALLTYPES
	if (overlay_type == G42ImageDescriptor::Color8)
	{
		/*  Gray1 + Color8 promotes Gray1 and Color8 to Color24/32 */
		if (OutputDescriptor -> GetImageType() == G42ImageDescriptor::Color24)
		{
			FilterOverlay (InputDescriptor, InputData, 
				SecondImageDescriptor, SecondImageData, 
				OutputDescriptor, OutputData, 
				G42ImageDataGray1::G42ConvertToG42Color(),
				G42ImageDataColor8::G42ConvertToG42Color 
					(SecondImageDescriptor -> GetPalette()),
				G42ImageDataColor24::G42ConvertFromG42Color(), 
				Point, DrawColor, DrawColorMode, OverlayType, TransType, 
				DG1, DC8, DC24, DTS1, IG1R, IC8R, IC24, ITS1R, ITS1);
		}
		else
		{
			FilterOverlay (InputDescriptor, InputData, 
				SecondImageDescriptor, SecondImageData, 
				OutputDescriptor, OutputData, 
				G42ImageDataGray1::G42ConvertToG42Color(),
				G42ImageDataColor8::G42ConvertToG42Color 
					(SecondImageDescriptor -> GetPalette()),
				G42ImageDataColor32::G42ConvertFromG42Color(), 
				Point, DrawColor, DrawColorMode, OverlayType, TransType, 
				DG1, DC8, DC32, DTS1, IG1R, IC8R, IC32, ITS1R, ITS1);
		}
	}	
	else if (overlay_type == G42ImageDescriptor::Color24)
	{
		/*  Gray1 + Color24 promotes Gray1 to Color24  */
		FilterOverlay (InputDescriptor, InputData, 
			SecondImageDescriptor, SecondImageData, 
			OutputDescriptor, OutputData, 
			G42ImageDataGray1::G42ConvertToG42Color(),
			G42ImageDataColor24::G42ConvertToG42Color(),
			G42ImageDataColor24::G42ConvertFromG42Color(), 
			Point, DrawColor, DrawColorMode, OverlayType, TransType, 
			DG1, DC24, DC24, DTS1, IG1R, IC24R, IC24, ITS1R, ITS1);
	}
	else if (overlay_type == G42ImageDescriptor::Color32)
	{
		/*  Gray1 + Color32 promotes Gray8 to Color32  */
		FilterOverlay (InputDescriptor, InputData, 
			SecondImageDescriptor, SecondImageData, 
			OutputDescriptor, OutputData, 
			G42ImageDataGray1::G42ConvertToG42Color(),
			G42ImageDataColor32::G42ConvertToG42Color(),
			G42ImageDataColor32::G42ConvertFromG42Color(), 
			Point, DrawColor, DrawColorMode, OverlayType, TransType, 
			DG1, DC32, DC32, DTS1, IG1R, IC32R, IC32, ITS1R, ITS1);
	}
	else if (overlay_type == G42ImageDescriptor::Gray8)
	{
		/*  Gray1 + Gray8 promotes Gray1 to Gray8  */
		FilterOverlay (InputDescriptor, InputData, 
			SecondImageDescriptor, SecondImageData, 
			OutputDescriptor, OutputData, 
			G42ImageDataGray1::G42ConvertToG42Color(),
			G42ImageDataGray8::G42ConvertToG42Color(),
			G42ImageDataGray8::G42ConvertFromG42Color(), 
			Point, DrawColor, DrawColorMode, OverlayType, TransType, 
			DG1, DG8, DG8, DTS1, IG1R, IG8R, IG8, ITS1R, ITS1);
	}
	else if (overlay_type == G42ImageDescriptor::Gray1)
	{
		/*  Gray1 + Gray1 does not promote   */
		FilterOverlay (InputDescriptor, InputData, 
			SecondImageDescriptor, SecondImageData, 
			OutputDescriptor, OutputData, 
			G42ImageDataGray1::G42ConvertToG42Color(),
			G42ImageDataGray1::G42ConvertToG42Color(), 
			G42ImageDataGray1::G42ConvertFromG42Color(), 
			Point, DrawColor, DrawColorMode, OverlayType, TransType, 
			DG1, DG1, DG1, DTS1, IG1R, IG1R, IG1, ITS1R, ITS1);		
	}
#else
if (overlay_type) {}
#endif
}
G42ImageFilter *
G42RMS::MakeCopy (void) const
{
	return new G42RMS;
}
