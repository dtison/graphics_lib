#include "gifrotat.h"
#include "gilab.h"
#define ALLTYPES
// #define USE_TRANSPARENCY
/* -------------------------- 
  	Templated Filter Functions 
   -------------------------- */
// Rotate 90
template <class SourceData, class DestData, 
class SourceIterator, class DestIterator, class Pixel>
void Filter90 (DescriptorRef source_desc, DataRef source_data, 
	DescriptorRef dest_desc, DataRef dest_data,  
	SourceData *, DestData *, SourceIterator *, DestIterator *, Pixel *)
{
	uint32 width 	= dest_desc -> GetWidth();
	uint32 height 	= dest_desc -> GetHeight();
	G42ReadCachedImage <SourceData, SourceIterator> 
		source_image (source_desc, source_data);
	G42WriteCachedImage <DestData, DestIterator> 
		dest_image (dest_desc, dest_data);
	for (uint32 i = 0; i < height; i++)
	{
		DestIterator dest_ptr = dest_image [i];
		dest_ptr += (width - 1);
		for (uint32 j = 0; j < width; j++)
		{
			SourceIterator source_ptr = source_image [j];	// (Source y)
			source_ptr += i;									// (Source x)
			*dest_ptr-- = (Pixel) *source_ptr;
		}	
	}
}
// Rotate 180
template <class SourceData, class DestData, 
class SourceIterator, class DestIterator, class Pixel>
void Filter180 (DescriptorRef source_desc, DataRef source_data, 
	DescriptorRef dest_desc, DataRef dest_data,
	SourceData *, DestData *, SourceIterator *, DestIterator *, Pixel *)
{
	uint32 width 	= dest_desc -> GetWidth();
	uint32 height 	= dest_desc -> GetHeight();
	G42ReadCachedImage <SourceData, SourceIterator> 
		source_image (source_desc, source_data);
	G42WriteCachedImage <DestData, DestIterator> 
		dest_image (dest_desc, dest_data);
	for (uint32 i = 0; i < height; i++)
	{
		DestIterator dest_ptr = dest_image [height - i - 1];
		dest_ptr += (width - 1);
		SourceIterator source_ptr = source_image [i];	// (Source y)
		for (uint32 j = 0; j < width; j++)
		{
			*dest_ptr-- = (Pixel) *source_ptr++;
		}	
	}
}
// Rotate 270 function
template <class SourceData, class DestData, 
class SourceIterator, class DestIterator, class Pixel>
void Filter270 (DescriptorRef source_desc, DataRef source_data, 
	DescriptorRef dest_desc, DataRef dest_data, 
	SourceData *, DestData *, SourceIterator *, DestIterator *, Pixel *)
{
	uint32 width 	= dest_desc -> GetWidth();
	uint32 height 	= dest_desc -> GetHeight();
	G42ReadCachedImage <SourceData, SourceIterator> 
		source_image (source_desc, source_data);
	G42WriteCachedImage <DestData, DestIterator> 
		dest_image (dest_desc, dest_data);
	for (uint32 i = 0; i < height; i++)
	{
		DestIterator dest_ptr = dest_image [i];
		for (uint32 j = 0; j < width; j++)
		{
			SourceIterator source_ptr = source_image [j];	// (Source y)
			source_ptr += i;											// (Source x)
			*dest_ptr++ = (Pixel) *source_ptr;
		}	
	}
}
// Rotate angle function (Also rotates transparency channel) 
template <class SourceData, class DestData, class TransData,
class SourceIterator, class DestIterator, 
class SourceTransIterator, class DestTransIterator,
class SourcePixelConv, class DestPixelConv>
void FilterAngle (DescriptorRef source_desc, DataRef source_data, 
	DescriptorRef dest_desc, DataRef dest_data, 
	SourcePixelConv source_converter, DestPixelConv dest_converter, 
	G42Rotation * rotation, G42Rotate::G42RotateType type, 
	const G42Color & background, 
	bool transparent_background, // temp? wants to be in G42color
	SourceData *, DestData *, TransData *, SourceIterator *, DestIterator *, 
	SourceTransIterator *, DestTransIterator *)
{
	G42TransReadCachedImage <SourceData, SourceIterator, TransData,
		SourceTransIterator> source_image (source_desc, source_data);
	G42ReadCachedImage <SourceData, SourceIterator> 
		source_image_next (source_desc, source_data);
	G42TransWriteCachedImage <DestData, DestIterator, TransData, 
		DestTransIterator> dest_image (dest_desc, dest_data);
	G42Point source_mid = rotation -> GetSourceAxis();
	G42Point dest_mid = rotation -> GetDestAxis();
	G42Size source_size = rotation -> GetSourceSize();
	G42Size dest_size = rotation -> GetDestSize();
	/* Normalize Y position to 0  */
	#ifdef NOTYET
	double dyPos = (double) -dest_mid.Y;
	#endif
	int32 yPos = -dest_mid.Y;
	for (uint32 i = 0; i < dest_size.Height; i++)
	{
		/*  Grab destination data and transparency pointers  */
		DestIterator dest_ptr = dest_image [i];
		DestTransIterator dest_tptr = dest_image.GetTransIterator (i);
		/* Normalize X position to 0  */
		#ifdef NOTYET
		double dxPos = (double) -dest_mid.X;
		#endif
		int32 xPos = -dest_mid.X;
		for (uint32 j = 0; j < dest_size.Width; j++)
		{
			/*  Point to be rotated  */
			G42Point point (xPos, yPos);
			G42Color color (background);
			bool trans = true;	
			bool in_background = true;		// Assume yes
			if (type == G42Rotate::RotateAngleAntialias)
			{
				double row_delta;
				double col_delta;
				rotation -> RotatePoint (point, row_delta, col_delta);
				int32 source_x = point.X;
				int32 source_y = point.Y;
				int32 source_y_next = source_y + 1;
				G42Color colors [4];
				colors [0] = colors [1] = colors [2] = 
				colors [3] = background;
				bool x_in_bounds = (source_x >= 0 && 
					source_x < (source_size.Width - 1));
				bool nx_in_bounds = ((source_x + 1) >= 0 &&
					source_x < (source_size.Width - 2));
				bool y_in_bounds = (source_y >= 0 && 
					source_y < (source_size.Height - 1));
				bool ny_in_bounds = (source_y_next >= 0 && 
					source_y_next < (source_size.Height - 1));
				if (! y_in_bounds) {
					source_y = 0;
				}
				if (! ny_in_bounds) {
					source_y_next = 0;
				}
				
				SourceIterator curr_ptr = source_image [source_y];
				SourceIterator next_ptr = source_image_next [source_y_next];
	
				/*  Optimization so we don't interpolate on all background colors */
				if (x_in_bounds && y_in_bounds) 
				{
					colors [0] = source_converter (curr_ptr [source_x]);
					in_background = false;
				}
				if (nx_in_bounds && y_in_bounds) 
				{
					colors [1] = source_converter (curr_ptr [source_x + 1]);
					in_background = false;
				}
				if (x_in_bounds && ny_in_bounds) 
				{
					colors [2] = source_converter (next_ptr [source_x]);
					in_background = false;
				}
				if (nx_in_bounds && ny_in_bounds) 
				{
					colors [3] = source_converter (next_ptr [source_x + 1]);
					in_background = false;
				}
				if (! in_background) 
				{
					/*  Interpolate for current pixel + 3 surrounding values  */
					color = Interpolate4Colors (colors, row_delta, col_delta);		
					/*  Transparency  */
					SourceTransIterator source_tptr = 
						source_image.GetTransIterator (source_y);
					trans = source_tptr [source_x];
				}
			}
			else
			{
				/*  Straight pixel relocation  */
				rotation -> RotatePoint (point);
				if (point.X >= 0 && point.X <= (source_size.Width - 1) &&
					point.Y >= 0 && point.Y <= (source_size.Height - 1))
				{
					in_background = false;
					SourceIterator source_ptr = source_image [point.Y];
					color = source_converter (source_ptr [point.X]);
					/*  Transparency  */
					SourceTransIterator source_tptr = 
						source_image.GetTransIterator (point.Y);
					trans = source_tptr [point.X];
				}
			}
			*dest_ptr++ = dest_converter (color);
			/*  Transparency  */
			*dest_tptr++ = (in_background && transparent_background) ? false : trans;
			#ifdef NOTYET
			dxPos += 1.0;
			#endif
			xPos++;
		}
		#ifdef NOTYET
		dyPos += 1.0;
		#endif
		yPos++;
	}
}
G42Rotate::G42Rotate (G42Rotate::G42RotateType type, double angle,
	G42Color background)
:
	Angle (angle), BackgroundColor (background), RotateType (type)
{ }
//123456789012345678901234567890123456789012345678901234567890123456789012345678
void
G42Rotate::HandleFilter (void)
{
	G42ImageDescriptor::G42ImageType type = InputDescriptor -> GetImageType();
#ifdef ALLTYPES
	if (type == G42ImageDescriptor::Color8)
	{
		if (RotateType == Rotate90)
		{		
			Filter90 (InputDescriptor, InputData, OutputDescriptor,
				OutputData, DC8, DC8, IC8R, IC8, PB8);
		}
		else if (RotateType == Rotate180)
		{
			Filter180 (InputDescriptor, InputData, OutputDescriptor, 
				OutputData, DC8, DC8, IC8R, IC8, PB8);
		}
		else if (RotateType == Rotate270)
		{
			Filter270 (InputDescriptor, InputData, OutputDescriptor, 
				OutputData, DC8, DC8, IC8R, IC8, PB8);
		}
		else if (RotateType == RotateAngle || RotateType == RotateAngleAntialias)
		{
			/*  Color8 promotes to Color24/Color32 for Antialias */
			if (OutputDescriptor -> GetImageType() == G42ImageDescriptor::Color24)
			{
				FilterAngle (InputDescriptor, InputData, 
					OutputDescriptor, OutputData,
					G42ImageDataColor8::G42ConvertToG42Color 
						(InputDescriptor -> GetPalette()),
					G42ImageDataColor32::G42ConvertFromG42Color(),			
					Rotation, RotateType, BackgroundColor, true,
					DC8, DC24, DTS1, IC8R, IC24, ITS1R, ITS1);
			}
			else
			{
				FilterAngle (InputDescriptor, InputData, 
					OutputDescriptor, OutputData,
					G42ImageDataColor8::G42ConvertToG42Color 
						(InputDescriptor -> GetPalette()),
					G42ImageDataColor32::G42ConvertFromG42Color(),			
					Rotation, RotateType, BackgroundColor, true,
					DC8, DC32, DTS1, IC8R, IC32, ITS1R, ITS1);
			}
		}
	}	
	else if (type == G42ImageDescriptor::Color24)
	{
		if (RotateType == Rotate90)
		{		
			Filter90 (InputDescriptor, InputData, OutputDescriptor,
				OutputData, DC24, DC24, IC24R, IC24, PC24R);
		}
		else if (RotateType == Rotate180)
		{
			Filter180 (InputDescriptor, InputData, OutputDescriptor, 
				OutputData, DC24, DC24, IC24R, IC24, PC24R);
		}
		else if (RotateType == Rotate270)
		{
			Filter270 (InputDescriptor, InputData, OutputDescriptor, 
				OutputData, DC24, DC24, IC24R, IC24, PC24R);
		}
		else if (RotateType == RotateAngle || RotateType == RotateAngleAntialias)
		{
			FilterAngle (InputDescriptor, InputData, 
				OutputDescriptor, OutputData,
				G42ImageDataColor24::G42ConvertToG42Color(),
				G42ImageDataColor24::G42ConvertFromG42Color(),			
				Rotation, RotateType, BackgroundColor, true,
				DC24, DC24, DTS1, IC24R, IC24, ITS1R, ITS1);
		}
	}
	else 
#endif	// ALLTYPES	
	if (type == G42ImageDescriptor::Color32)
	{
		if (RotateType == Rotate90)
		{		
			Filter90 (InputDescriptor, InputData, OutputDescriptor, 
				OutputData, DC32, DC32, IC32R, IC32, PC32R);
		}
		else if (RotateType == Rotate180)
		{		
			Filter180 (InputDescriptor, InputData, OutputDescriptor, 
				OutputData, DC32, DC32, IC32R, IC32, PC32R);
		}
		else if (RotateType == Rotate270)
		{		
			Filter270 (InputDescriptor, InputData, OutputDescriptor, 
				OutputData, DC32, DC32, IC32R, IC32, PC32R);
		}
		else if (RotateType == RotateAngle || RotateType == RotateAngleAntialias)
		{
			FilterAngle (InputDescriptor, InputData, 
				OutputDescriptor, OutputData,
				G42ImageDataColor32::G42ConvertToG42Color(),
				G42ImageDataColor32::G42ConvertFromG42Color(),			
				Rotation, RotateType, BackgroundColor, true,
				DC32, DC32, DTS1, IC32R, IC32, ITS1R, ITS1);
		}
	}
#ifdef ALLTYPES
	else if (type == G42ImageDescriptor::Gray8)
	{
		if (RotateType == Rotate90)
		{		
			Filter90 (InputDescriptor, InputData, OutputDescriptor, 
				OutputData, DG8, DG8, IG8R, IG8, PB8); 
		}
		else if (RotateType == Rotate180)
		{		
			Filter180 (InputDescriptor, InputData, OutputDescriptor, 
				OutputData, DG8, DG8, IG8R, IG8, PB8); 
		}
		else if (RotateType == Rotate270)
		{		
			Filter270 (InputDescriptor, InputData, OutputDescriptor, 
				OutputData, DG8, DG8, IG8R, IG8, PB8); 
		}
		else if (RotateType == RotateAngle || RotateType == RotateAngleAntialias)
		{
			FilterAngle (InputDescriptor, InputData, 
				OutputDescriptor, OutputData,
				G42ImageDataGray8::G42ConvertToG42Color(),
				G42ImageDataGray8::G42ConvertFromG42Color(),			
				Rotation, RotateType, BackgroundColor, true,
				DG8, DG8, DTS1, IG8R, IG8, ITS1R, ITS1);
		}
	}
	else if (type == G42ImageDescriptor::Gray1)
	{
		if (RotateType == Rotate90)
		{		
			Filter90 (InputDescriptor, InputData, OutputDescriptor, OutputData, 
				DG1, DG1, IG1R, IG1, PB8);
		}
		else if (RotateType == Rotate180)
		{		
			Filter180 (InputDescriptor, InputData, OutputDescriptor, OutputData, 
				DG1, DG1, IG1R, IG1, PB8);
		}
		else if (RotateType == Rotate270)
		{		
			Filter270 (InputDescriptor, InputData, OutputDescriptor, OutputData, 
				DG1, DG1, IG1R, IG1, PB8);
		}
		else if (RotateType == RotateAngle || RotateType == RotateAngleAntialias)
		{
			FilterAngle (InputDescriptor, InputData, 
				OutputDescriptor, OutputData,
				G42ImageDataGray1::G42ConvertToG42Color(),
				G42ImageDataGray1::G42ConvertFromG42Color(),			
				Rotation, RotateType, BackgroundColor, true,
				DG1, DG8, DTS1, IG1R, IG8, ITS1R, ITS1);
		}
	}
#endif	// ALLTYPES
#ifdef ALLTYPES
	/*  Handle transparency if applicable  */
	if (OutputDescriptor -> IsTransparent())
	{
		if (RotateType == Rotate90)
		{
			Filter90 (InputDescriptor, InputData, OutputDescriptor, OutputData, 
				 DT1, DT1, IT1R, IT1, PB8);		
		}
		else if (RotateType == Rotate180)
		{
			Filter180 (InputDescriptor, InputData, OutputDescriptor, OutputData, 
				 DT1, DT1, IT1R, IT1, PB8);		
		}
		else if (RotateType == Rotate270)
		{		
			Filter270 (InputDescriptor, InputData, OutputDescriptor, OutputData, 
				 DT1, DT1, IT1R, IT1, PB8);		
		}
		/*  (RotateAngle and RotateAngleAntialias handle transparency themselves)  */ 
		
	}
#endif	// ALLTYPES
}
G42ImageFilter *
G42Rotate::MakeCopy (void) const
{
	return new G42Rotate (RotateType, Angle);
}
void
G42Rotate::ProcessFrame (void)
{
	/*  Setup output descriptor Make a new descriptor that describes the output data  */
	uint32 width 	= InputDescriptor -> GetWidth();
	uint32 height 	= InputDescriptor -> GetHeight();
	if (RotateType == Rotate90 || RotateType == Rotate270)
	{
		/*  Reverse width and height  */
		swap (width, height);
	}
	else if (RotateType == RotateAngle || RotateType == RotateAngleAntialias)
	{
		if (! Rotation) {
			Rotation = new G42Rotation (G42Size (width, height), Angle);
		}
		G42Size dest_size = Rotation -> GetDestSize();
		width  = dest_size.Width;
		height = dest_size.Height;
		/*  Add an extra line for antialiasing?  */
//		if (RotateType == RotateAngleAntialias)
		{
			width++;
			height++;
		}
	}
	G42ImageDescriptor::G42ImageType type = InputDescriptor -> GetImageType();
	if (RotateType != RotateAngle && RotateType != RotateAngleAntialias)
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
	#ifdef USE_TRANSPARENCY
	if (true) {
		OutputDescriptor -> SetTransparency (true);
	}
	#endif
	
	CreateOutputData();
	HandleFilter();
	Receiver -> Process (new G42ImageDescriptor (*OutputDescriptor), this);
	SendOutputData();
	/*  Clear everything  */
	ClearDataAndDescriptor (true);
}