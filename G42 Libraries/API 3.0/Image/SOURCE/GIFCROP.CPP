#include "gifcrop.h"
// TODO:  Why does Rect GetWidth() return value 1 too high? 
// (Look at CropRect.GetWidth() - 1 in ProcessFrame)
/* -------------------------- 
  	Templated Filter Functions 
   -------------------------- */
// Crop function
template <class SourceData, class DestData, class TransData,
	class SourceIterator, class DestIterator,
	class SourceTransIterator, class DestTransIterator,
	class Pixel>
void FilterCrop (DescriptorRef source_desc, DataRef source_data, 
	DescriptorRef dest_desc, DataRef dest_data, 
	G42Rect & crop_rect, SourceData *, DestData *, TransData *,
	SourceIterator *, DestIterator *, 
	SourceTransIterator *, DestTransIterator *,
	Pixel *)
{
	uint32 width 	= dest_desc -> GetWidth();
	uint32 height 	= dest_desc -> GetHeight();
	G42TransReadCachedImage <SourceData, SourceIterator, TransData,
		SourceTransIterator> source_image (source_desc, source_data);
	G42TransWriteCachedImage <DestData, DestIterator, TransData, 
		DestTransIterator> dest_image (dest_desc, dest_data);
	int32 dest_row = 0;
	for (uint32 i = crop_rect.Top; i < crop_rect.Bottom; i++)
	{
		SourceIterator source_ptr = source_image [i];
		SourceTransIterator source_tptr = source_image.GetTransIterator (i);
		DestIterator dest_ptr = dest_image [dest_row];
		DestTransIterator dest_tptr = dest_image.GetTransIterator (dest_row);
		source_ptr += crop_rect.Left;
		int32 dest_col= 0;
		for (uint32 j = crop_rect.Left; j < crop_rect.Right; j++)		
		{
			*dest_ptr++ = (Pixel) *source_ptr++;
			/*  Handle transparency */
			bool trans = (bool) *source_tptr++;
			if (source_desc -> HasRegion()) {	
				trans = source_desc -> 
					GetRegion().PointInRegion (G42Point (dest_col, dest_row));
			}
			*dest_tptr++ = trans;
			dest_col++;
		}
		dest_row++;
	}	
}
G42Crop::G42Crop (const G42Rect & crop_rect)
: CropRect (crop_rect)
{ 
	if (CropRect.GetWidth() <= 0 || CropRect.GetHeight() <= 0) {
		throw G42InvalidParameterException();
	}
}
/*  Filter  */
//123456789012345678901234567890123456789012345678901234567890123456789012345678
void
G42Crop::HandleFilter (void)
{
	G42ImageDescriptor::G42ImageType type = InputDescriptor -> GetImageType();
	/*  Nobody promotes or converts */
	if (type == G42ImageDescriptor::Color8)
	{
		FilterCrop (InputDescriptor, InputData, OutputDescriptor, OutputData, 
			CropRect, DC8, DC8, DTS1, IC8R, IC8, ITS1R, ITS1, PB8);
	}
	else if (type == G42ImageDescriptor::Color24)
	{	
		FilterCrop (InputDescriptor, InputData, OutputDescriptor, OutputData, 
			CropRect, DC24, DC24, DTS1, IC24R, IC24, ITS1R, ITS1, PC24R);
	}
	else if (type == G42ImageDescriptor::Color32)
	{		
		FilterCrop (InputDescriptor, InputData, OutputDescriptor, OutputData, 
			CropRect, DC32, DC32, DTS1, IC32R, IC32, ITS1R, ITS1, PC32R);
	}
	else if (type == G42ImageDescriptor::Gray8)
	{
		FilterCrop (InputDescriptor, InputData, OutputDescriptor, OutputData, 
			CropRect, DG8, DG8, DTS1, IG8R, IG8, ITS1R, ITS1, PB8);
	}
	else if (type == G42ImageDescriptor::Gray1)
	{	
		FilterCrop (InputDescriptor, InputData, OutputDescriptor, OutputData, 
			CropRect, DG1, DG1, DTS1, IG1R, IG1, ITS1R, ITS1, PB8);
	}
}
G42ImageFilter *
G42Crop::MakeCopy (void) const
{
	return new G42Crop (CropRect);
}
void
G42Crop::ProcessFrame (void)
{
	/*  Be sure crop rect is within image  */
	CropRect &= G42Rect (0, 0, InputDescriptor -> GetWidth() - 1, 
		InputDescriptor -> GetHeight() - 1);
	if (CropRect.GetWidth() <= 0 || CropRect.GetHeight() <= 0) {
		throw G42CropOutsideImageException();
	}
	/*  Setup output descriptor Make a new descriptor that describes the output data  */
	uint32 width 	= CropRect.GetWidth() - 1;
	uint32 height 	= CropRect.GetHeight() - 1;
	OutputDescriptor = new G42ImageDescriptor (*InputDescriptor, width, height);
	/*  Turn on transparency for regions  */
	OutputDescriptor -> SetTransparency (true);
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