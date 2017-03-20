
#if !defined(G42_ERRORMAN_H)
/*-----------------------------------------------------------------------------
	file:			gerrman.h
	author:		John R. Pazdernik
	classes:		G42ErrorManager
	date:			?
	revision:	?
	The G42ErrorManager class is responsible for handling the communication of
	errors, warnings and their respective information amongst objects.  It is an
	abstract class, and must have a class derived from it in the main section of
	the application. This derived class must override the DisplayError and
	DisplayWarning functions to deal with the error or warning in the desired
	fashion.  Only one error and one warning at a time can be logged, so after
	an error, no other errors will be logged until Clear() is called.
-----------------------------------------------------------------------------*/
// Compiler Directives
#define G42_ERRORMAN_H
#include "gtype.h"
//#include "errtab.rh"
#define STRICT
#include <windows.h>
/*-----------------------------------------------------------------------------
	class G42ErrorManager
-----------------------------------------------------------------------------*/
class G42ErrorManager
{
public:
	// constructor/destructor
	G42ErrorManager(HINSTANCE hinst);
	virtual ~G42ErrorManager(void);
	// member functions
	bool IsValid(void) const
			{return Valid;}
	void FatalError(int16 fatal_err_num);
	void Error(int16 err_num);
	void Warn(int16 warn_num);
	void ClearFatalError(void)
			{FatalErrNum = 0;}
	void ClearError(void)
			{ErrNum = 0;}
	void ClearWarning(void)
			{WarnNum = 0;}
	bool HadFatalError(void) const
			{return (FatalErrNum != 0);}
	bool HadError(void) const
			{return (ErrNum != 0);}
	bool HadWarning(void) const
			{return (WarnNum != 0);}
	int16 GetFatalErrorNum(void) const
			{return FatalErrNum;}
	int16 GetErrorNum(void) const
			{return ErrNum;}
	int16 GetWarningNum(void) const
			{return WarnNum;}
	const char * GetFatalErrorText(void);
	const char * GetErrorText(void);
	const char * GetWarningText(void);
protected:
	// member functions
	virtual void DisplayFatalError(void) = 0;	// Must override this function.
	virtual void DisplayError(void) = 0;		// Must override this function.
	virtual void DisplayWarning(void) = 0;		// Must override this function.
	// data members
	HINSTANCE HInst;		// Used to load the error text from the resource
private:
	// data members
	int16    FatalErrNum;
	int16    ErrNum;              // Reference number for String Table lookup.
	int16    WarnNum;             // Reference number for String Table lookup.
	char     FatalErrBuf[128];
	char     ErrBuf[128];         // Buffer for holding the error text
	char     WarnBuf[128];        // Buffer for holding the Warning text
	bool     Valid;
	// Hidden copy constructor & operator=
	G42ErrorManager(G42ErrorManager &) {}
	G42ErrorManager & operator= (G42ErrorManager &)
			{return *this;}
};
/*-----------------------------------------------------------------------------
	class DefaultErrorManager
-----------------------------------------------------------------------------*/
class DefaultErrorManager : public G42ErrorManager
{
public:
	// constructor/destructor
	DefaultErrorManager(void) : G42ErrorManager(0) {}
	~DefaultErrorManager(void) {}
	virtual void DisplayFatalError(void)
			{ClearFatalError();}
	virtual void DisplayError(void)
			{ClearError();}
	virtual void DisplayWarning(void)
			{ClearWarning();}
private:
	// hidden copy constructor & operator=
	DefaultErrorManager(DefaultErrorManager & Ref)
			: G42ErrorManager(Ref.HInst) {}
	DefaultErrorManager & operator= (DefaultErrorManager &)
			{return *this;}
};
#endif // G42_ERRORMAN_H
