#include "CommEditURLDlg.h"
#include "CSlider.h"
#include "CommRes.h"
CommEditURLDlg::CommEditURLDlg (void)
{
}
CommEditURLDlg::~CommEditURLDlg (void)
{
}
void CommEditURLDlg::ListenToMessage (MessageT inMessage, void *ioParam)
{
	CommandT command = cmd_Nothing;
	// switch (command)...
}
DlgStatus
CommEditURLDlg::Dialog (LCommander *inSuper, ResIDT inDialogID, 
	CommEditURLInfo *info)
{
	StDialogHandler	theHandler (inDialogID, inSuper);
	LWindow	*theDialog = theHandler.GetDialog();
	DlgWindow = theDialog;
	EditURLEdit1 = (LTextEdit *) DlgWindow -> FindPaneByID (pane_EditURLEdit1);
	EditURLEdit2 = (LTextEdit *) DlgWindow -> FindPaneByID (pane_EditURLEdit2);
	EditURLEdit3 = (LTextEdit *) DlgWindow -> FindPaneByID (pane_EditURLEdit3);
	if (strlen (info -> URL))
		EditURLEdit1 -> SetDescriptor (c2pstr (info -> URL));
	if (strlen (info -> Comment))
		EditURLEdit2 -> SetDescriptor (c2pstr (info -> Comment));
	if (strlen (info -> AltText))
		EditURLEdit3 -> SetDescriptor (c2pstr (info -> AltText));
	theDialog -> Show();
	DlgStatus returnValue;
	Boolean done = false;
	while (! done) 
	{
		MessageT hitMessage = theHandler.DoDialog();
		switch (hitMessage)
		{
			case msg_Cancel:
			{
				done = true;
				returnValue = Cancel;
			}
			break;
			case msg_OK:
			{
				GetLocalInfo (info);
				done = true;
				returnValue = Ok;
			}
			break;
		}
	}
	return (returnValue);
}
void 
CommEditURLDlg::GetLocalInfo (CommEditURLInfo *info)
{
	Str255 pString;
	LTextEdit *edit = (LTextEdit *) DlgWindow -> FindPaneByID (pane_EditURLEdit1);
	edit -> GetDescriptor (pString);
	strcpy (info -> URL, p2cstr (pString));
	edit = (LTextEdit *) DlgWindow -> FindPaneByID (pane_EditURLEdit2);
	edit -> GetDescriptor (pString);
	strcpy (info -> Comment, p2cstr (pString));
	edit = (LTextEdit *) DlgWindow -> FindPaneByID (pane_EditURLEdit3);
	edit -> GetDescriptor (pString);
	strcpy (info -> AltText, p2cstr (pString));		
}
/***********************************************************/
CommEditURLInfo::CommEditURLInfo (void)
{
	URL [0] 	= 0;
	Comment [0] = 0;
	AltText [0] = 0;
}
CommEditURLInfo::~CommEditURLInfo (void)
{
}