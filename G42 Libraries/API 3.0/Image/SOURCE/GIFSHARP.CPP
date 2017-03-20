#include "gifsharp.h"
#include "gilab.h"
/* -------------------------- 
  	Templated Filter Functions 
   -------------------------- */
/*  Most efficient for true color data, but will work for all data types  */
// Unsharp Mask function (LAB)
template <class SourceData, class DestData, 
	class SourceIterator, class DestIterator,
	class SourcePixelConv, class DestPixelConv>
void FilterSharpen (DescriptorRef source_desc, DataRef source_data, 
	DescriptorRef dest_desc, DataRef dest_data, 
	SourcePixelConv source_converter, DestPixelConv dest_converter, 
	int16 sharpness, G42Sharpen::G42SharpenType type, 
	SourceData *, DestData *, SourceIterator *, DestIterator *)
{
	uint32 width 	= dest_desc -> GetWidth();
	uint32 height 	= dest_desc -> GetHeight();
	G42ReadCachedImage <SourceData, SourceIterator> 
		source_image_prev (source_desc, source_data);
	G42ReadCachedImage <SourceData, SourceIterator> 
		source_image_curr (source_desc, source_data);
	G42ReadCachedImage <SourceData, SourceIterator> 
		source_image_next (source_desc, source_data);
	G42WriteCachedImage <DestData, DestIterator> 
		dest_image (dest_desc, dest_data);
	G42CIEColor::SetScreenGamma (1.8);	// 1.0 works well?
	G42LABColor::SetUseTable (true);
	/*  Do nothing on really small images  */
	if (width < 2 || height < 3) {
		return;
	}
	/*  Set up 3 x 3 pixel window indices  */
	uint32 prev_index = 0;
	uint32 curr_index = 1;
	uint32 next_index = 2;
	/*  Send out first line  */
	CopyPixels (source_image_curr [0], dest_image [0], 
		source_converter, dest_converter, width);
	for (uint32 i = 1; i < (height - 1); i++)
	{
		SourceIterator prev_ptr = source_image_prev [prev_index];
		SourceIterator curr_ptr = source_image_curr [curr_index];
		SourceIterator next_ptr = source_image_next [next_index];
		DestIterator dest_ptr = dest_image [i];
		G42LABColor lcolors [3][3];
		lcolors [0][0] = G42LABColor (source_converter (prev_ptr [0]));
		lcolors [0][1] = G42LABColor (source_converter (prev_ptr [1]));
		lcolors [0][2] = G42LABColor (source_converter (prev_ptr [2]));
		lcolors [1][0] = G42LABColor (source_converter (curr_ptr [0]));
		lcolors [1][1] = G42LABColor (source_converter (curr_ptr [1]));
		lcolors [1][2] = G42LABColor (source_converter (curr_ptr [2]));
		lcolors [2][0] = G42LABColor (source_converter (next_ptr [0]));
		lcolors [2][1] = G42LABColor (source_converter (next_ptr [1]));
		lcolors [2][2] = G42LABColor (source_converter (next_ptr [2]));
		/*  Send out first pixel  */
		*dest_ptr++ = dest_converter (source_converter (*curr_ptr++));
		for (uint32 j = 1; j < (width - 1); j++)
		{
			G42LABColor & lcolor = lcolors [1][1];	// (Current color)
			intf l = (intf) lcolor.L;
         /*  Sum the 8 surrounding pixels  */
			intf tmp = (lcolors [0][0].L + lcolors [0][1].L +
         				lcolors [0][2].L + lcolors [1][0].L +
                     lcolors [1][2].L + lcolors [2][0].L +
                     lcolors [2][1].L + lcolors [2][2].L);
			if (type == G42Sharpen::Unsharp_Mask)
			{
				/*  Find average and difference  */
				tmp >>= 3;
				tmp  = (l - tmp);
				tmp += ((tmp < 0) ? -sharpness : sharpness);
				l += tmp;
			}
			else
			{
				/*  Classic highpass  */
	 			intf delta = (l << 3) - tmp;
	 			l += ((delta * sharpness) >> 4);
			}
 			lcolor.L = (byte) max (min ((intf) l, (intf) 255), (intf) 0);
			*dest_ptr++ = dest_converter (G42Color (lcolor));
			/*  Move pixel window over (New prev)  */
	 		lcolors [0][0] = lcolors [0][1];
 			lcolors [0][1] = lcolors [0][2];
  			lcolors [0][2] = source_converter (*prev_ptr++);
			/*  New Current */
	 		lcolors [1][0] = lcolors [1][1];
 			lcolors [1][1] = lcolors [1][2];
  			lcolors [1][2] = source_converter (*curr_ptr++);
			/*  New Next   */
	 		lcolors [2][0] = lcolors [2][1];
 			lcolors [2][1] = lcolors [2][2];
  			lcolors [2][2] = source_converter (*next_ptr++);
		}
		/*  Send out last pixel  	*/
		*dest_ptr++ = dest_converter (source_converter (*curr_ptr++));
		/*  Advance all the indices  */		
		prev_index++;
		curr_index++;
		next_index++;
	}	
	/*  Send out last line  */
	CopyPixels (source_image_curr [height - 1], dest_image [height - 1], 
		source_converter, dest_converter, width);	
}
#if 0	// Uses RGB only, not LAB
template <class SourceData, class DestData, 
	class SourceIterator, class DestIterator,
	class SourcePixelConv, class DestPixelConv>
void FilterUnsharp (DescriptorRef source_desc, DataRef source_data, 
	DescriptorRef dest_desc, DataRef dest_data, 
	SourcePixelConv source_converter, DestPixelConv dest_converter, 
	int16 sharpness, SourceData *, DestData *, 
	SourceIterator *, DestIterator *)
{
	uint32 width 	= dest_desc -> GetWidth();
	uint32 height 	= dest_desc -> GetHeight();
	G42ReadCachedImage <SourceData, SourceIterator> source_image (source_desc, source_data);
	G42WriteCachedImage <DestData, DestIterator> dest_image (dest_desc, dest_data);
	/*  Do nothing on really small images  */
	if (width < 2 || height < 3) {
		return;
	}
	/*  Set up 3 x 3 pixel window indices  */
	uint32 prev_index = 0;
	uint32 curr_index = 1;
	uint32 next_index = 2;
	/*  Send out first line  */
	CopyPixels (source_image [0], dest_image [0], 
		source_converter, dest_converter, width);
	for (uint32 i = 1; i < (height - 1); i++)
	{
		SourceIterator prev_ptr = source_image [prev_index];
		SourceIterator curr_ptr = source_image [curr_index];
		SourceIterator next_ptr = source_image [next_index];
		DestIterator dest_ptr = dest_image [i];
		G42Color colors [3][3];
		colors [0][0] = source_converter (prev_ptr [0]);
		colors [0][1] = source_converter (prev_ptr [1]);
		colors [0][2] = source_converter (prev_ptr [2]);
		colors [1][0] = source_converter (curr_ptr [0]);
		colors [1][1] = source_converter (curr_ptr [1]);
		colors [1][2] = source_converter (curr_ptr [2]);
		colors [2][0] = source_converter (next_ptr [0]);
		colors [2][1] = source_converter (next_ptr [1]);
		colors [2][2] = source_converter (next_ptr [2]);
		/*  Send out first pixel  */
		*dest_ptr++ = dest_converter (source_converter (*curr_ptr++));
		for (uint32 j = 1; j < (width - 1); j++)
		{
			G42Color color = colors [1][1];	// (Current color)
			int16 red 	= (int16) color.Red;
			int16 green = (int16) color.Green;
			int16 blue 	= (int16) color.Blue;
				
         /*  Find average for the 8 surrounding pixels  */
			int16 tmp = (colors [0][0].Red + colors [0][1].Red + colors [0][2].Red);
			tmp += (colors [1][0].Red + colors [1][2].Red);
			tmp += (colors [2][0].Red + colors [2][1].Red + colors [2][2].Red);
			tmp >>= 3;
			tmp  = (red - tmp);
			tmp += ((tmp < 0) ? -sharpness : sharpness);	
			red += tmp;
			red = max (min ((int16) red, (int16) 255), (int16) 0);
		
			tmp = (colors [0][0].Green + colors [0][1].Green + colors [0][2].Green);
			tmp += (colors [1][0].Green + colors [1][2].Green);
			tmp += (colors [2][0].Green + colors [2][1].Green + colors [2][2].Green);
			tmp >>= 3;
			tmp  = (green - tmp);
			tmp += ((tmp < 0) ? -sharpness : sharpness);	
			green += tmp;
			green = max (min ((int16) green, (int16) 255), (int16) 0);
			tmp = (colors [0][0].Blue + colors [0][1].Blue + colors [0][2].Blue);
			tmp += (colors [1][0].Blue + colors [1][2].Blue);
			tmp += (colors [2][0].Blue + colors [2][1].Blue + colors [2][2].Blue);
			tmp >>= 3;
			tmp  = (blue - tmp);
			tmp += ((tmp < 0) ? -sharpness : sharpness);	
			blue += tmp;
			blue = max (min ((int16) blue, (int16) 255), (int16) 0);
			
			*dest_ptr++ = dest_converter (G42Color (red, green, blue));
			
			/*  Move pixel window over (New prev)  */
	 		colors [0][0] = colors [0][1];
 			colors [0][1] = colors [0][2];
  			colors [0][2] = source_converter (*prev_ptr++);
			/*  New Current */
	 		colors [1][0] = colors [1][1];
 			colors [1][1] = colors [1][2];
  			colors [1][2] = source_converter (*curr_ptr++);
			/*  New Next   */
	 		colors [2][0] = colors [2][1];
 			colors [2][1] = colors [2][2];
  			colors [2][2] = source_converter (*next_ptr++);
		}
		/*  Send out last pixel  	*/
		*dest_ptr++ = dest_converter (source_converter (*curr_ptr++));
		/*  Advance all the indices  */		
		prev_index++;
		curr_index++;
		next_index++;
	}	
	/*  Send out last line  */
	CopyPixels (source_image [height - 1], dest_image [height - 1], 
		source_converter, dest_converter, width);	
}
#endif
G42Sharpen::G42Sharpen (G42Sharpen::G42SharpenType type, int16 sharpness)
:
	SharpenType (type), Sharpness (sharpness)
{ 
	if (sharpness < 0 || Sharpness > 100)
	{
		throw (G42InvalidParameterException(0));
	}
}
/*  Sharpen Filter:
	 Color8 promotes to Color24/Color32 
	 Gray1 promotes to Gray 8  */
//123456789012345678901234567890123456789012345678901234567890123456789012345678
void
G42Sharpen::HandleFilter (void)
{
	G42ImageDescriptor::G42ImageType type = InputDescriptor -> GetImageType();
	if (type == G42ImageDescriptor::Color8)
	{
		/*  Color8 promotes to Color24/Color32  */
		if (OutputDescriptor -> GetImageType() == G42ImageDescriptor::Color24)
		{
			FilterSharpen (InputDescriptor, InputData, OutputDescriptor, OutputData, 
				G42ImageDataColor8::G42ConvertToG42Color (InputDescriptor -> GetPalette()),
				G42ImageDataColor24::G42ConvertFromG42Color(),
				Sharpness, SharpenType, DC8, DC24, IC8R, IC24);
		}
		else
		{
			FilterSharpen (InputDescriptor, InputData, OutputDescriptor, OutputData, 
				G42ImageDataColor8::G42ConvertToG42Color (InputDescriptor -> GetPalette()),	
				G42ImageDataColor32::G42ConvertFromG42Color(),
				Sharpness, SharpenType, DC8, DC32, IC8R, IC32);
		} 
	}
	else if (type == G42ImageDescriptor::Color24)
	{	
		FilterSharpen (InputDescriptor, InputData, OutputDescriptor, OutputData, 
			G42ImageDataColor24::G42ConvertToG42Color(),
			G42ImageDataColor24::G42ConvertFromG42Color(),
			Sharpness, SharpenType, DC24, DC24, IC24R, IC24);
	}
	else if (type == G42ImageDescriptor::Color32)
	{		
		FilterSharpen (InputDescriptor, InputData, OutputDescriptor, OutputData, 
			G42ImageDataColor32::G42ConvertToG42Color(),
			G42ImageDataColor32::G42ConvertFromG42Color(),
			Sharpness, SharpenType, DC32, DC32, IC32R, IC32);
	}
	else if (type == G42ImageDescriptor::Gray8)
	{
		FilterSharpen (InputDescriptor, InputData, OutputDescriptor, OutputData, 
			G42ImageDataGray8::G42ConvertToG42Color(), 
			G42ImageDataGray8::G42ConvertFromG42Color(), 
			Sharpness, SharpenType, DG8, DG8, IG8R, IG8);
	}
	else if (type == G42ImageDescriptor::Gray1)
	{
		/*  Gray1 promotes to Gray8  */
	
		FilterSharpen (InputDescriptor, InputData, OutputDescriptor, OutputData, 
			G42ImageDataGray1::G42ConvertToG42Color(),
			G42ImageDataGray8::G42ConvertFromG42Color(),
			Sharpness, SharpenType, DG1, DG8, IG1R, IG8);
	}
	CopyTransparency();
}
G42ImageFilter *
G42Sharpen::MakeCopy (void) const
{
	return new G42Sharpen (SharpenType, Sharpness);
}
void
G42Sharpen::ProcessFrame (void)
{
	/*  Setup output descriptor Make a new descriptor that describes the output data  */
	uint32 width 	= InputDescriptor -> GetWidth();
	uint32 height 	= InputDescriptor -> GetHeight();
	G42ImageDescriptor::G42ImageType type = InputDescriptor -> GetImageType();
	switch (type)
	{
		case G42ImageDescriptor::Color8:
		{
			G42ImageDescriptor::G42ImageType type = G42GetImageType (24, true);
			OutputDescriptor = new G42ImageDescriptor (*InputDescriptor, width, 
				height, type);
		}
		break;
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
	}
	
	CreateOutputData();
	HandleFilter();
	Receiver -> Process (new G42ImageDescriptor (*OutputDescriptor), this);
	SendOutputData();
	/*  Clear everything  */
	InputData.Clear(); 
	InputDescriptor.Clear();
	OutputData.Clear(); 
	OutputDescriptor.Clear();	
}