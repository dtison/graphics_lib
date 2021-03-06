#pragma once
#include <LListener.h>
#include <UModalDialogs.h>
#include <string.h>
#include "CommRes.h"
#define URL_TEXT_SIZE 256
#ifdef KH_
class LTextEdit;
#endif
class CommEditURLInfo
{
public:
					CommEditURLInfo (void);
	virtual			~CommEditURLInfo (void);
	char URL [URL_TEXT_SIZE];
	char Comment [URL_TEXT_SIZE];
	char AltText [URL_TEXT_SIZE];
};
class CommEditURLDlg: public LListener
{
public:
					CommEditURLDlg (void);
	virtual			~CommEditURLDlg (void);
	virtual void	ListenToMessage (MessageT inMessage, void *ioParam);
	DlgStatus 		Dialog (LCommander *inSuper, ResIDT inDialogID,
						CommEditURLInfo *info);
	LWindow 		*DlgWindow;
	void 			GetLocalInfo (CommEditURLInfo *info);
	LTextEdit		*EditURLEdit1;
	LTextEdit		*EditURLEdit2;
	LTextEdit		*EditURLEdit3;	
private:
};
