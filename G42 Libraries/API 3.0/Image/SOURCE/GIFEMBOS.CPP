#include "gifembos.h"
int16 table[19][3] = {{512, 0, 0}, {486, 0, 26},
							{451, 1, 60}, {408, 4, 100},
							{357, 10, 145}, {301, 19, 192},
							{241, 30, 241}, {180, 45, 287},
							{128, 64, 320}, {88, 88, 336},
							{64, 128, 320}, {45, 180, 287},
							{30, 241, 241}, {19, 301, 192},
							{10, 357, 145}, {4, 408, 100},
							{1, 451, 60}, {0, 486, 26},
							{0, 512, 0}};
/* -------------------------- 
  	Templated Filter Functions 
   -------------------------- */
//  Emboss function
/*  Most efficient for true color data, but will work for all data types  */
template <class SourceData, class DestData, 
	class SourceIterator, class DestIterator, 
	class SourcePixelConv, class DestPixelConv>
void FilterEmboss (DescriptorRef source_desc, DataRef source_data,
	DescriptorRef dest_desc, DataRef dest_data,
	SourcePixelConv source_converter, DestPixelConv dest_converter,
	int16 light_angle, SourceData *, DestData *,
	SourceIterator *, DestIterator *)
{
	uint32 width 	= source_desc -> GetWidth();
	uint32 height 	= dest_desc -> GetHeight();
	G42ReadCachedImage <SourceData, SourceIterator> 
		source_image (source_desc, source_data);
	G42ReadCachedImage <SourceData, SourceIterator> 
		source_image_next (source_desc, source_data);
	G42WriteCachedImage <DestData, DestIterator> 
		dest_image (dest_desc, dest_data);
 	/*  Intf removes Borland significant digits warnings  */
	intf angle = max (min ((intf) light_angle, (intf) 90), (intf) 0);
	angle = (angle + 3) / 5;
	/*  Do nothing on really small images  */
	if (width < 2 || height < 3) {
		return;
	}
	/*  Set up 2 x 2 pixel window indices  */
	uint32 curr_index = 0;
	uint32 next_index = 1;
	uint32 i;	// Watcom-pacifier
	for (i = 0; i < (height - 1); i++)
	{
		SourceIterator curr_ptr = source_image [curr_index];
		SourceIterator next_ptr = source_image_next [next_index];
		DestIterator dest_ptr = dest_image [i];
		byte grays [2][2];
		grays [0][0] = source_converter (curr_ptr [0]);
		grays [0][1] = source_converter (curr_ptr [1]);
		grays [1][0] = source_converter (next_ptr [0]);
		grays [1][1] = source_converter (next_ptr [1]);
		/*  Send out first pixel  */
		*dest_ptr++ = dest_converter (source_converter (*curr_ptr++));
		for (uint32 j = 1; j < width; j++)
		{
			int32 grad = 0x80 +
				(grays [0][0] - (((table[angle][0]) * 
					grays [0][1] + (table[angle][1]) * 
					grays [1][0] + (table[angle][2]) * 
					grays [1][1]) >> 9));
			grad = max (min ((int32) grad, (int32) 0xFF), (int32) 0);
			*dest_ptr++ = dest_converter ((byte) grad);
			/*  Move pixel window over  */
	 		grays [0][0] = grays [0][1];
  			grays [0][1] = source_converter (*curr_ptr++);
	 		grays [1][0] = grays [1][1];
  			grays [1][1] = source_converter (*next_ptr++);
		}
		/*  Send out last pixel  	*/
		*dest_ptr++ = dest_converter (source_converter (*curr_ptr++));
		/*  Advance all the indices  */		
		curr_index++;
		next_index++;
	}	
	/*  Send out last line  */
	#ifdef DOESNT_WORK
	CopyPixels (source_image [height - 1], dest_image [height - 1], 
		source_converter, dest_converter, width);	
	#endif
/*  Q:  WHY can't we use the function above for Color8?  */
	SourceIterator source_ptr = source_image [height - 1];
	DestIterator dest_ptr = dest_image [height - 1];
	for (i = 0; i < width; i++) 
	{
		*dest_ptr++ = dest_converter (source_converter (*source_ptr++));
	}
#if 0	// A test..
	CopyPixels (source_ptr, dest_ptr, 
		source_converter, dest_converter, width);	
#endif
}
G42Emboss::G42Emboss (int16 emboss_angle)
:
	EmbossAngle (emboss_angle)
{}
/*  Emboss Filter: 
	 Color24/32/8 becomes Gray8 
	 Gray1 promotes to Gray8 ? */
//123456789012345678901234567890123456789012345678901234567890123456789012345678
void
G42Emboss::HandleFilter (void)
{
	G42ImageDescriptor::G42ImageType type = InputDescriptor -> GetImageType();
	/*  All cases convert to Gray8 */
	#if 1
	if (type == G42ImageDescriptor::Color8)
	{
		FilterEmboss (InputDescriptor, InputData, 
			OutputDescriptor, OutputData, 
			G42ImageDataColor8::G42ConvertToGray8 
				(InputDescriptor -> GetPalette(), 256),
			G42ImageDataGray8::G42ConvertFromGray8(), 
			EmbossAngle, DC8, DG8, IC8R, IG8);
	}
	else 
	#endif
	if (type == G42ImageDescriptor::Color24)
	{
		FilterEmboss (InputDescriptor, InputData, OutputDescriptor, OutputData, 
			G42ImageDataColor24::G42ConvertToGray8(),
			G42ImageDataGray8::G42ConvertFromGray8(), 
			EmbossAngle, DC24, DG8, IC24R, IG8);
	}
	else if (type == G42ImageDescriptor::Color32)
	{
		FilterEmboss (InputDescriptor, InputData, OutputDescriptor, OutputData, 
			G42ImageDataColor32::G42ConvertToGray8(),
			G42ImageDataGray8::G42ConvertFromGray8(), 
			EmbossAngle, DC32, DG8, IC32R, IG8);
	}
	else if (type == G42ImageDescriptor::Gray8)
	{
		FilterEmboss (InputDescriptor, InputData, 
			OutputDescriptor, OutputData, 
			G42ImageDataGray8::G42ConvertToGray8(),
			G42ImageDataGray8::G42ConvertFromGray8(), 
			EmbossAngle, DG8, DG8, IG8R, IG8);
	}
	else if (type == G42ImageDescriptor::Gray1)
	{	
		FilterEmboss (InputDescriptor, InputData, 
			OutputDescriptor, OutputData, 
			G42ImageDataGray1::G42ConvertToGray8(),
			G42ImageDataGray8::G42ConvertFromGray8(),
			EmbossAngle, DG1, DG8, IG1R, IG8);
	}
	/*  Copy transparency over  */
	CopyTransparency();	
}
G42ImageFilter *
G42Emboss::MakeCopy (void) const
{
	return new G42Emboss (EmbossAngle);
}
/*  Do processing here  */
void
G42Emboss::ProcessFrame (void)
{
	/*  Make a new descriptor that describes the output data  */
	uint32 width 	= InputDescriptor -> GetWidth();
	uint32 height 	= InputDescriptor -> GetHeight();
	/*  All cases convert to Gray8  */
	G42ImageDescriptor::G42ImageType type = G42GetImageType (8, false);
	OutputDescriptor = new G42ImageDescriptor (*InputDescriptor, width, 
		height, type);
	
	CreateOutputData();
	HandleFilter();
	Receiver -> Process (new G42ImageDescriptor (*OutputDescriptor), this);
	SendOutputData();
	/*  Clear everything  */
	ClearDataAndDescriptor (true);
}
