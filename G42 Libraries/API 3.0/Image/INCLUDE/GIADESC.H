// giadesc.h - animated image descriptor
#if !defined(GIADESC_H)
#define GIADESC_H
#include "gimdesc.h"
class G42AnimatedImageDescriptor : public G42MultipleImageDescriptor
{
public:
	G42AnimatedImageDescriptor(void);
	G42AnimatedImageDescriptor(uint32 width, uint32 height,
		bool repeat = false, uint32 num_images = 0);
	G42AnimatedImageDescriptor(G42MultipleImageDescriptor & ref,
		uint32 width, uint32 height, bool repeat = false);
	virtual ~G42AnimatedImageDescriptor(void);
	void Set(uint32 width, uint32 height,
		bool repeat = false, uint32 num_images = 0);
	uint32 GetWidth(void) const
		{ return Width; }
	uint32 GetHeight(void) const
		{ return Height; }
	bool GetRepeat(void) const
		{ return Repeat; }
	void SetRepeat(bool repeat = true)
		{ Repeat = repeat; }
	void SetWidth(uint32 width)
		{ Width = width; }
	void SetHeight(uint32 height)
		{ Height = height; }
	virtual G42MultipleImageDescriptor * MakeCopy(void);
protected:
	void Copy(G42AnimatedImageDescriptor & ref);
	G42AnimatedImageDescriptor(G42AnimatedImageDescriptor & ref);
private:
	G42AnimatedImageDescriptor & operator = (G42AnimatedImageDescriptor &)
		{ return *this; }
	uint32				Width;
	uint32				Height;
	bool					Repeat;
};
#endif // GIADESC_H
