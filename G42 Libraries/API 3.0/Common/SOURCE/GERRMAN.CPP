// GERRMAN.CPP - G42ErrorManager class
/*-----------------------------------------------------------------------------
	Modifications:
-----------------------------------------------------------------------------*/
#include "gerrman.h"
#pragma hdrstop
/*-----------------------------------------------------------------------------
	G42ErrorManager member functions:
-----------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------
	Function - ErrorManager Constructor
	Parameters:
		HINSTANCE hinst - A valid handle the the instance of the application.
-----------------------------------------------------------------------------*/
G42ErrorManager::G42ErrorManager(HINSTANCE hinst)
	: FatalErrNum(0), ErrNum(0), WarnNum(0), HInst(hinst)
{
	Valid = true;
}
/*-------------------------------------------------------------------------------
	Function - ErrorManager Destructor
-----------------------------------------------------------------------------*/
G42ErrorManager::~G42ErrorManager(void)
{}
/*-------------------------------------------------------------------------------
	Function - FatalError
	Parameters:
		int16 err_num - A value representing the String lookup Table reference.
	Description:
		Calling this function sets the current error in the Manager, and calls
		the DisplayError function for processing.  If an error is already set,
		this function does nothing.
-----------------------------------------------------------------------------*/
void
G42ErrorManager::FatalError(int16 fatal_err_num)
{
	if (FatalErrNum != 0)
		return;
	FatalErrNum = fatal_err_num;
	DisplayFatalError();
}
/*-------------------------------------------------------------------------------
	Function - Error
	Parameters:
		int16 err_num - A value representing the String lookup Table reference.
	Description:
		Calling this function sets the current error in the Manager, and calls
		the DisplayError function for processing.  If an error is already set,
		this function does nothing.
-----------------------------------------------------------------------------*/
void
G42ErrorManager::Error(int16 err_num)
{
	if (ErrNum != 0)
		return;
	ErrNum = err_num;
	DisplayError();
}
/*-----------------------------------------------------------------------------
	Function - Warn
	Parameters:
		int16 warn_num - required - A value representing the String lookup Table
			reference.
	Description:
		Calling this function sets the current warning in the Manager, and calls
		the DisplayWarning function for processing.  If a warning is already set,
		this function does nothing.
-----------------------------------------------------------------------------*/
void
G42ErrorManager::Warn(int16 warn_num)
{
	if (WarnNum != 0)
		return;
	WarnNum = warn_num;
	DisplayWarning();
}
/*-----------------------------------------------------------------------------
	Function - GetFatalErrorText
	Returns:
		const char * - A pointer to the error text string.
	Description:
		Using the previously set error reference number, this function looks in
		the resource string table and retrieves the appropriate text, which it
		copies into the ErrBuf data member.  It then returns a pointer to this
		string.
-----------------------------------------------------------------------------*/
const char *
G42ErrorManager::GetFatalErrorText(void)
{
	if (FatalErrNum == 0)
		return (char *)NULL;
	int32 ret_val = LoadString(HInst, FatalErrNum, FatalErrBuf, 128);
	if (ret_val == 0L)
		return (char *)NULL;
	return FatalErrBuf;
}
/*-----------------------------------------------------------------------------
	Function - GetErrorText
	Returns:
		const char * - A pointer to the error text string.
	Description:
		Using the previously set error reference number, this function looks in
		the resource string table and retrieves the appropriate text, which it
		copies into the ErrBuf data member.  It then returns a pointer to this
		string.
-----------------------------------------------------------------------------*/
const char *
G42ErrorManager::GetErrorText(void)
{
	if (ErrNum == 0)
		return (char *)NULL;
	int32 ret_val = LoadString(HInst, ErrNum, ErrBuf, 128);
	if (ret_val == 0L)
		return (char *)NULL;
	return ErrBuf;
}
/*-----------------------------------------------------------------------------
	Function - GetWarningText
	Returns:
		const char * - A pointer to the warning text string.
	Description:
		Using the previously set warning reference number, this function looks
		in the resource string table and retrieves the appropriate text, which
		it copies into the WarnBuf data member.  It then returns a pointer to
		this string.
-----------------------------------------------------------------------------*/
const char *
G42ErrorManager::GetWarningText(void)
{
	if (WarnNum == 0)
		return (char *)NULL;
	int32 ret_val = LoadString(HInst, WarnNum, WarnBuf, 128);
	if (ret_val == 0L)
		return (char *)NULL;
	return WarnBuf;
}
