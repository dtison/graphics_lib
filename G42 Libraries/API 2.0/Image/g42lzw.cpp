// g42lzw.cpp - Lzw decompressor
/*************************************************************
	File:          g42lzw.cpp
   Copyright (c) 1996, Group 42, Inc.
	Description:   Decompresses lzw streams
   Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
   Modification History:
      Code   Date   Name and Description
***************************************************************/
#include "g42itype.h"
#include "g42lzw.h"
G42Lzw::G42Lzw(int row_bytes, G42LzwType type, int code_size) :
	G42Compression(row_bytes),
	LzwType(type), Bits(code_size), OldCode(LZW_NO_CODE),
	OldToken(LZW_NO_CODE), BitsAvailable(8), SaveCode(0),
	Shift(0)
{
	Bits2 = 1 << Bits;
	NextCode = Bits2 + 2;
	CodeSize = Bits + 1;
	CodeSize2 = 1 << CodeSize;
	BitsNeeded = CodeSize;
	if (LzwType == LzwGif)
	{
		if (CodeSize < 8)
			Mask = 0xff >> (8 - CodeSize);
		else
			Mask = 0xff;
	}
	else
	{
		Shift = 1;
		Mask = 0xff;
	}
}
G42Lzw::~G42Lzw()
{
}
void
G42Lzw::ProcessBuffer(byte * input_buffer, int num_input_buffer)
{
	if (Done || !Valid)
		return;
	byte * input_ptr = input_buffer;
	while (input_ptr < input_buffer + num_input_buffer)
	{
		/* GES: gif and tif have radically different ideas about
			bit order, etc. */
		if (LzwType == LzwGif)
		{
			ThisCode = SaveCode;
			if (Shift > 0)
				ThisCode |= (((int)(*input_ptr) & Mask) << Shift);
			else
				ThisCode |= (((int)(*input_ptr) & Mask) >> (-Shift));
			if (BitsNeeded > BitsAvailable)
			{
				BitsNeeded -= BitsAvailable;
				BitsAvailable = 8;
				input_ptr++;
				Shift = (CodeSize - BitsNeeded);
				if (BitsNeeded > 8)
					Mask = 0xff;
				else
					Mask = (0xff >> (8 - BitsNeeded));
				SaveCode = ThisCode;
				continue; // not enough data for code yet
			}
			else if (BitsNeeded == BitsAvailable)
			{
				BitsNeeded = CodeSize;
				BitsAvailable = 8;
				input_ptr++;
				Shift = 0;
				if (CodeSize < 8)
					Mask = 0xff >> (8 - CodeSize);
				else
					Mask = 0xff;
				SaveCode = 0;
			}
			else
			{
				SaveCode = 0;
				BitsAvailable -= BitsNeeded;
				BitsNeeded = CodeSize;
				Shift = BitsAvailable - 8; // Shift needs to be negitive here
				if (BitsAvailable > BitsNeeded)
					Mask = 0xff >> (BitsAvailable - BitsNeeded);
				else
					Mask = 0xff;
				SaveCode = 0;
			}
		}
		else // Tiff support
		{
			ThisCode = SaveCode;
			if (Shift > 0)
				ThisCode |= (((int)(*input_ptr) & Mask) << Shift);
			else
				ThisCode |= (((int)(*input_ptr) & Mask) >> (-Shift));
			if (BitsNeeded > BitsAvailable)
			{
				BitsNeeded -= BitsAvailable;
				BitsAvailable = 8;
				input_ptr++;
				Shift = (BitsNeeded - 8);
				Mask = 0xff;
				SaveCode = ThisCode;
				continue; // not enough data for code yet
			}
			else if (BitsNeeded == BitsAvailable)
			{
				BitsNeeded = CodeSize;
				BitsAvailable = 8;
				input_ptr++;
				Shift = CodeSize - 8;
				Mask = 0xff;
				SaveCode = 0;
			}
			else
			{
				SaveCode = 0;
				BitsAvailable -= BitsNeeded;
				BitsNeeded = CodeSize;
				Shift = CodeSize - BitsAvailable;
				Mask = 0xff >> (8 - BitsAvailable);
				SaveCode = 0;
			}
		}
//		ThisCode &= WordMaskTable[CodeSize];
		CurrentCode = ThisCode;
		if (ThisCode == (Bits2 + 1)) // end of image
		{
			Done = true;
			break;	/* found EOI */
		}
		if (ThisCode > NextCode) // error
		{
			Valid = false;
			return;
		}
		if (ThisCode == Bits2) // reset table
		{
			BitsNeeded -= (CodeSize - (Bits + 1));
			NextCode = Bits2 + 2;
			CodeSize = (Bits + 1);
			CodeSize2 = (1 << CodeSize);
			OldToken = OldCode = LZW_NO_CODE;
			if (LzwType == LzwGif)
			{
				if (BitsAvailable > BitsNeeded)
					Mask = 0xff >> (BitsAvailable - BitsNeeded);
				else
					Mask = 0xff;
			}
			else
			{
				Shift = BitsNeeded - BitsAvailable;
				Mask = 0xff >> (8 - BitsAvailable);
         }
			continue;
		}
		u = FirstCodeStack;
		if (ThisCode == NextCode)
		{
			if (OldCode == LZW_NO_CODE)
			{
				Valid = false;
				return;
			}
			*(u)++ = OldToken;
			ThisCode = OldCode;
		}
		while (ThisCode >= Bits2)
		{
			*(u)++ = LastCodeStack[ThisCode];
			ThisCode = CodeStack[ThisCode];
		}
		OldToken = ThisCode;
		do
		{
			PutByte(ThisCode);
         if (IsDone())
         	return;
#if 0
			LineBuffer[(BytePtr)++] = ThisCode;
			if (BytePtr >= RowBytes)
			{
				OutputRow(LineBuffer);
				BytePtr = 0;
			}
#endif
			if (u <= FirstCodeStack)
				break;
			ThisCode = *--(u);
		} while(1);
		int TiffFudge;
		if (LzwType == LzwGif)
			TiffFudge = 0;
		else
			TiffFudge = 1;
		if (NextCode + TiffFudge < 4096 && OldCode != LZW_NO_CODE)
		{
			CodeStack[NextCode] = OldCode;
			LastCodeStack[NextCode] = OldToken;
			NextCode++;
			if (NextCode >= CodeSize2 - TiffFudge && CodeSize < 12)
			{
				CodeSize2 = (1 << (++CodeSize));
				BitsNeeded++;
				if (LzwType == LzwGif)
				{
					if (Mask < 0xff)
					{
						Mask <<= 1;
						Mask |= 1;
					}
				}
				else
				{
					Shift = BitsNeeded - BitsAvailable;
					Mask = 0xff >> (8 - BitsAvailable);
				}
			}
		}
		OldCode = CurrentCode;
	}
	return;
}
