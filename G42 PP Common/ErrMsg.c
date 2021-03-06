/*
	This contains code for handling an error dialog box.
	
	If you want to display an error code and string, call ErrMsgCode,
	otherwise, for string only, just call ErrMsg.
*/
#define KEEP_GOING 1
#define DEBUGGER 2
#define EXITTOSHELL 3
//MW Added prototypes
void Msg(Str255 msg);
void ErrMsgCode(Str255 msg, short code);
void ErrMsg(Str255 msg);
void Msg(Str255 msg)
/*
	Display an Alert with the string passed.
*/
{
	ParamText(msg,nil,nil,nil);
	Alert (130, nil);
}
void ErrMsgCode(Str255 msg, short code)
/*
	Display the error alert with
	an error code.
	
	This handy alert will also display 
	memerr and reserr for you.
*/
{ 
	Str31	codeStr;
	Str31	memErrStr;
	Str31	resErrStr;
	short	disposition;
	
	NumToString(code,codeStr);
//	MW Changed the MemErr and ResErr lowmemory globals to use LMGet..Err accessor functions
	NumToString(LMGetMemErr(),memErrStr);
	NumToString(LMGetResErr(),resErrStr);
	
	ParamText(msg, codeStr, memErrStr, resErrStr);
	disposition = Alert (131, nil);
	
	switch(disposition)
	{
		case	KEEP_GOING:		return;
								break;
		case	DEBUGGER:		DebugStr("\p Doing a Stack Crawl;sc6");
								break;
		case	EXITTOSHELL:	ExitToShell();
								break;
	}
}
void ErrMsg(Str255 msg)
{
	ErrMsgCode(msg, 0);
}
