#include "gidata.h"
G42ImageData::G42ImageData(G42ImageData & ref)
:
	G42DataKey(ref),
	StripNum(0), NumRows(0), RowBytes(0),
	#ifdef USE_ROWS
	Rows(0), TransRows(0),
	#endif
	#ifdef USE_OFFSETS
	RowOffsets(0), TransRowOffsets(0),
	#endif
	StripStartInImage(0), TransRowBytes(0), TransOffset(0)
{
	CopyInfo(ref);
}
G42ImageData &
G42ImageData::operator = (G42ImageData & ref)
{
	Copy(ref);
	return *this;
}
G42ImageData::G42ImageData(G42HeapManager & heap,
	uint32 strip_num, uint32 num_rows, uint32 row_bytes,
	uint32 strip_start_in_image, uint32 width, uint32 trans_row_bytes)
		throw (G42InvalidParameterException)
:
	G42DataKey(heap, (row_bytes + trans_row_bytes) * num_rows, true),
	StripNum(0), NumRows(0), RowBytes(0),
	#ifdef USE_ROWS
	Rows(0), TransRows(0),
	#endif
	#ifdef USE_OFFSETS
	RowOffsets(0), TransRowOffsets(0),
	#endif
	StripStartInImage(0), TransOffset(0), TransRowBytes(0), Width(width)
{
	SetInfo(strip_num, num_rows, row_bytes, strip_start_in_image,
		width, trans_row_bytes);
}
void
G42ImageData::Set(uint32 strip_num, uint32 num_rows, uint32 row_bytes,
	uint32 strip_start_in_image, uint32 width, uint32 trans_row_bytes)
		throw (G42InvalidParameterException)
{
	if (num_rows == 0 || row_bytes == 0)
	{
		throw G42InvalidParameterException();
	}
	Clear();
	RowBytes = row_bytes;
	StripNum = strip_num;
	NumRows = num_rows;
	StripStartInImage = strip_start_in_image;
	Width = width;
	TransRowBytes = trans_row_bytes;
	TransOffset = RowBytes * NumRows;
	Handle = Heap.Allocate((RowBytes + TransRowBytes) * NumRows, true);
}
void
G42ImageData::SetInfo(uint32 strip_num, uint32 num_rows, uint32 row_bytes,
	uint32 strip_start_in_image, uint32 width, uint32 trans_row_bytes)
		throw (G42InvalidParameterException)
{
	if (num_rows == 0 || row_bytes == 0)
	{
		throw G42InvalidParameterException();
	}
	ClearRows();
	RowBytes = row_bytes;
	TransRowBytes = trans_row_bytes;
	Width = width;
	StripNum = strip_num;
	NumRows = num_rows;
	StripStartInImage = strip_start_in_image;
	TransOffset = RowBytes * NumRows;
}
void
G42ImageData::BuildRows(byte * ptr)
{
#ifdef USE_ROWS
	if (!Rows)
		Rows = new byte * [NumRows];
	for (int i = 0; i < NumRows; i++)
	{
		#ifdef MSWIN
		Rows[i] = ptr + (NumRows - i - 1) * RowBytes;
		#else
		Rows[i] = ptr + i * RowBytes;
		#endif
	}
	if (TransRowBytes > 0)
	{
		if (!TransRows)
			TransRows = new byte * [NumRows];
		for (int i = 0; i < NumRows; i++)
		{
			#ifdef MSWIN
			TransRows[i] = ptr + TransOffset + (NumRows - i - 1) * TransRowBytes;
			#else
			TransRows[i] = ptr + TransOffset + i * TransRowBytes;
			#endif
		}
	}
#endif
#ifdef USE_OFFSETS
	Ptr = ptr;
	
	if (!RowOffsets)
	{
		RowOffsets = new uint32 [NumRows];
		for (int i = 0; i < NumRows; i++)
		{
			#ifdef MSWIN
			RowOffsets[i] = (NumRows - i - 1) * RowBytes;
			#else
			RowOffsets[i] = i * RowBytes;
			#endif
		}
		if (TransRowBytes > 0)
		{
			if (!TransRowOffsets)
			{
				TransRowOffsets = new uint32 [NumRows];
				for (int i = 0; i < NumRows; i++)
				{
					#ifdef MSWIN
					TransRowOffsets[i] = TransOffset + (NumRows - i - 1) * TransRowBytes;
					#else
					TransRowOffsets[i] = TransOffset + i * TransRowBytes;
					#endif
				}
			}
		}
	}
#endif
}
void
G42ImageData::ClearRows()
{
#ifdef USE_ROWS
	if (Rows)
	{
		delete [] Rows;
		Rows = 0;
	}
	if (TransRows)
	{
		delete [] TransRows;
		TransRows = 0;
	}
#endif
#ifdef USE_OFFSETS
	if (RowOffsets)
	{
		delete [] RowOffsets;
		RowOffsets = 0;
	}
	if (TransRowOffsets)
	{
		delete [] TransRowOffsets;
		TransRowOffsets = 0;
	}
#endif
}
void
G42ImageData::Clear()
{
	ClearRows();
	G42DataKey::Clear();
}
G42ImageData::~G42ImageData()
{
	Clear();
}
void
G42ImageData::Copy(G42ImageData & ref)
{
	if (&ref != this)
	{
		SetInfo(ref.StripNum, ref.NumRows, ref.RowBytes, ref.StripStartInImage,
			ref.Width, ref.TransRowBytes);
		G42DataKey::Copy(ref);
	}
}
void
G42ImageData::CopyInfo(G42ImageData & ref)
{
	if (&ref != this)
	{
		SetInfo(ref.StripNum, ref.NumRows, ref.RowBytes, ref.StripStartInImage,
			ref.Width, ref.TransRowBytes);
	}
}
G42ImageDataColor8::G42ConvertToGray8::G42ConvertToGray8(
	const G42Color * palette, intf num_palette)
:
	ColorPalette(palette)
{
	GrayPalette.Set(new byte [num_palette]);
	for (intf i = 0; i < num_palette; i++)
	{
		GrayPalette[i] = (byte)((
			(uint32)ColorPalette[i].Red * (uint32)306 +
			(uint32)ColorPalette[i].Green * (uint32)601 +
			(uint32)ColorPalette[i].Blue * (uint32)117 +
			(uint32)512) >> 10);
	}
}
G42ImageDataColor8::G42ConvertToGray8::~G42ConvertToGray8(void)
{
}
G42ImageData *
G42ImageDataColor8::MakeCopy(void)
{
	G42ImageDataColor8 * copy = new G42ImageDataColor8(*this);
	G42ThrowIfNull(copy);
	return copy;
}
G42ImageData *
G42ImageDataColor24::MakeCopy(void)
{
	G42ImageDataColor24 * copy = new G42ImageDataColor24(*this);
	G42ThrowIfNull(copy);
	return copy;
}
G42ImageData *
G42ImageDataColor32::MakeCopy(void)
{
	G42ImageDataColor32 * copy = new G42ImageDataColor32(*this);
	G42ThrowIfNull(copy);
	return copy;
}
G42ImageData *
G42ImageDataGray8::MakeCopy(void)
{
	G42ImageDataGray8 * copy = new G42ImageDataGray8(*this);
	G42ThrowIfNull(copy);
	return copy;
}
G42ImageData *
G42ImageDataGray1::MakeCopy(void)
{
	G42ImageDataGray1 * copy = new G42ImageDataGray1(*this);
	G42ThrowIfNull(copy);
	return copy;
}
