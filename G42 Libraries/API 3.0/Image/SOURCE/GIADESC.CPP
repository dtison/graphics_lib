
#include "giadesc.h"
G42AnimatedImageDescriptor::G42AnimatedImageDescriptor(void)
:
	Width(0), Height(0), Repeat(0)
{
}
G42AnimatedImageDescriptor::G42AnimatedImageDescriptor(uint32 width,
	uint32 height, bool repeat, uint32 num_images)
:
	G42MultipleImageDescriptor(num_images), Width(width), Height(height),
	Repeat(repeat)
{
}
G42AnimatedImageDescriptor::G42AnimatedImageDescriptor(
	G42MultipleImageDescriptor & ref,
	uint32 width, uint32 height, bool repeat)
:
	G42MultipleImageDescriptor(ref), Width(width), Height(height),
	Repeat(repeat)
{
}
G42AnimatedImageDescriptor::G42AnimatedImageDescriptor(
	G42AnimatedImageDescriptor & ref)
:
	Width(0), Height(0), Repeat(0)
{
	Copy(ref);
}
G42MultipleImageDescriptor *
G42AnimatedImageDescriptor::MakeCopy(void)
{
	return new G42AnimatedImageDescriptor(*this);
}
void
G42AnimatedImageDescriptor::Copy(G42AnimatedImageDescriptor & ref)
{
	if (&ref == this)
		return;
	Width = ref.Width;
	Height = ref.Height;
	Repeat = ref.Repeat;
	G42MultipleImageDescriptor::Copy(ref);
}
G42AnimatedImageDescriptor::~G42AnimatedImageDescriptor(void)
{
}
void
G42AnimatedImageDescriptor::Set(uint32 width, uint32 height,
	bool repeat, uint32 num_images)
{
	Width = width;
	Height = height;
	Repeat = repeat;
	G42MultipleImageDescriptor::Set(num_images);
}
