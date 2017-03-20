// gitjpeg.h - jpeg image type id and options
#if !defined(GITJPEG_H)
#define GITJPEG_H
#include "girwopt.h"
const intf G42JpegTypeId = 0x103;
class G42ImageJpegOptions : public G42ImageReadWriteOptions
{
public:
	G42ImageJpegOptions(bool progressive = false,
		byte quality = 90, bool photo = false, bool write = true)
	:
		G42ImageReadWriteOptions(G42JpegTypeId, true, 24, progressive, write),
		Quality(quality), Photo(photo)
			{}
	virtual ~G42ImageJpegOptions(void) {}
	virtual G42BaseOptions * MakeCopy(void)
		{return new G42ImageJpegOptions(*this);}
	byte GetQuality(void) const
		{ return Quality; }
	bool IsPhoto(void) const
		{ return Photo; }
	void SetQuality(byte quality)
		{ Quality = quality; }
	void SetPhoto(bool photo)
		{ Photo = photo; }
protected:
	G42ImageJpegOptions(G42ImageJpegOptions & ref)
	:
		G42ImageReadWriteOptions(ref),
		Quality(ref.Quality), Photo(ref.Photo)
			{}
	byte Quality;
	bool Photo;
};
#endif // GITJPEG_H
