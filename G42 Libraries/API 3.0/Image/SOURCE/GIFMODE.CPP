#include "gifmode.h"
#include "gtmpmem.h"
/*  Undefine this if you don't want dither to zig-zag */
#define USE_SERPENTINE_RASTER
/* -------------------------- 
  	Templated Filter Functions 
   -------------------------- */
template <class SourceData, class DestData, 
	class SourceIterator, class DestIterator,
	class SourcePixelConv>
void FilterColor8 (DescriptorRef source_desc, DataRef source_data, 
	DescriptorRef dest_desc, DataRef dest_data, 
	SourcePixelConv source_converter,
	const G42OptimizedPalette * dither_palette, 
	G42ImageMode::G42ImageModeType output_mode,
	SourceData *, DestData *, SourceIterator *, DestIterator *)
{
	uint32 width 	= dest_desc -> GetWidth();
	uint32 height 	= dest_desc -> GetHeight();
	G42ReadCachedImage <SourceData, SourceIterator> source_image (source_desc, source_data);
	G42WriteCachedImage <DestData, DestIterator> dest_image (dest_desc, dest_data);
	const G42Color * org_palette = dither_palette -> GetPalette();
	if (output_mode == G42ImageMode::MatchColor8)	// No error diffusion
	{
		for (uint32 i = 0; i < height; i++)
		{
			SourceIterator source_ptr = source_image [i];
			DestIterator dest_ptr = dest_image [i];
			for (uint32 j = 0; j < width; j++)
			{
				G42Color color = source_converter(*source_ptr++);
				byte val = (byte)(dither_palette->GetPixel(color));
				*dest_ptr++ = val;
			}
		}
	}
	else
	{
		bool is_forward = true;
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
				byte val = (byte)(dither_palette->GetPixel(G42Color(
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
template <class SourceData, class DestData, 
	class SourceIterator, class DestIterator,
	class SourcePixelConv>
void FilterGray8 (DescriptorRef source_desc, DataRef source_data, 
	DescriptorRef dest_desc, DataRef dest_data, 
	SourcePixelConv source_converter, int16 output_levels,
	G42ImageMode::G42ImageModeType output_mode,
	SourceData *, DestData *, SourceIterator *, DestIterator *)
{
	uint32 width 	= dest_desc -> GetWidth();
	uint32 height 	= dest_desc -> GetHeight();
	G42ReadCachedImage <SourceData, SourceIterator> source_image (source_desc, source_data);
	G42WriteCachedImage <DestData, DestIterator> dest_image (dest_desc, dest_data);
	/*  Create a lut for output levels */
	G42TempMem <byte> table (256);
	intf levels = ((intf) output_levels - (intf) 1);
	for (int16 i = 0; i < 256; i++)
	{
		/*  Scale the levels into a Gray8  */
		table [i] = (byte) (((levels * (intf) i) / (intf) 255) * 
			((intf) 255 / levels));
	}
	if (output_mode == G42ImageMode::MatchGray8)	// No error diffusion
	{
		for (uint32 i = 0; i < height; i++)
		{
			SourceIterator source_ptr = source_image [i];
			DestIterator dest_ptr = dest_image [i];
			for (uint32 j = 0; j < width; j++)
			{
				*dest_ptr++ = table [(byte) source_converter (*source_ptr++)];
			}
		}
	}
	else
	{
		bool is_forward = true;
		G42DitherError error (width);
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
				intf gray = source_converter (*source_ptr) + error.GetError();
				/*  Slightly optimized clipping w/CodeWarrior brackets  		  */
				/*  (For some reason here using the min/max templates failed) */
				if (gray < 0) {
					gray = 0;
				}
				else if (gray > 255) {
					gray = 255;
				}
					
				byte val = table [gray];
				error.SetError ((intf) gray - (intf) val);
	
				*dest_ptr = val;
				/*  Advance iterator pointers  */
				source_ptr += offset;
				dest_ptr += offset;
			}
			#ifdef USE_SERPENTINE_RASTER
			/*  Toggle direction  */
			if (is_forward)
			{
				error.SetDirection (G42DitherError::Reverse);
			}
			else
			{
				error.SetDirection (G42DitherError::Forward);
			}
			is_forward = ! is_forward;
			#endif
			error.NextRow();
		}
	}
}
template <class SourceData, class DestData, 
	class SourceIterator, class DestIterator,
	class SourcePixelConv, class DestPixelConv>
void FilterRGBColor (DescriptorRef source_desc, DataRef source_data, 
	DescriptorRef dest_desc, DataRef dest_data, 
	SourcePixelConv source_converter, 
	DestPixelConv dest_converter, 
	SourceData *, DestData *, SourceIterator *, DestIterator *)
{
	uint32 width 	= dest_desc -> GetWidth();
	uint32 height 	= dest_desc -> GetHeight();
	G42ReadCachedImage <SourceData, SourceIterator> source_image (source_desc, source_data);
	G42WriteCachedImage <DestData, DestIterator> dest_image (dest_desc, dest_data);
	for (uint32 i = 0; i < height; i++)
	{
		SourceIterator source_ptr = source_image [i];
		DestIterator dest_ptr = dest_image [i];
		for (uint32 j = 0; j < width; j++)
		{
			*dest_ptr++ = dest_converter (source_converter (*source_ptr++));
		}
	}
}
template <class SourceData, class DestData, 
	class SourceIterator, class DestIterator,
	class SourcePixelConv>
void FilterGray1 (DescriptorRef source_desc, DataRef source_data, 
	DescriptorRef dest_desc, DataRef dest_data, 
	SourcePixelConv source_converter, 
	int16 threshold, G42ImageMode::G42ImageModeType output_mode,
	SourceData *, DestData *, SourceIterator *, DestIterator *)
{
	uint32 width 	= dest_desc -> GetWidth();
	uint32 height 	= dest_desc -> GetHeight();
	G42ReadCachedImage <SourceData, SourceIterator> source_image (source_desc, source_data);
	G42WriteCachedImage <DestData, DestIterator> dest_image (dest_desc, dest_data);
	if (output_mode == G42ImageMode::MatchGray1)	// No error diffusion
	{
		for (uint32 i = 0; i < height; i++)
		{
			SourceIterator source_ptr = source_image [i];
			DestIterator dest_ptr = dest_image [i];
			for (uint32 j = 0; j < width; j++)
			{
				*dest_ptr++ = (source_converter (*source_ptr++)) > 
					threshold ? true : false;
			}
		}
	}
	else
	{
		bool is_forward = true;
		G42DitherError error (width);
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
				intf gray = source_converter (*source_ptr) + error.GetError();
				/*  Slightly optimized clipping w/CodeWarrior brackets  		  */
				/*  (For some reason here using the min/max templates failed) */
				if (gray < 0) {
					gray = 0;
				}
				else if (gray > 255) {
					gray = 255;
				}
					
				byte val = (byte) ((gray > threshold) ? (intf) 255 : (intf) 0);
				error.SetError ((intf) gray - (intf) val);
	
				*dest_ptr = val == 255 ? true : false;
				/*  Advance iterator pointers  */
				source_ptr += offset;
				dest_ptr += offset;
			}
			#ifdef USE_SERPENTINE_RASTER
			/*  Toggle direction  */
			if (is_forward)
			{
				error.SetDirection (G42DitherError::Reverse);
			}
			else
			{
				error.SetDirection (G42DitherError::Forward);
			}
			is_forward = ! is_forward;
			#endif
			error.NextRow();
		}
	}
}
G42ImageMode::G42ImageMode (G42ImageModeType output_mode,
	const G42OptimizedPalette * dither_pal)
:  DitherPalette (dither_pal), OutputLevels (0), OutputMode (output_mode), 
   Threshold (0)
{ }
G42ImageMode::G42ImageMode (G42ImageModeType output_mode,
	int16 output_levels, int16 threshold)
: DitherPalette (0), OutputLevels (output_levels), OutputMode (output_mode),  
  Threshold (threshold)
{
	if (OutputLevels < 2 || OutputLevels > 255) {
		throw (G42InvalidParameterException (0));
	}
}
/*  Dither Filter:
	 Color24/32 converts to Color8 
	 Gray8 converts to ...  */
//123456789012345678901234567890123456789012345678901234567890123456789012345678
void
G42ImageMode::HandleFilter (void)
{
	G42ImageDescriptor::G42ImageType type = InputDescriptor -> GetImageType();
	switch (OutputMode)
	{
		case RGBColor:
		{
			/*  Convert to Color24/32  */
			if (type == G42ImageDescriptor::Color8)
			{		
				if (OutputDescriptor -> GetImageType() == G42ImageDescriptor::Color24)
				{
					FilterRGBColor (InputDescriptor, InputData, 
						OutputDescriptor, OutputData, 
						G42ImageDataColor8::G42ConvertToG42Color (InputDescriptor ->
						GetPalette()),
						G42ImageDataColor24::G42ConvertFromG42Color(),
						DC8, DC24, IC8R, IC24);
				}
				else
				{
					FilterRGBColor (InputDescriptor, InputData, 
						OutputDescriptor, OutputData, 
						G42ImageDataColor8::G42ConvertToG42Color (InputDescriptor ->
						GetPalette()),
						G42ImageDataColor32::G42ConvertFromG42Color(),
						DC8, DC32, IC8R, IC32);
				}
			}
			else if (type == G42ImageDescriptor::Color24)
			{
				FilterRGBColor (InputDescriptor, InputData, 
					OutputDescriptor, OutputData, 
					G42ImageDataColor24::G42ConvertToG42Color(),
					G42ImageDataColor24::G42ConvertFromG42Color(),
					DC24, DC24, IC24R, IC24);
			}
			else if (type == G42ImageDescriptor::Color32)
			{
				FilterRGBColor (InputDescriptor, InputData, 
					OutputDescriptor, OutputData, 
					G42ImageDataColor32::G42ConvertToG42Color(),
					G42ImageDataColor32::G42ConvertFromG42Color(),
					DC32, DC32, IC32R, IC32);
			}
			else if (type == G42ImageDescriptor::Gray8)
			{
				if (OutputDescriptor -> GetImageType() == G42ImageDescriptor::Color24)
				{
					FilterRGBColor (InputDescriptor, InputData, 
						OutputDescriptor, OutputData, 
						G42ImageDataGray8::G42ConvertToG42Color(),
						G42ImageDataColor24::G42ConvertFromG42Color(),
						DG8, DC24, IG8R, IC24);
				}
				else
				{
					FilterRGBColor (InputDescriptor, InputData, 
						OutputDescriptor, OutputData, 
						G42ImageDataGray8::G42ConvertToG42Color(),
						G42ImageDataColor32::G42ConvertFromG42Color(),
						DG8, DC32, IG8R, IC32);
				}
			}
			else if (type == G42ImageDescriptor::Gray1)
			{
				if (OutputDescriptor -> GetImageType() == G42ImageDescriptor::Color24)
				{
					FilterRGBColor (InputDescriptor, InputData, 
						OutputDescriptor, OutputData, 
						G42ImageDataGray1::G42ConvertToG42Color(),
						G42ImageDataColor24::G42ConvertFromG42Color(),
						DG1, DC24, IG1R, IC24);
				}
				else
				{
					FilterRGBColor (InputDescriptor, InputData, 
						OutputDescriptor, OutputData, 
						G42ImageDataGray1::G42ConvertToG42Color(),
						G42ImageDataColor32::G42ConvertFromG42Color(),
						DG1, DC32, IG1R, IC32);
				}
			}
		}		
		break;
		case DitherColor8:
		case MatchColor8:
		{
			/*  Convert to Color8  */
			if (type == G42ImageDescriptor::Color8)
			{		
				FilterColor8 (InputDescriptor, InputData, 
					OutputDescriptor, OutputData, 
					G42ImageDataColor8::G42ConvertToG42Color (InputDescriptor ->
					GetPalette()),
					DitherPalette, OutputMode, DC8, DC8, IC8R, IC8);
			}
			else if (type == G42ImageDescriptor::Color24)
			{
				FilterColor8 (InputDescriptor, InputData, 
					OutputDescriptor, OutputData, 
					G42ImageDataColor24::G42ConvertToG42Color (),
					DitherPalette, OutputMode, DC24, DC8, IC24R, IC8);
			}
			else if (type == G42ImageDescriptor::Color32)
			{
				FilterColor8 (InputDescriptor, InputData, 
					OutputDescriptor, OutputData, 
					G42ImageDataColor32::G42ConvertToG42Color (),
					DitherPalette, OutputMode, DC32, DC8, IC32R, IC8);
			}
			else if (type == G42ImageDescriptor::Gray8)
			{
				FilterColor8 (InputDescriptor, InputData, 
					OutputDescriptor, OutputData, 
					G42ImageDataGray8::G42ConvertToG42Color(), 
					DitherPalette, OutputMode, DG8, DC8, IG8R, IC8);
			}
			else if (type == G42ImageDescriptor::Gray1)
			{
				FilterColor8 (InputDescriptor, InputData, 
					OutputDescriptor, OutputData, 
					G42ImageDataGray1::G42ConvertToG42Color(),
					DitherPalette, OutputMode, DG1, DC8, IG1R, IC8);
			}		
		}
		break;
		case DitherGray8:
		case MatchGray8:
		{
			/*  Convert to Gray8  */
			if (type == G42ImageDescriptor::Color8)
			{		
				FilterGray8 (InputDescriptor, InputData, 
					OutputDescriptor, OutputData, 
					G42ImageDataColor8::G42ConvertToGray8 
					(InputDescriptor -> GetPalette(), 256),
 					OutputLevels, OutputMode, DC8, DG8, IC8R, IG8);
			}
			else if (type == G42ImageDescriptor::Color24)
			{
				FilterGray8 (InputDescriptor, InputData, 
					OutputDescriptor, OutputData, 
					G42ImageDataColor24::G42ConvertToGray8(),
					OutputLevels, OutputMode, DC24, DG8, IC24R, IG8);
			}
			else if (type == G42ImageDescriptor::Color32)
			{
				FilterGray8 (InputDescriptor, InputData, 
					OutputDescriptor, OutputData, 
					G42ImageDataColor32::G42ConvertToGray8(),
					OutputLevels, OutputMode, DC32, DG8, IC32R, IG8);
			}
			else if (type == G42ImageDescriptor::Gray8)
			{
				FilterGray8 (InputDescriptor, InputData, 
					OutputDescriptor, OutputData, 
					G42ImageDataGray8::G42ConvertToGray8(), 
					OutputLevels, OutputMode, DG8, DG8, IG8R, IG8);
			}
			else if (type == G42ImageDescriptor::Gray1)
			{
				FilterGray8 (InputDescriptor, InputData, 
					OutputDescriptor, OutputData, 
					G42ImageDataGray1::G42ConvertToGray8(),
					OutputLevels, OutputMode, DG1, DG8, IG1R, IG8);
			}		
		}
		break;		
		case DitherGray1:
		case MatchGray1:
		{
			/*  Convert to Gray1 */
			if (type == G42ImageDescriptor::Color8)
			{		
				FilterGray1 (InputDescriptor, InputData, 
					OutputDescriptor, OutputData, 
					G42ImageDataColor8::G42ConvertToGray8 (InputDescriptor ->
					GetPalette(), 256),
					Threshold, OutputMode, DC8, DG1, IC8R, IG1);
			}
			else if (type == G42ImageDescriptor::Color24)
			{
				FilterGray1 (InputDescriptor, InputData, 
					OutputDescriptor, OutputData, 
					G42ImageDataColor24::G42ConvertToGray8(),
					Threshold, OutputMode, DC24, DG1, IC24R, IG1);
			}
			else if (type == G42ImageDescriptor::Color32)
			{
				FilterGray1 (InputDescriptor, InputData, 
					OutputDescriptor, OutputData, 
					G42ImageDataColor32::G42ConvertToGray8(),
					Threshold, OutputMode, DC32, DG1, IC32R, IG1);
			}
			else if (type == G42ImageDescriptor::Gray8)
			{
				FilterGray1 (InputDescriptor, InputData, 
					OutputDescriptor, OutputData, 
					G42ImageDataGray8::G42ConvertToGray8(), 
					Threshold, OutputMode, DG8, DG1, IG8R, IG1);
			}
			else if (type == G42ImageDescriptor::Gray1)
			{
				FilterGray1 (InputDescriptor, InputData, 
					OutputDescriptor, OutputData, 
					G42ImageDataGray1::G42ConvertToGray8(),
					Threshold, OutputMode, DG1, DG1, IG1R, IG1);
			}		
		}
	}
	CopyTransparency();
}
G42ImageFilter *
G42ImageMode::MakeCopy (void) const
{
	G42ImageFilter * filter;
	switch (OutputMode)
	{	
		case G42ImageMode::DitherColor8:
		case G42ImageMode::MatchColor8:
			filter = new G42ImageMode (OutputMode, DitherPalette);
		break;
		case G42ImageMode::RGBColor:
		case G42ImageMode::DitherGray8:
		case G42ImageMode::MatchGray8:
		case G42ImageMode::DitherGray1:
		case G42ImageMode::MatchGray1:
			filter = new G42ImageMode (OutputMode, OutputLevels, Threshold);
		break;
	}
	return filter;
}
void
G42ImageMode::ProcessFrame (void)
{
	/*  Setup output descriptor Make a new descriptor that describes the output data  */
	uint32 width 	= InputDescriptor -> GetWidth();
	uint32 height 	= InputDescriptor -> GetHeight();
	switch (OutputMode)
	{
		case RGBColor:
		{
			/*  Output image is Color24/32  */
			G42ImageDescriptor::G42ImageType type = G42GetImageType (24, true);
			OutputDescriptor = new G42ImageDescriptor (*InputDescriptor, width, 
				height, type);
		}
		break;
		case DitherColor8:
		case MatchColor8:
		{
			/*  Output image is Color8  */
			G42ImageDescriptor::G42ImageType type = G42GetImageType (8, true);
			OutputDescriptor = new G42ImageDescriptor (*InputDescriptor, width, 
				height, type);
			uint16 num_colors = (uint16) DitherPalette -> GetNumPalette();
			const G42Color * opt_palette = DitherPalette -> GetPalette();
			OutputDescriptor -> SetNumPalette (num_colors);
			G42Color * palette = OutputDescriptor -> GetPalette();
			for (uint16 i = 0; i < num_colors; i++)
			{
				palette [i] = opt_palette [i]; 
			}
		}
		break;
		case DitherGray8:
		case MatchGray8:
		{
			/*  Output image is Gray8  */
			G42ImageDescriptor::G42ImageType type = G42GetImageType (8, false);
			OutputDescriptor = new G42ImageDescriptor (*InputDescriptor, width, 
				height, type);
		}
		break;
		case DitherGray1:
		case MatchGray1:
		{
			/*  Output image is Gray1  */
			G42ImageDescriptor::G42ImageType type = G42GetImageType (1, false);
			OutputDescriptor = new G42ImageDescriptor (*InputDescriptor, width, 
				height, type);
		}
		break;
	}
	CreateOutputData();
	HandleFilter();
	Receiver -> Process (new G42ImageDescriptor (*OutputDescriptor), this);
	SendOutputData();
	/*  Clear everything  */
	ClearDataAndDescriptor (true);
}