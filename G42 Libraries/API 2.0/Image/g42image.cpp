// g42image.cpp - Image class
#include <g42itype.h>
#include <g42image.h>
#include <g42iview.h>
#include <g42iter.h>
G42Image::G42Image(G42Data * data_core, G42EventUpdateHandler * event_handler) :
	NumViewArray(0), Data(0), Info(0), DataCore(data_core), Valid(true),
	Dirty(false), GetRowBuffer(0), GetRowBufferSize(0), GetMaskRowBufferSize(0),
	Mask(0), GetMaskRowBuffer(0),
   EventHandler(event_handler), OwnEventHandler(false)
{
	if (!EventHandler)
   {
		EventHandler = new DefaultEventHandler();
      OwnEventHandler = true;
   }
   if (!EventHandler)
   	Valid = false;
}
G42Image::~G42Image()
{
	if (Data)
	{
		delete Data;
		Data = 0;
	}
	if (EventHandler && OwnEventHandler)
   {
   	delete EventHandler;
      EventHandler = 0;
   }
   
	if (Mask)
	{
		delete Mask;
      Data = 0;
	}
	if (Info)
		delete Info;
	Info = 0;
	if (GetRowBuffer)
	{
		delete [] GetRowBuffer;
		GetRowBuffer = 0;
	}
	if (GetMaskRowBuffer)
	{
		delete [] GetMaskRowBuffer;
		GetMaskRowBuffer = 0;
	}
	for (int i = 0; i < NumViewArray; i++)
	{
		ViewArray[i]->ImageBeingDeleted(this);
	}
}
G42Image::G42Image(G42Image & Ref) :
	NumViewArray(0), Data(0), Info(0), DataCore(Ref.DataCore), Valid(true),
	Dirty(Ref.Dirty), GetRowBuffer(0), Mask(0), GetMaskRowBuffer(0),
	GetRowBufferSize(0), EventHandler(0), OwnEventHandler(false),
	GetMaskRowBufferSize(0)
{
	if (Ref.Info)
		Info = new G42ImageInfo(*Ref.Info);
	else
		Info = 0;
	Valid = Ref.Valid;
	if (Ref.Data)
		Data = new G42ImageData(*Ref.Data);
	else
		Data = 0;
	if (Ref.Mask)
		Mask = new G42ImageData(*Ref.Mask);
	else
		Mask = 0;
   if (Ref.OwnsEventHandler())
   {
   	EventHandler = new DefaultEventHandler();
      OwnEventHandler = true;
   }
   else
   	EventHandler = Ref.GetEventHandler();
}
G42Image::G42Image(G42Image & Ref,
	uint32 x, uint32 y, uint32 width, uint32 height) :
		NumViewArray(0), Data(0), Info(0), DataCore(Ref.DataCore), Valid(true),
		GetRowBuffer(0), Dirty(true), Mask(0), GetMaskRowBuffer(0),
		GetRowBufferSize(0), EventHandler(0), OwnEventHandler(false),
		GetMaskRowBufferSize(0)
{
	if (Ref.Info && Ref.Data)
	{
		Info = new G42ImageInfo(*Ref.Info);
		Info->Width = width;
		Info->Height = height;
		Valid = Ref.Valid;
		Data = new G42ImageData(*Ref.Data, x, y, width, height);
		if (Ref.Mask)
			Mask = new G42ImageData(*Ref.Mask, x, y, width, height);
	}
	else
	{
		Valid = false;
      return;
	}
   if (Ref.OwnsEventHandler())
   {
   	EventHandler = new DefaultEventHandler();
      OwnEventHandler = true;
   }
   else
   	EventHandler = Ref.GetEventHandler();
}
G42Image &
G42Image::operator= (G42Image & Ref)
{
	if (&Ref == this)
		return *this;
	if (Info)
		delete Info;
	if (Data)
		delete Data;
	if (Mask)
		delete Mask;
	DataCore = Ref.DataCore;
	if (Ref.Info)
		Info = new G42ImageInfo(*Ref.Info);
	else
		Info = 0;
	if (Ref.Data)
		Data = new G42ImageData(*Ref.Data);
	else
		Data = 0;
	if (Ref.Mask)
		Mask = new G42ImageData(*Ref.Mask);
	else
		Mask = 0;
   if (Ref.OwnsEventHandler())
   {
   	EventHandler = new DefaultEventHandler();
      OwnEventHandler = true;
   }
   else
   {
   	EventHandler = Ref.GetEventHandler();
      OwnEventHandler = false;
   }
	Valid = Ref.Valid;
	Dirty = Ref.Dirty;
	for (int i = 0; i < NumViewArray; i++)
	{
		ViewArray[i]->ImageChanged();
	}
	return *this;
}
G42Image::G42Image(G42Data * data_core, const G42ImageInfo * info,
	bool partial, G42EventUpdateHandler * event_handler) :
		NumViewArray(0), Data(0), Info(0), DataCore(data_core), Valid(true),
		Dirty(false), GetRowBuffer(0), Mask(0), GetMaskRowBuffer(0),
		GetRowBufferSize(0), EventHandler(event_handler), OwnEventHandler(false),
		GetMaskRowBufferSize(0)
{
	Info = new G42ImageInfo(*info);
	Data = new G42ImageData(DataCore, Info->Width, Info->Height,
		Info->PixelDepth, Info->NumPalette, partial);
	if (!Data)
	{
		Valid = false;
		return;
	}
	if (Info->NumPalette)
		Data->SetPalette(Info->ImagePalette);
	Data->SetFinished(true);
	Data->SetValidStartRow(0);
	Data->SetValidNumRows(Info->Height);
	if (info->IsTransparent)
	{
		Mask = new G42ImageData(DataCore, Info->Width, Info->Height,
			1, 0, partial);
		if (!Mask)
		{
			Valid = false;
			return;
		}
		Mask->SetFinished(true);
		Mask->SetValidStartRow(0);
		Mask->SetValidNumRows(Info->Height);
	}
   if (!EventHandler)
   {
   	EventHandler = new DefaultEventHandler();
      OwnEventHandler = true;
   }
   if (!EventHandler)
   {
   	Valid = false;
      return;
   }
}
void
G42Image::SetPartial(uint x, uint y, uint width, uint height)
{
	if (Data)
		Data->SetPartial(x, y, width, height);
}
bool
G42Image::NeedPartialUpdate(void)
{
	if (Data)
		return Data->NeedPartialUpdate();
	return false;
}
uint
G42Image::GetPartialUpdateTileX(void)
{
	if (Data)
		return Data->GetPartialUpdateTileX();
	return 0;
}
uint
G42Image::GetPartialUpdateTileY(void)
{
	if (Data)
		return Data->GetPartialUpdateTileY();
	return 0;
}
uint
G42Image::GetPartialUpdateTileWidth(void)
{
	if (Data)
		return Data->GetPartialUpdateTileWidth();
	return 0;
}
uint
G42Image::GetPartialUpdateTileHeight(void)
{
	if (Data)
		return Data->GetPartialUpdateTileHeight();
	return 0;
}
void
G42Image::UpdatePartial(void)
{
	if (Data)
		Data->UpdatePartial();
}
void
G42Image::AddView(G42ImageViewer * view)
{
	ViewArray[NumViewArray] = view;
	NumViewArray++;
}
void G42Image::RemoveView(G42ImageViewer * view)
{
	for (int i = 0; i < NumViewArray; i++)
	{
		if (ViewArray[i] == view)
		{
			NumViewArray--;
			while (i < NumViewArray)
			{
				ViewArray[i] = ViewArray[i + 1];
				i++;
			}
			break;
		}
	}
}
void G42Image::SetInfo(G42ImageInfo & info)
{
	if (Info)
	{
		delete Info;
		Info = 0;
	}
	if (Data)
	{
		delete Data;
		Data = 0;
	}
	if (Mask)
	{
		delete Mask;
		Mask = 0;
	}
	Info = new G42ImageInfo(info);
	Data = new G42ImageData(DataCore, Info->Width, Info->Height,
		Info->PixelDepth, Info->NumPalette);
	if (!Data)
	{
		Valid = false;
		return;
	}
	if (Info->NumPalette)
		Data->SetPalette(Info->ImagePalette);
	if (Info->IsTransparent)
	{
		Mask = new G42ImageData(DataCore, Info->Width, Info->Height,
			1, 0);
		if (!Mask)
		{
			Valid = false;
			return;
		}
		Mask->SetFinished(true);
		Mask->SetValidStartRow(0);
		Mask->SetValidNumRows(Info->Height);
	}
	for (int i = 0; i < NumViewArray; i++)
	{
		ViewArray[i]->HaveImageInfo();
	}
}
byte *
G42Image::GetRow(uint32 row_num, uint32 offset, // rows in any order
	uint length, int depth) // offset and length in pixels
{
	if (!depth)
		depth = Data->GetDepth();
	if (depth > 8)
		depth = 8 * TrueColorPixelSize;
	uint32 new_size = (uint32)((Data->GetWidth() * max((uint32)depth,
		(uint32)Data->GetDepth()) + 7) >> 3);
	if (!GetRowBuffer || new_size > GetRowBufferSize)
	{
		if (GetRowBuffer)
			delete [] GetRowBuffer;
		GetRowBuffer = new byte [(uint)(new_size)];
		if (!GetRowBuffer)
			return 0;
		GetRowBufferSize = new_size;
	}
	uint tile_row = (uint)((row_num + (uint32)Data->GetFirstTileOffsetY()) /
		(uint32)Data->GetTileHeight());
	uint tile_row_num;
	if (tile_row)
		tile_row_num = (uint)(row_num - ((uint32)Data->GetTileHeight() *
			(uint32)tile_row - (uint32)Data->GetFirstTileOffsetY()));
	else
		tile_row_num = (uint)row_num + Data->GetFirstTileOffsetY();
	uint tile_column_start = (uint)((offset +
		(uint32)Data->GetFirstTileOffsetX()) /
		(uint32)Data->GetTileWidth());
	uint tile_column_end = (uint)((offset + length - 1 +
		(uint32)Data->GetFirstTileOffsetX()) /
		(uint32)Data->GetTileWidth());
	uint tile_column_offset;
	if (tile_column_start)
		tile_column_offset = (uint)(offset -
			((uint32)Data->GetTileWidth() *
			(uint32)tile_column_start - (uint32)Data->GetFirstTileOffsetX()));
	else
		tile_column_offset = (uint)offset + Data->GetFirstTileOffsetX();
	uint tile_column_last;
	if (tile_column_end)
		tile_column_last = (uint)(offset + length -
			((uint32)Data->GetTileWidth() *
			(uint32)(tile_column_end) - (uint32)Data->GetFirstTileOffsetX()));
	else
		tile_column_last = (uint)offset + length + Data->GetFirstTileOffsetX();
	byte * row_ptr = GetRowBuffer;
	uint tile_column_width = Data->GetTileWidth();
	int row_offset = 0;
	for (int column = tile_column_start; column <= tile_column_end; column++)
	{
		if (column == tile_column_end)
			tile_column_width = tile_column_last;
		G42LockedImageTile tile(Data->GetTile(tile_row, column));
		G42ImageRow * irow = tile.GetTile()->GetRow(tile_row_num);
		if (Data->GetDepth() >= 8)
		{
			uint bytes = (uint)(((uint32)(tile_column_width -
				tile_column_offset) * (uint32)Data->GetDepth() + 7) >> 3);
			memcpy(row_ptr,
				*irow + ((tile_column_offset * Data->GetDepth() + 7) >> 3), bytes);
			row_ptr += bytes;
		}
		else if (Data->GetDepth() == 4)
		{
			if ((tile_column_offset & 1) == row_offset)
			{
				uint bytes;
				byte * sptr = *irow + (tile_column_offset >> 1);
				if (row_offset)
				{
					bytes = ((tile_column_width - tile_column_offset) >> 1);
					*row_ptr &= 0xf0;
					*row_ptr |= *sptr & 0xf;
					row_ptr++;
					sptr++;
				}
				else
				{
					bytes = ((tile_column_width - tile_column_offset + 1) >> 1);
				}
				memcpy(row_ptr, sptr, bytes);
				row_ptr += bytes;
				row_offset = (tile_column_width - tile_column_offset +
					row_offset) & 1;
			}
			else
			{
				byte * sptr = *irow + (tile_column_offset >> 1);
				int ss = 4 - (4 * (tile_column_offset & 1));
				int ds = 4 * row_offset;
				for (uint i = 0; i < tile_column_width - tile_column_offset; i++)
				{
					int v = ((*sptr) >> ss) & 0xf;
					*row_ptr &= (0xf0 >> ds);
					*row_ptr |= (v << ds);
					if (ss)
					{
						ss = 0;
					}
					else
					{
						ss = 4;
						sptr++;
					}
					if (ds)
					{
						ds = 0;
					}
					else
					{
						ds = 4;
						row_ptr++;
					}
				}
				row_offset = 1 - (ds >> 2);
			}
		}
		else // monochrome
		{
			if ((tile_column_offset & 7) == row_offset)
			{
				uint bytes;
				byte * sptr = *irow + (tile_column_offset >> 3);
				if (row_offset)
				{
					bytes = ((tile_column_width - tile_column_offset) >> 3);
					*row_ptr &= (0xff << (8 - row_offset)) & 0xff;
					*row_ptr |= *sptr & (0xff >> row_offset);
					row_ptr++;
					sptr++;
				}
				else
				{
					bytes = ((tile_column_width - tile_column_offset + 7) >> 3);
				}
				memcpy(row_ptr, sptr, bytes);
				row_ptr += bytes;
				row_offset = (tile_column_width - tile_column_offset +
					row_offset) & 7;
			}
			else
			{
				byte * sptr = *irow + (tile_column_offset >> 3);
				int ss = 1 << (7 - (tile_column_offset & 7));
				int ds = 1 << (7 - row_offset);
				for (uint i = 0; i < tile_column_width - tile_column_offset; i++)
				{
					if ((*sptr) & ss)
						*row_ptr |= ds;
					else
						*row_ptr &= ~ds;
					if (ss == 1)
					{
						ss = 0x80;
						sptr++;
					}
					else
					{
						ss >>= 1;
					}
					if (ds > 1)
					{
						ds >>= 1;
					}
					else
					{
						ds = 0x80;
						row_ptr++;
					}
				}
				row_offset = 7;
				ds >>= 1;
				while (ds)
				{
					row_offset--;
					ds >>= 1;
				}
			}
		}
		tile_column_offset = 0;
	}
	if (depth != Data->GetDepth())
	{
		if (depth > 8)
		{
			if (Data->GetDepth() == 8 && Data->GetNumPalette())
			{
				const G42Color * pal = Data->GetPalette();
				G42Byte24BitIterator dp(GetRowBuffer);
				dp.Forward(length - 1);
				byte * sp = GetRowBuffer + length - 1;
				for (uint32 i = 0; i < length; i++)
				{
					dp.Set(pal[*sp].blue, pal[*sp].green, pal[*sp].red);
					sp--;
					dp.Decrement();
				}
			}
			else if (Data->GetDepth() == 8)
			{
				G42Byte24BitIterator dp(GetRowBuffer);
				dp.Forward(length - 1);
				byte * sp = GetRowBuffer + length - 1;
				for (uint32 i = 0; i < length; i++)
				{
					dp.Set(*sp, *sp, *sp);
					sp--;
					dp.Decrement();
				}
			}
			else if (Data->GetDepth() == 4 && Data->GetNumPalette())
			{
				const G42Color * pal = Data->GetPalette();
				G42Byte24BitIterator dp(GetRowBuffer);
				dp.Forward(length - 1);
				byte * sp = GetRowBuffer + ((length - 1) >> 1);
				int ss = ((length & 1) << 2);
				for (uint32 i = 0; i < length; i++)
				{
					int v = ((*sp >> ss) & 0xf);
					dp.Set(pal[v].blue, pal[v].green, pal[v].red);
					if (!ss)
					{
						ss = 4;
					}
					else
					{
						sp--;
						ss = 0;
					}
					dp.Decrement();
				}
			}
			else if (Data->GetDepth() == 4)
			{
				G42Byte24BitIterator dp(GetRowBuffer);
				dp.Forward(length - 1);
				byte * sp = GetRowBuffer + ((length - 1) >> 1);
				int ss = ((length & 1) << 2);
				for (uint32 i = 0; i < length; i++)
				{
					int v = ((*sp >> ss) & 0xf);
					dp.Set(v, v, v);
					if (!ss)
					{
						ss = 4;
					}
					else
					{
						sp--;
						ss = 0;
					}
					dp.Decrement();
				}
			}
			else if (Data->GetDepth() == 1 && Data->GetNumPalette())
			{
				const G42Color * pal = Data->GetPalette();
				G42Byte24BitIterator dp(GetRowBuffer);
				dp.Forward(length - 1);
				byte * sp = GetRowBuffer + ((length - 1) >> 3);
				int ss = (7 - ((length - 1) & 7));
				for (uint32 i = 0; i < length; i++)
				{
					int v = ((*sp >> ss) & 1);
					dp.Set(pal[v].blue, pal[v].green, pal[v].red);
					if (ss < 7)
					{
						ss++;
					}
					else
					{
						sp--;
						ss = 0;
					}
					dp.Decrement();
				}
			}
			else if (Data->GetDepth() == 1)
			{
				G42Byte24BitIterator dp(GetRowBuffer);
				dp.Forward(length - 1);
				byte * sp = GetRowBuffer + ((length - 1) >> 3);
				int ss = (7 - ((length - 1) & 7));
				for (uint32 i = 0; i < length; i++)
				{
					int v = ((*sp >> ss) & 1);
					dp.Set(v, v, v);
					if (ss < 7)
					{
						ss++;
					}
					else
					{
						sp--;
						ss = 0;
					}
					dp.Decrement();
				}
			}
		}
		else if (depth == 8)
		{
			if (Data->GetDepth() == 4 && Data->GetNumPalette())
			{
				byte * dp = GetRowBuffer + length - 1;
				byte * sp = GetRowBuffer + ((length - 1) >> 1);
				int ss = ((length & 1) << 2);
				for (uint32 i = 0; i < length; i++)
				{
					int v = ((*sp >> ss) & 0xf);
					*dp-- = v;
					if (!ss)
					{
						ss = 4;
					}
					else
					{
						sp--;
						ss = 0;
					}
				}
			}
			else if (Data->GetDepth() == 4)
			{
				byte * dp = GetRowBuffer + length - 1;
				byte * sp = GetRowBuffer + ((length - 1) >> 1);
				int ss = ((length & 1) << 2);
				for (uint32 i = 0; i < length; i++)
				{
					int v = ((*sp >> ss) & 0xf);
					*dp-- = v | (v << 4);
					if (!ss)
					{
						ss = 4;
					}
					else
					{
						sp--;
						ss = 0;
					}
				}
			}
			else if (Data->GetDepth() == 1 && Data->GetNumPalette())
			{
				byte * dp = GetRowBuffer + length - 1;
				byte * sp = GetRowBuffer + ((length - 1) >> 3);
				int ss = (7 - ((length - 1) & 7));
				for (uint32 i = 0; i < length; i++)
				{
					int v = ((*sp >> ss) & 1);
					*dp-- = v;
					if (ss < 7)
					{
						ss++;
					}
					else
					{
						sp--;
						ss = 0;
					}
				}
			}
			else if (Data->GetDepth() == 1)
			{
				byte * dp = GetRowBuffer + length - 1;
				byte * sp = GetRowBuffer + ((length - 1) >> 3);
				int ss = (7 - ((length - 1) & 7));
				for (uint32 i = 0; i < length; i++)
				{
					int v = ((*sp >> ss) & 1);
					if (v)
						*dp-- = 0xff;
					else
						*dp-- = 0;
					if (ss < 7)
					{
						ss++;
					}
					else
					{
						sp--;
						ss = 0;
					}
				}
			}
		}
		else if (depth == 4)
		{
			if (Data->GetDepth() == 1 && Data->GetNumPalette())
			{
				byte * dp = GetRowBuffer + ((length - 1) >> 1);
				*dp = 0;
				int ds = ((length & 1) << 2);
				byte * sp = GetRowBuffer + ((length - 1) >> 3);
				int ss = (7 - ((length - 1) & 7));
				for (uint32 i = 0; i < length; i++)
				{
					int v = ((*sp >> ss) & 1);
					*dp |= (v << ds);
					if (ss < 7)
					{
						ss++;
					}
					else
					{
						sp--;
						ss = 0;
					}
					if (!ds)
					{
						ds = 4;
					}
					else
					{
						dp--;
						*dp = 0;
						ds = 0;
					}
				}
			}
			else if (Data->GetDepth() == 1)
			{
				byte * dp = GetRowBuffer + ((length - 1) >> 1);
				*dp = 0;
				int ds = ((length & 1) << 2);
				byte * sp = GetRowBuffer + ((length - 1) >> 3);
				int ss = (7 - ((length - 1) & 7));
				for (uint32 i = 0; i < length; i++)
				{
					int v = ((*sp >> ss) & 1);
					if (v)
						*dp |= (0xf << ds);
					if (ss < 7)
					{
						ss++;
					}
					else
					{
						sp--;
						ss = 0;
					}
					if (!ds)
					{
						ds = 4;
					}
					else
					{
						dp--;
						*dp = 0;
						ds = 0;
					}
				}
			}
		}
	}
	return GetRowBuffer;
}
void
G42Image::SetRow(byte * row, uint32 row_num, uint32 offset, // rows in any order
		uint length) // offset and length in pixels
{
	uint tile_row = (uint)((row_num + (uint32)Data->GetFirstTileOffsetY()) /
		(uint32)Data->GetTileHeight());
	uint tile_row_num;
	if (tile_row)
		tile_row_num = (uint)(row_num - ((uint32)Data->GetTileHeight() *
			(uint32)tile_row - (uint32)Data->GetFirstTileOffsetY()));
	else
		tile_row_num = (uint)row_num + Data->GetFirstTileOffsetY();
	uint tile_column_start = (uint)((offset +
		(uint32)Data->GetFirstTileOffsetX()) /
		(uint32)Data->GetTileWidth());
	uint tile_column_end = (uint)((offset + length - 1 +
		(uint32)Data->GetFirstTileOffsetX()) /
		(uint32)Data->GetTileWidth());
	uint tile_column_offset;
	if (tile_column_start)
		tile_column_offset = (uint)(offset -
			((uint32)Data->GetTileWidth() *
			(uint32)tile_column_start - (uint32)Data->GetFirstTileOffsetX()));
	else
		tile_column_offset = (uint)offset + Data->GetFirstTileOffsetX();
	uint tile_column_last;
	if (tile_column_end)
		tile_column_last = (uint)(offset + length -
			((uint32)Data->GetTileWidth() *
			(uint32)(tile_column_end) - (uint32)Data->GetFirstTileOffsetX()));
	else
		tile_column_last = (uint)offset + length + Data->GetFirstTileOffsetX();
	byte * row_ptr = row;
	uint tile_column_width = Data->GetTileWidth();
	for (int column = tile_column_start; column <= tile_column_end; column++)
	{
		if (column == tile_column_end)
			tile_column_width = tile_column_last;
		G42LockedImageTile tile(Data->GetTile(tile_row, column));
		G42ImageRow * irow = tile.GetTile()->GetRow(tile_row_num);
		uint bytes = (uint)(((uint32)(tile_column_width -
			tile_column_offset) * (uint32)Data->GetDepth() + 7) >> 3);
		memcpy(*irow, row_ptr, bytes);
		row_ptr += bytes;
		tile_column_offset = 0;
		irow->MarkModified();
	}
}
byte *
G42Image::GetMaskRow(uint32 row_num, uint32 offset, // rows in any order
	uint length) // offset and length in pixels
{
	uint32 new_size = (uint32)((Mask->GetWidth() *
		(uint32)Mask->GetDepth() + 7) >> 3);
	if (!GetMaskRowBuffer || new_size > GetMaskRowBufferSize)
	{
		if (GetMaskRowBuffer)
			delete [] GetMaskRowBuffer;
		GetMaskRowBuffer = new byte [(uint)(new_size)];
		if (!GetMaskRowBuffer)
			return 0;
		GetMaskRowBufferSize = new_size;
	}
	uint tile_row = (uint)((row_num + (uint32)Mask->GetFirstTileOffsetY()) /
		(uint32)Mask->GetTileHeight());
	uint tile_row_num;
	if (tile_row)
		tile_row_num = (uint)(row_num - ((uint32)Mask->GetTileHeight() *
			(uint32)tile_row - (uint32)Mask->GetFirstTileOffsetY()));
	else
		tile_row_num = (uint)row_num + Mask->GetFirstTileOffsetY();
	uint tile_column_start = (uint)((offset +
		(uint32)Mask->GetFirstTileOffsetX()) /
		(uint32)Mask->GetTileWidth());
	uint tile_column_end = (uint)((offset + length - 1 +
		(uint32)Mask->GetFirstTileOffsetX()) /
		(uint32)Mask->GetTileWidth());
	uint tile_column_offset;
	if (tile_column_start)
		tile_column_offset = (uint)(offset -
			((uint32)Mask->GetTileWidth() *
			(uint32)tile_column_start - (uint32)Mask->GetFirstTileOffsetX()));
	else
		tile_column_offset = (uint)offset + Mask->GetFirstTileOffsetX();
	uint tile_column_last;
	if (tile_column_end)
		tile_column_last = (uint)(offset + length -
			((uint32)Mask->GetTileWidth() *
			(uint32)(tile_column_end) - (uint32)Mask->GetFirstTileOffsetX()));
	else
		tile_column_last = (uint)offset + length + Mask->GetFirstTileOffsetX();
	byte * row_ptr = GetMaskRowBuffer;
	uint tile_column_width = Mask->GetTileWidth();
	for (int column = tile_column_start; column <= tile_column_end; column++)
	{
		if (column == tile_column_end)
			tile_column_width = tile_column_last;
		G42LockedImageTile tile(Mask->GetTile(tile_row, column));
		G42ImageRow * irow = tile.GetTile()->GetRow(tile_row_num);
		uint bytes = (uint)(((uint32)(tile_column_width -
			tile_column_offset) * (uint32)Mask->GetDepth() + 7) >> 3);
		memcpy(row_ptr, *irow, bytes);
		row_ptr += bytes;
		tile_column_offset = 0;
	}
	return GetMaskRowBuffer;
#if 0
	if (!GetMaskRowBuffer)
	{
		GetMaskRowBuffer = new byte [(uint)((Mask->GetWidth() *
			Mask->GetDepth() + 7) >> 3)];
		if (!GetMaskRowBuffer)
      	return 0;
	}
	uint tile_row = (uint)((row_num + (uint32)Mask->GetFirstTileOffsetY()) /
		(uint32)Mask->GetTileHeight());
	uint tile_row_num;
	if (tile_row)
		tile_row_num = (uint)(row_num - ((uint32)Mask->GetTileHeight() *
			(uint32)tile_row - (uint32)Mask->GetFirstTileOffsetY()));
	else
		tile_row_num = (uint)row_num + Mask->GetFirstTileOffsetY();
	uint tile_column_start = (uint)((offset +
		(uint32)Mask->GetFirstTileOffsetX()) /
		(uint32)Mask->GetTileWidth());
	uint tile_column_end = (uint)((offset + length - 1 +
		(uint32)Mask->GetFirstTileOffsetX()) /
		(uint32)Mask->GetTileWidth());
	uint tile_column_offset;
	if (tile_column_start)
		tile_column_offset = (uint)(offset -
			((uint32)Mask->GetTileWidth() *
			(uint32)tile_column_start - (uint32)Mask->GetFirstTileOffsetX()));
	else
		tile_column_offset = (uint)offset + Mask->GetFirstTileOffsetX();
	uint tile_column_last;
	if (tile_column_end)
		tile_column_last = (uint)(offset + length -
			((uint32)Mask->GetTileWidth() *
			(uint32)(tile_column_end) - (uint32)Mask->GetFirstTileOffsetX()));
	else
		tile_column_last = (uint)offset + length + Mask->GetFirstTileOffsetX();
	byte * row_ptr = GetMaskRowBuffer;
	uint tile_column_width = Mask->GetTileWidth();
	for (int column = tile_column_start; column <= tile_column_end; column++)
	{
		if (column == tile_column_end)
			tile_column_width = tile_column_last;
		G42LockedImageTile tile(Mask->GetTile(tile_row, column));
		G42ImageRow * irow = tile.GetTile()->GetRow(tile_row_num);
		uint bytes = (uint)(((uint32)(tile_column_width -
			tile_column_offset) * (uint32)Mask->GetDepth() + 7) >> 3);
		memcpy(row_ptr, *irow, bytes);
		row_ptr += bytes;
		tile_column_offset = 0;
	}
	return GetMaskRowBuffer;
#endif	
}
void
G42Image::SetMaskRow(byte * row, uint32 row_num, uint32 offset, // rows in any order
		uint length) // offset and length in pixels
{
	uint tile_row = (uint)((row_num + (uint32)Mask->GetFirstTileOffsetY()) /
		(uint32)Mask->GetTileHeight());
	uint tile_row_num;
	if (tile_row)
		tile_row_num = (uint)(row_num - ((uint32)Mask->GetTileHeight() *
			(uint32)tile_row - (uint32)Mask->GetFirstTileOffsetY()));
	else
		tile_row_num = (uint)row_num + Mask->GetFirstTileOffsetY();
	uint tile_column_start = (uint)((offset +
		(uint32)Mask->GetFirstTileOffsetX()) /
		(uint32)Mask->GetTileWidth());
	uint tile_column_end = (uint)((offset + length - 1 +
		(uint32)Mask->GetFirstTileOffsetX()) /
		(uint32)Mask->GetTileWidth());
	uint tile_column_offset;
	if (tile_column_start)
		tile_column_offset = (uint)(offset -
			((uint32)Mask->GetTileWidth() *
			(uint32)tile_column_start - (uint32)Mask->GetFirstTileOffsetX()));
	else
		tile_column_offset = (uint)offset + Mask->GetFirstTileOffsetX();
	uint tile_column_last;
	if (tile_column_end)
		tile_column_last = (uint)(offset + length -
			((uint32)Mask->GetTileWidth() *
			(uint32)(tile_column_end) - (uint32)Mask->GetFirstTileOffsetX()));
	else
		tile_column_last = (uint)offset + length + Mask->GetFirstTileOffsetX();
	byte * row_ptr = row;
	uint tile_column_width = Mask->GetTileWidth();
	for (int column = tile_column_start; column <= tile_column_end; column++)
	{
		if (column == tile_column_end)
			tile_column_width = tile_column_last;
		G42LockedImageTile tile(Mask->GetTile(tile_row, column));
		G42ImageRow * irow = tile.GetTile()->GetRow(tile_row_num);
		uint bytes = (uint)(((uint32)(tile_column_width -
			tile_column_offset) * (uint32)Mask->GetDepth() + 7) >> 3);
		memcpy(*irow, row_ptr, bytes);
		row_ptr += bytes;
		tile_column_offset = 0;
		irow->MarkModified();
	}
#if 0
	uint tile_row = (uint)((row_num + (uint32)Mask->GetFirstTileOffsetY()) /
		(uint32)Mask->GetTileHeight());
	uint tile_row_num;
	if (tile_row)
		tile_row_num = (uint)(row_num - ((uint32)Mask->GetTileHeight() *
			(uint32)tile_row - (uint32)Mask->GetFirstTileOffsetY()));
	else
		tile_row_num = (uint)row_num + Mask->GetFirstTileOffsetY();
	uint tile_column_start = (uint)((offset +
		(uint32)Mask->GetFirstTileOffsetX()) /
		(uint32)Mask->GetTileWidth());
	uint tile_column_end = (uint)((offset + length - 1 +
		(uint32)Mask->GetFirstTileOffsetX()) /
		(uint32)Mask->GetTileWidth());
	uint tile_column_offset;
	if (tile_column_start)
		tile_column_offset = (uint)(offset -
			((uint32)Mask->GetTileWidth() *
			(uint32)tile_column_start - (uint32)Mask->GetFirstTileOffsetX()));
	else
		tile_column_offset = (uint)offset + Mask->GetFirstTileOffsetX();
	uint tile_column_last;
	if (tile_column_end)
		tile_column_last = (uint)(offset + length -
			((uint32)Mask->GetTileWidth() *
			(uint32)(tile_column_end) - (uint32)Mask->GetFirstTileOffsetX()));
	else
		tile_column_last = (uint)offset + length + Mask->GetFirstTileOffsetX();
	byte * row_ptr = row;
	uint tile_column_width = Mask->GetTileWidth();
	for (int column = tile_column_start; column <= tile_column_end; column++)
	{
		if (column == tile_column_end)
			tile_column_width = tile_column_last;
		G42LockedImageTile tile(Mask->GetTile(tile_row, column));
		G42ImageRow * irow = tile.GetTile()->GetRow(tile_row_num);
		uint bytes = (uint)(((uint32)(tile_column_width -
			tile_column_offset) * (uint32)Mask->GetDepth() + 7) >> 3);
		memcpy(*irow, row_ptr, bytes);
		row_ptr += bytes;
		tile_column_offset = 0;
		irow->MarkModified();
	}
#endif
}
void
G42Image::MarkRowValid(uint32 row)
{
	if (!Data || Data->IsFinished())
		return;
	const uint32 start_row = Data->GetValidStartRow();		
	if (row < start_row)
	{
		Data->SetValidStartRow(row);
		Data->SetValidNumRows(Data->GetValidNumRows() + start_row - row);
	}
	else if (row >= Data->GetValidStartRow() + Data->GetValidNumRows())
	{
		Data->SetValidNumRows(row - start_row + 1);
	}
	for (int i = 0; i < NumViewArray; i++)
	{
		ViewArray[i]->HaveImageRow(row);
	}
}
void
G42Image::EndImage(G42ImageInfo & info)
{
	if (info.ImageType == Pcx && info.NumPalette && Data)
	{
		Data->SetPalette(info.ImagePalette);
		RebuildInfo();
	}
	if (Data)
		Data->SetFinished(true);
	if (Mask)
		Mask->SetFinished(true);
	for (int i = 0; i < NumViewArray; i++)
	{
		ViewArray[i]->HaveImageEnd();
	}
}
void
G42Image::RebuildInfo(void)
{
	if (!Data || !Info)
   	return;
   	if (Mask)
   		Info->IsTransparent = true;
   	else
   		Info->IsTransparent = false;
	Info->Width = Data->GetWidth();
	Info->Height = Data->GetHeight();
	Info->PixelDepth = Data->GetDepth();
	if (Info->PixelDepth > 8)
		Info->ColorType = G42ImageInfo::ColorTypeTrueColor;
	else if (Data->GetNumPalette())
		Info->ColorType = G42ImageInfo::ColorTypePalette;
	else
		Info->ColorType = G42ImageInfo::ColorTypeGrayscale;
	if (Info->NumPalette < Data->GetNumPalette())
	{
		Info->ImagePalette.Clear();
		if (Data->GetNumPalette())
		{
			G42Color * pal = new G42Color[Data->GetNumPalette()];
			Info->ImagePalette.Set(pal);
		}
		else
		{
			return;
		}
	}
	Info->NumPalette = Data->GetNumPalette();
	for (int i = 0; i < Info->NumPalette; i++)
	{
		((G42Color *)(Info->ImagePalette))[i] = Data->GetPalette()[i];
	}
}
