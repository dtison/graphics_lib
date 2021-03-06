#if !defined(GRBUF_H)
#define GRBUF_H
#include "gtype.h"
class G42VirtualBufferWrap;
class G42VirtualBuffer
{
friend	G42VirtualBufferWrap;
public:
	G42VirtualBuffer(void);
	~G42VirtualBuffer(void);
	bool IsValid(void) const
		{return Valid;}
	uint32 GetNumBytes(void) const
		{return NumBytes;}
	void AddBytes(byte * buffer, int32 bytes);
	void ReleaseBytes(int32 bytes);
	void StoreBytes(void);
	operator byte * (void) const;
private:
	byte *	Buffer;
	int32 	BufferSize;
	uint32	NumBytes;
	int32		CurrentPtr;
	bool		Storing;
	bool		Valid;
	void Clear(void);
	// Hidden Copy Contructor and operator=
	G42VirtualBuffer(G42VirtualBuffer &);
	G42VirtualBuffer & operator= (G42VirtualBuffer &)
		{return *this;}
};
class G42VirtualBufferWrap
{
public:
	G42VirtualBufferWrap(G42VirtualBuffer & buffer);
	~G42VirtualBufferWrap(void);
private:
	G42VirtualBuffer &	Buffer;
	// Hidden Copy Contructor and operator=
	G42VirtualBufferWrap(G42VirtualBufferWrap &);
	G42VirtualBufferWrap & operator= (G42VirtualBufferWrap &)
		{return *this;}
};
inline
G42VirtualBuffer::operator byte *(void) const
{
	if (Storing)
		return Buffer + CurrentPtr;
	else
		return Buffer;
}
#endif // GRBUF_H sentry
