/*-----------------------------------------------------------------------------
	file:			gtmpmem.h
	author:		David Ison
	classes:	 	
	date:			01/06/97
	revision:	3.0
	Desc:			(Class really designed for Macintosh; pass-thru to operator new 
					on MSWIN.)  Attempts to allocate a chunk of "temporary" memory. 
					If it can't, it tries to pull it from the application heap.  
					If that fails, it throws an exception.
-----------------------------------------------------------------------------*/
#if !defined(GTMPMEM_H)
#define GTMPMEM_H
#include "gtype.h"
#ifdef OLDWAY
class G42TempMem
{
public:
	G42TempMem (int32 size) throw (G42InvalidParameterException, G42MemoryException);
	~G42TempMem (void);	// No need for virtual
	operator byte * (void) const {return (TempBuffer);}
private:
	byte 		* TempBuffer;
	#ifdef MACOS
	Handle	TempHandle;
	#endif
};
#endif
template <class Type>
class G42TempMem
{
public:
	G42TempMem (int32 count, bool init = false);
	~G42TempMem (void);	// No need for virtual
	operator Type * (void) const {return ((Type *) TempBuffer);}
	Type & operator [] (uint32 index) 
		{return *((Type *) (TempBuffer + (index * sizeof (Type))));}
private:
	byte 		* TempBuffer;
	#ifdef MACOS
	Handle	TempHandle;
	#endif
};
template <class Type>
G42TempMem<Type>::G42TempMem (int32 count, bool init)
: 
	TempBuffer (0)
	#ifdef MACOS
	,TempHandle (0)
	#endif
{
	if (count <= 0) {
		throw (G42InvalidParameterException (0));
	}
	
	uint32 size = count * sizeof (Type);
	#ifdef MSWIN
	TempBuffer = new byte [size];
	#endif
	#ifdef MACOS  
	/*  First try to allocate from the system heap  */
	OSErr	err 	= noErr;
	TempHandle 	= ::TempNewHandle (size, &err);
	if (err) 
	{	
		/*  No room in system heap, try to pull it from the app heap  */
		TempHandle = ::NewHandle (size);
	}
	if (TempHandle) 
	{	
		::HLock (TempHandle);
		TempBuffer = (byte *) *TempHandle;
	}
	else 
	{
		throw (G42MemoryException());		
	}
	#endif
	if (init)
	{
		memset (TempBuffer, 0, size);
	}
}
template <class Type>
G42TempMem<Type>::~G42TempMem (void)
{
	if (TempBuffer)
	{
		#ifdef MSWIN
		delete [] TempBuffer;
		#endif
		#ifdef MACOS
		::HUnlock (TempHandle);
		::DisposeHandle (TempHandle);
		#endif
	}
}
#endif // GTMPMEM_H sentry
