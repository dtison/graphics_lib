#include "g42iter.h"
G42ImageData1BitIterator::G42ImageData1BitIterator(G42ImageData * data, bool init)
	: Data(data), CurrentPtr(0), CurrentRow(0), CurrentColumn(0),
	CurrentTileRow(0), Offset(0), Mask(0x80), MinColumn(0), MinRow(0), MinOffset(0),
	MinTileRow(0), Tile(0), MaxMask(0), MinMask(0)
{
	MaxColumn = Data->GetNumTilesAcross() - 1;
	MaxRow = Data->GetNumTilesDown() - 1;
	if (init)
	{
		Tile = new G42ModifiableImageTile(Data->GetTile(0, 0));
		CurrentTileRow = Tile->GetTile()->GetTileOffsetY();
      MinTileRow = CurrentTileRow;
		MaxRowInTile = CurrentTileRow + (Tile->GetTile()->GetTileHeight() - 1);
		Row = Tile->GetTile()->GetRow(CurrentTileRow);
		Offset = Tile->GetTile()->GetTileOffsetX() >> 3;
      MinOffset = Offset;
		int remainder = (Tile->GetTile()->GetTileOffsetX() - (MinOffset << 3));
		if (remainder == 0)
			MinMask = 0x80;
		else
      	MinMask = (1 << (7 - remainder));
      Mask = MinMask;
		CurrentPtr = *Row + Offset;
		MaxPtr = *Row + ((Tile->GetTile()->GetTileOffsetX()
			+ Tile->GetTile()->GetTileWidth() - 1) >> 3);
		G42ImageTile * tile = Data->GetTile(0, MaxColumn);
		MaxOffset = ((tile->GetTileOffsetX() + tile ->GetTileWidth() - 1) >> 3);
		remainder = ((tile->GetTileOffsetX() + tile->GetTileWidth() - 1)
			- (MaxOffset << 3));
      if (remainder == 7)
      	MaxMask = 0;
		else
      	MaxMask = (1 << (6 - remainder));
		tile = Data->GetTile(MaxRow, 0);
		MaxTileRow = (tile->GetTileOffsetY() + tile->GetTileHeight() - 1);
	}
}
G42ImageData1BitIterator::~G42ImageData1BitIterator(void)
{
	delete Tile;
}
void
G42ImageData1BitIterator::SetArea(int left, int top, int right, int bottom)
{
	MinRow = Data->GetTileRowIndex(top);
	MaxRow = Data->GetTileRowIndex(bottom);
	MinColumn = Data->GetTileColumnIndex(left);
	MaxColumn = Data->GetTileColumnIndex(right);
	G42ImageTile * tile = Data->GetTile(MinRow, MinColumn);
	MinOffset = ((left - tile->GetImageOffsetX() + tile->GetTileOffsetX()) >> 3);
	int remainder = ((left - tile->GetImageOffsetX() + tile->GetTileOffsetX())
   	- (MinOffset << 3));
	if (remainder == 0)
		MinMask = 0x80;
	else
		MinMask = (1 << (7 - remainder));
	MinTileRow = (top - tile->GetImageOffsetY() + tile->GetTileOffsetY());
	tile = Data->GetTile(MaxRow, MaxColumn);
	MaxOffset = ((right - tile->GetImageOffsetX() + tile->GetTileOffsetX()) >> 3);
	MaxTileRow = bottom - tile->GetImageOffsetY() + tile->GetTileOffsetY();
	remainder = ((right - tile->GetImageOffsetX() + tile->GetTileOffsetX())
		- (MaxOffset << 3));
	if (remainder == 7)
		MaxMask = 0;
	else
		MaxMask = (1 << (6 - remainder));
	Mask = MinMask;
	CurrentColumn = MinColumn;
	Offset = MinOffset;
	CurrentRow = MinRow;
	CurrentTileRow = MinTileRow;
	if (Tile)
   	delete Tile;
	Tile = new G42ModifiableImageTile(Data->GetTile(CurrentRow, CurrentColumn));
	Row = Tile->GetTile()->GetRow(CurrentTileRow);
	CurrentPtr = *Row + Offset;
	if (CurrentColumn == MaxColumn)
		MaxPtr = *Row + MaxOffset;
	else
		MaxPtr = *Row + ((Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1) >> 3);
	if (CurrentRow == MaxRow)
		MaxRowInTile = MaxTileRow;
	else
		MaxRowInTile = (Tile->GetTile()->GetTileOffsetY() +
			Tile->GetTile()->GetTileHeight() - 1);
}
void
G42ImageData1BitIterator::Set8(int val)
{
	if (val)
		*CurrentPtr |= Mask;
	else
   	*CurrentPtr &= ~Mask;
}
bool
G42ImageData1BitIterator::Increment(void)
{
	if ((CurrentColumn == MaxColumn) && (CurrentPtr == MaxPtr))
	{
		Mask >>= 1;
		if (Mask > MaxMask)
			return true;
	}
	else if (Mask > 0x01)
	{
		Mask >>= 1;
      return true;
	}
	CurrentPtr++;
	Offset++;
   Mask = 0x80;
	if (CurrentPtr > MaxPtr)
	{
		CurrentColumn++;
		if (CurrentColumn > MaxColumn)
		{
			CurrentColumn--;
			CurrentPtr--;
			Offset--;
			return false;
		}
      Tile->MarkModified();
		delete Tile;
		Tile = new G42ModifiableImageTile(Data->GetTile(CurrentRow, CurrentColumn));
		Row = Tile->GetTile()->GetRow(CurrentTileRow);
		Offset = 0;
		CurrentPtr = *Row;
		if (CurrentColumn == MaxColumn)
			MaxPtr = *Row + MaxOffset;
		else
			MaxPtr = *Row + ((Tile->GetTile()->GetTileWidth() - 1) >> 3);
	}
	return true;
}
bool
G42ImageData1BitIterator::Increment8(void)
{
	CurrentPtr++;
	Offset++;
	Mask = 0x80;
	if (CurrentPtr > MaxPtr)
	{
		CurrentColumn++;
		if (CurrentColumn > MaxColumn)
		{
			CurrentColumn--;
			CurrentPtr--;
			Offset--;
			return false;
		}
      Tile->MarkModified();
		delete Tile;
		Tile = new G42ModifiableImageTile(Data->GetTile(CurrentRow, CurrentColumn));
		Row = Tile->GetTile()->GetRow(CurrentTileRow);
		Offset = 0;
		CurrentPtr = *Row;
		if (CurrentColumn == MaxColumn)
			MaxPtr = *Row + MaxOffset;
		else
			MaxPtr = *Row + ((Tile->GetTile()->GetTileWidth() - 1) >> 3);
	}
	return true;
}
bool
G42ImageData1BitIterator::Decrement(void)
{
	if ((CurrentColumn == MinColumn) && (Offset == MinOffset))
   {
   	if (Mask < MinMask)
      {
      	Mask <<= 1;
        	return true;
      }
   }
	else if (Mask < 0x80)
	{
		Mask <<= 1;
      return true;
	}
	CurrentPtr--;
	Offset--;
   Mask = 0x01;
	int min_offset = 0;
	if (CurrentColumn == MinColumn)
		min_offset = MinOffset;
	if (Offset < min_offset)
	{
		CurrentColumn--;
		if (CurrentColumn < MinColumn)
		{
			CurrentColumn = MinColumn;
			CurrentPtr++;
			Offset++;
			return false;
		}
		Tile->MarkModified();
		delete Tile;
		Tile = new G42ModifiableImageTile(Data->GetTile(CurrentRow, CurrentColumn));
		Row = Tile->GetTile()->GetRow(CurrentTileRow);
		Offset = ((Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1) >> 3);
		CurrentPtr = *Row + Offset;
		MaxPtr = CurrentPtr;
	}
   return true;
}
bool
G42ImageData1BitIterator::IncRow(void)
{
	CurrentTileRow++;
	if (CurrentTileRow > MaxRowInTile)
	{
		CurrentRow++;
		if (CurrentRow > MaxRow)
		{
			CurrentRow--;
			CurrentTileRow--;
			return false;
		}
		CurrentTileRow = 0;
		delete Tile;
		Tile = new G42ModifiableImageTile(Data->GetTile(CurrentRow, CurrentColumn));
		if (CurrentRow == MaxRow)
			MaxRowInTile = MaxTileRow;
		else
			MaxRowInTile = (Tile->GetTile()->GetTileOffsetY() +
				Tile->GetTile()->GetTileHeight() - 1);
	}
	Row = Tile->GetTile()->GetRow(CurrentTileRow);
	CurrentPtr = *Row + Offset;
	if (CurrentColumn == MaxColumn)
		MaxPtr = *Row + MaxOffset;
	else
		MaxPtr = *Row + ((Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1) >> 3);
	return true;
}
bool
G42ImageData1BitIterator::DecRow(void)
{
	CurrentTileRow--;
	int min_tile_row = 0;
	if (CurrentRow == MinRow)
		min_tile_row = MinTileRow;
	if (CurrentTileRow < min_tile_row)
	{
		CurrentRow--;
		if (CurrentRow < MinRow)
		{
			CurrentRow++;
			CurrentTileRow++;
			return FALSE;
		}
		delete Tile;
		Tile = new G42ModifiableImageTile(Data->GetTile(CurrentRow, CurrentColumn));
		CurrentTileRow = (Tile->GetTile()->GetTileOffsetY() +
			Tile->GetTile()->GetTileHeight() - 1);
		MaxRowInTile = CurrentTileRow;
	}
	Row = Tile->GetTile()->GetRow(CurrentTileRow);
	CurrentPtr = *Row + Offset;
	if (CurrentColumn == MaxColumn)
		MaxPtr = *Row + MaxOffset;
	else
		MaxPtr = *Row + ((Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1) >> 3);
	return true;
}
bool
G42ImageData1BitIterator::NextRow(void)
{
	int old_row = CurrentRow;
	int old_column = CurrentColumn;
	CurrentTileRow++;
	Mask = MinMask;
	if (CurrentTileRow > MaxRowInTile)
	{
		CurrentRow++;
		if (CurrentRow > MaxRow)
		{
			CurrentRow--;
			CurrentTileRow--;
			return false;
		}
		CurrentTileRow = 0;
	}
	CurrentColumn = MinColumn;
	if (CurrentRow != old_row || CurrentColumn != old_column)
	{
		Tile->MarkModified();
		delete Tile;
		Tile = new G42ModifiableImageTile(Data->GetTile(CurrentRow, CurrentColumn));
	}
	if (CurrentRow == MaxRow)
		MaxRowInTile = MaxTileRow;
	else
		MaxRowInTile = (Tile->GetTile()->GetTileOffsetY() +
			Tile->GetTile()->GetTileHeight() - 1);
	Row = Tile->GetTile()->GetRow(CurrentTileRow);
	Offset = MinOffset;
	CurrentPtr = *Row + Offset;
	if (CurrentColumn == MaxColumn)
		MaxPtr = *Row + MaxOffset;
	else
		MaxPtr = *Row + ((Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1) >> 3);
	return true;
}
void
G42ImageData1BitIterator::ResetRow(void)
{
	int old_row = CurrentRow;
	int old_column = CurrentColumn;
	CurrentColumn = MinColumn;
	Mask = MinMask;
	if (CurrentRow != old_row || CurrentColumn != old_column)
	{
		Tile->MarkModified();
		delete Tile;
		Tile = new G42ModifiableImageTile(Data->GetTile(CurrentRow, CurrentColumn));
	}
	Row = Tile->GetTile()->GetRow(CurrentTileRow);
	Offset = MinOffset;
	CurrentPtr = *Row + Offset;
	if (CurrentColumn == MaxColumn)
		MaxPtr = *Row + MaxOffset;
	else
		MaxPtr = *Row + ((Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1) >> 3);
}
void
G42ImageData1BitIterator::SetRowEnd(void)
{
	CurrentColumn = MaxColumn;
	Tile->MarkModified();
	delete Tile;
	Tile = new G42ModifiableImageTile(Data->GetTile(CurrentRow, CurrentColumn));
	Row = Tile->GetTile()->GetRow(CurrentTileRow);
	Offset = MaxOffset;
	CurrentPtr = *Row + Offset;
	MaxPtr = CurrentPtr;
	if (MaxMask == 0)
		Mask = 0x01;
	else
		Mask = (MaxMask << 1);
}
void
G42ImageData1BitIterator::SetLastRow(void)
{
	CurrentRow = MaxRow;
	CurrentTileRow = MaxTileRow;
	Tile->MarkModified();
	delete Tile;
	Tile = new G42ModifiableImageTile(Data->GetTile(CurrentRow, CurrentColumn));
	Row = Tile->GetTile()->GetRow(CurrentTileRow);
	CurrentPtr = *Row + Offset;
	if (CurrentColumn == MaxColumn)
		MaxPtr = *Row + MaxOffset;
	else
		MaxPtr = *Row + ((Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1) >> 3);
}
void
G42ImageData1BitIterator::ToTopRow(void)
{
	CurrentRow = MinRow;
	CurrentTileRow = MinTileRow;
	Tile->MarkModified();
	delete Tile;
	Tile = new G42ModifiableImageTile(Data->GetTile(CurrentRow, CurrentColumn));
	Row = Tile->GetTile()->GetRow(CurrentTileRow);
	CurrentPtr = *Row + Offset;
	if (CurrentColumn == MaxColumn)
		MaxPtr = *Row + MaxOffset;
	else
		MaxPtr = *Row + ((Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1) >> 3);
	if (CurrentRow == MaxRow)
		MaxRowInTile = MaxTileRow;
	else
		MaxRowInTile = (Tile->GetTile()->GetTileOffsetY() +
			Tile->GetTile()->GetTileHeight() - 1);
}
G42LockedID1BitIterator::G42LockedID1BitIterator(G42ImageData * data, bool init)
	: Data(data), CurrentPtr(0), CurrentRow(0), CurrentColumn(0),
	CurrentTileRow(0), Offset(0), Mask(0x80), MinColumn(0), MinRow(0), MinOffset(0),
	MinTileRow(0), Tile(0), MaxMask(0), MinMask(0)
{
	MaxColumn = Data->GetNumTilesAcross() - 1;
	MaxRow = Data->GetNumTilesDown() - 1;
	if (init)
	{
		Tile = new G42LockedImageTile(Data->GetTile(0, 0));
		CurrentTileRow = Tile->GetTile()->GetTileOffsetY();
      MinTileRow = CurrentTileRow;
		MaxRowInTile = CurrentTileRow + (Tile->GetTile()->GetTileHeight() - 1);
		Row = Tile->GetTile()->GetRow(CurrentTileRow);
		Offset = Tile->GetTile()->GetTileOffsetX() >> 3;
      MinOffset = Offset;
		int remainder = (Tile->GetTile()->GetTileOffsetX() - (MinOffset << 3));
		if (remainder == 0)
			MinMask = 0x80;
		else
      	MinMask = (1 << (7 - remainder));
		CurrentPtr = *Row + Offset;
		MaxPtr = *Row + ((Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1) >> 3);
		G42ImageTile * tile = Data->GetTile(0, MaxColumn);
		MaxOffset = ((tile->GetTileOffsetX() + tile ->GetTileWidth() - 1) >> 3);
		remainder = ((tile->GetTileOffsetX() + tile->GetTileWidth() - 1)
			- (MaxOffset << 3));
		if (remainder == 7)
			MaxMask = 0;
		else
      	MaxMask = (1 << (6 - remainder));
		tile = Data->GetTile(MaxRow, 0);
		MaxTileRow = (tile->GetTileOffsetY() + tile->GetTileHeight() - 1);
	}}
G42LockedID1BitIterator::~G42LockedID1BitIterator(void)
{
	delete Tile;
}
void
G42LockedID1BitIterator::Reset(void)
{
   delete Tile;
   CurrentColumn = MinColumn;
   CurrentRow = MinRow;
   Tile = new G42LockedImageTile(Data->GetTile(CurrentRow, CurrentColumn));
   CurrentTileRow = MinTileRow;
   Row = Tile->GetTile()->GetRow(CurrentTileRow);
   Offset = MinOffset;
   CurrentPtr = *Row + Offset;
   if (CurrentColumn == MaxColumn)
   	MaxPtr = *Row + MaxOffset;
   else
		MaxPtr = *Row + ((Tile->GetTile()->GetTileOffsetX() +
      	Tile->GetTile()->GetTileWidth() - 1) >> 3);
   if (CurrentRow == MaxRow)
   	MaxRowInTile = MaxTileRow;
   else
		MaxRowInTile = CurrentTileRow + Tile->GetTile()->GetTileHeight() - 1;
}
void
G42LockedID1BitIterator::SetArea(int left, int top, int right, int bottom)
{
	MinRow = Data->GetTileRowIndex(top);
	MaxRow = Data->GetTileRowIndex(bottom);
	MinColumn = Data->GetTileColumnIndex(left);
	MaxColumn = Data->GetTileColumnIndex(right);
	G42ImageTile * tile = Data->GetTile(MinRow, MinColumn);
	MinOffset = ((left - tile->GetImageOffsetX() + tile->GetTileOffsetX()) >> 3);
	int remainder = ((left - tile->GetImageOffsetX() + tile->GetTileOffsetX())
   	- (MinOffset << 3));
	if (remainder == 0)
		MinMask = 0x80;
	else
		MinMask = (1 << (7 - remainder));
	MinTileRow = (top - tile->GetImageOffsetY() + tile->GetTileOffsetY());
	tile = Data->GetTile(MaxRow, MaxColumn);
	MaxOffset = ((right - tile->GetImageOffsetX() + tile->GetTileOffsetX()) >> 3);
	MaxTileRow = bottom - tile->GetImageOffsetY() + tile->GetTileOffsetY();
	remainder = ((right - tile->GetImageOffsetX() + tile->GetTileOffsetX())
		- (MaxOffset << 3));
	if (remainder == 7)
		MaxMask = 0;
	else
		MaxMask = (1 << (6 - remainder));
	Mask = MinMask;
	CurrentColumn = MinColumn;
	Offset = MinOffset;
	CurrentRow = MinRow;
	CurrentTileRow = MinTileRow;
	if (Tile)
		delete Tile;
	Tile = new G42LockedImageTile(Data->GetTile(CurrentRow, CurrentColumn));
	Row = Tile->GetTile()->GetRow(CurrentTileRow);
	CurrentPtr = *Row + Offset;
	if (CurrentColumn == MaxColumn)
		MaxPtr = *Row + MaxOffset;
	else
		MaxPtr = *Row + ((Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1) >> 3);
	if (CurrentRow == MaxRow)
		MaxRowInTile = MaxTileRow;
	else
		MaxRowInTile = (Tile->GetTile()->GetTileOffsetY() +
			Tile->GetTile()->GetTileHeight() - 1);
}
bool
G42LockedID1BitIterator::Increment8(void)
{
	CurrentPtr++;
	Offset++;
	Mask = 0x80;
	if (CurrentPtr > MaxPtr)
	{
		CurrentColumn++;
		if (CurrentColumn > MaxColumn)
		{
			CurrentColumn--;
			CurrentPtr--;
			Offset--;
			return false;
		}
		delete Tile;
		Tile = new G42LockedImageTile(Data->GetTile(CurrentRow, CurrentColumn));
		Row = Tile->GetTile()->GetRow(CurrentTileRow);
		Offset = 0;
		CurrentPtr = *Row;
		if (CurrentColumn == MaxColumn)
			MaxPtr = *Row + MaxOffset;
		else
			MaxPtr = *Row + ((Tile->GetTile()->GetTileWidth() - 1) >> 3);
	}
	return true;
}
bool
G42LockedID1BitIterator::Increment(void)
{
	if ((CurrentColumn == MaxColumn) && (CurrentPtr == MaxPtr))
	{
   	Mask >>= 1;
		if (Mask > MaxMask)
			return true;
	}
	else if (Mask > 0x01)
	{
		Mask >>= 1;
      return true;
	}
	CurrentPtr++;
	Offset++;
	Mask = 0x80;
	if (CurrentPtr > MaxPtr)
	{
		CurrentColumn++;
		if (CurrentColumn > MaxColumn)
		{
			CurrentColumn--;
			CurrentPtr--;
			Offset--;
			return false;
		}
		delete Tile;
		Tile = new G42LockedImageTile(Data->GetTile(CurrentRow, CurrentColumn));
		Row = Tile->GetTile()->GetRow(CurrentTileRow);
		Offset = 0;
		CurrentPtr = *Row;
		if (CurrentColumn == MaxColumn)
			MaxPtr = *Row + MaxOffset;
		else
			MaxPtr = *Row + ((Tile->GetTile()->GetTileWidth() - 1) >> 3);
	}
	return true;
}
bool
G42LockedID1BitIterator::Decrement(void)
{
	if ((CurrentColumn == MinColumn) && (Offset == MinOffset))
   {
   	if (Mask < MinMask)
      {
      	Mask <<= 1;
        	return true;
      }
   }
	else if (Mask < 0x80)
	{
		Mask <<= 1;
		return true;
	}
	CurrentPtr--;
	Offset--;
	Mask = 0x01;
	int min_offset = 0;
	if (CurrentColumn == MinColumn)
		min_offset = MinOffset;
	if (Offset < min_offset)
	{
		CurrentColumn--;
		if (CurrentColumn < MinColumn)
		{
			CurrentColumn = 0;
			CurrentPtr++;
			Offset++;
			return false;
		}
		delete Tile;
		Tile = new G42LockedImageTile(Data->GetTile(CurrentRow, CurrentColumn));
		Row = Tile->GetTile()->GetRow(CurrentTileRow);
		Offset = ((Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1) >> 3);
		CurrentPtr = *Row + Offset;
		MaxPtr = CurrentPtr;
	}
	return true;
}
bool
G42LockedID1BitIterator::IncRow(void)
{
	CurrentTileRow++;
	if (CurrentTileRow > MaxRowInTile)
	{
		CurrentRow++;
		if (CurrentRow > MaxRow)
		{
			CurrentRow--;
			CurrentTileRow--;
			return false;
		}
		CurrentTileRow = 0;
		delete Tile;
		Tile = new G42LockedImageTile(Data->GetTile(CurrentRow, CurrentColumn));
		if (CurrentRow == MaxRow)
			MaxRowInTile = MaxTileRow;
		else
			MaxRowInTile = (Tile->GetTile()->GetTileOffsetY() +
				Tile->GetTile()->GetTileHeight() - 1);
	}
	Row = Tile->GetTile()->GetRow(CurrentTileRow);
	CurrentPtr = *Row + Offset;
	if (CurrentColumn == MaxColumn)
		MaxPtr = *Row + MaxOffset;
	else
		MaxPtr = *Row + ((Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1) >> 3);
	return true;
}
bool
G42LockedID1BitIterator::DecRow(void)
{
	CurrentTileRow--;
	int min_tile_row = 0;
	if (CurrentRow == MinRow)
		min_tile_row = MinTileRow;
	if (CurrentTileRow < min_tile_row)
	{
		CurrentRow--;
		if (CurrentRow < MinRow)
		{
			CurrentRow++;
			CurrentTileRow++;
			return false;
		}
		delete Tile;
		Tile = new G42LockedImageTile(Data->GetTile(CurrentRow, CurrentColumn));
		CurrentTileRow = (Tile->GetTile()->GetTileOffsetY() +
				Tile->GetTile()->GetTileHeight() - 1);
		MaxRowInTile = CurrentTileRow;
	}
	Row = Tile->GetTile()->GetRow(CurrentTileRow);
	CurrentPtr = *Row + Offset;
	if (CurrentColumn == MaxColumn)
		MaxPtr = *Row + MaxOffset;
	else
		MaxPtr = *Row + ((Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1) >> 3);
	return true;
}
bool
G42LockedID1BitIterator::NextRow(void)
{
	int old_row = CurrentRow;
	int old_column = CurrentColumn;
	CurrentTileRow++;
	Mask = MinMask;
	if (CurrentTileRow > MaxRowInTile)
	{
		CurrentRow++;
		if (CurrentRow > MaxRow)
		{
			CurrentRow--;
			CurrentTileRow--;
			return false;
		}
		CurrentTileRow = 0;
	}
	CurrentColumn = MinColumn;
	if (CurrentRow != old_row || CurrentColumn != old_column)
	{
		delete Tile;
		Tile = new G42LockedImageTile(Data->GetTile(CurrentRow, CurrentColumn));
	}
	if (CurrentRow == MaxRow)
		MaxRowInTile = MaxTileRow;
	else
		MaxRowInTile = (Tile->GetTile()->GetTileOffsetY() +
			Tile->GetTile()->GetTileHeight() - 1);
	Row = Tile->GetTile()->GetRow(CurrentTileRow);
	Offset = MinOffset;
	CurrentPtr = *Row + Offset;
	if (CurrentColumn == MaxColumn)
		MaxPtr = *Row + MaxOffset;
	else
		MaxPtr = *Row + ((Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1) >> 3);
	return true;
}
void
G42LockedID1BitIterator::ResetRow(void)
{
	int old_row = CurrentRow;
	int old_column = CurrentColumn;
	CurrentColumn = MinColumn;
	Mask = MinMask;
	if (CurrentRow != old_row || CurrentColumn != old_column)
	{
		delete Tile;
		Tile = new G42LockedImageTile(Data->GetTile(CurrentRow, CurrentColumn));
	}
	Row = Tile->GetTile()->GetRow(CurrentTileRow);
	Offset = MinOffset;
	CurrentPtr = *Row + Offset;
	if (CurrentColumn == MaxColumn)
		MaxPtr = *Row + MaxOffset;
	else
		MaxPtr = *Row + ((Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1) >> 3);
}
void
G42LockedID1BitIterator::SetLastRow(void)
{
	CurrentRow = MaxRow;
	CurrentTileRow = MaxTileRow;
	delete Tile;
	Tile = new G42LockedImageTile(Data->GetTile(CurrentRow, CurrentColumn));
	Row = Tile->GetTile()->GetRow(CurrentTileRow);
	CurrentPtr = *Row + Offset;
	if (CurrentColumn == MaxColumn)
		MaxPtr = *Row + MaxOffset;
	else
		MaxPtr = *Row + ((Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1) >> 3);
}
