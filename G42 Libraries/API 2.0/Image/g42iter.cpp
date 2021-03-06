#include "g42iter.h"
G42ImageData24BitIterator::G42ImageData24BitIterator(G42ImageData * data, bool init)
	: Data(data), CurrentPtr(0), CurrentRow(0), CurrentColumn(0),
	CurrentTileRow(0), Offset(0), MinColumn(0), MinRow(0), MinOffset(0),
	MinTileRow(0), Tile(0)
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
		Offset = Tile->GetTile()->GetTileOffsetX() * TrueColorPixelSize;
		MinOffset = Offset;
		CurrentPtr = *Row + Offset;
		MaxPtr = CurrentPtr + ((Tile->GetTile()->GetTileWidth()-1) * TrueColorPixelSize);
		G42ImageTile * tile = Data->GetTile(0, MaxColumn);
		MaxOffset = (tile->GetTileOffsetX() + tile ->GetTileWidth() - 1) * TrueColorPixelSize;
		tile = Data->GetTile(MaxRow, 0);
		MaxTileRow = (tile->GetTileOffsetY() + tile->GetTileHeight() - 1);
	}
}
G42ImageData24BitIterator::~G42ImageData24BitIterator(void)
{
	delete Tile;
}
void
G42ImageData24BitIterator::Reset(void)
{
	MinColumn = 0;
	MaxColumn = Data->GetNumTilesAcross() - 1;
	MinRow = 0;
	MaxRow = Data->GetNumTilesDown() - 1;
	CurrentRow = 0;
	CurrentColumn = 0;
	Offset = 0;
	delete Tile;
	Tile = new G42ModifiableImageTile(Data->GetTile(0, 0));
	MinTileRow = Tile->GetTile()->GetTileOffsetY();
	MaxRowInTile = (MinTileRow + Tile->GetTile()->GetTileHeight() - 1);
	CurrentTileRow = MinTileRow;
	Row = Tile->GetTile()->GetRow(CurrentTileRow);
	MinOffset = Tile->GetTile()->GetTileOffsetX() * TrueColorPixelSize;
	Offset = MinOffset;
	CurrentPtr = *Row + Offset;
	MaxPtr = *Row + MinOffset +
		((Tile->GetTile()->GetTileWidth() - 1) * TrueColorPixelSize);
	G42ImageTile * tile = Data->GetTile(0, MaxColumn);
	MaxOffset = ((tile->GetTileOffsetX() + tile ->GetTileWidth() - 1) * TrueColorPixelSize);
	tile = Data->GetTile(MaxRow, 0);
	MaxTileRow = (tile->GetTileOffsetY() + tile->GetTileHeight() - 1);
}
void
G42ImageData24BitIterator::SetArea(int left, int top, int right, int bottom)
{
	MinRow = Data->GetTileRowIndex(top);
	MaxRow = Data->GetTileRowIndex(bottom);
	MinColumn = Data->GetTileColumnIndex(left);
	MaxColumn = Data->GetTileColumnIndex(right);
	G42ImageTile * tile = Data->GetTile(MinRow, MinColumn);
	MinOffset = (left - tile->GetImageOffsetX() + tile->GetTileOffsetX()) * TrueColorPixelSize;
	MinTileRow = (top - tile->GetImageOffsetY() + tile->GetTileOffsetY());
	tile = Data->GetTile(MaxRow, MaxColumn);
	MaxOffset = (right - tile->GetImageOffsetX() + tile->GetTileOffsetX()) * TrueColorPixelSize;
	MaxTileRow = bottom - tile->GetImageOffsetY() + tile->GetTileOffsetY();
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
			Tile->GetTile()->GetTileWidth() - 1) * TrueColorPixelSize);
	if (CurrentRow == MaxRow)
		MaxRowInTile = MaxTileRow;
	else
		MaxRowInTile = (Tile->GetTile()->GetTileOffsetY() +
			Tile->GetTile()->GetTileHeight() - 1);
}
bool
G42ImageData24BitIterator::Increment(void)
{
	CurrentPtr += TrueColorPixelSize;
	Offset += TrueColorPixelSize;
	if (CurrentPtr > MaxPtr)
	{
		CurrentColumn++;
		if (CurrentColumn > MaxColumn)
		{
			CurrentColumn--;
			CurrentPtr -= TrueColorPixelSize;
			Offset -= TrueColorPixelSize;
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
			MaxPtr = *Row + ((Tile->GetTile()->GetTileWidth()-1) * TrueColorPixelSize);
	}
	return true;
}
bool
G42ImageData24BitIterator::Decrement(void)
{
	CurrentPtr -= TrueColorPixelSize;
	Offset -= TrueColorPixelSize;
	int min_offset = 0;
	if (CurrentColumn == MinColumn)
		min_offset = MinOffset;
	if (Offset < min_offset)
	{
		CurrentColumn--;
		if (CurrentColumn < MinColumn)
		{
			CurrentColumn = MinColumn;
			CurrentPtr += TrueColorPixelSize;
			Offset += TrueColorPixelSize;
			return false;
		}
		Tile->MarkModified();
		delete Tile;
		Tile = new G42ModifiableImageTile(Data->GetTile(CurrentRow, CurrentColumn));
		Row = Tile->GetTile()->GetRow(CurrentTileRow);
		Offset = (Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1) * TrueColorPixelSize;
		CurrentPtr = *Row + Offset;
		MaxPtr = CurrentPtr;
	}
	return true;
}
bool
G42ImageData24BitIterator::IncRow(void)
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
			Tile->GetTile()->GetTileWidth()-1) * TrueColorPixelSize);
	return true;
}
bool
G42ImageData24BitIterator::DecRow(void)
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
			Tile->GetTile()->GetTileWidth()-1) * TrueColorPixelSize);
	return true;
}
bool
G42ImageData24BitIterator::NextRow(void)
{
	int old_row = CurrentRow;
	int old_column = CurrentColumn;
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
			Tile->GetTile()->GetTileWidth() - 1) * TrueColorPixelSize);
	return true;
}
void
G42ImageData24BitIterator::ResetRow(void)
{
	int old_row = CurrentRow;
	int old_column = CurrentColumn;
	CurrentColumn = MinColumn;
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
			Tile->GetTile()->GetTileWidth() - 1) * TrueColorPixelSize);
}
void
G42ImageData24BitIterator::SetRowEnd(void)
{
	CurrentColumn = MaxColumn;
	Tile->MarkModified();
	delete Tile;
	Tile = new G42ModifiableImageTile(Data->GetTile(CurrentRow, CurrentColumn));
	Row = Tile->GetTile()->GetRow(CurrentTileRow);
	Offset = MaxOffset;
	CurrentPtr = *Row + Offset;
	MaxPtr = CurrentPtr;
}
void
G42ImageData24BitIterator::SetLastRow(void)
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
			Tile->GetTile()->GetTileWidth() - 1) * TrueColorPixelSize);
}
void
G42ImageData24BitIterator::ToTopRow(void)
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
			Tile->GetTile()->GetTileWidth() - 1) * TrueColorPixelSize);
	if (CurrentRow == MaxRow)
		MaxRowInTile = MaxTileRow;
	else
		MaxRowInTile = (Tile->GetTile()->GetTileOffsetY() +
			Tile->GetTile()->GetTileHeight() - 1);
}
G42LockedID24BitIterator::G42LockedID24BitIterator(G42ImageData * data, bool init)
	: Data(data), CurrentPtr(0), CurrentRow(0), CurrentColumn(0),
	CurrentTileRow(0), Offset(0), MinColumn(0), MinRow(0), MinOffset(0),
	MinTileRow(0), Tile(0)
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
		Offset = Tile->GetTile()->GetTileOffsetX() * TrueColorPixelSize;
		MinOffset = Offset;
		CurrentPtr = *Row + Offset;
		MaxPtr = *Row + Offset + ((Tile->GetTile()->GetTileWidth()-1) * TrueColorPixelSize);
		G42ImageTile * tile = Data->GetTile(0, MaxColumn);
		MaxOffset = (tile->GetTileOffsetX() + tile->GetTileWidth() - 1) * TrueColorPixelSize;
		tile = Data->GetTile(MaxRow, 0);
		MaxTileRow = (tile->GetTileOffsetY() + tile->GetTileHeight() - 1);
	}
}
G42LockedID24BitIterator::~G42LockedID24BitIterator(void)
{
	delete Tile;
}
void
G42LockedID24BitIterator::SetArea(int left, int top, int right, int bottom)
{
	MinRow = Data->GetTileRowIndex(top);
	MaxRow = Data->GetTileRowIndex(bottom);
	MinColumn = Data->GetTileColumnIndex(left);
	MaxColumn = Data->GetTileColumnIndex(right);
	G42ImageTile * tile = Data->GetTile(MinRow, MinColumn);
	MinOffset = (left - tile->GetImageOffsetX() + tile->GetTileOffsetX()) * TrueColorPixelSize;
	MinTileRow = (top - tile->GetImageOffsetY() + tile->GetTileOffsetY());
	tile = Data->GetTile(MaxRow, MaxColumn);
	MaxOffset = (right - tile->GetImageOffsetX() + tile->GetTileOffsetX()) * TrueColorPixelSize;
	MaxTileRow = bottom - tile->GetImageOffsetY() + tile->GetTileOffsetY();
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
		MaxPtr = *Row + ((Tile->GetTile()->GetTileOffsetX()
			+ Tile->GetTile()->GetTileWidth() - 1) * TrueColorPixelSize);
	if (CurrentRow == MaxRow)
		MaxRowInTile = MaxTileRow;
	else
		MaxRowInTile = (Tile->GetTile()->GetTileOffsetY() +
			Tile->GetTile()->GetTileHeight() - 1);
}
void
G42LockedID24BitIterator::Reset(void)
{
	MinColumn = 0;
	MaxColumn = Data->GetNumTilesAcross() - 1;
	MinRow = 0;
	MaxRow = Data->GetNumTilesDown() - 1;
	CurrentRow = 0;
	CurrentColumn = 0;
	delete Tile;
	Tile = new G42LockedImageTile(Data->GetTile(0, 0));
	MinTileRow = Tile->GetTile()->GetTileOffsetY();
	MaxRowInTile = (MinTileRow + Tile->GetTile()->GetTileHeight() - 1);
	CurrentTileRow = MinTileRow;
	Row = Tile->GetTile()->GetRow(CurrentTileRow);
	MinOffset = Tile->GetTile()->GetTileOffsetX() * TrueColorPixelSize;
	Offset = MinOffset;
	CurrentPtr = *Row + Offset;
	MaxPtr = *Row + MinOffset +
		((Tile->GetTile()->GetTileWidth() - 1) * TrueColorPixelSize);
	G42ImageTile * tile = Data->GetTile(0, MaxColumn);
	MaxOffset = (tile->GetTileOffsetX() + tile ->GetTileWidth() - 1) * TrueColorPixelSize;
	tile = Data->GetTile(MaxRow, 0);
	MaxTileRow = (tile->GetTileOffsetY() + tile->GetTileHeight() - 1); 
}
bool
G42LockedID24BitIterator::Increment(void)
{
	CurrentPtr += TrueColorPixelSize;
	Offset += TrueColorPixelSize;
	if (CurrentPtr > MaxPtr)
	{
		CurrentColumn++;
		if (CurrentColumn > MaxColumn)
		{
			CurrentColumn--;
			CurrentPtr -= TrueColorPixelSize;
			Offset -= TrueColorPixelSize;
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
			MaxPtr = *Row + ((Tile->GetTile()->GetTileWidth()-1) * TrueColorPixelSize);
	}
	return true;
}
bool
G42LockedID24BitIterator::Decrement(void)
{
	CurrentPtr -= TrueColorPixelSize;
	Offset -= TrueColorPixelSize;
	int min_offset = 0;
	if (CurrentColumn == MinColumn)
		min_offset = MinOffset;
	if (Offset < min_offset)
	{
		CurrentColumn--;
		if (CurrentColumn < MinColumn)
		{
			CurrentColumn = MinColumn;
			CurrentPtr += TrueColorPixelSize;
			Offset += TrueColorPixelSize;
			return false;
		}
		delete Tile;
		Tile = new G42LockedImageTile(Data->GetTile(CurrentRow, CurrentColumn));
		Row = Tile->GetTile()->GetRow(CurrentTileRow);
		Offset = (Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1) * TrueColorPixelSize;
		CurrentPtr = *Row + Offset;
		MaxPtr = CurrentPtr;
	}
	return true;
}
bool
G42LockedID24BitIterator::IncRow(void)
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
			Tile->GetTile()->GetTileWidth() - 1) * TrueColorPixelSize);
	return true;
}
bool
G42LockedID24BitIterator::DecRow(void)
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
			Tile->GetTile()->GetTileWidth() - 1) * TrueColorPixelSize);
	return true;
}
bool
G42LockedID24BitIterator::NextRow(void)
{
	int old_row = CurrentRow;
	int old_column = CurrentColumn;
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
		MaxRowInTile = Tile->GetTile()->GetTileOffsetY() +
			(Tile->GetTile()->GetTileHeight() - 1);
	Row = Tile->GetTile()->GetRow(CurrentTileRow);
	Offset = MinOffset;
	CurrentPtr = *Row + Offset;
	if (CurrentColumn == MaxColumn)
		MaxPtr = *Row + MaxOffset;
	else
		MaxPtr = *Row + ((Tile->GetTile()->GetTileOffsetX()
			+ Tile->GetTile()->GetTileWidth() - 1) * TrueColorPixelSize);
	return true;
}
void
G42LockedID24BitIterator::ResetRow(void)
{
	int old_row = CurrentRow;
	int old_column = CurrentColumn;
	CurrentColumn = MinColumn;
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
			Tile->GetTile()->GetTileWidth() - 1) * TrueColorPixelSize);
}
void
G42LockedID24BitIterator::SetLastRow(void)
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
			Tile->GetTile()->GetTileWidth() - 1) * TrueColorPixelSize);
}
G42ImageData8BitIterator::G42ImageData8BitIterator(G42ImageData * data, bool init)
	: Data(data), CurrentPtr(0), CurrentRow(0), CurrentColumn(0),
	CurrentTileRow(0), Offset(0), MinColumn(0), MinRow(0), MinOffset(0),
	MinTileRow(0), Tile(0)
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
		Offset = Tile->GetTile()->GetTileOffsetX();
      MinOffset = Offset;
		CurrentPtr = *Row + Offset;
		MaxPtr = *Row + Offset + Tile->GetTile()->GetTileWidth() - 1;
		G42ImageTile * tile = Data->GetTile(0, MaxColumn);
		MaxOffset = (tile->GetTileOffsetX() + tile ->GetTileWidth() - 1);
		tile = Data->GetTile(MaxRow, 0);
		MaxTileRow = (tile->GetTileOffsetY() + tile->GetTileHeight() - 1);
	}
}
G42ImageData8BitIterator::~G42ImageData8BitIterator(void)
{
	delete Tile;
}
void
G42ImageData8BitIterator::Reset(void)
{
 	Tile->MarkModified();
   delete Tile;
   CurrentColumn = MinColumn;
   CurrentRow = MinRow;
   Tile = new G42ModifiableImageTile(Data->GetTile(CurrentRow, CurrentColumn));
   CurrentTileRow = MinTileRow;
   Row = Tile->GetTile()->GetRow(CurrentTileRow);
   Offset = MinOffset;
   CurrentPtr = *Row + Offset;
   if (CurrentColumn == MaxColumn)
   	MaxPtr = *Row + MaxOffset;
   else
		MaxPtr = CurrentPtr + Tile->GetTile()->GetTileWidth() - 1;
   if (CurrentRow == MaxRow)
   	MaxRowInTile = MaxTileRow;
   else
		MaxRowInTile = CurrentTileRow + Tile->GetTile()->GetTileHeight() - 1;
}
void
G42ImageData8BitIterator::SetArea(int left, int top, int right, int bottom)
{
	MinRow = Data->GetTileRowIndex(top);
	MaxRow = Data->GetTileRowIndex(bottom);
	MinColumn = Data->GetTileColumnIndex(left);
	MaxColumn = Data->GetTileColumnIndex(right);
	G42ImageTile * tile = Data->GetTile(MinRow, MinColumn);
	MinOffset = (left - tile->GetImageOffsetX() + tile->GetTileOffsetX());
	MinTileRow = (top - tile->GetImageOffsetY() + tile->GetTileOffsetY());
	tile = Data->GetTile(MaxRow, MaxColumn);
	MaxOffset = (right - tile->GetImageOffsetX() + tile->GetTileOffsetX());
	MaxTileRow = bottom - tile->GetImageOffsetY() + tile->GetTileOffsetY();
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
		MaxPtr = *Row + Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1;
	if (CurrentRow == MaxRow)
		MaxRowInTile = MaxTileRow;
	else
		MaxRowInTile = (Tile->GetTile()->GetTileOffsetY() +
			Tile->GetTile()->GetTileHeight() - 1);
}
bool
G42ImageData8BitIterator::Increment(void)
{
	CurrentPtr++;
   Offset++;
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
			MaxPtr = *Row + Tile->GetTile()->GetTileWidth() - 1;
	}
	return true;
}
bool
G42ImageData8BitIterator::Decrement(void)
{
	CurrentPtr--;
	Offset--;
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
		Offset = Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1;
		CurrentPtr = *Row + Offset;
		if (CurrentColumn == MaxColumn)
			MaxPtr = *Row + MaxOffset;
		else
			MaxPtr = *Row + Tile->GetTile()->GetTileWidth() - 1;
	}
	return true;
}
bool
G42ImageData8BitIterator::IncRow(void)
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
		MaxPtr = *Row + Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1;
	return true;
}
bool
G42ImageData8BitIterator::DecRow(void)
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
		MaxPtr = *Row + Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1;
	return true;
}
bool
G42ImageData8BitIterator::NextRow(void)
{
	int old_row = CurrentRow;
	int old_column = CurrentColumn;
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
		MaxPtr = *Row + Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1;
	return true;
}
void
G42ImageData8BitIterator::ResetRow(void)
{
	int old_row = CurrentRow;
	int old_column = CurrentColumn;
	CurrentColumn = MinColumn;
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
		MaxPtr = *Row + Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1;
}
void
G42ImageData8BitIterator::SetRowEnd(void)
{
	CurrentColumn = MaxColumn;
	Tile->MarkModified();
	delete Tile;
	Tile = new G42ModifiableImageTile(Data->GetTile(CurrentRow, CurrentColumn));
	Row = Tile->GetTile()->GetRow(CurrentTileRow);
	Offset = MaxOffset;
	CurrentPtr = *Row + Offset;
	MaxPtr = CurrentPtr;
}
void
G42ImageData8BitIterator::SetLastRow(void)
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
		MaxPtr = *Row + Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1;
}
void
G42ImageData8BitIterator::ToTopRow(void)
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
		MaxPtr = *Row + Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1;
	if (CurrentRow == MaxRow)
		MaxRowInTile = MaxTileRow;
	else
		MaxRowInTile = Tile->GetTile()->GetTileOffsetY() +
			Tile->GetTile()->GetHeight() - 1;
}
G42LockedID8BitIterator::G42LockedID8BitIterator(G42ImageData * data, bool init)
	: Data(data), CurrentPtr(0), CurrentRow(0), CurrentColumn(0),
	CurrentTileRow(0), Offset(0), MinColumn(0), MinRow(0), MinOffset(0),
	MinTileRow(0), Tile(0)
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
		Offset = Tile->GetTile()->GetTileOffsetX();
		MinOffset = Offset;
		CurrentPtr = *Row + Offset;
		MaxPtr = *Row + Offset + Tile->GetTile()->GetTileWidth() - 1;
		G42ImageTile * tile = Data->GetTile(0, MaxColumn);
		MaxOffset = (tile->GetTileOffsetX() + tile ->GetTileWidth() - 1);
		tile = Data->GetTile(MaxRow, 0);
		MaxTileRow = (tile->GetTileOffsetY() + tile->GetTileHeight() - 1);
	}
}
G42LockedID8BitIterator::~G42LockedID8BitIterator(void)
{
	delete Tile;
}
void
G42LockedID8BitIterator::Reset(void)
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
		MaxPtr = CurrentPtr + Tile->GetTile()->GetTileWidth() - 1;
   if (CurrentRow == MaxRow)
   	MaxRowInTile = MaxTileRow;
   else
		MaxRowInTile = CurrentTileRow + Tile->GetTile()->GetTileHeight() - 1;
}
void
G42LockedID8BitIterator::SetArea(int left, int top, int right, int bottom)
{
	MinRow = Data->GetTileRowIndex(top);
	MaxRow = Data->GetTileRowIndex(bottom);
	MinColumn = Data->GetTileColumnIndex(left);
	MaxColumn = Data->GetTileColumnIndex(right);
	G42ImageTile * tile = Data->GetTile(MinRow, MinColumn);
	MinOffset = (left - tile->GetImageOffsetX() + tile->GetTileOffsetX());
	MinTileRow = (top - tile->GetImageOffsetY() + tile->GetTileOffsetY());
	tile = Data->GetTile(MaxRow, MaxColumn);
	MaxOffset = (right - tile->GetImageOffsetX() + tile->GetTileOffsetX());
	MaxTileRow = bottom - tile->GetImageOffsetY() + tile->GetTileOffsetY();
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
		MaxPtr = *Row + Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1;
	if (CurrentRow == MaxRow)
		MaxRowInTile = MaxTileRow;
	else
		MaxRowInTile = (Tile->GetTile()->GetTileOffsetY() +
			Tile->GetTile()->GetTileHeight() - 1);
}
bool
G42LockedID8BitIterator::Increment(void)
{
	CurrentPtr++;
   Offset++;
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
			MaxPtr = *Row + Tile->GetTile()->GetTileWidth() - 1;
	}
	return true;
}
bool
G42LockedID8BitIterator::Decrement(void)
{
	CurrentPtr--;
	Offset--;
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
		Offset = Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1;
		CurrentPtr = *Row + Offset;
		if (CurrentColumn == MaxColumn)
			MaxPtr = *Row + MaxOffset;
		else
			MaxPtr = *Row + Tile->GetTile()->GetTileWidth() - 1;
	}
	return true;
}
bool
G42LockedID8BitIterator::IncRow(void)
{
	CurrentTileRow++;
	if (CurrentTileRow > MaxRowInTile)
	{
		CurrentRow++;
		if (CurrentRow > MaxRow)
		{
			CurrentRow--;
			CurrentTileRow--;
			return FALSE;
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
		MaxPtr = *Row + Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1;
	return true;
}
bool
G42LockedID8BitIterator::DecRow(void)
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
		MaxPtr = *Row + Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1;
	return TRUE;
}
bool
G42LockedID8BitIterator::NextRow(void)
{
	int old_row = CurrentRow;
	int old_column = CurrentColumn;
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
		MaxRowInTile = Tile->GetTile()->GetTileOffsetY() +
			(Tile->GetTile()->GetTileHeight() - 1);
	Row = Tile->GetTile()->GetRow(CurrentTileRow);
	Offset = MinOffset;
	CurrentPtr = *Row + Offset;
	if (CurrentColumn == MaxColumn)
		MaxPtr = *Row + MaxOffset;
	else
		MaxPtr = *Row + Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1;
	return true;
}
void
G42LockedID8BitIterator::ResetRow(void)
{
	int old_row = CurrentRow;
	int old_column = CurrentColumn;
	CurrentColumn = MinColumn;
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
		MaxPtr = *Row + Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1;
}
void
G42LockedID8BitIterator::SetRowEnd(void)
{
	CurrentColumn = MaxColumn;
	delete Tile;
	Tile = new G42LockedImageTile(Data->GetTile(CurrentRow, CurrentColumn));
	Row = Tile->GetTile()->GetRow(CurrentTileRow);
	Offset = MaxOffset;
	CurrentPtr = *Row + Offset;
	MaxPtr = CurrentPtr;
}
void
G42LockedID8BitIterator::SetLastRow(void)
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
		MaxPtr = *Row + Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1;
}
G42ImageData4BitIterator::G42ImageData4BitIterator(G42ImageData * data, bool init)
	: Data(data), CurrentPtr(0), CurrentRow(0), CurrentColumn(0),
	CurrentTileRow(0), Offset(0), HiNibble(true), MinColumn(0), MinRow(0), MinOffset(0),
	MinTileRow(0), Tile(0), MaxHiNibble(true), MinHiNibble(true)
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
		Offset = (Tile->GetTile()->GetTileOffsetX() >> 1);
      MinOffset = Offset;
		CurrentPtr = *Row + Offset;
      if (Tile->GetTile()->GetTileOffsetX() & 0x01)
	      MinHiNibble = false;
      HiNibble = MinHiNibble;
		MaxPtr = *Row + ((Tile->GetTile()->GetTileOffsetX() +
      	Tile->GetTile()->GetTileWidth() - 1) >> 1);
		G42ImageTile * tile = Data->GetTile(0, MaxColumn);
		MaxOffset = ((tile->GetTileOffsetX() + tile->GetTileWidth() - 1) >> 1);
      if ((tile->GetTileOffsetX() + tile->GetTileWidth() - 1) & 0x01)
      	MaxHiNibble = false;
		tile = Data->GetTile(MaxRow, 0);
		MaxTileRow = (tile->GetTileOffsetY() + tile->GetTileHeight() - 1);
	}
}
G42ImageData4BitIterator::~G42ImageData4BitIterator(void)
{
	delete Tile;
}
void
G42ImageData4BitIterator::Reset(void)
{
 	Tile->MarkModified();
   delete Tile;
   CurrentColumn = MinColumn;
   CurrentRow = MinRow;
   Tile = new G42ModifiableImageTile(Data->GetTile(CurrentRow, CurrentColumn));
   CurrentTileRow = MinTileRow;
   Row = Tile->GetTile()->GetRow(CurrentTileRow);
   Offset = MinOffset;
   CurrentPtr = *Row + Offset;
   HiNibble = MinHiNibble;
   if (CurrentColumn == MaxColumn)
   	MaxPtr = *Row + MaxOffset;
   else
		MaxPtr = *Row + ((Tile->GetTile()->GetTileOffsetX() +
      	Tile->GetTile()->GetTileWidth() - 1) >> 1);
   if (CurrentRow == MaxRow)
   	MaxRowInTile = MaxTileRow;
   else
		MaxRowInTile = CurrentTileRow + Tile->GetTile()->GetTileHeight() - 1;
}
void
G42ImageData4BitIterator::SetArea(int left, int top, int right, int bottom)
{
	MinRow = Data->GetTileRowIndex(top);
	MaxRow = Data->GetTileRowIndex(bottom);
	MinColumn = Data->GetTileColumnIndex(left);
	MaxColumn = Data->GetTileColumnIndex(right);
	G42ImageTile * tile = Data->GetTile(MinRow, MinColumn);
	MinOffset = ((left - tile->GetImageOffsetX() + tile->GetTileOffsetX()) >> 1);
   MinHiNibble = true;
   if ((left - tile->GetImageOffsetX() + tile->GetTileOffsetX()) & 0x01)
   	MinHiNibble = false;
	MinTileRow = (top - tile->GetImageOffsetY() + tile->GetTileOffsetY());
	tile = Data->GetTile(MaxRow, MaxColumn);
	MaxOffset = ((right - tile->GetImageOffsetX() + tile->GetTileOffsetX()) >> 1);
	MaxHiNibble = true;
   if ((right - tile->GetImageOffsetX() + tile->GetTileOffsetX()) & 0x01)
   	MaxHiNibble = false;
	MaxTileRow = bottom - tile->GetImageOffsetY() + tile->GetTileOffsetY();
	CurrentColumn = MinColumn;
	Offset = MinOffset;
   HiNibble = MinHiNibble;
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
			Tile->GetTile()->GetTileWidth() - 1) >> 1);
	if (CurrentRow == MaxRow)
		MaxRowInTile = MaxTileRow;
	else
		MaxRowInTile = (Tile->GetTile()->GetTileOffsetY() +
			Tile->GetTile()->GetTileHeight() - 1);
}
void
G42ImageData4BitIterator::Set4(unsigned char val)
{
	if (HiNibble)
	{
		*CurrentPtr &= 0x0f;
		*CurrentPtr |= (val << 4);
	}
	else
	{
		*CurrentPtr &= 0xf0;
		*CurrentPtr |= val;
	}
}
bool
G42ImageData4BitIterator::Increment(void)
{
	if ((CurrentColumn == MaxColumn) && (CurrentPtr == MaxPtr))
   {
     	if (HiNibble == MaxHiNibble)
        	return false;
      HiNibble = false;
      return true;
   }
	if (HiNibble)
	{
		HiNibble = false;
		return true;
	}
	HiNibble = true;
	CurrentPtr++;
	Offset++;
	if (CurrentPtr > MaxPtr)
	{
		CurrentColumn++;
		Tile->MarkModified();
		delete Tile;
		Tile = new G42ModifiableImageTile(Data->GetTile(CurrentRow, CurrentColumn));
		Row = Tile->GetTile()->GetRow(CurrentTileRow);
		Offset = 0;
		CurrentPtr = *Row;
		if (CurrentColumn == MaxColumn)
			MaxPtr = *Row + MaxOffset;
		else
			MaxPtr = *Row + ((Tile->GetTile()->GetTileWidth() - 1) >> 1);
	}
	return true;
}
bool
G42ImageData4BitIterator::Decrement(void)
{
	if ((CurrentColumn == MinColumn) && (Offset == MinOffset))
   {
   	if (HiNibble == MinHiNibble)
      	return false;
      HiNibble = true;
      return true;
   }
	if (!HiNibble)
	{
		HiNibble = true;
      return true;
	}
   HiNibble = false;
	CurrentPtr--;
	Offset--;
	if (Offset < 0)
	{
		CurrentColumn--;
		Tile->MarkModified();
		delete Tile;
		Tile = new G42ModifiableImageTile(Data->GetTile(CurrentRow, CurrentColumn));
		Row = Tile->GetTile()->GetRow(CurrentTileRow);
		Offset = ((Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() - 1) >> 1);
		CurrentPtr = *Row + Offset;
		MaxPtr = CurrentPtr;
		HiNibble = false;
	}
   return true;
}
bool
G42ImageData4BitIterator::IncRow(void)
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
			Tile->GetTile()->GetTileWidth() - 1) >> 1);
	return true;
}
bool
G42ImageData4BitIterator::DecRow(void)
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
			Tile->GetTile()->GetTileWidth() - 1) >> 1);
	return true;
}
bool
G42ImageData4BitIterator::NextRow(void)
{
	int old_row = CurrentRow;
	int old_column = CurrentColumn;
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
	}
	CurrentColumn = MinColumn;
	if (CurrentRow != old_row || CurrentColumn != old_column)
	{
		Tile->MarkModified();
		delete Tile;
		Tile = new G42ModifiableImageTile(Data->GetTile(CurrentRow, CurrentColumn));
	}
	HiNibble = MinHiNibble;
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
			Tile->GetTile()->GetTileWidth() - 1) >> 1);
	return true;
}
void
G42ImageData4BitIterator::ResetRow(void)
{
	int old_row = CurrentRow;
	int old_column = CurrentColumn;
	CurrentColumn = MinColumn;
	HiNibble = MinHiNibble;
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
			Tile->GetTile()->GetTileWidth() - 1) >> 1);
}
void
G42ImageData4BitIterator::SetRowEnd(void)
{
	CurrentColumn = MaxColumn;
	Tile->MarkModified();
	delete Tile;
	Tile = new G42ModifiableImageTile(Data->GetTile(CurrentRow, CurrentColumn));
	HiNibble = MaxHiNibble;
	Row = Tile->GetTile()->GetRow(CurrentTileRow);
	Offset = MaxOffset;
	CurrentPtr = *Row + Offset;
	MaxPtr = CurrentPtr;
}
void
G42ImageData4BitIterator::SetLastRow(void)
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
			Tile->GetTile()->GetTileWidth() - 1) >> 1);
}
void
G42ImageData4BitIterator::ToTopRow(void)
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
			Tile->GetTile()->GetTileWidth() - 1) >> 1);
	if (CurrentRow == MaxRow)
		MaxRowInTile = MaxTileRow;
	else
		MaxRowInTile = (Tile->GetTile()->GetTileOffsetY() +
			Tile->GetTile()->GetTileHeight() - 1);
}
G42LockedID4BitIterator::G42LockedID4BitIterator(G42ImageData * data, bool init)
	: Data(data), CurrentPtr(0), CurrentRow(0), CurrentColumn(0),
	CurrentTileRow(0), Offset(0), HiNibble(true), MinColumn(0), MinRow(0), MinOffset(0),
	MinTileRow(0), Tile(0), MaxHiNibble(true), MinHiNibble(true)
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
		Offset = (Tile->GetTile()->GetTileOffsetX() + 1) >> 1;
		MinOffset = Offset;
		CurrentPtr = *Row + Offset;
      if (Tile->GetTile()->GetTileOffsetX() & 0x01)
	      MinHiNibble = false;
      HiNibble = MinHiNibble;
		MaxPtr = *Row + ((Tile->GetTile()->GetTileOffsetX() +
      	Tile->GetTile()->GetTileWidth() - 1) >> 1);
		G42ImageTile * tile = Data->GetTile(0, MaxColumn);
		MaxOffset = ((tile->GetTileOffsetX() + tile->GetTileWidth() - 1) >> 1);
      if ((tile->GetTileOffsetX() + tile->GetTileWidth() - 1) & 0x01)
      	MaxHiNibble = false;
		tile = Data->GetTile(MaxRow, 0);
		MaxTileRow = (tile->GetTileOffsetY() + tile->GetTileHeight() - 1);
	}
}
G42LockedID4BitIterator::~G42LockedID4BitIterator(void)
{
	delete Tile;
}
void
G42LockedID4BitIterator::Reset(void)
{
   delete Tile;
   CurrentColumn = MinColumn;
   CurrentRow = MinRow;
   Tile = new G42LockedImageTile(Data->GetTile(CurrentRow, CurrentColumn));
   CurrentTileRow = MinTileRow;
   Row = Tile->GetTile()->GetRow(CurrentTileRow);
   Offset = MinOffset;
   CurrentPtr = *Row + Offset;
   HiNibble = MinHiNibble;
   if (CurrentColumn == MaxColumn)
   	MaxPtr = *Row + MaxOffset;
   else
		MaxPtr = *Row + ((Tile->GetTile()->GetTileOffsetX() +
      	Tile->GetTile()->GetTileWidth() - 1) >> 1);
   if (CurrentRow == MaxRow)
   	MaxRowInTile = MaxTileRow;
   else
		MaxRowInTile = CurrentTileRow + Tile->GetTile()->GetTileHeight() - 1;
}
void
G42LockedID4BitIterator::SetArea(int left, int top, int right, int bottom)
{
	MinRow = Data->GetTileRowIndex(top);
	MaxRow = Data->GetTileRowIndex(bottom);
	MinColumn = Data->GetTileColumnIndex(left);
	MaxColumn = Data->GetTileColumnIndex(right);
	G42ImageTile * tile = Data->GetTile(MinRow, MinColumn);
	MinOffset = ((left - tile->GetImageOffsetX() + tile->GetTileOffsetX()) >> 1);
   MinHiNibble = true;
   if ((left - tile->GetImageOffsetX() + tile->GetTileOffsetX()) & 0x01)
   	MinHiNibble = false;
	MinTileRow = (top - tile->GetImageOffsetY() + tile->GetTileOffsetY());
	tile = Data->GetTile(MaxRow, MaxColumn);
	MaxOffset = ((right - tile->GetImageOffsetX() + tile->GetTileOffsetX()) >> 1);
	MaxHiNibble = true;
   if ((right - tile->GetImageOffsetX() + tile->GetTileOffsetX()) & 0x01)
   	MaxHiNibble = false;
	MaxTileRow = bottom - tile->GetImageOffsetY() + tile->GetTileOffsetY();
	CurrentColumn = MinColumn;
	Offset = MinOffset;
   HiNibble = MinHiNibble;
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
			Tile->GetTile()->GetTileWidth() - 1) >> 1);
	if (CurrentRow == MaxRow)
		MaxRowInTile = MaxTileRow;
	else
		MaxRowInTile = (Tile->GetTile()->GetTileOffsetY() +
			Tile->GetTile()->GetTileHeight() - 1);
}
bool
G42LockedID4BitIterator::Increment(void)
{
	if ((CurrentColumn == MaxColumn) && (CurrentPtr == MaxPtr))
   {
     	if (HiNibble == MaxHiNibble)
        	return false;
      HiNibble = false;
      return true;
   }
	if (HiNibble)
	{
		HiNibble = false;
		return true;
	}
	CurrentPtr++;
	Offset++;
	HiNibble = true;
	if (CurrentPtr > MaxPtr)
	{
		CurrentColumn++;
		delete Tile;
		Tile = new G42LockedImageTile(Data->GetTile(CurrentRow, CurrentColumn));
		Row = Tile->GetTile()->GetRow(CurrentTileRow);
		Offset = 0;
		CurrentPtr = *Row;
		if (CurrentColumn == MaxColumn)
			MaxPtr = *Row + MaxOffset;
		else
			MaxPtr = *Row + ((Tile->GetTile()->GetTileWidth() - 1) >> 1);
	}
	return true;
}
bool
G42LockedID4BitIterator::Decrement(void)
{
	if ((CurrentColumn == MinColumn) && (Offset == MinOffset))
   {
   	if (HiNibble == MinHiNibble)
      	return false;
      HiNibble = true;
      return true;
   }
	if (!HiNibble)
	{
		HiNibble = true;
		return true;
	}
	CurrentPtr--;
	Offset--;
	HiNibble = false;
	if (Offset < 0)
	{
		CurrentColumn--;
		delete Tile;
		Tile = new G42LockedImageTile(Data->GetTile(CurrentRow, CurrentColumn));
		Row = Tile->GetTile()->GetRow(CurrentTileRow);
		Offset = ((Tile->GetTile()->GetTileOffsetX() +
			Tile->GetTile()->GetTileWidth() -1) >> 1);
		CurrentPtr = *Row + Offset;
		MaxPtr = CurrentPtr;
		HiNibble = false;
	}
	return true;
}
bool
G42LockedID4BitIterator::IncRow(void)
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
			Tile->GetTile()->GetTileWidth() - 1) >> 1);
	return true;
}
bool
G42LockedID4BitIterator::DecRow(void)
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
			Tile->GetTile()->GetTileWidth() - 1) >> 1);
	return true;
}
bool
G42LockedID4BitIterator::NextRow(void)
{
	int old_row = CurrentRow;
	int old_column = CurrentColumn;
	CurrentTileRow++;
	if (CurrentTileRow > MaxRowInTile)
	{
		CurrentRow++;
		if (CurrentRow > MaxRow)
		{
			CurrentRow--;
			CurrentTileRow--;
			return FALSE;
		}
		CurrentTileRow = MinColumn;
	}
	CurrentColumn = MinColumn;
	if (CurrentRow != old_row || CurrentColumn != old_column)
	{
		delete Tile;
		Tile = new G42LockedImageTile(Data->GetTile(CurrentRow, CurrentColumn));
	}
	HiNibble = MinHiNibble;
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
			Tile->GetTile()->GetTileWidth() - 1) >> 1);
	return true;
}
void
G42LockedID4BitIterator::ResetRow(void)
{
	int old_row = CurrentRow;
	int old_column = CurrentColumn;
	CurrentColumn = MinColumn;
   HiNibble = MinHiNibble;
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
			Tile->GetTile()->GetTileWidth() - 1) >> 1);
}
void
G42LockedID4BitIterator::SetLastRow(void)
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
			Tile->GetTile()->GetTileWidth() - 1) >> 1);
}
