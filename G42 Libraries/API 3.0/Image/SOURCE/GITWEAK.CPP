// gitweak.cpp - image tweaker
// this is not optimized, as we lock and unlock the strip every time.
// we need to save off the 5 cached images.
#include "gitweak.h"
#include "gicached.h"
#include "gifilter.h"
G42ImageTweaker::G42ImageTweaker(G42Image & image, uint32 frame_index)
:
	Image(image), FrameIndex(frame_index)
{
}
G42ImageTweaker::~G42ImageTweaker(void)
{
}
template <class Data, class Iterator, class Converter>
G42Color
G42GetColorAtPoint(const G42Point & point, G42Image & Image, uint32 frame_index,
	Converter convert, Data *, Iterator *)
{
	const G42ImageDescriptor & desc_ref = Image.GetInfo(frame_index);
	const G42ImageDescriptor * desc = &desc_ref;
	G42ImageData ** data = Image.GetDataArray(frame_index);
	G42ReadCachedImageBase<Data, Iterator, const G42ImageDescriptor *,
			G42ImageData **>
		cached(desc, data);
	Iterator i = cached[point.Y];
	i += point.X;
	return convert(*i);
}
G42Color
G42ImageTweaker::GetColorAtPoint(const G42Point & point)
{
	switch (Image.GetInfo(FrameIndex).GetImageType())
	{
		case G42ImageDescriptor::Gray1:
		{
			G42Color c = G42GetColorAtPoint(point, Image, FrameIndex,
				G42ImageDataGray1::G42ConvertToG42Color(),
				(G42ImageDataGray1 *)0,
				(G42ImageDataGray1::G42ConstIterator::G42Row::G42Iterator *)0);
			return c;
		}
		case G42ImageDescriptor::Gray8:
		{
			G42Color c = G42GetColorAtPoint(point, Image, FrameIndex,
				G42ImageDataGray8::G42ConvertToG42Color(),
				(G42ImageDataGray8 *)0,
				(G42ImageDataGray8::G42ConstIterator::G42Row::G42Iterator *)0);
			return c;
		}
		case G42ImageDescriptor::Color8:
		{
			G42Color c = G42GetColorAtPoint(point, Image, FrameIndex,
				G42ImageDataColor8::G42ConvertToG42Color(
					Image.GetInfo(FrameIndex).GetPalette()),
				(G42ImageDataColor8 *)0,
				(G42ImageDataColor8::G42ConstIterator::G42Row::G42Iterator *)0);
			return c;
		}
		case G42ImageDescriptor::Color24:
		{
			G42Color c = G42GetColorAtPoint(point, Image, FrameIndex,
				G42ImageDataColor24::G42ConvertToG42Color(),
				(G42ImageDataColor24 *)0,
				(G42ImageDataColor24::G42ConstIterator::G42Row::G42Iterator *)0);
			return c;
		}
		case G42ImageDescriptor::Color32:
		{
			G42Color c = G42GetColorAtPoint(point, Image, FrameIndex,
				G42ImageDataColor32::G42ConvertToG42Color(),
				(G42ImageDataColor32 *)0,
				(G42ImageDataColor32::G42ConstIterator::G42Row::G42Iterator *)0);
			return c;
		}
	}
	return G42Color();
}
/* This gets done later, when we write the pixel editor mk controller
void
G42ImageTweaker::SetColorAtPoint(const G42Point & point, const G42Color & color)
{
}
*/
