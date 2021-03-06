// gnotify.h - G42Notifier, G42NotifierVerifier
#if !defined (GNOTIFY_H)
#define GNOTIFY_H
#include "gtype.h"
/*  Classes and things for callback messages  */
class G42NotifyMessage
{
public:
	G42NotifyMessage (void) {}
	virtual ~G42NotifyMessage (void) {}
protected:
	// Hidden operator= and copy constructor
	G42NotifyMessage(G42NotifyMessage &) {}
	G42NotifyMessage & operator=(G42NotifyMessage &)
		{return *this;}
};
/*  Abstract class for callback messages  */
class G42Notifier
{
public:
	G42Notifier(void) {}
	virtual ~G42Notifier(void) {}
	virtual bool ProcessMessage (G42NotifyMessage * /* message */)
		{return false;}
protected:
	// Hidden copy constructor and operator=
	G42Notifier(G42Notifier &) {}
	G42Notifier & operator=(G42Notifier &)
		{return *this;}
};
// simple class that forces there to be a notifier no matter what, so we
// don't have to always check.  To use, just set it with a notifier,
// and use it with ->ProcessMessage(msg); or .ProcessMessage(msg);
// use as a class member function, and you never have to worry about it.
class G42NotifierVerifier
{
public:
	G42NotifierVerifier(G42Notifier & notifier) :
		Notifier(&notifier), NeedDelete(false) {}
	G42NotifierVerifier(G42Notifier * notifier = 0) :
		Notifier(notifier), NeedDelete(false) { Verify(); }
	~G42NotifierVerifier(void)
		{ Clear(); }
	G42Notifier * operator -> (void)
		{ return Notifier; }
	bool ProcessMessage(G42NotifyMessage * message)
		{ return Notifier->ProcessMessage(message); }
	G42Notifier & operator = (G42Notifier & notifier)
		{ Set(notifier); return *Notifier; }
	G42Notifier * operator = (G42Notifier * notifier)
		{ Set(notifier); return Notifier; }
	operator G42Notifier & (void)
		{ return *Notifier; }
	operator G42Notifier * (void)
		{ return Notifier; }
	G42Notifier & operator * (void)
		{ return *Notifier; }
private:
	G42Notifier * Notifier;
	bool NeedDelete;
	void Set(G42Notifier & notifier)
		{ Clear(); Notifier = &notifier; NeedDelete = false; }
	void Set(G42Notifier * notifier)
		{ Clear(); Notifier = notifier; NeedDelete = false; Verify(); }
	void Verify(void)
	{
		if (!Notifier)
		{
			Notifier = new G42Notifier;
			NeedDelete = true;
		}
	}
	void Clear()
	{
		if (Notifier && NeedDelete)
		{
			delete Notifier;
			NeedDelete = false;
			Notifier = 0;
		}
	}
};
#endif
