// grbuf.cpp - virtual buffer
#include "grbuf.h"
G42VirtualBuffer::G42VirtualBuffer(void)
	: Buffer(0), BufferSize(0), CurrentPtr(-1L), NumBytes(0L),
	Storing(false), Valid(true)
{}
G42VirtualBuffer::~G42VirtualBuffer(void)
{
	Clear();
}
void
G42VirtualBuffer::AddBytes(byte * buffer, int32 bytes)
{
	if (!Valid)
		return;
	if (Storing)
	{
		if ((CurrentPtr + NumBytes + bytes) > BufferSize)
		{
			BufferSize =  NumBytes + bytes;
			BufferSize += (16384L - BufferSize % 16384L);
			byte * tmp;
			try
			{
				tmp = new byte [BufferSize];
			}
			catch (xalloc)
			{
				Valid = false;
				throw G42MemoryException();
			}
			memcpy(tmp, Buffer + CurrentPtr, NumBytes);
			memcpy(tmp + NumBytes, buffer, bytes);
			CurrentPtr = 0L;
			delete [] Buffer;
			Buffer = tmp;
		}
		else
			memcpy(Buffer + CurrentPtr + NumBytes, buffer, bytes);
		NumBytes += bytes;
	}
	else
	{
		Buffer = buffer;
		NumBytes = bytes;
	}
}
void
G42VirtualBuffer::ReleaseBytes(int32 bytes)
{
	if (!Valid)
		return;
		 
	if (bytes < NumBytes)
	{
		NumBytes -= bytes;
		if (Storing)
			CurrentPtr += bytes;
		else
			Buffer += bytes;
		return;
	}
	if (Storing && Buffer)
		delete [] Buffer;
	Buffer = 0;
	NumBytes = 0L;
	Storing = false;
}
void
G42VirtualBuffer::StoreBytes(void)
{
	if (!Valid || Storing)
		return;
		
	Storing = true;
	BufferSize = NumBytes;
	BufferSize += (16384L - BufferSize % 16384L);
	byte * tmp;
	try
	{
		tmp = new byte [BufferSize];
	}
	catch (xalloc)
	{
		Valid = false;
		throw G42MemoryException();
	}
	memcpy(tmp, Buffer, NumBytes);
	CurrentPtr = 0L;
	Buffer = tmp;
}
void
G42VirtualBuffer::Clear(void)
{
	if (Valid && Storing && Buffer)
		delete [] Buffer;
	Buffer = 0;
}
G42VirtualBufferWrap::G42VirtualBufferWrap(G42VirtualBuffer & buffer)
	: Buffer(buffer)
{}
G42VirtualBufferWrap::~G42VirtualBufferWrap(void)
{
	try
	{
		Buffer.StoreBytes();
	}
	catch (...)
	{}
}
