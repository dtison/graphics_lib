#include "gifdithr.h"
#include "gidesc.h"
#include "gidata.h"
#include "gtmpmem.h"
/*
	1. Gray8 and Gray1 to Color8
	2. Convert to Gray 8 (a. power of 2  b. "192 levels" table = i * 191 / 255) 
	3. Convert Gray1
	4. Convert to true color	
*/
/*  Undefine this if you don't want dither to zig-zag */
#define USE_SERPENTINE_RASTER
/* -------------------------- 
  	Templated Filter Functions 
   -------------------------- */
template <class SourceData, class DestData, 
	class SourceIterator, class DestIterator,
	class SourcePixelConv>
void FilterDither (DescriptorRef source_desc, DataRef source_data, 
	DescriptorRef dest_desc, DataRef dest_data, 
	SourcePixelConv source_converter,
	const G42OptimizedPalette * opt_palette, bool match,
	SourceData *, DestData *, SourceIterator *, DestIterator *)
{
	uint32 width 	= dest_desc -> GetWidth();
	uint32 height 	= dest_desc -> GetHeight();
	G42ReadCachedImage <SourceData, SourceIterator> source_image (source_desc, source_data);
	G42WriteCachedImage <DestData, DestIterator> dest_image (dest_desc, dest_data);
	const G42Color * org_palette = opt_palette -> GetPalette();
	bool is_forward = true;
	if (match)	// No error diffusion
	{
		for (uint32 i = 0; i < height; i++)
		{
			SourceIterator source_ptr = source_image [i];
			DestIterator dest_ptr = dest_image [i];
			for (uint32 j = 0; j < width; j++)
			{
				G42Color color = source_converter(*source_ptr++);
				byte val = (byte)(opt_palette->GetPixel(color));
				*dest_ptr++ = val;
			}
		}
	}
	else
	{
		G42DitherError red_error (width);
		G42DitherError green_error (width);
		G42DitherError blue_error (width);
		for (uint32 i = 0; i < height; i++)
		{
			SourceIterator source_ptr = source_image [i];
			DestIterator dest_ptr = dest_image [i];
			intf offset = 1;	// Iterator advancer
			if (! is_forward)
			{
				source_ptr += width - 1;
				dest_ptr += width - 1;
				offset = -1;
			}
			for (uint32 j = 0; j < width; j++)
			{
				G42Color c(source_converter(*source_ptr));
				intf red = c.Red + red_error.GetError();
				intf green = c.Green  + green_error.GetError();
				intf blue = c.Blue + blue_error.GetError();
				/*  Slightly optimized clipping w/CodeWarrior brackets  		  */
				/*  (For some reason here using the min/max templates failed) */
				if (red < 0) {
					red = 0;
				}
				else if (red > 255) {
					red = 255;
				}
				if (green < 0) {
					green = 0;
				}
				else if (green > 255) {
					green = 255;
				}
				if (blue < 0) {
					blue = 0;
				}
				else if (blue > 255) {
					blue = 255;
				}
				byte val = (byte)(opt_palette->GetPixel(G42Color(
					(byte)red, (byte)green, (byte)blue)));
				red_error.SetError(red - org_palette[val].Red);
				green_error.SetError(green - org_palette[val].Green);
				blue_error.SetError(blue - org_palette[val].Blue);
				*dest_ptr = val;
				/*  Advance iterator pointers  */
				source_ptr += offset;
				dest_ptr += offset;
			}
			#ifdef USE_SERPENTINE_RASTER
			/*  Toggle direction  */
			if (is_forward)
			{
				red_error.SetDirection (G42DitherError::Reverse);
				green_error.SetDirection (G42DitherError::Reverse);
				blue_error.SetDirection (G42DitherError::Reverse);
			}
			else
			{
				red_error.SetDirection (G42DitherError::Forward);
				green_error.SetDirection (G42DitherError::Forward);
				blue_error.SetDirection (G42DitherError::Forward);
			}
			is_forward = ! is_forward;
			#endif
			red_error.NextRow();
			green_error.NextRow();
			blue_error.NextRow();		
		}
	}
}
G42Dither::G42Dither (const G42OptimizedPalette * dither_pal, 
	G42Dither::G42DitherType type, bool match)
: DitherType (type), Match (match), DitherPalette (dither_pal)
{ }
G42Dither::~G42Dither()
{ }
void
G42Dither::HandleData (G42ImageData * strip, G42PathComponent *)
{
	SetInputData (strip);
}
void
G42Dither::HandleDescriptor (G42ImageDescriptor * desc, G42PathComponent *)
{
	SetInputDescriptor (desc);
}
/*  Do processing here  */
void
G42Dither::HandleEnd (G42PathEnd * end, G42PathComponent *)
{
	/*  Setup output descriptor Make a new descriptor that describes the output data  */
	uint32 width 	= InputDescriptor -> GetWidth();
	uint32 height 	= InputDescriptor -> GetHeight();
	G42ImageDescriptor::G42ImageType type = InputDescriptor -> GetImageType();
	switch (type)
	{
		case G42ImageDescriptor::Color24:
		case G42ImageDescriptor::Color32:
		case G42ImageDescriptor::Color8:
//		case G42ImageDescriptor::Gray8:	// Need this
//		case G42ImageDescriptor::Gray1:	// Need this (maybe)
		{
			G42ImageDescriptor::G42ImageType type = G42GetImageType (8, true);
			OutputDescriptor = new G42ImageDescriptor (*InputDescriptor, width, 
				height, type);
			uint16 num_colors = DitherPalette -> GetNumPalette();
			const G42Color * opt_palette = DitherPalette -> GetPalette();
			OutputDescriptor -> SetNumPalette (num_colors);
			G42Color * palette = OutputDescriptor -> GetPalette();
			for (uint16 i = 0; i < num_colors; i++)
			{
				palette [i] = opt_palette [i]; 
			}
		}
		break;
		#if 0
		case G42ImageDescriptor::Gray1:
		{
			G42ImageDescriptor::G42ImageType type = G42GetImageType (8, false);
			OutputDescriptor = new G42ImageDescriptor (*InputDescriptor, width, 
				height, type);
		}
		break;
		default:
			OutputDescriptor = new G42ImageDescriptor (*InputDescriptor, width, height);
		break;
		#endif
	}
	
	CreateOutputData();
	HandleFilter();
	Receiver -> Process (new G42ImageDescriptor (*OutputDescriptor), this);
	SendOutputData();
	Receiver -> Process (end, this);
}
/*  Dither Filter:
	 Color24/32 converts to Color8 
	 Gray8 converts to ...  */
//123456789012345678901234567890123456789012345678901234567890123456789012345678
void
G42Dither::HandleFilter (void)
{
	G42ImageDescriptor::G42ImageType type = InputDescriptor -> GetImageType();
	if (type == G42ImageDescriptor::Color8)
	{		
		/*  Color8 re-dithers to Color8  */
		FilterDither (InputDescriptor, InputData, OutputDescriptor, OutputData, 
			G42ImageDataColor8::G42ConvertToG42Color (InputDescriptor ->
			GetPalette()), DitherPalette, Match, DC8, DC8, IC8R, IC8);
	}
	else if (type == G42ImageDescriptor::Color24)
	{
		/*  Color24 converts to Color8  */
			FilterDither (InputDescriptor, InputData, OutputDescriptor, OutputData, 
				G42ImageDataColor24::G42ConvertToG42Color (),
				DitherPalette, Match, DC24, DC8, IC24R, IC8);
	}
	else if (type == G42ImageDescriptor::Color32)
	{
		/*  Color32 converts to Color8  */
			FilterDither (InputDescriptor, InputData, OutputDescriptor, OutputData, 
				G42ImageDataColor32::G42ConvertToG42Color (),
				DitherPalette, Match, DC32, DC8, IC32R, IC8);
	}
	#if 0
	else if (type == G42ImageDescriptor::Gray8)
	{
		FilterDither (InputDescriptor, InputData, OutputDescriptor, OutputData, 
			G42ImageDataGray8::G42ConvertToG42Color(), 
			G42ImageDataGray8::G42ConvertFromG42Color(), 
			Sharpness, DitherType, DG8, DG8, IG8R, IG8);
	}
	else if (type == G42ImageDescriptor::Gray1)
	{
		/*  Gray1 promotes to Gray8  */
	
		FilterDither (InputDescriptor, InputData, OutputDescriptor, OutputData, 
			G42ImageDataGray1::G42ConvertToG42Color(),
			G42ImageDataGray8::G42ConvertFromG42Color(),
			Sharpness, DitherType, DG1, DG8, IG1R, IG8);
	}
	#endif
	CopyTransparency();
}
G42ImageFilter *
G42Dither::MakeCopy (void) const
{
	return new G42Dither (DitherPalette, DitherType, Match);
}
