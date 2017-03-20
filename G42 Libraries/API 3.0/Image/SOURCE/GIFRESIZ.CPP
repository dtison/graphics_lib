#include "gifresiz.h"
#if 0
#ifdef MACOS	// Try out the optimizer
#pragma global_optimizer on
#pragma optimization_level 4
#endif
#endif
/* -------------------------- 
  	Templated Filter Functions 
   -------------------------- */
//  Resize functions
/*  Most efficient for true color data, but will work for all data types  */
// Truncation resize
template <class SourceData, class DestData, 
	class SourceIterator, class DestIterator, 
 	class Pixel>
void FilterStretch (DescriptorRef source_desc, DataRef source_data, 
	DescriptorRef dest_desc, DataRef dest_data, 
	uint32 dest_width, uint32 dest_height, 
	SourceData *, DestData *, 
	SourceIterator *, DestIterator *, Pixel *)
{
	uint32 source_width  = source_desc -> GetWidth();
	uint32 source_height = source_desc -> GetHeight();
	G42ReadCachedImage <SourceData, SourceIterator> 
		source_image (source_desc, source_data);
	G42WriteCachedImage <DestData, DestIterator> 
		dest_image (dest_desc, dest_data);
	/*  Set up calculations  */
	double scale_h = (double) dest_width / (double) source_width;
   double scale_v = (double) dest_height / (double) source_height;
	for (uint32 i = 0; i < dest_height; i++)
   {
		uint32 curr_index = ((double) i / scale_v);
		SourceIterator curr_ptr = source_image [curr_index];
		DestIterator dest_ptr = dest_image [i];
  		for (uint32 j = 0; j < dest_width; j++)
  		{
	    	uint32 source_index = (uint32) ((double) j / scale_h);
			*dest_ptr++ = (Pixel) curr_ptr [source_index];
  		}
   }
}
// Linear averaging resize
// Q:  Can optimize by avoiding (double) typecast on every pixel ?
template <class SourceData, class DestData, 
	class SourceIterator, class DestIterator, 
	class SourcePixelConv, class DestPixelConv>
void FilterAverage (DescriptorRef source_desc, DataRef source_data, 
	DescriptorRef dest_desc, DataRef dest_data, 
	SourcePixelConv source_converter, DestPixelConv dest_converter, 
	uint32 dest_width, uint32 dest_height, 
	SourceData *, DestData *, 
	SourceIterator *, DestIterator *)
{
	uint32 source_width  = source_desc -> GetWidth();
	uint32 source_height = source_desc -> GetHeight();
	G42ReadCachedImage <SourceData, SourceIterator> 
		source_image (source_desc, source_data);
	G42ReadCachedImage <SourceData, SourceIterator> 
		source_image_next (source_desc, source_data);
	G42WriteCachedImage <DestData, DestIterator> 
		dest_image (dest_desc, dest_data);
	/*  Set up calculations  */
	double scale_h = (double) dest_width / (double) source_width;
   double scale_v = (double) dest_height / (double) source_height;
	for (uint32 i = 0; i < dest_height; i++)
   {
		uint32 curr_index = ((double) i / scale_v);
		uint32 next_index =  curr_index + 1;
		/*  Don't index beyond bottom of image  */
		if (curr_index == (source_height - 1)) 
			next_index--;
		SourceIterator curr_ptr = source_image [curr_index];
		SourceIterator next_ptr = source_image_next [next_index];
		DestIterator dest_ptr = dest_image [i];
  		for (uint32 j = 0; j < (dest_width - 1); j++)
  		{
	    	uint32 source_index = (uint32) ((double) j / scale_h);
	    	uint32 nsource_index = source_index + 1;
			if (nsource_index > (source_width - 1)) {
				nsource_index--;
			}
			/*  Find an average for current pixel + 3 surrounding values  */
			G42Color c1 = source_converter (curr_ptr [source_index]);
			G42Color c2 = source_converter (curr_ptr [nsource_index]);
			G42Color c3 = source_converter (next_ptr [source_index]);
			G42Color c4 = source_converter (next_ptr [nsource_index]);
 			byte red = (byte) (((intf) c1.Red + (intf) c2.Red +
 				(intf) c3.Red + (intf) c4.Red) >> 2);
 			byte green = (byte) (((intf) c1.Green + (intf) c2.Green +
 				(intf) c3.Green + (intf) c4.Green) >> 2);
 			byte blue = (byte) (((intf) c1.Blue + (intf) c2.Blue +
 				(intf) c3.Blue + (intf) c4.Blue) >> 2);
			*dest_ptr++ = dest_converter (G42Color (red, green, blue));
 		}
  		/*  Take care of last pixel  */
	  	uint32 source_index = (uint32) ((double) (dest_width - 1) / scale_h);
		*dest_ptr++ = dest_converter (source_converter (curr_ptr [source_index]));
   }
}
// Interpolation Filter
template <class SourceData, class DestData, 
	class SourceIterator, class DestIterator, 
	class SourcePixelConv, class DestPixelConv>
void FilterInterpolate (DescriptorRef source_desc, DataRef source_data, 
	DescriptorRef dest_desc, DataRef dest_data, 
	SourcePixelConv source_converter, DestPixelConv dest_converter, 
	uint32 dest_width, uint32 dest_height, 
	SourceData *, DestData *, 
	SourceIterator *, DestIterator *)
{
	uint32 source_width  = source_desc -> GetWidth();
	uint32 source_height = source_desc -> GetHeight();
	G42ReadCachedImage <SourceData, SourceIterator> 
		source_image (source_desc, source_data);
	G42ReadCachedImage <SourceData, SourceIterator> 
		source_image_next (source_desc, source_data);
	G42WriteCachedImage <DestData, DestIterator> 
		dest_image (dest_desc, dest_data);
	/*  Set up calculations  */
	double scale_h = (double) dest_width / (double) source_width;
   double scale_v = (double) dest_height / (double) source_height;
	for (uint32 i = 0; i < dest_height; i++)
   {
		double row_addr = ((double) i / scale_v);
		uint32 curr_row_num = (uint32) row_addr;
		uint32 next_row_num =  curr_row_num + 1;
		/*  Don't index beyond bottom of image  */
		if (curr_row_num == (source_height - 1)) 
			next_row_num--;
		double row_delta = row_addr - (double) curr_row_num;
		/*  Grab pointers  */		
		SourceIterator curr_ptr = source_image [curr_row_num];
		SourceIterator next_ptr = source_image_next [next_row_num];
		DestIterator dest_ptr = dest_image [i];
		/*  Do a row  */
  		for (uint32 j = 0; j < (dest_width - 1); j++)
  		{			
			double col_addr = ((double) j / scale_h);
			uint32 curr_col_num = (uint32) col_addr;
			double col_delta = col_addr - (double) curr_col_num;
	    	uint32 next_col_num = curr_col_num + 1;
			if (next_col_num > (source_width - 1)) {
				next_col_num--;
			}
			/*  Interpolate for current pixel + 3 surrounding values  */
			G42Color colors [4];
			colors [0] = source_converter (curr_ptr [curr_col_num]);
			colors [1] = source_converter (curr_ptr [next_col_num]);
			colors [2] = source_converter (next_ptr [curr_col_num]);
			colors [3] = source_converter (next_ptr [next_col_num]);
 			*dest_ptr++ = dest_converter (Interpolate4Colors (colors, 
 				row_delta, col_delta));
  		}
  		/*  Take care of last pixel  */
	  	uint32 source_index = (uint32) ((double) (dest_width - 1) / scale_h);
		*dest_ptr++ = dest_converter (source_converter (curr_ptr [source_index]));
   }
}
G42Resize::G42Resize (uint32 new_width, uint32 new_height, 
	G42Resize::G42ResizeType type)
:	NewWidth (new_width), NewHeight (new_height), ResizeType (type)
{
	if (NewWidth == 0 || NewHeight == 0)	// TODO:  Is there a maximum to resize to?
	{
		throw (G42InvalidParameterException (0));
	}
}
/*  Resize Filter: 
	 Color8 promotes to Color24/Color32 
	 Gray1 promotes to Gray8  */
//123456789012345678901234567890123456789012345678901234567890123456789012345678
void
G42Resize::HandleFilter (void)
{
	G42ImageDescriptor::G42ImageType type = InputDescriptor -> GetImageType();
	if (type == G42ImageDescriptor::Color8)
	{
		if (ResizeType == Truncate)
		{
			FilterStretch (InputDescriptor, InputData, OutputDescriptor, OutputData, 
				NewWidth, NewHeight, DC8, DC8, IC8R, IC8, PB8);
		}
		else if (ResizeType == Average)
		{
			/*  Color8 promotes to Color24/Color32  */
			if (OutputDescriptor -> GetImageType() == G42ImageDescriptor::Color24)
			{
				FilterAverage (InputDescriptor, InputData, 
					OutputDescriptor, OutputData, 
					G42ImageDataColor8::G42ConvertToG42Color 
						(InputDescriptor -> GetPalette()),
					G42ImageDataColor24::G42ConvertFromG42Color(), 
					NewWidth, NewHeight, DC8, DC24, IC8R, IC24);
			}
			else 
			{
				FilterAverage (InputDescriptor, InputData, 
					OutputDescriptor, OutputData, 
					G42ImageDataColor8::G42ConvertToG42Color 
						(InputDescriptor -> GetPalette()),
					G42ImageDataColor32::G42ConvertFromG42Color(), 
					NewWidth, NewHeight, DC8, DC32, IC8R, IC32);
			} 
		}
		else
		{
			/*  Color8 promotes to Color24/Color32  */
			if (OutputDescriptor -> GetImageType() == G42ImageDescriptor::Color24)
			{
				FilterInterpolate (InputDescriptor, InputData, 
					OutputDescriptor, OutputData, 
					G42ImageDataColor8::G42ConvertToG42Color 
						(InputDescriptor -> GetPalette()),
					G42ImageDataColor24::G42ConvertFromG42Color(), 
					NewWidth, NewHeight, DC8, DC24, IC8R, IC24);
			}
			else 
			{
				FilterInterpolate (InputDescriptor, InputData, 
					OutputDescriptor, OutputData, 
					G42ImageDataColor8::G42ConvertToG42Color 
						(InputDescriptor -> GetPalette()),
					G42ImageDataColor32::G42ConvertFromG42Color(), 
					NewWidth, NewHeight, DC8, DC32, IC8R, IC32);
			} 
		}
	}
	else if (type == G42ImageDescriptor::Color24)
	{
		if (ResizeType == Truncate)
		{
			FilterStretch (InputDescriptor, InputData, OutputDescriptor, OutputData, 
				NewWidth, NewHeight, DC24, DC24, IC24R, IC24, PC24R);
		}
		else if (ResizeType == Average)
		{
			FilterAverage (InputDescriptor, InputData, 
				OutputDescriptor, OutputData, 
				G42ImageDataColor24::G42ConvertToG42Color(), 
				G42ImageDataColor24::G42ConvertFromG42Color(), 
				NewWidth, NewHeight, DC24, DC24, IC24R, IC24);
		}
		else
		{
			FilterInterpolate (InputDescriptor, InputData, 
				OutputDescriptor, OutputData, 
				G42ImageDataColor24::G42ConvertToG42Color(), 
				G42ImageDataColor24::G42ConvertFromG42Color(), 
				NewWidth, NewHeight, DC24, DC24, IC24R, IC24);
		}
	}
	else if (type == G42ImageDescriptor::Color32)
	{
		if (ResizeType == Truncate)
		{
			FilterStretch (InputDescriptor, InputData, OutputDescriptor, OutputData, 
				NewWidth, NewHeight, DC32, DC32, IC32R, IC32, PC32R);
 
		}
		else if (ResizeType == Average)
		{
			FilterAverage (InputDescriptor, InputData, 
				OutputDescriptor, OutputData, 
				G42ImageDataColor32::G42ConvertToG42Color(),
				G42ImageDataColor32::G42ConvertFromG42Color(), 
				NewWidth, NewHeight, DC32, DC32, IC32R, IC32);
		}
		else
		{
			FilterInterpolate (InputDescriptor, InputData, 
				OutputDescriptor, OutputData, 
				G42ImageDataColor32::G42ConvertToG42Color(),
				G42ImageDataColor32::G42ConvertFromG42Color(), 
				NewWidth, NewHeight, DC32, DC32, IC32R, IC32);
		}
	}
	else if (type == G42ImageDescriptor::Gray8)
	{
		if (ResizeType == Truncate)
		{
			FilterStretch (InputDescriptor, InputData, OutputDescriptor, OutputData, 
				NewWidth, NewHeight, DG8, DG8, IG8R, IG8, PB8);
		}
		else if (ResizeType == Average)
		{
			FilterAverage (InputDescriptor, InputData, 
				OutputDescriptor, OutputData, 
				G42ImageDataGray8::G42ConvertToG42Color(),
				G42ImageDataGray8::G42ConvertFromG42Color(), 
				NewWidth, NewHeight, DG8, DG8, IG8R, IG8);
		}
		else
		{
			FilterInterpolate (InputDescriptor, InputData, 
				OutputDescriptor, OutputData, 
				G42ImageDataGray8::G42ConvertToG42Color(),
				G42ImageDataGray8::G42ConvertFromG42Color(), 
				NewWidth, NewHeight, DG8, DG8, IG8R, IG8);
		}
		
	}
	else if (type == G42ImageDescriptor::Gray1)
	{
		if (ResizeType == Truncate)	
		{
			FilterStretch (InputDescriptor, InputData, OutputDescriptor, OutputData, 
				NewWidth, NewHeight, DG1, DG1, IG1R, IG1, PB8);
		}
		else if (ResizeType == Average)
		{		
			/*  Gray1 promotes to Gray8  */
			FilterAverage (InputDescriptor, InputData, 
				OutputDescriptor, OutputData, 
				G42ImageDataGray1::G42ConvertToG42Color(),
				G42ImageDataGray8::G42ConvertFromG42Color(),
					NewWidth, NewHeight, DG1, DG8, IG1R, IG8);
		}
		else
		{		
			/*  Gray1 promotes to Gray8  */
			FilterInterpolate (InputDescriptor, InputData, 
				OutputDescriptor, OutputData, 
				G42ImageDataGray1::G42ConvertToG42Color(),
				G42ImageDataGray8::G42ConvertFromG42Color(),
					NewWidth, NewHeight, DG1, DG8, IG1R, IG8);
		}
	}
	/*  Handle transparency if applicable  */
	if (OutputDescriptor -> IsTransparent())
	{
		if (ResizeType == Truncate)	
		{
			FilterStretch (InputDescriptor, InputData, OutputDescriptor, OutputData, 
				NewWidth, NewHeight, DT1, DT1, IT1R, IT1, PB8);		
		}
		else if (ResizeType == Average)
		{
			FilterAverage (InputDescriptor, InputData, OutputDescriptor, OutputData, 
				G42ImageDataGray1::G42ConvertToG42Color(),
				G42ImageDataGray1::G42ConvertFromG42Color(),
				NewWidth, NewHeight, DT1, DT1, IT1R, IT1);	
		}
		else
		{
			FilterInterpolate (InputDescriptor, InputData, OutputDescriptor, OutputData, 
				G42ImageDataGray1::G42ConvertToG42Color(),
				G42ImageDataGray1::G42ConvertFromG42Color(),
				NewWidth, NewHeight, DT1, DT1, IT1R, IT1);	
		}
	}
}
G42ImageFilter *
G42Resize::MakeCopy (void) const
{
	return new G42Resize (NewWidth, NewHeight);
}
void
G42Resize::ProcessFrame (void)
{
	/*  Make a new descriptor that describes the output data  */
	uint32 width 	= NewWidth;
	uint32 height 	= NewHeight;
	G42ImageDescriptor::G42ImageType type = InputDescriptor -> GetImageType();
	if (ResizeType == Truncate)
	{
		OutputDescriptor = new G42ImageDescriptor (*InputDescriptor, 
			width, height);
	}
	else
	{
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
	}
	CreateOutputData();
	HandleFilter();
	Receiver -> Process (new G42ImageDescriptor (*OutputDescriptor), this);
	SendOutputData();
	/*  Clear everything  */
	ClearDataAndDescriptor (true);
}
