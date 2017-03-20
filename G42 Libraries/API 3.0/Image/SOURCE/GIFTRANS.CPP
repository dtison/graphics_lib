
#include "giftrans.h"
#include "gidesc.h"
#include "gidata.h"
#include "gilab.h"
/* --------------------------
	Templated Filter Functions
	-------------------------- */
template <class SourceData, class DestData, class DestTransData,
	class SourceIterator, class DestIterator, class DestTransIterator,
	class SourcePixelConv>
void FilterTransparency (DescriptorRef source_desc, DataRef source_data,
	DescriptorRef dest_desc, DataRef dest_data,
	SourcePixelConv source_converter,
	G42Color & trans_color,
	SourceData *, DestData *, DestTransData *,
	SourceIterator *, DestIterator *, DestTransIterator *)
{
	uint32 width 	= dest_desc -> GetWidth();
	uint32 height 	= dest_desc -> GetHeight();
	G42ReadCachedImage <SourceData, SourceIterator> source_image
		(source_desc, source_data);
	G42TransWriteCachedImage <DestData, DestIterator,
		DestTransData, DestTransIterator> dest_image (dest_desc, dest_data);
	for (uint32 i = 0; i < height; i++)
	{
		SourceIterator source_ptr = source_image [i];
		DestIterator dest_ptr = dest_image [i];
		DestTransIterator trans_ptr = dest_image.GetTransIterator(i);
		for (uint32 j = 0; j < width; j++)
		{
			*trans_ptr++ = !(bool)(trans_color == source_converter(*source_ptr));
			*dest_ptr++ = source_ptr++;
		}
	}
}
template <class SourceData, class DestData,
	class SourceIterator, class DestIterator>
void FilterRemoveTransparency (DescriptorRef source_desc, DataRef source_data,
	DescriptorRef dest_desc, DataRef dest_data,
	SourceData *, DestData *, SourceIterator *, DestIterator *)
{
	uint32 width 	= dest_desc -> GetWidth();
	uint32 height 	= dest_desc -> GetHeight();
	G42ReadCachedImage <SourceData, SourceIterator>
		source_image (source_desc, source_data);
	G42WriteCachedImage <DestData, DestIterator>
		dest_image (dest_desc, dest_data);
	for (uint32 i = 0; i < height; i++)
	{
		SourceIterator source_ptr = source_image [i];
		DestIterator dest_ptr = dest_image [i];
		for (uint32 j = 0; j < width; j++)
		{
			*dest_ptr++ = source_ptr++;
		}
	}
}
G42Transparency::G42Transparency (const G42Color & color)
:
	TransparentColor(color), Transparent(true)
{
}
G42Transparency::G42Transparency (void)
:
	TransparentColor(), Transparent(false)
{
}
G42Transparency::~G42Transparency()
{ }
void
G42Transparency::HandleData (G42ImageData * strip, G42PathComponent *)
{
	SetInputData (strip);
}
void
G42Transparency::HandleDescriptor (G42ImageDescriptor * desc, G42PathComponent *)
{
	if (HaveFirstDescriptor) 
	{
		/*  It's time to run the filter on the frame */
		ProcessFrame();
	}
	else 
	{
		HaveFirstDescriptor = true;
	}
	SetInputDescriptor (desc);
}
/*  Do processing here  */
void
G42Transparency::HandleEnd (G42PathEnd * end, G42PathComponent *)
{
	ProcessFrame();
	Receiver->Process(end, this);
}
void
G42Transparency::ProcessFrame (void)
{
	if (Transparent == InputDescriptor->IsTransparent())
	{
		OutputDescriptor = new G42ImageDescriptor(*InputDescriptor);
	}
	else
	{
		OutputDescriptor = new G42ImageDescriptor(InputDescriptor->GetWidth(),
			InputDescriptor->GetHeight(), InputDescriptor->GetImageType(),
			InputDescriptor->GetHeap(), Transparent);
		if (InputDescriptor->IsPaletted())
		{
			OutputDescriptor->SetNumPalette(InputDescriptor->GetNumPalette());
			const G42Color * input_palette = InputDescriptor->GetPalette();
			G42Color * output_palette = OutputDescriptor->GetPalette();
			for (uintf i = 0; i < InputDescriptor->GetNumPalette(); i++)
			{
				output_palette[i] = input_palette[i];
			}
		}
	}
	if (Transparent)
	{
		OutputDescriptor->SetTransparency(true);
		OutputDescriptor->SetTransparentColor(TransparentColor);
	}
	else
	{
		OutputDescriptor->SetTransparency(false);
	}
	CreateOutputData();
	HandleFilter();
	Receiver->Process(new G42ImageDescriptor (*OutputDescriptor), this);
	SendOutputData();
	/*  Clear everything  */
	InputData.Clear(); 
	InputDescriptor.Clear();
	OutputData.Clear(); 
	OutputDescriptor.Clear();	
}
/*  Transparency Filter: */
void
G42Transparency::HandleFilter (void)
{
	G42ImageDescriptor::G42ImageType type = InputDescriptor -> GetImageType();
	if (type == G42ImageDescriptor::Gray1)
	{
		if (Transparent)
		{
			FilterTransparency(InputDescriptor, InputData, OutputDescriptor,
				OutputData, G42ImageDataGray1::G42ConvertToG42Color(),
				TransparentColor, DG1, DG1, DTS1, IG1R, IG1, ITS1);
		}
		else
		{
			FilterRemoveTransparency(InputDescriptor, InputData, OutputDescriptor,
				OutputData, DG1, DG1, IG1R, IG1);
		}
	}
	else if (type == G42ImageDescriptor::Gray8)
	{
		if (Transparent)
		{
			FilterTransparency(InputDescriptor, InputData, OutputDescriptor,
				OutputData, G42ImageDataGray8::G42ConvertToG42Color(),
				TransparentColor, DG8, DG8, DTS1, IG8R, IG8, ITS1);
		}
		else
		{
			FilterRemoveTransparency(InputDescriptor, InputData, OutputDescriptor,
				OutputData, DG8, DG8, IG8R, IG8);
		}
	}
	else if (type == G42ImageDescriptor::Color8)
	{
		if (Transparent)
		{
			FilterTransparency(InputDescriptor, InputData, OutputDescriptor,
				OutputData, G42ImageDataColor8::G42ConvertToG42Color(
					InputDescriptor->GetPalette()),
				TransparentColor, DC8, DC8, DTS1, IC8R, IC8, ITS1);
		}
		else
		{
			FilterRemoveTransparency(InputDescriptor, InputData, OutputDescriptor,
				OutputData, DC8, DC8, IC8R, IC8);
		}
	}
	else if (type == G42ImageDescriptor::Color24)
	{
		if (Transparent)
		{
			FilterTransparency(InputDescriptor, InputData, OutputDescriptor,
				OutputData, G42ImageDataColor24::G42ConvertToG42Color(),
				TransparentColor, DC24, DC24, DTS1, IC24R, IC24, ITS1);
		}
		else
		{
			FilterRemoveTransparency(InputDescriptor, InputData, OutputDescriptor,
				OutputData, DC24, DC24, IC24R, IC24);
		}
	}
	else if (type == G42ImageDescriptor::Color32)
	{
		if (Transparent)
		{
			FilterTransparency(InputDescriptor, InputData, OutputDescriptor,
				OutputData, G42ImageDataColor32::G42ConvertToG42Color(),
				TransparentColor, DC32, DC32, DTS1, IC32R, IC32, ITS1);
		}
		else
		{
			FilterRemoveTransparency(InputDescriptor, InputData, OutputDescriptor,
				OutputData, DC32, DC32, IC32R, IC32);
		}
	}
}
G42ImageFilter *
G42Transparency::MakeCopy (void) const
{
	if (Transparent)
	{
		return new G42Transparency(TransparentColor);
	}
	return new G42Transparency();
}
