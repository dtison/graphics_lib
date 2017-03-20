#include "gifovl.h"
#ifdef MACOS
#include "gmacfl.h"
#endif
#ifdef MACOS
//#define USE_PROFILER
#ifdef USE_PROFILER
#include <profiler.h>
#endif
#endif
// TODO's  1. Cleanup what in Cleanup?  2. Palette checker
// LEFT OFF:  Just tested color32 + color32 overlay.  Need to test Gray esp.
#define ALLTYPES
const byte Yes = 1;
const byte No  = 0;
const byte G42Color8Index 	= 0;
const byte G42Color24Index = 1;
const byte G42Gray8Index 	= 2;
const byte G42Gray1Index 	= 3;
byte OutputDepths [4][4] = {{24, 24, 24, 24},			// Color8
									 {24, 24, 24, 24},			// Color24/32	
									 {24, 24, 8, 8},				// Gray8
									 {24, 24, 8, 1}};				// Gray1
byte OutputIsColor [4][4] = {{Yes, Yes, Yes, Yes},
								     {Yes, Yes, Yes, Yes},	
									  {Yes, Yes, No, No},	
									  {Yes, Yes, No, No}};
#if 0
byte OutputConverters [4][4] = {{Yes, Yes, Yes, Yes},
									 	  {Yes, No, Yes, Yes},	
										  {Yes, Yes, No, No},	// not sure about last No	
										  {Yes, Yes, No, No}};  //  "  "      "     " two No's
#endif
/* -------------------------- 
  	Templated Filter Functions 
   -------------------------- */
/*  Most efficient for true color data, but will work for all data types  */
template <class SourceData, class OverlData, class DestData, class TransData,
	class SourceIterator, class OverlayIterator, class DestIterator, 
	class SourceTransIterator, class DestTransIterator,  
	class SourcePixelConv, class OverlayPixelConv, class DestPixelConv>
void FilterOverlay (DescriptorRef source_desc, DataRef source_data, 
	DescriptorRef overlay_desc, DataRef overlay_data, 
	DescriptorRef dest_desc, DataRef dest_data, 
	SourcePixelConv source_converter, 
	OverlayPixelConv overlay_converter, 
	DestPixelConv dest_converter, 
	G42Point point, G42Color draw_color, bool draw_color_mode,
	G42Overlay::G42OverlayType overlay_type,
	G42Overlay::G42TransType trans_type,
	SourceData *, OverlData *, DestData *, TransData *,
	SourceIterator *, OverlayIterator *, DestIterator *, 
	SourceTransIterator *, DestTransIterator *)
{
	uint32 source_width 		= source_desc -> GetWidth();
	uint32 source_height 	= source_desc -> GetHeight();
	uint32 overlay_width 	= overlay_desc -> GetWidth();
	uint32 overlay_height	= overlay_desc -> GetHeight();
	uint32 dest_width 		= dest_desc -> GetWidth();
	G42TransReadCachedImage <SourceData, SourceIterator, 
		TransData, SourceTransIterator> source_image (source_desc, source_data);
	G42TransReadCachedImage <OverlData, OverlayIterator, 
		TransData, SourceTransIterator> overlay_image (overlay_desc, overlay_data);
	G42TransWriteCachedImage <DestData, DestIterator, TransData, 
		DestTransIterator> dest_image (dest_desc, dest_data);
	if (trans_type) {}
	#ifdef USE_PROFILER
//	ProfilerSetStatus (true);
	#endif
#define TRY_REGION
	if (overlay_type == G42Overlay::OverlayClip)
	{
		/*  Assumes point is within source image [bad things can happen otherwise]  */
		uint32 source_index 	= 0;
		uint32 dest_index 	= 0;
		uint32 & curr_index 	= dest_index;
		/*  Transfer height is height in dest where overlay begins  */
		uint32 transfer_height = max ((int32) 0, point.Y);
		uint32 i;
		for (i = 0; i < transfer_height; i++)
		{
			SourceIterator source_ptr  		= source_image [source_index];
			DestIterator dest_ptr 				= dest_image [dest_index];
			/*  Get transparency pointers  */
			SourceTransIterator source_tptr  = source_image.GetTransIterator (source_index);
			DestTransIterator dest_tptr		= dest_image.GetTransIterator (dest_index);
			for (uint32 j = 0; j < dest_width; j++) 
			{	
				*dest_ptr++ = dest_converter (source_converter (*source_ptr++));
				*dest_tptr++ = (bool) *source_tptr++;
			}
			source_index++;
			dest_index++;
		}
						
		uint32 overlay_index = (point.Y < 0) ? -point.Y : 0;
		if (overlay_index > overlay_height) {
			throw (G42OverlayOutsideImageException(0));
		}
		/*  Transfer height is height of area containing overlay  */
		transfer_height = (point.Y < 0) ? overlay_height + point.Y :
			overlay_height;
		/*  Clip for overlays that extend below the source image  */
		if ((transfer_height + curr_index) > source_height) {	
			transfer_height = (source_height - curr_index);
		}
		/*  Place in the overlay image  */
		for (i = 0; i < transfer_height; i++)
		{
			SourceIterator source_ptr   		= source_image [source_index];
			DestIterator dest_ptr 		 		= dest_image [dest_index];
			/*  Get transparency pointers  */
			SourceTransIterator source_tptr  = source_image.GetTransIterator (source_index);
			DestTransIterator dest_tptr		= dest_image.GetTransIterator (dest_index);
			/*  Copy left side  */
			uint32 transfer_width = max ((int32) 0, point.X);		
			uint32 curr_col = 0;
			uint32 j;
			for (j = 0; j < transfer_width; j++)
			{
				*dest_ptr++ = dest_converter (source_converter (*source_ptr++));
				*dest_tptr++ = (bool) *source_tptr++;
				curr_col++;
			}
			/*  Transfer height is height of area containing overlay  */
			transfer_width = (point.X < 0) ? overlay_width + point.X :
				overlay_width;
			/*  Clip for overlays that extend below the source image  */
			if ((transfer_width + curr_col) > source_width)
			{	
				transfer_width = (source_width - curr_col);
			}
			/*  Copy overlay  */
			OverlayIterator overlay_ptr = overlay_image [overlay_index];
			if (draw_color_mode)
			{
				for (uint32 j = 0; j < transfer_width; j++)
				{
					G42Color overlay_color = overlay_converter (*overlay_ptr++);
					const G42Color * colorp = &((overlay_color.Green > 240) 
						? draw_color : source_converter (*source_ptr));
					*dest_ptr++ = dest_converter (*colorp);	
					source_ptr++;
					source_tptr++;
					dest_tptr++;
				}
			}
			else
			{
				for (uint32 j = 0; j < transfer_width; j++)
				{
#ifdef TRY_REGION
#ifdef USE_PROFILER
	ProfilerSetStatus (true);
#endif
					if (overlay_desc -> HasRegion())
					{	
#if 1
					 	int16 overlay_part = overlay_desc  -> 
					 		GetRegion().PointInRegionAntialias (G42Point (j, overlay_index));
#else
					 	int16 overlay_part = 255;
					 	
						if (! GlobalRegion -> 
							PointInRegion (G42Point (j, overlay_index))) {
							overlay_part = 0;
						}
#endif	
						if (overlay_part > 128)
							overlay_part++;
						int16 source_part = (int16) (256 - overlay_part);
						
						G42Color source_color = source_converter (*source_ptr);
						G42Color overlay_color = overlay_converter (*overlay_ptr);
						G42Color antialiased_color;				
						if (overlay_part == 0) {
							antialiased_color = source_color;
						}
						else if (overlay_part == 256) {
							antialiased_color = overlay_color;
						}
						else 
						{
							intf red = (overlay_part * overlay_color.Red);
							red += (source_part * source_color.Red);
							red >>= 8;
							intf green = (overlay_part * overlay_color.Green);
							green += (source_part * source_color.Green);
							green >>= 8;
							intf blue = (overlay_part * overlay_color.Blue);
							blue += (source_part * source_color.Blue);
							blue >>= 8;
							antialiased_color = G42Color ((byte) red, (byte) green,
								(byte) blue);
						}
						*dest_ptr++ = dest_converter (antialiased_color);
						
					}
					else {
						*dest_ptr++ = dest_converter (source_converter (*source_ptr));
					}
#ifdef USE_PROFILER
			ProfilerSetStatus (false);
#endif
#else
					*dest_ptr++ = dest_converter (overlay_converter (*overlay_ptr));
#endif TRY_REGION
					source_ptr++;
					source_tptr++;
					overlay_ptr++;
					dest_tptr++;
				}
			}
			/*  Copy right side  */
			for (j = point.X + overlay_width; j < dest_width; j++)
			{
				*dest_ptr++ = dest_converter (source_converter (*source_ptr++));		
				*dest_tptr++ = (bool) *source_tptr++;
			}
			source_index++;
			dest_index++;
			overlay_index++;
		}
		/*  Transfer height is height of area below overlay  */
		transfer_height = source_height - curr_index;
		for (i = 0; i < transfer_height; i++)
		{
			SourceIterator source_ptr  		= source_image [source_index];
			DestIterator dest_ptr 				= dest_image [dest_index];
			/*  Get transparency pointers  */
			SourceTransIterator source_tptr  = source_image.GetTransIterator (source_index);
			DestTransIterator dest_tptr		= dest_image.GetTransIterator (dest_index);
			for (uint32 j = 0; j < dest_width; j++) 
			{
				*dest_ptr++ = dest_converter (source_converter (*source_ptr++));
				*dest_tptr++ = (bool) *source_tptr++;
			}
			source_index++;
			dest_index++;
		}	
	}
	else	//OverlayUnion
	{
	
	}
	#ifdef USE_PROFILER
//	ProfilerSetStatus (false);
	ProfilerDump ("\pTestApp.prof");
	ProfilerClear();
	#endif
}
G42Overlay::G42Overlay (const G42Point & point,
	G42Overlay::G42OverlayType overlay_type,
	G42Overlay::G42TransType trans_type)
: 	DrawColor (G42Color (0, 0, 0)), DrawColorMode (false),
	OverlayType (overlay_type), Point (point), TransType (trans_type)
{
}
G42Overlay::G42Overlay (const G42Point & point, G42Color & draw_color,
	G42OverlayType overlay_type, G42TransType trans_type)
: 	DrawColor (draw_color), DrawColorMode (true), 
	OverlayType (overlay_type), Point (point), TransType (trans_type)
{}
byte 
G42Overlay::GetIndexFromType (G42ImageDescriptor::G42ImageType type)
{
	byte value = G42Color8Index;
	switch (type)
	{
		case G42ImageDescriptor::Color24:
		case G42ImageDescriptor::Color32:
			value = G42Color24Index;
		break;
		case G42ImageDescriptor::Gray8:
			value = G42Gray8Index;
		break;
		case G42ImageDescriptor::Gray1:
			value = G42Gray1Index;
		break;	
	}
	return value;
}
/*  Do processing here  */
void
G42Overlay::HandleEnd (G42PathEnd * end, G42PathComponent * sender)
{
	/*  Process on overlay sender call  */
	if (sender == SecondSender) 
	{
		/*  Setup output descriptor   */
		uint32 width 	= InputDescriptor -> GetWidth();
		uint32 height 	= InputDescriptor -> GetHeight();
		if (DrawColorMode)
		{
			/*  Output Descriptor is always Color24/32  */
			G42ImageDescriptor::G42ImageType type = G42GetImageType (24, true);
			OutputDescriptor = new G42ImageDescriptor (*InputDescriptor, width, 
				height, type);
		}
		else	// Ordinary overlay
		{
			// TODO: This is clip mode.  (Need union mode)
			/*  (Test point and be sure it's in the source image)  */
			/*  Determine output descriptor based on two merged images  */
			byte image_index 	 = GetIndexFromType (InputDescriptor -> GetImageType());
			byte overlay_index = GetIndexFromType (SecondImageDescriptor -> GetImageType());
			byte depth = OutputDepths [image_index] [overlay_index];
			bool is_color = (bool) OutputIsColor [image_index] [overlay_index];
			G42ImageDescriptor::G42ImageType type = G42GetImageType (depth, is_color);
			#if 1
			OutputDescriptor = new G42ImageDescriptor (*InputDescriptor, width, 
				height, type);
			#else
			OutputDescriptor = new G42ImageDescriptor (*InputDescriptor);
			#endif
		}
		CreateOutputData();
		HandleFilter();
		Receiver -> Process (new G42ImageDescriptor (*OutputDescriptor), this);
		SendOutputData();
		Receiver -> Process (end, this);
	}
}
//123456789012345678901234567890123456789012345678901234567890123456789012345678
void
G42Overlay::HandleFilter (void)
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
G42Overlay::HandleColor8 (G42ImageDescriptor::G42ImageType overlay_type)
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
G42Overlay::HandleColor24 (G42ImageDescriptor::G42ImageType overlay_type)
{
#ifdef ALLTYPES
	if (overlay_type == G42ImageDescriptor::Color8)
	{
		/*  Color24 + Color8 promote Color8 to Color24 */
		FilterOverlay (InputDescriptor, InputData, 
			SecondImageDescriptor, SecondImageData, 
			OutputDescriptor, OutputData, 
			G42ImageDataColor24::G42ConvertToG42Color(),
			G42ImageDataColor8::G42ConvertToG42Color 
				(SecondImageDescriptor -> GetPalette()),
			G42ImageDataColor24::G42ConvertFromG42Color(), 
				Point, DrawColor, DrawColorMode, OverlayType, TransType, 
			DC24, DC8, DC24, DTS1, IC24R, IC8R, IC24, ITS1R, ITS1);
	}	
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
G42Overlay::HandleColor32 (G42ImageDescriptor::G42ImageType overlay_type)
{
#ifdef ALLTYPES
	if (overlay_type == G42ImageDescriptor::Color8)
	{
		/*  Color32 + Color8 promote Color8 to Color32 */
		FilterOverlay (InputDescriptor, InputData, 
			SecondImageDescriptor, SecondImageData, 
			OutputDescriptor, OutputData, 
			G42ImageDataColor32::G42ConvertToG42Color(),
			G42ImageDataColor8::G42ConvertToG42Color 
				(SecondImageDescriptor -> GetPalette()),
			G42ImageDataColor32::G42ConvertFromG42Color(), 
			Point, DrawColor, DrawColorMode, OverlayType, TransType, 
			DC32, DC8, DC32, DTS1, IC32R, IC8R, IC32, ITS1R, ITS1);
	}	
	else 
#endif
	if (overlay_type == G42ImageDescriptor::Color32)
	{
	#ifdef NOUSE_PROFILER
	ProfilerSetStatus (true);
	#endif
		FilterOverlay (InputDescriptor, InputData, 
			SecondImageDescriptor, SecondImageData, 
			OutputDescriptor, OutputData, 
			G42ImageDataColor32::G42ConvertToG42Color(), 
			G42ImageDataColor32::G42ConvertToG42Color(), 
			G42ImageDataColor32::G42ConvertFromG42Color(), 
			Point, DrawColor, DrawColorMode, OverlayType, TransType, 
			DC32, DC32, DC32, DTS1, IC32R, IC32R, IC32, ITS1R, ITS1);		
	#ifdef NOUSE_PROFILER
	ProfilerSetStatus (false);
	ProfilerDump ("\pTestApp.prof");
	ProfilerClear();
	#endif
	}
#ifdef ALLTYPES
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
G42Overlay::HandleGray8 (G42ImageDescriptor::G42ImageType overlay_type)
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
	#endif
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
G42Overlay::HandleGray1 (G42ImageDescriptor::G42ImageType overlay_type)
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
G42Overlay::MakeCopy (void) const
{
//	return new G42Overlay (Point, OverlayType, TransType);
}
