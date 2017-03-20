#if ! defined (GIFDITHR_H)
#define GIFDITHR_H
#include "gifilter.h"
#include "giopal.h"
class G42Dither : public G42ImageFilter
{
public:		
	enum					G42DitherType {ColorDither, BilevelDither, Graysum};
							G42Dither (const G42OptimizedPalette * dither_pal, 
								G42DitherType type = ColorDither, bool match = false);
	virtual 				~G42Dither (void);
	virtual void 		HandleData (G42ImageData * data, 
								G42PathComponent *);
	virtual void 		HandleDescriptor (G42ImageDescriptor * descriptor, 
								G42PathComponent *);
	virtual void 		HandleEnd (G42PathEnd * end, 
								G42PathComponent *);
	virtual 				G42ImageFilter * MakeCopy(void) const;
protected:	
	void					HandleFilter (void);
	// Variables
	G42DitherType		DitherType;
	bool					Match;
	const 				G42OptimizedPalette * DitherPalette;
};
/*  Support class for G42Dither  */
class G42DitherError
{
public:
	enum G42Direction {Forward, Reverse};
	G42DitherError(int width, int clip = 256, G42Direction direction = Forward)
	:
		Width(width + 2), ErrorRow(0), NextErrorRow(0),
		ErrorPtr(0), NextErrorPtr(0), Clip(clip), Direction (direction)
	{
		ErrorRow = new int [Width];
		NextErrorRow = new int [Width];
		memset(ErrorRow, 0, Width * sizeof (int));
		memset(NextErrorRow, 0, Width * sizeof (int));
		ErrorPtr = ErrorRow;
		NextErrorPtr = NextErrorRow;
		ErrorPtr +=  (Direction == Forward) ? 1 : Width - 1;
		NextErrorPtr += (Direction == Forward) ? 0: Width;
		Clip2 = Clip << 1;
		Clip3 = Clip + Clip2;
	}
	~G42DitherError(void)
	{
		delete [] ErrorRow;
		delete [] NextErrorRow;
	}
	int GetError(void)
	{
		int err = *ErrorPtr;
		if (err >= Clip3)
			err = Clip2;
		else if (err > Clip)
			err = (Clip + ((err - Clip) >> 1));
		else if (err <= -Clip3)
			err = -Clip2;
		else if (err < -Clip)
			err = (-Clip - ((-err - Clip) >> 1));
		/*  Done with error  */
		*ErrorPtr = 0;	
		/*  Advance pointer  */
		intf offset = (Direction == Forward) ? 1: -1;
		ErrorPtr += offset;		
		
		return err;
	}
	void SetError(int err)
	{
		int error1;
		int error3;
		int error5;
		if (err > 0)
		{
			error1 = (err >> 4);
			error3 = (err >> 3) + error1;
			error5 = (err >> 2) + error1;
		}
		else
		{
			error1 = -((-err) >> 4);
			error3 = -((-err) >> 3) + error1;
			error5 = -((-err) >> 2) + error1;
		}
		intf fwd_offset = (Direction == Forward) ? 1: -1;
		intf rev_offset = (Direction == Forward) ? -1: 1;
		*(NextErrorPtr) += error3;
		NextErrorPtr += fwd_offset;
		*(NextErrorPtr) += error5;
		NextErrorPtr += fwd_offset;
		*(NextErrorPtr) += error1;
		NextErrorPtr += rev_offset;
		*ErrorPtr += err - (error1 + error3 + error5);
	}
	G42Direction GetDirection (void) {return Direction;}
	void SetDirection (G42Direction direction) {Direction = direction;}
	void NextRow(void)
	{
		swap (ErrorRow, NextErrorRow);
		ErrorPtr = ErrorRow;
		NextErrorPtr = NextErrorRow;
		ErrorPtr +=  (Direction == Forward) ? 1 : Width - 1;
		NextErrorPtr += (Direction == Forward) ? 0: Width;
	}
protected:
	int Width;
	int * ErrorRow;
	int * NextErrorRow;
	int * ErrorPtr;
	int * NextErrorPtr;
	int Clip;
	int Clip2;
	int Clip3;
	G42Direction Direction;
};
#endif
