/*-----------------------------------------------------------------------------
	file:			gtools.h
	author:		?
	classes:		G42Ptr
	date:			?
	revision:	?
-----------------------------------------------------------------------------*/
// Compiler directives
#if !defined(G42_TOOLS_H)
#define G42_TOOLS_H
#include "gtype.h"
/*-----------------------------------------------------------------------------
	class G42Ptr
	Description:
		G42Ptr is a pointer template class that automatically deletes its pointer
		when it goes out of scope.  This can be very useful for exception
		handling, etc.  It also has a default constructor for use in
		G42Vectors (above).  It has a Set and Clear function, as well as
		a Release and Take function to transfer ownership to and from G42Ptr's.
		While operator = and the copy constructor have been hidden, it is
		important to be careful because bad things can be done with the Set
		function. Do not Set a pointer that you don't want deleted.  Use a plain
		pointer (a naked pointer) or a reference instead. The Set function will
		throw an allocation error if it is passed a null pointer.  If pointer
		needs to be set to null, use Clear.  G42Ptr has an operator T * and an
		operator -> override for ease of use.  It should be able to be used much
		like a pointer.  Sometimes it gets confused between the const and the
		non-const forms of these overrides.  If that is true, it will need to be
		cast it manually.  One common problem is checking the pointer for null,
		in which case the function NotNull() has been provided so the following
		can be done:
			if (ptr.NotNull())
-----------------------------------------------------------------------------*/
//#if !defined(G42_EXPAND_PTR)
//#if defined(__TURBOC__)
//#pragma option -Jgx
//#pragma option -vi
//#endif
//#endif
template <class T>
class G42Ptr
{
public:
	// constructors/destructors
	G42Ptr(void); // throw();
	G42Ptr(T * ptr); // throw(xalloc)
	~G42Ptr(void); // throw();
	// member functions
	void Clear(void); // throw();
//	const T * GetConstPtr(void) const; // throw();
//	const T & GetConstReference(void) const;
			// throw(G42NullDereferenceException);
	T * GetPtr(void) const; // throw();
	T & GetReference(void) const; // throw(G42NullDereferenceException);
	bool NotNull(void) const; // throw();
	bool operator ! (void) const; // throw();
	T * Release(void); // throw();
	void Set(T * ptr); // throw(xalloc)
	void Take(G42Ptr<T> & ref); // throw();
	G42Ptr<T> & operator = (T * ptr); // throw();
	// operators
	operator T * (void) const; // throw();
	T & operator * (void) const; // throw(G42NullDereferenceException);
	T * operator -> (void) const; // throw(G42NullDereferenceException);
//	operator const T * (void) const; // throw();
//	const T * operator -> (void) const; // throw(G42NullDereferenceException);
private:
	// data members
	T * Ptr;
	// hidden copy constructor and operator =
	G42Ptr(G42Ptr <T> &) {}
	G42Ptr<T> & operator = (G42Ptr<T> &)
			{return *this;}
};
/*-----------------------------------------------------------------------------
	class G42APtr
	Description:
		Same as G42Ptr, but with arrays.  It uses delete [] instead of delete,
		and it has an operator [] override instead of an operator -> override.
-----------------------------------------------------------------------------*/
template <class T>
class G42APtr
{
public:
	// constructors/destructors
	G42APtr(void); // throw();
	G42APtr(T * ptr); // throw(xalloc)
	~G42APtr(void); // throw();
	// member functions
	void Clear(void); // throw();
//	const T * GetConstPtr(void) const; // throw();
//	const T & GetConstReference(size_t index) const;
			// throw(G42NullDereferenceException);
	T * GetPtr(void) const; // throw();
	T & GetReference(size_t index) const; // throw(G42NullDereferenceException);
	bool NotNull(void) const; // throw();
	bool operator ! (void) const; // throw();
	T * Release(void); // throw();
	void Set(T * ptr); // throw(xalloc)
	void Take(G42APtr<T> & ref); // throw();
	G42APtr<T> & operator = (T * ptr); // throw();
	// operators
	operator T * (void) const; // throw();
	T & operator [] (size_t index) const; // throw(G42NullDereferenceException);
//	operator const T * (void) const; // throw();
//	const T & operator [] (size_t index) const;
			// throw(G42NullDereferenceException);
private:
	// data members
	T * Ptr;
	// hidden copy constructor and operator =
	G42APtr(G42APtr <T> &) {}
	G42APtr<T> & operator = (G42APtr<T> &)
		{return *this;}
};
//#if !defined(G42_EXPAND_PTR)
//#if defined(__TURBOC__)
//#pragma option -Jg.
//#pragma option -vi.
//#endif
//#endif
/*-----------------------------------------------------------------------------
	class G42Vector
	Description:
		An expandable array that works just like a regular array except you don't
		have to allocate space or worry about running over the array bounds.
		Operator [] is overridden so you can access it just like a normal array.
		Whatever T is must have a default constructor.  Operator [] may throw a
		memory allocation exception and an out of bounds exception, so be
		prepared to handle these cases.
		This class often has a int paired with it to keep track of the number of
		elements in the array.  Also, it often is used as an array of pointers.
-----------------------------------------------------------------------------*/
// This is a define I put in when I added the capability to use G42Ptr.
// I don't have enough experience with it, so I'm unsure if it will
// really work as advertized.  If there are problems, try commenting out
// this section and see if that fixes it.  If it does, please tell me, so
// I can fix any problems with the pointer classes.
//#define USE_PTR
template <class T>
class G42Vector
{
public:
	// constructor/destructor
	G42Vector(void);
	~G42Vector(void);
	// member functions
	void Clear(void);
	// operators
	T & operator [] (int32 index);
			// throw(G42MemoryException, G42OutOfBoundsException);
	const T & operator [] (int32 index) const; // throw(G42OutOfBoundsException);
private:
	// data members
#ifdef USE_PTR
	G42APtr	<G42Ptr <T>> Array;	// the array in the self-deleting form
#else
	T **		Array; 					// the array in naked pointer form
#endif
	int32		NumHashes; 				// number of hash entries
	int32		NumIndeces; 			// total number of indeces currently held
	// member functions
	void ExpandArray(int32 index); // throw(G42MemoryException);
	// hidden copy constructor and operator =
	G42Vector(G42Vector &) {}
	G42Vector & operator = (G42Vector &)
			{return *this;}
};
/*  An abbreviation for G42Vector */
//#define G42XArray G42Vector
#define G42ExpandableArray G42Vector
/*-----------------------------------------------------------------------------
	G42Ptr inline functions:
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
	Function - G42Ptr constructor
	Description:
		Clears pointer to null
-----------------------------------------------------------------------------*/
template <class T>
inline
G42Ptr<T>::G42Ptr(void)
	: Ptr(0)
{
}
/*-----------------------------------------------------------------------------
	Function - G42Ptr constructor
	Parameters:
		T * ptr - Pointer to set to.
	Exceptions:
		xalloc on null
	Description:
		Sets pointer to what is passed in.  Note that this does the MAC null
		test, so you can do:
			G42Ptr<T> ptr(new T (...));
-----------------------------------------------------------------------------*/
template <class T>
inline
G42Ptr<T>::G42Ptr(T * ptr) // throws(xalloc)
	: Ptr(0)
{
	Set(ptr);
}
/*-----------------------------------------------------------------------------
	Function - ~G42Ptr destructor
	Description:
		Deletes Ptr if it isn't null.  Assume if Ptr isn't null, it needs deleted
		(general assumption of the class).
-----------------------------------------------------------------------------*/
template <class T>
inline
G42Ptr<T>::~G42Ptr(void)
{
	Clear();
}
/*-----------------------------------------------------------------------------
	Function - Clear
	Description:
		Deletes Ptr if it isn't null.  Assume if Ptr isn't null, it needs deleted
		(general assumption of the class).
-----------------------------------------------------------------------------*/
template <class T>
inline void
G42Ptr<T>::Clear(void)
{
	if (Ptr)
	{
		delete Ptr;
		Ptr = 0;
	}
}
/*-----------------------------------------------------------------------------
	Function - GetConstPtr
	Returns:
		const T * - A const form of the pointer (could be null).
	Description:
		Can be used if class is const.
-----------------------------------------------------------------------------*/
/*
template <class T>
inline const T *
G42Ptr<T>::GetConstPtr(void) const // throw();
{
	return Ptr;
}
*/
/*-----------------------------------------------------------------------------
	Function - GetConstReference
	Returns:
		const T & - A const reference.
	Exceptions:
		G42NullDereferenceException
	Description:
		Can be used if class is const.
-----------------------------------------------------------------------------*/
/*
template <class T>
const T &
G42Ptr<T>::GetConstReference(void) const // throw(G42NullDereferenceException);
{
	if (!Ptr)
		throw G42NullDereferenceException();
	return *Ptr;
}
*/
/*-----------------------------------------------------------------------------
	Function - GetPtr
	Returns:
		T * - The pointer (could be null)
-----------------------------------------------------------------------------*/
template <class T>
inline T *
G42Ptr<T>::GetPtr(void) const // throw();
{
	return Ptr;
}
/*-----------------------------------------------------------------------------
	Function - GetReference
	Returns:
		T & - A const reference to what the pointer was pointing towards.
	Exceptions:
		G42NullDereferenceException
-----------------------------------------------------------------------------*/
template <class T>
inline T &
G42Ptr<T>::GetReference(void) const // throw(G42NullDereferenceException);
{
	if (!Ptr)
		throw G42NullDereferenceException();
	return *Ptr;
}
/*-----------------------------------------------------------------------------
	Function - NotNull
	Returns:
		bool - True if ptr is not null.  False if it is null.
	Description:
		Mainly used as if (ptr.NotNull()).  Converts ptr to a bool.
-----------------------------------------------------------------------------*/
template <class T>
inline bool
G42Ptr<T>::NotNull(void) const
{
	return (Ptr != 0);
}
/*-----------------------------------------------------------------------------
	Function - operator !
	Returns:
		bool - True if ptr is null.  False if it is not null.
	Description:
		Mainly used as if (ptr.NotNull()).  Converts ptr to a bool.
-----------------------------------------------------------------------------*/
template <class T>
inline bool
G42Ptr<T>::operator ! (void) const
{
	return (Ptr == 0);
}
/*-----------------------------------------------------------------------------
	Function - Release
	Returns:
		T * - The pointer being released.
	Description:
		Used when giving up ownership of the pointer to someone else. Zeros out
		the pointer and returns it.  Assume whoever gets the pointer will delete
		it.
-----------------------------------------------------------------------------*/
template <class T>
inline T *
G42Ptr<T>::Release(void)
{
	T * ptr = Ptr;		// save value so we can return it
	Ptr = 0;
	return ptr;
}
/*-----------------------------------------------------------------------------
	Function - Set
	Parameters:
		T * ptr - The pointer to set to.
	Exceptions:
		xalloc if ptr is null
	Description:
		Deletes any old pointer and sets Ptr to the new pointer.  If the new
		pointer is null, it will throw xalloc.  Assume we own ptr and need to
		delete it when we go out of scope, and if ptr is null, it came from a
		new and xalloc needs to be thrown (which will happen before old pointer
		is cleared).
-----------------------------------------------------------------------------*/
template <class T>
inline void
G42Ptr<T>::Set(T * ptr) // throws(xalloc)
{
	G42ThrowIfNull(ptr);
	Clear();
	Ptr = ptr;
}
/*-----------------------------------------------------------------------------
	Function - Set
	Parameters:
		T * ptr - The pointer to set to.
	Description:
		Deletes any old pointer and sets Ptr to the new pointer.  Assume we own
		ptr and need to delete it when we go out of scope.
-----------------------------------------------------------------------------*/
template <class T>
inline G42Ptr<T> &
G42Ptr<T>::operator = (T * ptr) // throws()
{
	Clear();
	Ptr = ptr;
	return *this;
}
/*-----------------------------------------------------------------------------
	Function - Take
	Parameters:
		G42Ptr<T> & ref - where to take the pointer from
	Description:
		Deletes any old pointer and sets Ptr to the pointer that ref owns. Clears
		out ref.  If ref doesn't have a pointer, it clears out this likewise.
-----------------------------------------------------------------------------*/
template <class T>
inline void
G42Ptr<T>::Take(G42Ptr<T> & ref)
{
	if (&ref != this)		// be sure we don't try to take from ourselves
	{
		if (ref.Ptr)		// be sure to check, so we don't throw an xalloc
		{
			Set(ref.Release());
		}
		else
		{
			Clear();
		}
	}
}
/*-----------------------------------------------------------------------------
	Function - operator T *
	Returns:
		T * - The pointer (could be null)
-----------------------------------------------------------------------------*/
template <class T>
inline
G42Ptr<T>::operator T * (void) const
{
	return Ptr;
}
/*-----------------------------------------------------------------------------
	Function - operator T *
	Returns:
		T * - The pointer (could be null)
-----------------------------------------------------------------------------*/
template <class T>
inline T &
G42Ptr<T>::operator * (void) const
{
	if (!Ptr)
		throw G42NullDereferenceException();
	return *Ptr;
}
/*-----------------------------------------------------------------------------
	Function - operator ->
	Returns:
		T * - a pointer to be dereferenced by ->
	Exceptions:
		G42NullDereferenceException
	Description:
		Returns a reference to what the pointer was pointing to.
-----------------------------------------------------------------------------*/
template <class T>
inline T *
G42Ptr<T>::operator -> (void) const
{
	if (!Ptr)
		throw G42NullDereferenceException();
	return Ptr;
}
/*-----------------------------------------------------------------------------
	Function - operator const T *
	Returns:
		const T * - a const form of the pointer (could be null).
	Description:
		Can be used if class is const.
-----------------------------------------------------------------------------*/
/*
template <class T>
inline
G42Ptr<T>::operator const T * (void) const
{
	return Ptr;
}
*/
/*-----------------------------------------------------------------------------
	Function - operator ->
	Returns:
		const T * - a const ptr to dereference with ->.
	Exceptions:
		G42NullDereferenceException
	Description:
		Can be used if class is const.
-----------------------------------------------------------------------------*/
/*
template <class T>
inline const T *
G42Ptr<T>::operator -> (void) const
{
	if (!Ptr)
		throw G42NullDereferenceException();
	return Ptr;
}
*/
/*-----------------------------------------------------------------------------
	G42APtr inline functions:
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
	Function - G42APtr constructor
	Description:
		Clears pointer to null
-----------------------------------------------------------------------------*/
template <class T>
inline
G42APtr<T>::G42APtr(void)
:
	Ptr(0)
{
}
/*-----------------------------------------------------------------------------
	Function - G42APtr constructor
	Parameters:
		T * ptr - Pointer to set to.
	Exceptions:
		xalloc on null
	Description:
		Sets pointer to what is passed in.  Note that this does the MAC null
		test, so you can do:
			G42APtr<T> ptr(new T (...));
-----------------------------------------------------------------------------*/
template <class T>
inline
G42APtr<T>::G42APtr(T * ptr)
:
	Ptr(0) // throws(xalloc)
{
	Set(ptr);
}
/*-----------------------------------------------------------------------------
	Function - ~G42APtr destructor
	Description:
		Deletes Ptr if it isn't null.  Assume if Ptr isn't null, it needs deleted
		(general assumption of the class).
-----------------------------------------------------------------------------*/
template <class T>
inline
G42APtr<T>::~G42APtr(void)
{
	Clear();
}
/*-----------------------------------------------------------------------------
	Function - Clear
	Description:
		Deletes Ptr if it isn't null.  Assume if Ptr isn't null, it needs deleted
		(general assumption of the class).
-----------------------------------------------------------------------------*/
template <class T>
inline void
G42APtr<T>::Clear(void)
{
	if (Ptr)
	{
		delete [] Ptr;
		Ptr = 0;
	}
}
/*-----------------------------------------------------------------------------
	Function - GetConstPtr
	Returns:
		const T * - a const form of the pointer (could be null)
	Description:
		Can be used if class is const.
-----------------------------------------------------------------------------*/
/*
template <class T>
inline const T *
G42APtr<T>::GetConstPtr(void) const // throw();
{
	return Ptr;
}
*/
/*-----------------------------------------------------------------------------
	Function - GetConstReference
	Parameters:
		size_t index - the index into the array that the reference come from
	Returns:
		const T & - a const reference at the requested array index.
	Exceptions:
		G42NullDereferenceException
	Description:
		Can be used if class is const.
-----------------------------------------------------------------------------*/
/*
template <class T>
inline const T &
G42APtr<T>::GetConstReference(size_t index) const
		// throw(G42NullDereferenceException)
{
	if (!Ptr)
		throw G42NullDereferenceException();
	return Ptr[index];
}
*/
/*-----------------------------------------------------------------------------
	Function - GetPtr
	Returns:
		T * - The pointer (could be null)
-----------------------------------------------------------------------------*/
template <class T>
inline T *
G42APtr<T>::GetPtr(void) const // throw();
{
	return Ptr;
}
/*-----------------------------------------------------------------------------
	Function - GetReference
	Parameters:
		size_t index - index into the array to get the reference from
	Returns:
		T & - a const reference to what the pointer was point towards.
	Exceptions:
		G42NullDereferenceException
-----------------------------------------------------------------------------*/
template <class T>
inline T &
G42APtr<T>::GetReference(size_t index) const // throw(G42NullDereferenceException);
{
	if (!Ptr)
		throw G42NullDereferenceException();
	return Ptr[index];
}
/*-----------------------------------------------------------------------------
	Function - NotNull
	Returns:
		bool - True if ptr is not null.  False if it is null.
	Description:
		Mainly used as if (ptr.NotNull()).  Converts ptr to a bool.
-----------------------------------------------------------------------------*/
template <class T>
inline bool
G42APtr<T>::NotNull(void) const
{
	return (Ptr != 0);
}
/*-----------------------------------------------------------------------------
	Function - operator !
	Returns:
		bool - True if ptr is null.  False if it is not null.
	Description:
		Mainly used as if (!ptr).  Converts ptr to a bool.
-----------------------------------------------------------------------------*/
template <class T>
inline bool
G42APtr<T>::operator ! (void) const
{
	return (Ptr == 0);
}
/*-----------------------------------------------------------------------------
	Function - Release
	Returns:
		T* - the pointer
	Assumptions:
		Whoever gets the pointer will delete it.
	Description:
		Used when giving up ownership of the pointer to someone else.  Zeros out
		the pointer and returns it.
-----------------------------------------------------------------------------*/
template <class T>
inline T *
G42APtr<T>::Release(void)
{
	T * ptr = Ptr; 	// save value so we can return it
	Ptr = 0;
	return ptr;
}
/*-----------------------------------------------------------------------------
	Function - Set
	Parameters:
		T * ptr - The pointer to set to
	Exceptions:
		xalloc if ptr is null
	Description:
		Deletes any old pointer and sets Ptr to the new pointer.  If the new
		pointer is null, it will throw xalloc.  Assume we own ptr and need to
		delete it when we go out of scope, and if ptr is null, it came from a
		new and xalloc needs to be thrown (which will happen before old pointer
		is cleared).
-----------------------------------------------------------------------------*/
template <class T>
inline void
G42APtr<T>::Set(T * ptr) // throws(xalloc)
{
	G42ThrowIfNull(ptr);
	Clear();
	Ptr = ptr;
}
/*-----------------------------------------------------------------------------
	Function - operator =
	Parameters:
		T * ptr - The pointer to set to
	Description:
		Deletes any old pointer and sets Ptr to the new pointer.  Assume we own
		ptr and need to delete it when we go out of scope.
-----------------------------------------------------------------------------*/
template <class T>
inline G42APtr<T> &
G42APtr<T>::operator = (T * ptr) // throw();
{
	Clear();
	Ptr = ptr;
	return *this;
}
/*-----------------------------------------------------------------------------
	Function - Take
	Parameters:
		G42APtr<T> & ref - where to take the pointer from
	Description:
		Deletes any old pointer and sets Ptr to the pointer that ref owns. Clears
		out ref.  If ref doesn't have a pointer, it clears out this likewise.
-----------------------------------------------------------------------------*/
template <class T>
inline void
G42APtr<T>::Take(G42APtr<T> & ref)
{
	if (&ref != this) 	// be sure we don't try to take from ourselves
	{
		if (ref.Ptr) 		// be sure to check, so we don't throw an xalloc
		{
			Set(ref.Release());
		}
		else
		{
			Clear();
		}
	}
}
/*-----------------------------------------------------------------------------
	Function - operator T *
	Returns:
		T * - The pointer (could be null)
-----------------------------------------------------------------------------*/
template <class T>
inline
G42APtr<T>::operator T * (void) const
{
	return Ptr;
}
/*-----------------------------------------------------------------------------
	Function - operator []
	Returns:
		T & - the reference to the array element index
	Exceptions:
		G42NullDereferenceException
-----------------------------------------------------------------------------*/
template <class T>
inline T &
G42APtr<T>::operator [] (size_t index) const
{
	if (!Ptr)
		throw G42NullDereferenceException();
	return Ptr[index];
}
/*-----------------------------------------------------------------------------
	Function - operator const T *
	Returns:
		const T * - a const form of the pointer (could be null)
	Description:
		Can be used if class is const.
-----------------------------------------------------------------------------*/
/*
template <class T>
inline
G42APtr<T>::operator const T * (void) const
{
	return Ptr;
}
*/
/*-----------------------------------------------------------------------------
	Function - operator []
	Parameters:
		size_t index
	Returns:
		const T & - a const reference to the array element index
	Exceptions:
		G42NullDereferenceException
-----------------------------------------------------------------------------*/
/*
template <class T>
inline const T &
G42APtr<T>::operator [] (size_t index) const
{
	if (!Ptr)
		throw G42NullDereferenceException();
	return Ptr[index];
}
*/
/*-----------------------------------------------------------------------------
	G42Vector inline functions:
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
	Function - G42Vector constructor
	Description:
		Default constructor, simply clears everthing to an initial state
-----------------------------------------------------------------------------*/
template <class T>
G42Vector<T>::G42Vector(void)
#ifdef USE_PTR
	: NumIndeces(0), NumHashes(0)
#else
	: Array(0), NumIndeces(0), NumHashes(0)
#endif
{}
/*-----------------------------------------------------------------------------
	Function - ~G42Vector destructor
	Description:
		Clear out all memory
-----------------------------------------------------------------------------*/
template <class T>
G42Vector<T>::~G42Vector(void)
{
	Clear();
}
/*-----------------------------------------------------------------------------
	Function - Clear
	Description:
		Clear out all memory
-----------------------------------------------------------------------------*/
template <class T>
void
G42Vector<T>::Clear(void)
{
	NumIndeces = 0;
#ifdef USE_PTR
	Array.Clear();		// G42APtr will clear itself and all hashes
#else
	if (Array)        // clear the array and all hashes
	{
		for (int32 i = 0; i < NumHashes; i++)
		{
			if (Array[i])
				delete [] Array[i];
		}
		delete [] Array;
	}
	Array = 0;
#endif
	NumHashes = 0;
}
/*-----------------------------------------------------------------------------
	Function - ExpandArray
	Parameters:
		int32 index - minimum indeces that need supported
	Exceptions:
		G42MemoryException
	Description:
		Expands array by adding hash tables until it can hold index number of
		indeces.  Assume index is a positive number.
-----------------------------------------------------------------------------*/
template <class T>
void
G42Vector<T>::ExpandArray(int32 index) // throw(MemoryError)
{
	// catch any memory allocation errors
	try
	{
		int32 new_num_hashes = ((index) >> 4) + 1;
#ifdef USE_PTR
		// looks ugly, but...  Throws xalloc if new returns null.
		G42APtr < G42Ptr < T > > new_array(new G42Ptr <T> [new_num_hashes]);
#else
		// this is ok bare, as no exceptions (beside xalloc at the new)
		// can be thrown until after we do the Array = new_array
		T ** new_array = new T * [new_num_hashes];
		G42ThrowIfNull(new_array);
#endif
#ifdef USE_PTR
		if (Array.NotNull()) // if (Array) gets confused with const problems
#else
		if (Array)
#endif
		{
#ifdef USE_PTR
			for (int n = 0; n < NumHashes; n++)
				new_array[n].Set(Array[n].Release());
#else
			for (int n = 0; n < NumHashes; n++)
				new_array[n] = Array[n];
			delete [] Array;
#endif
		}
#ifdef USE_PTR
		Array.Set(new_array.Release()); // auto delete old one, note Release()
#else
		Array = new_array;
#endif
		for (int32 i = NumHashes; i < new_num_hashes; i++)
		{
#ifdef USE_PTR
			Array[i].Set(new T [16]); // throws xalloc if new returns null
#else
			Array[i] = new T [16];
			G42ThrowIfNull(Array[i]);
#endif
			NumHashes++;	// keep adding so exceptions get current count
			NumIndeces += 16;
		}
	}
	catch (...)	// this should only be xalloc, but better be safe
	{
		throw G42MemoryException();
	}
}
/*-----------------------------------------------------------------------------
	Function - operator []
	Parameters:
		int32 index - index requested
	Returns:
		T & - reference (for lvalue) to array index requested
	Exceptions:
		G42OutOfBoundsException,
		G42MemoryException
	Description:
		Return requested index, expanding array if necessary
-----------------------------------------------------------------------------*/
template <class T>
inline T &
G42Vector<T>::operator [] (int32 index)
	// throw(G42OutOfBoundsException, G42MemoryException)
{
	if (index < 0)
		throw G42OutOfBoundsException();
	if (index >= NumIndeces)
		ExpandArray(index);
#ifdef USE_PTR
	return ((T *)Array[index >> 4])[index & 0xf];
			// (T *) needed because the pointers get confused with const.
			// Could also be done with GetReference calls.
#else
	return (Array[index >> 4])[index & 0xf];
#endif
}
/*-----------------------------------------------------------------------------
	Function - operator [] const
	Parameters:
		int32 index - index requested
	Returns:
		const T & - reference (for lvalue) to array index requested
	Exceptions:
		G42OutOfBoundsException,
		G42MemoryException
	Description:
		Return requested index.  Array will not expand, as it is marked const,
		and you can't modify new values anyway.
-----------------------------------------------------------------------------*/
template <class T>
inline const T &
G42Vector<T>::operator [] (int32 index) const
	// throw(G42OutOfBoundsException)
{
	if (index < 0 || index >= NumIndeces)
		throw G42OutOfBoundsException();
#ifdef USE_PTR
	return ((const T *)Array[index >> 4])[index & 0xf];
			// (const T *) needed because the pointers get confused with const.
			// Could also be done with GetConstReference calls.
#else
	return (Array[index >> 4])[index & 0xf];
#endif
}
int16 		G42SquareRoot (int32 val);
double 		G42SquareRoot (double val);
long double G42SquareRoot (long double val);
#endif // G42_TOOLS_H sentry
