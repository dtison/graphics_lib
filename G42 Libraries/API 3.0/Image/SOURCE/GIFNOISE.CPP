#include "gifnoise.h"
#include "gilab.h"
/*  A helper structure for sorting elements  */
struct RemoveNoiseElement
{
	byte				L;
	G42LABColor * lcolor;	
};
/* -------------------------- 
  	Templated Filter Functions 
   -------------------------- */
//  RemoveNoise function
/*  Most efficient for true color data, but will work for all data types  */
template <class SourceData, class DestData, 
	class SourceIterator, class DestIterator, 
	class SourcePixelConv, class DestPixelConv>
void FilterRemoveNoise (DescriptorRef source_desc, DataRef source_data, 
	DescriptorRef dest_desc, DataRef dest_data, 
	SourcePixelConv source_converter, DestPixelConv dest_converter,  
	int16 value, SourceData *, DestData *, 
	SourceIterator *, DestIterator *)
{
	uint32 width 	= source_desc -> GetWidth();
	uint32 height 	= dest_desc -> GetHeight();
	G42ReadCachedImage <SourceData, SourceIterator> 
		source_image_prev (source_desc, source_data);
	G42ReadCachedImage <SourceData, SourceIterator> 
		source_image_curr (source_desc, source_data);
	G42ReadCachedImage <SourceData, SourceIterator> 
		source_image_next (source_desc, source_data);
	G42WriteCachedImage <DestData, DestIterator> 
		dest_image (dest_desc, dest_data);
	G42CIEColor::SetScreenGamma (1.8);	// 1.0 works well for noise?
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
			RemoveNoiseElement elements [9];
			/*  This looks strange, but a prettier way hasn't been found yet  */
			elements [0].L = lcolors [0][0].L;	
			elements [0].lcolor = &lcolors [0][0];
			elements [1].L = lcolors [0][1].L;		
			elements [1].lcolor = &lcolors [0][1];
			elements [2].L = lcolors [0][2].L;		
			elements [2].lcolor = &lcolors [0][2];
			elements [3].L = lcolors [1][0].L;		
			elements [3].lcolor = &lcolors [1][0];
			elements [4].L = lcolors [1][1].L;		
			elements [4].lcolor = &lcolors [1][1];
			elements [5].L = lcolors [1][2].L;		
			elements [5].lcolor = &lcolors [1][2];
			elements [6].L = lcolors [2][0].L;		
			elements [6].lcolor = &lcolors [2][0];
			elements [7].L = lcolors [2][1].L;		
			elements [7].lcolor = &lcolors [2][1];
			elements [8].L = lcolors [2][2].L;		
			elements [8].lcolor = &lcolors [2][2];
			/*  Sort the elements  */
			RemoveNoiseElement & v = elements [0];
			for (int8 i = 1; i < 9; i++)
			{
				v = elements [i];
				int8 j = i;
				while (elements [j - 1].L > v.L && j > 0)
				{         
					elements [j] = elements [j - 1];
					j--;
				}
				elements [j] = v;
			}
			G42LABColor & curr_color 	= lcolors [1][0]; 
			G42LABColor & sorted_color = *elements [4].lcolor; 
       	/*  Intf removes Borland significant digits warnings  */
			intf difference = (intf) ((intf) curr_color.L -
         	(intf) sorted_color.L);
			if (difference < 0) {
				difference *= -1;
			}
			G42LABColor * pcolor = (difference > value) ? &sorted_color : &curr_color;
			*dest_ptr++ = dest_converter (G42Color (*pcolor));			
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
G42RemoveNoise::G42RemoveNoise (int16 value)
:
	RemoveNoiseValue (value)
{}
/*  RemoveNoise Filter: 
	 Color8 promotes to Color24/Color32 ?
	 Gray1 promotes to Gray8 ? */
//123456789012345678901234567890123456789012345678901234567890123456789012345678
void
G42RemoveNoise::HandleFilter (void)
{
	G42ImageDescriptor::G42ImageType type = InputDescriptor -> GetImageType();
	if (type == G42ImageDescriptor::Color8)
	{
		/*  Color8 promotes to Color24/Color32  */
		if (OutputDescriptor -> GetImageType() == G42ImageDescriptor::Color24)
		{
			FilterRemoveNoise (InputDescriptor, InputData, 
				OutputDescriptor, OutputData, 
				G42ImageDataColor8::G42ConvertToG42Color 
					(InputDescriptor -> GetPalette()),
				G42ImageDataColor24::G42ConvertFromG42Color(), 
				RemoveNoiseValue, DC8, DC24, IC8R, IC24);
		}
		else 
		{
			FilterRemoveNoise (InputDescriptor, InputData, 
				OutputDescriptor, OutputData, 
				G42ImageDataColor8::G42ConvertToG42Color 
					(InputDescriptor -> GetPalette()),
				G42ImageDataColor32::G42ConvertFromG42Color(), 
				RemoveNoiseValue, DC8, DC32, IC8R, IC32);
		} 
	}
	else if (type == G42ImageDescriptor::Color24)
	{
		FilterRemoveNoise (InputDescriptor, InputData, OutputDescriptor, OutputData, 
			G42ImageDataColor24::G42ConvertToG42Color(),
			G42ImageDataColor24::G42ConvertFromG42Color(), 
			RemoveNoiseValue, DC24, DC24, IC24R, IC24);
	}
	else if (type == G42ImageDescriptor::Color32)
	{
		FilterRemoveNoise (InputDescriptor, InputData, OutputDescriptor, OutputData, 
			G42ImageDataColor32::G42ConvertToG42Color(),
			G42ImageDataColor32::G42ConvertFromG42Color(), 
			RemoveNoiseValue, DC32, DC32, IC32R, IC32);
	}
	else if (type == G42ImageDescriptor::Gray8)
	{
		FilterRemoveNoise (InputDescriptor, InputData, 
			OutputDescriptor, OutputData, 
			G42ImageDataGray8::G42ConvertToG42Color(),
			G42ImageDataGray8::G42ConvertFromG42Color(), 
			RemoveNoiseValue, DG8, DG8, IG8R, IG8);
	}
	else if (type == G42ImageDescriptor::Gray1)
	{	
		/*  Gray1 promotes to Gray8  */
		FilterRemoveNoise (InputDescriptor, InputData, 
			OutputDescriptor, OutputData, 
			G42ImageDataGray1::G42ConvertToG42Color(),
			G42ImageDataGray8::G42ConvertFromG42Color(),
			RemoveNoiseValue, DG1, DG8, IG1R, IG8);
	}
	/*  Copy transparency over  */
	CopyTransparency();	
}
G42ImageFilter *
G42RemoveNoise::MakeCopy (void) const
{
	return new G42RemoveNoise (RemoveNoiseValue);
}
void
G42RemoveNoise::ProcessFrame (void)
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