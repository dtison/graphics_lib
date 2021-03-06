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
#include "giwpbm.h"
G42ImageWriterPbm::G42ImageWriterPbm(G42Notifier & notifier,
	G42BaseOptions * options)
:
	G42ImageWriterBase(notifier, options),
	TempRow(0), Type(PbmBinary), ImagePalette(0), NumPalette(0)
{
	SetInfo(G42PbmTypeId, "pbm", "Pbm", true);
}
G42ImageWriterPbm::G42ImageWriterPbm(void)
:
	G42ImageWriterBase(),
	TempRow(0), Type(PbmBinary), ImagePalette(0), NumPalette(0)
{
	SetInfo(G42PbmTypeId, "pbm", "Pbm", true);
}
G42ImageWriterPbm::~G42ImageWriterPbm()
{
	if (TempRow)
	{
		delete [] TempRow;
		TempRow = 0;
	}
	Valid = false;
}
bool
G42ImageWriterPbm::WriteInfo(G42ImageDescriptor * info)
{
	G42ImageWriterBase::WriteInfo(info); // set up base stuff
	ImagePalette = info->GetPalette();
	NumPalette = info->GetNumPalette();
	TrueDepth = (uintf)Depth;
	if (info->IsColor())
		Depth = 24;
	if (!Options || !dynamic_cast<G42ImagePbmOptions *>(Options))
	{
		G42ImagePbmOptions * options =
			new G42ImagePbmOptions();
		G42ThrowIfNull(options);
		AskForOptions(options);
	}
	G42ImagePbmOptions * options;
	if (Options)
	{
		options = dynamic_cast<G42ImagePbmOptions *>(Options);
	}
	if (options)
	{
		Type = options->GetPbmType();
	}
	else
	{
		Type = PbmAscii;
	}
	if (Type == PbmBinary)
	{
		if (Depth == 24)
			RowBytes = Width * 3;
		else
			RowBytes = Width;
	}
	else
	{
		RowBytes = 80; // we only need 70 of this, but best to be safe
	}
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
G42ImageWriterPbm::WriteRow(const byte * row, const byte *)
{
	if (Type == PbmBinary)
	{
		if (Depth == 24)
		{
			if (Descriptor->GetDepth() == 24)
			{
				G42ConstColor24Iterator sp(row);
				byte * dp = TempRow;
				for (uint32 i = 0; i < Width; i++)
				{
					G42ConstColor24 c(*sp);
					*dp++ = c.GetRed();
					*dp++ = c.GetGreen();
					*dp++ = c.GetBlue();
					sp++;
				}
			}
			else
			{
				G42ConstColor32Iterator sp(row);
				byte * dp = TempRow;
				for (uint32 i = 0; i < Width; i++)
				{
					G42ConstColor32 c(*sp);
					*dp++ = c.GetRed();
					*dp++ = c.GetGreen();
					*dp++ = c.GetBlue();
					sp++;
				}
			}
			G42ImageWriterBase::WriteRow(TempRow, 0);
		}
		else // grayscale and monochrome
		{
			if (Depth == 8)
			{
				G42ImageWriterBase::WriteRow(row, 0);
			}
			else if (Depth == 4)
			{
				const byte * sp = row;
				byte * dp = TempRow;
				int s = 4;
				for (int i = 0; i < Width; i++)
				{
					*dp++ = (byte)((*sp >> s) & 0xf);
					if (s)
					{
						s = 0;
					}
					else
					{
						s = 4;
						sp++;
					}
				}
				G42ImageWriterBase::WriteRow(TempRow, 0);
			}
			else
			{
				const byte * sp = row;
				byte * dp = TempRow;
				int m = 0x80;
				for (int i = 0; i < Width; i++)
				{
					*dp++ = (byte)((*sp & m) ? 1 : 0);
					if (m > 1)
					{
						m >>= 1;
					}
					else
					{
						m = 0x80;
						sp++;
					}
				}
				G42ImageWriterBase::WriteRow(TempRow, 0);
			}
		}
	}
	else // ascii
	{
		if (Depth == 24)
		{
			if (Descriptor->GetDepth() == 24)
			{
				G42ConstColor24Iterator sp(row);
				char * dp = (char *)TempRow;
				for (uint32 i = 0; i < Width; i++)
				{
					G42ConstColor24 c(*sp);
					sprintf(dp, "%d %d %d ", c.GetRed(), c.GetGreen(), c.GetBlue());
					dp += strlen(dp);
					if ((int)(dp - (char *)TempRow) > 57)
					{
						*(dp - 1) = '\n';
						PutBytes(TempRow, (int)(dp - (char *)TempRow));
						dp = (char *)TempRow;
					}
					sp++;
				}
				if (dp != (char *)TempRow)
				{
					*(dp - 1) = '\n';
					PutBytes(TempRow, (int)(dp - (char *)TempRow));
				}
			}
			else
			{
				G42ConstColor32Iterator sp(row);
				char * dp = (char *)TempRow;
				for (uint32 i = 0; i < Width; i++)
				{
					G42ConstColor32 c(*sp);
					sprintf(dp, "%d %d %d ", c.GetRed(), c.GetGreen(), c.GetBlue());
					dp += strlen(dp);
					if ((int)(dp - (char *)TempRow) > 57)
					{
						*(dp - 1) = '\n';
						PutBytes(TempRow, (int)(dp - (char *)TempRow));
						dp = (char *)TempRow;
					}
					sp++;
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
				byte v = (byte)((*sp & m) ? 1 : 0);
            *dp++ = (byte)('0' + v);
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
