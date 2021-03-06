// gmkmsg.h -- G42MouseKeyMessage (and subclasses)
#if ! defined (GMKMSG_H)
#define GMKMSG_H
#include "gtype.h"
#include "ggeomtry.h"
/*-----------------------------------------------------------------------------
	file:			gmkmsg.h
	author:			David Ison
	date:			10/24/96
	revision:		3.0
	Defines all the mouse and keyboard messages the client application will 
	be sending to the library. The library will process them and notify the
	application after processing the message.
----------------------------------------------------------------------------*/
/*  Values used in these messages  */
const int16 G42CtrlDown 			= 1;
const int16 G42ShiftDown 			= 2;
const int16 G42MacOptionDown		= 4;
const int16 G42NoButton	 			= 0;
const int16 G42LeftButton	 		= 1;
const int16 G42MiddleButton 		= 2;
const int16 G42RightButton 		= 4;
const int16 G42DefaultButton 		= G42LeftButton;
/*  Parent class for all messages  */
class G42MouseKeyMessage
{
public:
	virtual	~G42MouseKeyMessage (void) {}	// Needed in order to make a vtable
	int16 Flags;							// All messages will want to have options flags
};
/***  Mouse messages  ***/
class G42MouseMessage: public G42MouseKeyMessage
{
public:
	G42Point 	Point;
	int16 		Button;
};
class G42MouseDown : public G42MouseMessage
{};
class G42MouseMove : public G42MouseMessage
{};
class G42MouseUp : public G42MouseMessage
{};
class G42MouseDoubleClick : public G42MouseMessage
{};
/***  Keyboard messages  ***/
class G42KeyMessage : public G42MouseKeyMessage
{
public:
	int16 Key;
};
class G42KeyDown : public G42KeyMessage
{ };
class G42KeyUp : public G42KeyMessage
{ };
#endif // GMKMSG_H
