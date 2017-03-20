// g42mptr.h - multiple pointer template
#if !defined(G42MPTR_H)
#define G42MPTR_H
/* ----------------------------------------------
	This file holds some pointer templates, to make
	handling pointers easier.  The simple class is
	G42Ptr, which automatically deletes the pointer
	when it goes out of scope.  It includes a Set(...),
	Clear(...), and operator translater into the pointer,
	as well as an copy constructor, operator =,
	null constructor, and value constructor.  This class
	is intended to act like a real pointer, except with
	the auto-delete functions.  If you don't want the
	class to delete a pointer it is holding, use the
	Erase(...) function.  This function is not available
	in the Multi Classes below.
	The class G42APtr is just like the above, except it
	takes an array, so it uses delete [].
	The Multi classes (G42MultiPtr, G42MultiAPtr) have
	the extra ability to count how many pointers are
	pointing to a particular instance, and only deletes
	the instance when the last pointer clears.  Copies
	are made either through a copy constructor or an
	operator =.  The only time Set(...) should be used
	is the first time, immediately after the object is
   new'ed.
	----------------------------------------------*/
template <class T>
class G42Ptr
{
public:
	operator T * (void)
		{ return Value; }
	G42Ptr() : Value(0) {}
	G42Ptr(T * value) : Value(value) {}
	~G42Ptr()
		{ Clear(); }
	G42Ptr(G42Ptr & copy) : Value(copy.Value) {}
	G42Ptr & operator = (G42Ptr & copy)
	{
		if (&copy != this)
		{
			Clear();
			Value = copy.Value;
		}
		return *this;
	}
	void Set(T * value)
	{
		Clear();
		Value = value;
	}
	void Clear(void)
	{
		if (Value)
		{
			delete Value;
			Value = 0;
		}
	}
	void Erase(void)
	{
		Value = 0;
	}
protected:
	T * Value;
};
template <class T>
class G42APtr
{
public:
	operator T * (void)
		{ return Value; }
	G42APtr() : Value(0) {}
	G42APtr(T * value) : Value(value) {}
	~G42APtr()
		{ Clear(); }
	G42APtr(G42APtr & copy) : Value(copy.Value) {}
	G42APtr & operator = (G42APtr & copy)
	{
		if (&copy != this)
		{
			Clear();
			Value = copy.Value;
		}
		return *this;
	}
	void Set(T * value)
	{
		Clear();
		Value = value;
	}
	void Clear(void)
	{
		if (Value)
		{
			delete[] Value;
			Value = 0;
		}
	}
	void Erase(void)
	{
		Value = 0;
	}
protected:
	T * Value;
};
template <class T>
class G42MultiPtrP
{
public:
	T * Value;
	int Count;
	G42MultiPtrP(T * value) : Value(value), Count(0) {}
	~G42MultiPtrP()
		{ if (Value) delete Value; Value = 0; }
	G42MultiPtrP(G42MultiPtrP & ref) {} // don't use
	G42MultiPtrP & operator = (G42MultiPtrP & ref) {return ref;} // don't use
};
template <class T>
class G42MultiPtr
{
public:
	operator T * (void)
		{ if (MultiPtrP) return MultiPtrP->Value; return 0; }
	G42MultiPtr() : MultiPtrP(0) {}
	G42MultiPtr(T * value) : MultiPtrP(0)
	{
		MultiPtrP = new G42MultiPtrP<T>(value);
		if (MultiPtrP)
			MultiPtrP->Count++;
	}
	~G42MultiPtr()
	{
		Clear();
	}
	G42MultiPtr(G42MultiPtr & copy)
	{
		MultiPtrP = copy.MultiPtrP;
		if (MultiPtrP)
			MultiPtrP->Count++;
	}
	G42MultiPtr & operator = (G42MultiPtr & copy)
	{
		if (&copy != this)
		{
			Clear();
			MultiPtrP = copy.MultiPtrP;
			if (MultiPtrP)
				MultiPtrP->Count++;
		}
		return *this;
	}
	void Set(T * value)
	{
		Clear();
		MultiPtrP = new G42MultiPtrP<T>(value);
		if (MultiPtrP)
			MultiPtrP->Count++;
	}
	void Clear(void)
	{
		if (MultiPtrP)
		{
			MultiPtrP->Count--;
			if (!MultiPtrP->Count)
				delete MultiPtrP;
			MultiPtrP = 0;
		}
	}
protected:
	G42MultiPtrP<T> * MultiPtrP;
};
template <class T>
class G42MultiAPtrP
{
public:
	T * Value;
	int Count;
	G42MultiAPtrP(T * value) : Value(value), Count(0) {}
	~G42MultiAPtrP()
		{ if (Value) delete[] Value; Value = 0; }
	G42MultiAPtrP(G42MultiAPtrP & ref) {} // don't use
	G42MultiAPtrP & operator = (G42MultiAPtrP & ref) {return ref;} // don't use
};
template <class T>
class G42MultiAPtr
{
public:
	operator T * (void)
		{ if (MultiAPtrP) return MultiAPtrP->Value; return 0; }
	operator const T * (void) const
		{ if (MultiAPtrP) return MultiAPtrP->Value; return 0; }
	G42MultiAPtr() : MultiAPtrP(0) {}
	G42MultiAPtr(T * value) : MultiAPtrP(0)
	{
		MultiAPtrP = new G42MultiAPtrP<T>(value);
		if (MultiAPtrP)
			MultiAPtrP->Count++;
	}
	~G42MultiAPtr()
	{
		Clear();
	}
	G42MultiAPtr(const G42MultiAPtr & copy)
	{
		MultiAPtrP = copy.MultiAPtrP;
		if (MultiAPtrP)
			MultiAPtrP->Count++;
	}
	G42MultiAPtr & operator = (const G42MultiAPtr & copy)
	{
		if (&copy != this)
		{
			Clear();
			MultiAPtrP = copy.MultiAPtrP;
			if (MultiAPtrP)
				MultiAPtrP->Count++;
		}
		return *this;
	}
	void Set(T * value)
	{
   	Clear();
		MultiAPtrP = new G42MultiAPtrP<T>(value);
		if (MultiAPtrP)
			MultiAPtrP->Count++;
	}
	void Clear(void)
	{
		if (MultiAPtrP)
		{
			MultiAPtrP->Count--;
			if (!MultiAPtrP->Count)
				delete MultiAPtrP;
			MultiAPtrP = 0;
		}
	}
protected:
	G42MultiAPtrP<T> * MultiAPtrP;
};
#endif // G42MPTR_H
