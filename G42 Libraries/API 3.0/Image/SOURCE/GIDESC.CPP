
#include "gidesc.h"
#include "gidata.h"
G42ImageDescriptor::G42ImageDescriptor(uint32 width, uint32 height,
	G42ImageDescriptor::G42ImageType type, G42HeapManager & heap, bool trans)
:
	Width(0), Height(0), ImageType(Gray1), NumPalette(0), PixelDepth(1),
	RowsPerStrip(0), NumStrips(0), RowBytes(0), LastStripHeight(0),
	StripShift(0), StripMask(0), FirstStripHeight(0), FirstStripRemainder(0),
	Transparent(false), TransparentColorSet(false), TransparentPixel(-1),
	BackgroundColorSet(false), BackgroundPixel(-1), TransRowBytes(0),
	AlphaChannel(false), Heap(heap)
{
	Set(width, height, type, trans);
}
G42ImageDescriptor::G42ImageDescriptor(G42ImageDescriptor & ref)
:
	Width(0), Height(0), ImageType(Gray1), NumPalette(0), PixelDepth(1),
	RowsPerStrip(0), NumStrips(0), RowBytes(0), LastStripHeight(0),
	StripShift(0), StripMask(0), FirstStripHeight(0), FirstStripRemainder(0),
	Transparent(false), TransparentColorSet(false), TransparentPixel(-1),
	BackgroundColorSet(false), BackgroundPixel(-1), TransRowBytes(0),
	AlphaChannel(false), Heap(ref.Heap)
{
	Copy(ref);
}
G42ImageDescriptor::G42ImageDescriptor(G42ImageDescriptor & ref,
	uint32 width, uint32 height)
:
	Width(0), Height(0), ImageType(Gray1), NumPalette(0), PixelDepth(1),
	RowsPerStrip(0), NumStrips(0), RowBytes(0), LastStripHeight(0),
	StripShift(0), StripMask(0), FirstStripHeight(0), FirstStripRemainder(0),
	Transparent(false), TransparentColorSet(false), TransparentPixel(-1),
	BackgroundColorSet(false), BackgroundPixel(-1), TransRowBytes(0),
	AlphaChannel(false), Heap(ref.Heap)
{
	Copy(ref);
	FirstStripHeight = 0; // so it will be set below
	Set(width, height, ImageType, Transparent);
}
G42ImageDescriptor::G42ImageDescriptor(G42ImageDescriptor & ref,
	uint32 width, uint32 height, G42ImageType type)
:
	Width(0), Height(0), ImageType(Gray1), NumPalette(0), PixelDepth(1),
	RowsPerStrip(0), NumStrips(0), RowBytes(0), LastStripHeight(0),
	StripShift(0), StripMask(0), FirstStripHeight(0), FirstStripRemainder(0),
	Transparent(false), TransparentColorSet(false), TransparentPixel(-1),
	BackgroundColorSet(false), BackgroundPixel(-1), TransRowBytes(0),
	AlphaChannel(false), Heap(ref.Heap)
{
	Copy(ref);
	FirstStripHeight = 0; // so it will be set below
	Set(width, height, type, Transparent);
}
G42ImageDescriptor &
G42ImageDescriptor::operator = (G42ImageDescriptor & ref)
{
	Copy(ref);
	return *this;
}
void
G42ImageDescriptor::Copy(G42ImageDescriptor & ref)
{
	if (&ref == this)
		return;
	Width = ref.Width;
	Height = ref.Height;
	RowsPerStrip = ref.RowsPerStrip;
	NumStrips = ref.NumStrips;
	RowBytes = ref.RowBytes;
	TransRowBytes = ref.TransRowBytes;
	StripShift = ref.StripShift;
	StripMask = ref.StripMask;
	ImageType = ref.ImageType;
	NumPalette = ref.NumPalette;
	Transparent = ref.Transparent;
	AlphaChannel = ref.AlphaChannel;
	TransparentColorSet = ref.TransparentColorSet;
	TransparentColor = ref.TransparentColor;
	TransparentPixel = ref.TransparentPixel;
	BackgroundColorSet = ref.BackgroundColorSet;
	BackgroundColor = ref.BackgroundColor;
	BackgroundPixel = ref.BackgroundPixel;
	FirstStripHeight = ref.FirstStripHeight;
	FirstStripRemainder = ref.FirstStripRemainder;
	LastStripHeight = ref.LastStripHeight;
	PixelDepth = ref.PixelDepth;
	if (ImagePalette.NotNull())
	{
		ImagePalette.Clear();
	}
	if (ref.ImagePalette.NotNull() && ref.NumPalette)
	{
		ImagePalette.Set(new G42Color [ref.NumPalette]);
		for (intf i = 0; i < ref.NumPalette; i++)
		{
			ImagePalette[i] = ref.ImagePalette[i];
		}
	}
	/*  Region */
	if (ref.Region)
	{
		Region = new G42Region (*ref.Region);	
	}
	else
	{
		Region.Clear();
	}
}
G42ImageDescriptor::~G42ImageDescriptor(void)
{
}
void
G42ImageDescriptor::Set(uint32 width, uint32 height,
	G42ImageType type, byte strip_shift, uint32 first_strip_height)
{
	Width = width;
	Height = height;
	ImageType = type;
	StripShift = strip_shift;
	FirstStripHeight = first_strip_height;
	Calculate();
}
void
G42ImageDescriptor::Set(uint32 width, uint32 height,
	G42ImageType type, bool trans)
{
	Width = width;
	Height = height;
	ImageType = type;
	CalculateRowBytes();
	CalculateStripShift(Heap.GetMaxEntrySize(), trans);
	CalculateStrips();
}
void
G42ImageDescriptor::Calculate(void)
{
	CalculateRowBytes();
	CalculateStrips();
}
void
G42ImageDescriptor::CalculateRowBytes(void)
{
	PixelDepth = (byte)(ImageType & DepthMask);
	RowBytes = ((Width * PixelDepth + 7) >> 3);
	RowBytes = ((RowBytes + 3) & ~3);
	TransRowBytes = ((Width + 7) >> 3);
	TransRowBytes = ((TransRowBytes + 3) & ~3);
}
void
G42ImageDescriptor::CalculateStripShift(int32 max_strip_bytes, bool trans)
{
	uint32 bytes_per_row = RowBytes;
	if (trans)
		bytes_per_row += TransRowBytes;
	StripShift = 1;
	while (bytes_per_row * (1 << (StripShift + 1)) <= max_strip_bytes)
	{
		StripShift++;
	}
}
void
G42ImageDescriptor::CalculateStrips(void)
{
	RowsPerStrip = (1 << StripShift);
	if (!FirstStripHeight)
	{
		FirstStripHeight = RowsPerStrip;
	}
	StripMask = (uint16)(RowsPerStrip - 1);
	FirstStripRemainder = RowsPerStrip - FirstStripHeight;
	if (FirstStripHeight >= Height)
	{
		NumStrips = 1;
		FirstStripHeight = LastStripHeight = Height;
	}
	else
	{
		NumStrips = ((Height - 1 - FirstStripHeight) >> StripShift) + 2;
		LastStripHeight = Height - (FirstStripHeight + RowsPerStrip *
			(NumStrips - 2));
	}
}
G42ImageDescriptor::G42ImageType
G42GetImageType(byte depth, bool color)
{
	if (color)
	{
		if (depth <= 8)
		{
			return G42ImageDescriptor::Color8;
		}
		else
		{
			#ifdef MSWIN
			return G42ImageDescriptor::Color24;
			#endif
			#ifdef MACOS
			return G42ImageDescriptor::Color32;
			#endif
		}
	}
	else // grayscale
	{
		if (depth <= 1)
		{
			return G42ImageDescriptor::Gray1;
		}
		else
		{
			return G42ImageDescriptor::Gray8;
		}
	}
//	return G42ImageDescriptor::Gray1;
}
void
G42ImageDescriptor::SetNumPalette (uint16 num_palette)
{
	NumPalette = num_palette;
	if (NumPalette)
	{
		ImagePalette.Set(new G42Color [NumPalette]);
	}
	else
	{
		ImagePalette.Clear();
	}
}
void 
G42ImageDescriptor::SetRegion (const G42Region & region)
{
	Region = new G42Region (region);
	G42ThrowIfNull (Region);	
}
void
G42ImageDescriptor::SetTransparency(bool trans)
{
	Transparent = trans;
	if (Transparent)
	{
		TransRowBytes = ((Width + 7) >> 3);
		TransRowBytes = ((TransRowBytes + 3) & ~3);
	}
	else
	{
		TransRowBytes = 0;
		AlphaChannel = false;
	}
}
void
G42ImageDescriptor::SetAlphaChannel(bool state)
{
	AlphaChannel = state;
	if (AlphaChannel && !Transparent)
	{
		SetTransparency(true);
	}
	if (AlphaChannel)
	{
		TransparentColorSet = false;
		TransparentPixel = -1;
	}
}
void
G42ImageDescriptor::ClearTransparency(void)
{
	SetTransparency(false);
}
void
G42ImageDescriptor::ClearTransparentColor(void)
{
	TransparentColorSet = false;
}
void
G42ImageDescriptor::SetTransparentPixel (int16 trans_pixel)
{
	TransparentPixel = trans_pixel;
	TransparentColorSet = true;
	AlphaChannel = false;
	if (IsPaletted() && trans_pixel >= 0 && trans_pixel < NumPalette)
	{
		TransparentColor = ImagePalette[trans_pixel];
	}
	else if (!IsColor())
	{
		if (GetDepth() == 1)
		{
			byte v = (byte)(trans_pixel ? 1 : 0);
			TransparentColor = G42Color(v, v, v);
		}
		else
		{
			TransparentColor = G42Color((byte)trans_pixel, (byte)trans_pixel,
				(byte)trans_pixel);
		}
	}
}
void
G42ImageDescriptor::SetTransparentColor(const G42Color & trans_color)
{
	TransparentColor = trans_color;
	TransparentColorSet = true;
	TransparentPixel = -1;
	AlphaChannel = false;
	if (NumPalette)
	{
		for (uint16 i = 0; i < NumPalette; i++)
		{
			if (ImagePalette[i] == TransparentColor)
			{
				TransparentPixel = i;
				break;
			}
		}
	}
	else
	{
		TransparentPixel = -1;
	}
}
void
G42ImageDescriptor::ClearBackgroundColor(void)
{
	BackgroundColorSet = false;
}
void
G42ImageDescriptor::SetBackgroundPixel (int16 back_pixel)
{
	BackgroundPixel = back_pixel;
	BackgroundColorSet = true;
	if (IsPaletted() && back_pixel >= 0 && back_pixel < NumPalette)
	{
		BackgroundColor = ImagePalette[back_pixel];
	}
	else if (!IsColor())
	{
		if (GetDepth() == 1)
		{
			byte v = (byte)(back_pixel ? 1 : 0);
			BackgroundColor = G42Color(v, v, v);
		}
		else
		{
			BackgroundColor = G42Color((byte)back_pixel, (byte)back_pixel,
				(byte)back_pixel);
		}
	}
}
void
G42ImageDescriptor::SetBackgroundColor(G42Color & back_color)
{
	BackgroundColor = back_color;
	BackgroundColorSet = true;
	BackgroundPixel = -1;
	if (NumPalette)
	{
		for (uint16 i = 0; i < NumPalette; i++)
		{
			if (ImagePalette[i] == BackgroundColor)
			{
				BackgroundPixel = i;
				break;
			}
		}
	}
	else
	{
		BackgroundPixel = -1;
	}
}
G42ImageData *
G42ImageDescriptor::CreateData(uint32 strip_num) const
{
	G42ImageData * data = 0;
	uint32 num_rows = GetRowsInStrip(strip_num);
	uint32 row_bytes = RowBytes;
	uint32 strip_start_in_image = GetStripStartInImage(strip_num);
	uint32 width = Width;
	uint32 trans_row_bytes = (Transparent ? TransRowBytes : 0);
	switch (ImageType)
	{
		case G42ImageDescriptor::Color32:
			data = new G42ImageDataColor32(Heap, strip_num, num_rows,
				row_bytes, strip_start_in_image, width, trans_row_bytes);
			break;
		case G42ImageDescriptor::Color24:
			data = new G42ImageDataColor24(Heap, strip_num, num_rows,
				row_bytes, strip_start_in_image, width, trans_row_bytes);
			break;
		case G42ImageDescriptor::Color8:
			data = new G42ImageDataColor8(Heap, strip_num, num_rows,
				row_bytes, strip_start_in_image, width, trans_row_bytes);
			break;
		case G42ImageDescriptor::Gray8:
			data = new G42ImageDataGray8(Heap, strip_num, num_rows,
				row_bytes, strip_start_in_image, width, trans_row_bytes);
			break;
		case G42ImageDescriptor::Gray1:
			data = new G42ImageDataGray1(Heap, strip_num, num_rows,
				row_bytes, strip_start_in_image, width, trans_row_bytes);
			break;
	}
	return data;
}
