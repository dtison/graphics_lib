// g42wpbm.cpp - Pbm File
/*************************************************************
	File:          g42pbm.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of g42ribase that handles Pbm files
   Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
      Code   Date   Name and Description
***************************************************************/
#include "g42itype.h"
#include "g42wpbm.h"
#include "g42idata.h"
#include "g42iter.h"
G42PbmWriter::G42PbmWriter(G42RasterImageWriter & parent) :
	G42RasterWriterBase(parent), TempRow(0), Type(PbmBinary),
	ColorType(G42ImageInfo::ColorTypeTrueColor),
	ImagePalette(0), NumPalette(0)
{
}
G42PbmWriter::~G42PbmWriter()
{
	if (TempRow)
	{
		delete [] TempRow;
		TempRow = 0;
	}
	Valid = false;
}
bool
G42PbmWriter::WriteInfo(G42ImageInfo * info)
{
	G42RasterWriterBase::WriteInfo(info); // set up base stuff
	Type = info->ImageType;
	ColorType = info->ColorType;
	ImagePalette = info->ImagePalette;
	NumPalette = info->NumPalette;
	TrueDepth = (uint)Depth;
	if (ColorType == G42ImageInfo::ColorTypeTrueColor ||
			ColorType == G42ImageInfo::ColorTypePalette)
		Depth = 24;
	if (Type == PbmBinary)
		RowBytes = ((Width * (uint32)Depth + 7) >> 3);
	else
   	RowBytes = 80; // we only need 70 of this, but best to be safe
	TempRow = new byte [(size_t)RowBytes];
	if (!TempRow)
	{
		Valid = false;
		return false;
	}
	PutByte('P');
	if (Type == PbmAscii)
	{
		if (Depth == 24)
			PutByte('3');
		else if (Depth > 1)
			PutByte('2');
		else
			PutByte('1');
	}
	else
	{
		if (Depth == 24)
			PutByte('6');
		else if (Depth > 1)
			PutByte('5');
		else
			PutByte('4');
	}
	PutByte('\n'); // this being a unix format, this seems safest
	sprintf((char *)TempRow, "%lu", (uint32)Width);
	PutBytes(TempRow, strlen((char *)TempRow));
	PutByte('\n');
	sprintf((char *)TempRow, "%lu", (uint32)Height);
	PutBytes(TempRow, strlen((char *)TempRow));
	PutByte('\n');
	if (Depth > 1)
	{
		if (Depth == 4)
			PutBytes((byte *)"15\n", 3);
		else
			PutBytes((byte *)"255\n", 4);
	}
	return Valid;
}
bool
G42PbmWriter::WriteRow(const byte * row, const byte *)
{
	if (Type == PbmBinary)
	{
		if (Depth == 24)
		{
			if (TrueDepth == 1)
			{
				const byte * sp = row;
				byte * dp = TempRow;
				int m = 0x80;
				for (uint32 i = 0; i < Width; i++)
				{
					byte v = (*sp & m) ? 1 : 0;
					*dp++ = ImagePalette[v].red;
					*dp++ = ImagePalette[v].green;
					*dp++ = ImagePalette[v].blue;
					if (m > 1)
					{
						m >>= 1;
					}
					else
					{
						sp++;
						m = 0x80;
					}
				}
			}
			else if (TrueDepth == 4)
			{
				const byte * sp = row;
				byte * dp = TempRow;
				int s = 4;
				for (uint32 i = 0; i < Width; i++)
				{
					byte v = (byte)((*sp >> s) & 0xf);
					*dp++ = ImagePalette[v].red;
					*dp++ = ImagePalette[v].green;
					*dp++ = ImagePalette[v].blue;
					if (s)
					{
						s = 0;
					}
					else
					{
						sp++;
						s = 4;
					}
				}
			}
			else if (TrueDepth == 8)
			{
				const byte * sp = row;
				byte * dp = TempRow;
				for (uint32 i = 0; i < Width; i++)
				{
					byte v = *sp++;
					*dp++ = ImagePalette[v].red;
					*dp++ = ImagePalette[v].green;
					*dp++ = ImagePalette[v].blue;
				}
			}
			else
			{
				G42ConstByte24BitIterator sp(row);
				byte * dp = TempRow;
				for (uint32 i = 0; i < Width; i++)
				{
					*dp++ = sp.GetRed();
					*dp++ = sp.GetGreen();
					*dp++ = sp.GetBlue();
					sp.Increment();
				}
			}
			G42RasterWriterBase::WriteRow(TempRow, 0);
		}
		else // grayscale and monochrome
		{
			G42RasterWriterBase::WriteRow(row, 0);
		}
	}
	else // ascii
	{
		if (Depth == 24)
		{
			if (TrueDepth == 1)
			{
				const byte * sp = row;
				char * dp = (char *)TempRow;
				int m = 0x80;
				for (uint32 i = 0; i < Width; i++)
				{
					byte v = (*sp & m) ? 1 : 0;
					sprintf(dp, "%d %d %d ", ImagePalette[v].red,
						ImagePalette[v].green, ImagePalette[v].blue);
					dp += strlen(dp);
					if ((int)(dp - (char *)TempRow) > 57)
					{
						*(dp - 1) = '\n';
						PutBytes(TempRow, (int)(dp - (char *)TempRow));
						dp = (char *)TempRow;
					}
					if (m > 1)
					{
						m >>= 1;
					}
					else
					{
						sp++;
						m = 0x80;
					}
				}
				if (dp != (char *)TempRow)
				{
					*(dp - 1) = '\n';
					PutBytes(TempRow, (int)(dp - (char *)TempRow));
				}
			}
			else if (TrueDepth == 4)
			{
				const byte * sp = row;
				char * dp = (char *)TempRow;
				int s = 4;
				for (uint32 i = 0; i < Width; i++)
				{
					byte v = (byte)((*sp >> s) & 0xf);
					sprintf(dp, "%d %d %d ", ImagePalette[v].red,
						ImagePalette[v].green, ImagePalette[v].blue);
					dp += strlen(dp);
					if ((int)(dp - (char *)TempRow) > 57)
					{
						*(dp - 1) = '\n';
						PutBytes(TempRow, (int)(dp - (char *)TempRow));
						dp = (char *)TempRow;
					}
					if (s)
					{
						s = 0;
					}
					else
					{
						sp++;
						s = 4;
					}
				}
				if (dp != (char *)TempRow)
				{
					*(dp - 1) = '\n';
					PutBytes(TempRow, (int)(dp - (char *)TempRow));
				}
			}
			else if (TrueDepth == 8)
			{
				const byte * sp = row;
				char * dp = (char *)TempRow;
				for (uint32 i = 0; i < Width; i++)
				{
					byte v = *sp++;
					sprintf(dp, "%d %d %d ", ImagePalette[v].red,
						ImagePalette[v].green, ImagePalette[v].blue);
					dp += strlen(dp);
					if ((int)(dp - (char *)TempRow) > 57)
					{
						*(dp - 1) = '\n';
						PutBytes(TempRow, (int)(dp - (char *)TempRow));
						dp = (char *)TempRow;
					}
				}
				if (dp != (char *)TempRow)
				{
					*(dp - 1) = '\n';
					PutBytes(TempRow, (int)(dp - (char *)TempRow));
				}
			}
			else
			{
				G42ConstByte24BitIterator sp(row);
				char * dp = (char *)TempRow;
				for (uint32 i = 0; i < Width; i++)
				{
					sprintf(dp, "%d %d %d ", sp.GetRed(), sp.GetGreen(), sp.GetBlue());
					dp += strlen(dp);
					if ((int)(dp - (char *)TempRow) > 57)
					{
						*(dp - 1) = '\n';
						PutBytes(TempRow, (int)(dp - (char *)TempRow));
						dp = (char *)TempRow;
					}
					sp.Increment();
				}
				if (dp != (char *)TempRow)
				{
					*(dp - 1) = '\n';
					PutBytes(TempRow, (int)(dp - (char *)TempRow));
				}
			}
		}
		else if (TrueDepth == 4)
		{
			const byte * sp = row;
			char * dp = (char *)TempRow;
			int s = 4;
			for (uint32 i = 0; i < Width; i++)
			{
				byte v = (byte)((*sp >> s) & 0xf);
				sprintf(dp, "%d ", v);
				dp += strlen(dp);
				if ((int)(dp - (char *)TempRow) > 65)
				{
					*(dp - 1) = '\n';
					PutBytes(TempRow, (int)(dp - (char *)TempRow));
					dp = (char *)TempRow;
				}
				if (s)
				{
					s = 0;
				}
				else
				{
					sp++;
					s = 4;
				}
			}
			if (dp != (char *)TempRow)
			{
				*(dp - 1) = '\n';
				PutBytes(TempRow, (int)(dp - (char *)TempRow));
			}
		}
		else if (TrueDepth == 8)
		{
			const byte * sp = row;
			char * dp = (char *)TempRow;
			for (uint32 i = 0; i < Width; i++)
			{
				byte v = *sp++;
				sprintf(dp, "%d ", v);
				dp += strlen(dp);
				if ((int)(dp - (char *)TempRow) > 65)
				{
					*(dp - 1) = '\n';
					PutBytes(TempRow, (int)(dp - (char *)TempRow));
					dp = (char *)TempRow;
				}
			}
			if (dp != (char *)TempRow)
			{
				*(dp - 1) = '\n';
				PutBytes(TempRow, (int)(dp - (char *)TempRow));
			}
		}
		else
		{
			const byte * sp = row;
			char * dp = (char *)TempRow;
			int m = 0x80;
			for (uint32 i = 0; i < Width; i++)
			{
				byte v = (*sp & m) ? 1 : 0;
            *dp++ = '0' + v;
				if ((int)(dp - (char *)TempRow) > 67)
				{
					*dp++ = '\n';
					*dp++ = '\0';
					PutBytes(TempRow, (int)(dp - (char *)TempRow));
					dp = (char *)TempRow;
				}
				if (m > 1)
				{
					m >>= 1;
				}
				else
				{
					sp++;
					m = 0x80;
				}
			}
			if (dp != (char *)TempRow)
			{
				*dp++ = '\n';
				*dp++ = '\0';
				PutBytes(TempRow, (int)(dp - (char *)TempRow));
			}
		}
		CurrentRow++;
	}
	return Valid;
}
