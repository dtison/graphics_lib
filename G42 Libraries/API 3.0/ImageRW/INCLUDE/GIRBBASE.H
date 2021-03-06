// girbbase.h - G42ImageBackplaneBaseReader
#if !defined(GIRBBASE_H)
#define GIRBBASE_H
#include "grbbase.h"
#include "gidata.h"
#include "gidesc.h"
class G42ImageBackplaneBaseReader : public G42BackplaneBaseReader
{
public:
	G42ImageBackplaneBaseReader(G42RWInfo::G42TypeId type_id,
		const char * def_ext, const char * name_string,
		const char * pattern, intf pattern_length,
		intf major_version, intf minor_version, bool last);
	virtual ~G42ImageBackplaneBaseReader(void);
	// various things that will be different in the dll class
	G42ImageData * GetImageData(G42ImageDescriptor::G42ImageType type,
		G42HeapManager & heap, uint32 strip_num,
		uint32 num_rows, uint32 row_bytes, uint32 strip_start_in_image,
		uint32 width, uint32 trans_row_bytes = 0);
	void ClearImageData(G42ImageData * data);
	G42WriteImageData * GetWriteImageLock(G42ImageData * data);
	void ClearWriteImageLock(G42WriteImageData * lock);
	byte * GetRowFromLock(G42WriteImageData * data, uint32 row,
		bool trans = false);
protected:
	// Hidden Copy Contructor and operator=
	G42ImageBackplaneBaseReader(G42ImageBackplaneBaseReader & ref) :
		G42BackplaneBaseReader(ref) {}
	G42ImageBackplaneBaseReader & operator = (G42ImageBackplaneBaseReader & ref)
		{return *this;}
};
#endif // GIRBBASE_H sentry
