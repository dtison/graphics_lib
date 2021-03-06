// girbbase.h - G42ImageBackplaneBaseReader
#include "girbbase.h"
G42ImageBackplaneBaseReader::G42ImageBackplaneBaseReader(
	G42RWInfo::G42TypeId type_id,
	const char * def_ext, const char * name_string,
	const char * pattern, intf pattern_length,
	intf major_version, intf minor_version, bool last)
:
	G42BackplaneBaseReader(type_id, def_ext, name_string,
		major_version, minor_version, pattern, pattern_length,
		G42RWInfo::Image, last)
{
}
G42ImageBackplaneBaseReader::~G42ImageBackplaneBaseReader(void)
{
}
G42ImageData *
G42ImageBackplaneBaseReader::GetImageData(
	G42ImageDescriptor::G42ImageType type,
	G42HeapManager & heap, uint32 strip_num,
	uint32 num_rows, uint32 row_bytes, uint32 strip_start_in_image,
	uint32 width, uint32 trans_row_bytes)
{
	G42ImageData * data = 0;
	switch (type)
	{
		case G42ImageDescriptor::Color32:
			data = new G42ImageDataColor32(heap, strip_num, num_rows,
				row_bytes, strip_start_in_image, width, trans_row_bytes);
			break;
		case G42ImageDescriptor::Color24:
			data = new G42ImageDataColor24(heap, strip_num, num_rows,
				row_bytes, strip_start_in_image, width, trans_row_bytes);
			break;
		case G42ImageDescriptor::Color8:
			data = new G42ImageDataColor8(heap, strip_num, num_rows,
				row_bytes, strip_start_in_image, width, trans_row_bytes);
			break;
		case G42ImageDescriptor::Gray8:
			data = new G42ImageDataGray8(heap, strip_num, num_rows,
				row_bytes, strip_start_in_image, width, trans_row_bytes);
			break;
		case G42ImageDescriptor::Gray1:
			data = new G42ImageDataGray1(heap, strip_num, num_rows,
				row_bytes, strip_start_in_image, width, trans_row_bytes);
			break;
	}
	return data;
}
void
G42ImageBackplaneBaseReader::ClearImageData(G42ImageData * data)
{
	if (data)
	{
		delete data;
	}
}
G42WriteImageData *
G42ImageBackplaneBaseReader::GetWriteImageLock(G42ImageData * data)
{
	if (!data)
	{
		return 0;
	}
	G42WriteImageData * lock = new G42WriteImageData(*data);
	G42ThrowIfNull(lock);
	return lock;
}
void
G42ImageBackplaneBaseReader::ClearWriteImageLock(G42WriteImageData * lock)
{
	if (lock)
	{
		delete lock;
	}
}
byte *
G42ImageBackplaneBaseReader::GetRowFromLock(G42WriteImageData * data, uint32 row,
	bool trans)
{
	return data->GetRow(row, trans);
}
