// giaframe.cpp - animated image descriptor
#include "giaframe.h"
G42AnimatedImageFrame::G42AnimatedImageFrame(void)
:
	Offset(0, 0), Delay(0), UserInput(true), Replace(None)
{
}
G42AnimatedImageFrame::G42AnimatedImageFrame(G42Point & offset,
	int32 delay, bool user_input, G42Replace replace)
:
	Offset(offset), Delay(delay), UserInput(user_input), Replace(replace)
{
}
G42AnimatedImageFrame::G42AnimatedImageFrame(
	G42AnimatedImageFrame & ref)
:
	Offset(0, 0), Delay(0), UserInput(true), Replace(None)
{
	Copy(ref);
}
G42AnimatedImageFrame &
G42AnimatedImageFrame::operator = (G42AnimatedImageFrame & ref)
{
	Copy(ref);
	return *this;
}
void
G42AnimatedImageFrame::Copy(G42AnimatedImageFrame & ref)
{
	if (&ref == this)
		return;
	Offset = ref.Offset;
	Delay = ref.Delay;
	UserInput = ref.UserInput;
	Replace = ref.Replace;
}
G42AnimatedImageFrame::~G42AnimatedImageFrame(void)
{
}
void
G42AnimatedImageFrame::Set(G42Point & offset, int32 delay, bool user_input,
	G42Replace replace)
{
	Offset = offset;
	Delay = delay;
	UserInput = user_input;
	Replace = replace;
}
