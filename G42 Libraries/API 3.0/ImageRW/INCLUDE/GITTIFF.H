// gittiff.h - tiff image type id
#if !defined(GITTIFF_H)
#define GITTIFF_H
#include "girwopt.h"
const intf G42TiffTypeId = 0x107;
class G42ImageTiffOptions : public G42ImageReadWriteOptions
{
public:
	enum G42TiffCompressionType
		{
			NoCompression = 1,
			CCITTCompression = 2,
			Group3Compression = 3,
			Group4Compression = 4,
			LZWCompression = 5,
			PackBitsCompression = 32773
		};
	G42ImageTiffOptions(G42TiffCompressionType type = NoCompression,
		bool write = true)
	:
		G42ImageReadWriteOptions(G42TiffTypeId, type != NoCompression,
			24, false, write),
		CompressionType(type)
			{}
	virtual ~G42ImageTiffOptions(void) {}
	virtual G42BaseOptions * MakeCopy(void)
		{return new G42ImageTiffOptions(*this);}
	G42TiffCompressionType GetTiffCompressionType(void) const
		{ return CompressionType; }
	void SetTiffCompressionType(G42TiffCompressionType type)
		{ CompressionType = type; }
protected:
	G42ImageTiffOptions(G42ImageTiffOptions & ref)
	:
		G42ImageReadWriteOptions(ref),
		CompressionType(ref.CompressionType)
			{}
	G42TiffCompressionType CompressionType;
};
#endif // GITTIFF_H
