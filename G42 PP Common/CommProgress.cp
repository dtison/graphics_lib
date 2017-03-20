
#ifndef __QUICKDRAW__
#include <Quickdraw.h>
#endif
#include "CommProgress.h"
// ---------------------------------------------------------------------------
//		� Local Prototypes
// ---------------------------------------------------------------------------
pascal void DrawBar(WindowPtr theWindow, short itemNumber) ;
pascal void DrawButtonRect(WindowPtr theWindow, short itemNumber) ;
// ---------------------------------------------------------------------------
//		� CommProgress
// ---------------------------------------------------------------------------
CommProgress::CommProgress()
{	
	fDialog = NULL ;
	fCurrentPercent = 0.0 ;
	fStartStatePercent = 0.0 ;
	fCurrentStateSpacePercent = 0.0 ;
	fInfiniteDrawDelay = kDefaultInfiniteDrawDelay ;
	fPercentText = fProgressBar = fInfiniteBar = false ;
	fPercentTextItem = fProgressBarItem = fInfiniteBarItem = 0 ;
	fPercentTextDeltaLimit = 1 ;
	#ifdef USEOUTLINEPROC
	fButtonOutlineDrawProc = NewUserItemProc(DrawButtonRect) ;
	#endif
	#define USEDRAWBARPROC
	#ifdef USEDRAWBARPROC
	fProgressBarDrawProc = NewUserItemProc(DrawBar) ;
	#endif
}
// ---------------------------------------------------------------------------
//		� ~CommProgress
// ---------------------------------------------------------------------------
//	Destructor. Delete the dialog structure if it is still around
CommProgress::~CommProgress()
{
	if (fDialog)
		DisposeDialog(fDialog) ;
	#ifdef USEOUTLINEPROC
	if (fButtonOutlineDrawProc)
		DisposeRoutineDescriptor(fButtonOutlineDrawProc) ;
	#endif
	#ifdef USEDRAWBARPROC
	if (fProgressBarDrawProc)
		DisposeRoutineDescriptor(fProgressBarDrawProc) ;
	#endif
}
// ---------------------------------------------------------------------------
//		� SetParamText
// ---------------------------------------------------------------------------
//	Set the param text items in the dialog
void CommProgress::SetParamText()
{
	ParamText("\p", "\p", "\p", "\p") ;
}
// ---------------------------------------------------------------------------
//		� SetupDialog
// ---------------------------------------------------------------------------
//	Set the dialog parameters
short CommProgress::CreateTheDialog(short dlgResId)
{
	short						itemType ;
	Handle						theItem ;
	Rect						box ;
	short						err = noErr ;
	
	fDialog = GetNewDialog(dlgResId, NULL, (WindowPtr)-1L) ;
	if (fDialog)
	{
		SetWRefCon((WindowPtr)fDialog, (long)this) ;
		#ifdef USEOUTLINEPROC
			GetDialogItem(fDialog, kDefaultButtonOutlineIndex, &itemType, &theItem, &box) ;
		if (itemType == userItem)
		{
			SetDialogItem(fDialog, kDefaultButtonOutlineIndex, itemType, (Handle)fButtonOutlineDrawProc, &box) ;
		}
		#endif
	}
	else
		err = ResError() ;
	
	return err ;
}
short CommProgress::SetupDialog(short dlgResId)
{
	SetParamText() ;
	
	return CreateTheDialog(dlgResId) ;
}
short CommProgress::SetupDialog(short dlgResId, ConstStr255Param param0, ConstStr255Param param1, ConstStr255Param param2, ConstStr255Param param3)
{
	ParamText(param0, param1, param2, param3) ;
	
	return CreateTheDialog(dlgResId) ;
}
// ---------------------------------------------------------------------------
//		� SetProgressBar
// ---------------------------------------------------------------------------
//	Set parameters for using the standard progress bar
void CommProgress::SetProgressBar(short dlgItem)
{
	short						itemType ;
	Handle						theItem ;
	Rect						box ;
	GetDialogItem(fDialog, dlgItem, &itemType, &theItem, &box) ;
	if (itemType == userItem)
	{
		fProgressBar = true ;
		fProgressBarItem = dlgItem ;
		#ifdef USEDRAWBARPROC
		SetDialogItem(fDialog, fProgressBarItem, itemType, (Handle)fProgressBarDrawProc, &box) ;
		#endif
	}
	else
	{
		fProgressBar = false ;
		fProgressBarItem = 0 ;
	}
}
// ---------------------------------------------------------------------------
//		� SetInfiniteBar
// ---------------------------------------------------------------------------
//	Set parameters for using the infinite progress bar
void CommProgress::SetInfiniteBar(short dlgItem)
{
	short						itemType ;
	Handle						theItem ;
	Rect						box ;
	GetDialogItem(fDialog, dlgItem, &itemType, &theItem, &box) ;
	if (itemType == userItem)
	{
		fInfiniteBar = true ;
		fInfiniteBarItem = dlgItem ;
		#ifdef USEDRAWBARPROC
		SetDialogItem(fDialog, fInfiniteBarItem, itemType, (Handle)fProgressBarDrawProc, &box) ;
		#endif
	}
	else
	{
		fInfiniteBar = false ;
		fInfiniteBarItem = 0 ;
	}
}
// ---------------------------------------------------------------------------
//		� SetPercentText
// ---------------------------------------------------------------------------
//	Set parameters for using the textual percent indicator
void CommProgress::SetPercentText(short dlgItem, short deltaLimit)
{
	short						itemType ;
	Handle						theItem ;
	Rect						box ;
	GetDialogItem(fDialog, dlgItem, &itemType, &theItem, &box) ;
	if ((theItem != NULL) && (itemType == statText))
	{
		fPercentText = true ;
		fPercentTextItem = dlgItem ;
		fPercentTextDeltaLimit = deltaLimit ;
	}
	else
	{
		fPercentText = false ;
		fPercentTextItem = 0 ;
		fPercentTextDeltaLimit = 0 ;
	}
}
// ---------------------------------------------------------------------------
//		� UpdateProgressIndicator
// ---------------------------------------------------------------------------
//	Force a redraw of the progress indicator(s)
void CommProgress::UpdateProgressIndicator(float newPercent)
{
	GrafPtr						oldPort ;
	GetPort(&oldPort) ;
	SetPort((GrafPtr)fDialog) ;
	DrawProgressIndicator(newPercent, kIndicatorContent) ;
	SetPort(oldPort) ;
}
// ---------------------------------------------------------------------------
//		� SetCurrentState
// ---------------------------------------------------------------------------
//	Set the current progress state
void CommProgress::SetCurrentState(float statePercent)
{
	fCurrentPercent = fStartStatePercent + fCurrentStateSpacePercent ;
	UpdateProgressIndicator(fCurrentPercent) ;
	fStartStatePercent = fCurrentPercent ;
	fCurrentStateSpacePercent = statePercent - fStartStatePercent ;
	fCurrentStateSpace = 0 ;
	fCurrentStateValue = 0 ;
}
// ---------------------------------------------------------------------------
//		� SetStateSpace
// ---------------------------------------------------------------------------
//	Set the resolution of the current state
void CommProgress::SetStateSpace(float space)
{
	fCurrentStateSpace = space ;
}
// ---------------------------------------------------------------------------
//		� SetCurrentStateValue
// ---------------------------------------------------------------------------
//	Set the current position in this state
short CommProgress::SetCurrentStateValue(float value)
{
	float						currentPercent ;
	
	fCurrentStateValue = value ;
	currentPercent = (fCurrentStateSpacePercent / fCurrentStateSpace) * fCurrentStateValue ;
	fCurrentPercent = currentPercent + fStartStatePercent ;
	UpdateProgressIndicator(fCurrentPercent) ;
	
	if (fCurrentStateValue < fCurrentStateSpace)
		return kStateSpaceWithinLimit ;
	else
		return kStateSpaceExceedingLimit ;
}
// ---------------------------------------------------------------------------
//		� BeginModal
// ---------------------------------------------------------------------------
//	Set the Modal Dialog process
void CommProgress::BeginModal()
{
	fInfiniteDrawTime = TickCount() + fInfiniteDrawDelay ;
	ShowWindow((WindowPtr)fDialog) ;
	DrawDialog(fDialog) ;
}
// ---------------------------------------------------------------------------
//		� FlashButton
// ---------------------------------------------------------------------------
//	Flash the indicated button
Boolean CommProgress::FlashButton(DialogPtr theDialog, short buttonIndex)
{
	Rect						aRect ;
	ControlHandle				theItem ;
	short						itemType ;
	long						delayTime ;
	Boolean						retVal = false ;
	
	GetDialogItem(theDialog, buttonIndex, &itemType, (Handle*)&theItem, &aRect) ;
	if (itemType == (btnCtrl + ctrlItem))
	{
		if ((**theItem).contrlHilite != 255)   /* is it enabled ? */
		{
			HiliteControl(theItem, 1) ;
			Delay(6, &delayTime) ;
			HiliteControl(theItem, 0) ;
			
			retVal = true ;
		}
	}
	return retVal ;
}
// ---------------------------------------------------------------------------
//		� ProcessEvent
// ---------------------------------------------------------------------------
//	Process the event from ProcessModal()
//
Boolean CommProgress::ProcessEvent(EventRecord *theEvent, short *result)
{
	Boolean					usedEvent = FALSE ;
	
	*result = kDialogContinues ;
	switch(theEvent->what)
	{
		case updateEvt:
			BeginUpdate((WindowPtr)fDialog) ;
			DrawDialog(fDialog) ;
			EndUpdate((WindowPtr)fDialog) ;
			usedEvent = TRUE ;
			break ;
		case mouseDown:
			WindowPtr			theWindow ;
			Rect				aRect ;
			short				thePart ;
			
			thePart = FindWindow(theEvent->where, &theWindow) ;
			switch (thePart)
			{
				case inDrag:
					if (theWindow == (WindowPtr)fDialog)
					{
						aRect = qd.screenBits.bounds ;
						InsetRect(&aRect, -4, -4) ;
						DragWindow(theWindow, theEvent->where, &aRect) ;
						usedEvent = TRUE ;
					}
					break ;
				case inContent:
					if (theWindow == (WindowPtr)fDialog)
					{
						ControlHandle					control ;
						GrafPtr							oldPort ;
						short							itemType ;
						Rect							box ;
						ControlHandle					theItem ;
						
						GetPort(&oldPort) ;
						SetPort((GrafPtr)fDialog) ;
						GlobalToLocal(&theEvent->where) ;
						SetPort(oldPort) ;
						if (FindControl(theEvent->where, (WindowPtr)fDialog, &control))
						{
							if (TrackControl(control, theEvent->where, NULL))
							{
								GetDialogItem(fDialog, ok, &itemType, (Handle*)&theItem, &box) ;
								if (control == theItem)
									*result = kDialogOKHit ;
								else
									*result = kDialogCancelHit ;
							}
						}
					
						usedEvent = TRUE ;
					}
					break ;
				case inMenuBar:
					break ;
				
			}
			break ;
		
		case keyDown:
			short				keyCode ;
			if (FrontWindow() == (WindowRef)fDialog)
			{
				keyCode = BitAnd(theEvent->message, charCodeMask) ;
				switch (keyCode)
				{
					case 46:	/* . */
						usedEvent = TRUE ;
						if ((theEvent->modifiers & cmdKey) != cmdKey)
							break ;
					
					case 27:	/* Escape */
						usedEvent = TRUE ;
						if (FlashButton(fDialog, cancel))
							*result = kDialogCancelHit ;
						break ;
					
					case 13:	/* CR */
					case 3:		/* ENTER */
						usedEvent = TRUE ;
						if (FlashButton(fDialog, ok))
							*result = kDialogOKHit ;
						break ;
				}
			}
			break ;
	}
	
	return usedEvent ;
}
// ---------------------------------------------------------------------------
//		� ProcessIdle
// ---------------------------------------------------------------------------
//	Idle if we have no events occuring, needed for infinite bar.
short CommProgress::ProcessIdle()
{
	short				result = kDialogContinues ;
	
	if (fInfiniteBar)
	{
		if (TickCount() > fInfiniteDrawTime)
			UpdateProgressIndicator(fCurrentPercent) ;
	}
	if (fCurrentPercent >= 100)
		result = kDialogExceeding100pc ;
	return result ;
}
// ---------------------------------------------------------------------------
//		� ProcessModal
// ---------------------------------------------------------------------------
//	Do the general dialog processing
short CommProgress::ProcessModal()
{
	EventRecord					theEvent ;
	Boolean						ok ;
	short						result = kDialogContinues ;
	ok = GetNextEvent(everyEvent, &theEvent) ;
	if (ok)
		ProcessEvent(&theEvent, &result) ;
	if (result == kDialogContinues)
		result = ProcessIdle() ;
	return result ;
}
// ---------------------------------------------------------------------------
//		� CanProcessEvent
// ---------------------------------------------------------------------------
//	Pass an event to the dialog and let it process it if possible. Returning TRUE
//	if the event was used, FALSE if it was not.
Boolean CommProgress::CanProcessEvent(EventRecord *theEvent, short *result)
{
	Boolean						usedEvent ;
	usedEvent = ProcessEvent(theEvent, result) ;
	
	if (*result == kDialogContinues)
		ProcessIdle() ;
		
	return usedEvent ;
}
// ---------------------------------------------------------------------------
//		� EndModal
// ---------------------------------------------------------------------------
//	Finished with the modal dialog
void CommProgress::EndModal()
{
	FlushEvents(everyEvent, everyEvent) ;
	HideWindow((WindowPtr)fDialog) ;
}
// ---------------------------------------------------------------------------
//		� DrawProgressIndicator
// ---------------------------------------------------------------------------
//	Draw the appropriate dialog items
void CommProgress::DrawProgressIndicator(float percent, short part)
{
	if (fPercentText)
		DrawPercentIndicator(percent, fPercentTextDeltaLimit, fPercentTextItem, part) ;
	
	if (fProgressBar)
		DrawStdBarIndicator(percent, fProgressBarItem, part) ;
	
	if (fInfiniteBar)
	{
		if ((part & kIndicatorOutline) == kIndicatorOutline)
			DrawInfiniteBarIndicator(percent, fInfiniteBarItem, part) ;
		else
		if (TickCount() > fInfiniteDrawTime)
		{
			fInfiniteDrawTime = TickCount() + fInfiniteDrawDelay ;
			DrawInfiniteBarIndicator(percent, fInfiniteBarItem, part) ;
		}
	}
}
// ---------------------------------------------------------------------------
//		� DrawPercentIndicator
// ---------------------------------------------------------------------------
void CommProgress::DrawPercentIndicator(float percent, short deltaLimit, short itemIndex, short part)
{
	short							itemType ;
	Handle							item ;
	Rect							box ;
	Str255							theText ;
	long							theNum ;
	long							percentLong ;
	
	GetDialogItem(fDialog, itemIndex, &itemType, &item, &box) ;
	
//	GetIText(item, theText) ;
	StringToNum(theText, &theNum) ;
	percentLong = percent ;
	if (((percentLong - theNum) >= deltaLimit) &&
		(percentLong > 0) )
	{
		NumToString(percentLong, theText) ;
	//	SetIText(item, theText) ;
	}
}
// ---------------------------------------------------------------------------
//		� DrawStdBarIndicator
// ---------------------------------------------------------------------------
void CommProgress::DrawStdBarIndicator(float percent, short itemIndex, short part)
{
	short							itemType ;
	Handle							item ;
	Rect							box ;
	static short					previousRight = 0 ;
	RGBColor						barColor = {0x4444, 0x4444, 0x4444} ;
	RGBColor						emptyBarColor = {0xCCCC, 0xCCCC, 0xFFFF} ;
	RGBColor						currentForeColor ;
	RGBColor						currentBackColor ;
	
	GetDialogItem(fDialog, itemIndex, &itemType, &item, &box) ;
	
	//	Draw the frame of the progress indicator
	if ((part & kIndicatorOutline) == kIndicatorOutline)
	{
		//	Colour in the purple interior of the progress indicatorbar
		
		GetBackColor(&currentBackColor) ;
		RGBBackColor(&emptyBarColor) ;
		EraseRect(&box) ;
		RGBBackColor(&currentBackColor) ;
		//	Draw the black frame around the outside
		
		FrameRect(&box) ;
		previousRight = box.left + 1 ;		//	Ensure the entire body is drawn
	}
	//	Draw the body of the progress indicator
	
	if ((part & kIndicatorContent) == kIndicatorContent)
	{
		InsetRect(&box, 1, 1) ;
		box.right = box.left + ((float)(box.right - box.left) * (percent / 100.0)) ;
		if (box.right > previousRight)
		{
			if (previousRight > 0)
				box.left = previousRight ;
			previousRight = box.right ;
			
			GetForeColor(&currentForeColor) ;
			RGBForeColor(&barColor) ;
			PaintRect(&box) ;
			RGBForeColor(&currentForeColor) ;
		}
	}
}
// ---------------------------------------------------------------------------
//		� DrawInfiniteBarIndicator
// ---------------------------------------------------------------------------
void CommProgress::DrawInfiniteBarIndicator(float percent, short itemIndex, short part)
{
	short							itemType ;
	Handle							item ;
	Rect							box ;
	static short					originOffset = 0 ;
	PixPatHandle					thePattern ;
	
	GetDialogItem(fDialog, itemIndex, &itemType, &item, &box) ;
	//	Draw the outline of the progress indicator
	
	if ((part & kIndicatorOutline) == kIndicatorOutline)
	{
		FrameRect(&box) ;
	}
	//	Draw the body of the indicator
	
	InsetRect(&box, 1, 1) ;
	OffsetRect(&box, 0, originOffset) ;
	thePattern = GetPixPat(kInfinitePaternResID) ;
	PenPixPat(thePattern) ;
	SetOrigin(0, originOffset) ;
	PaintRect(&box) ;
	
	SetOrigin(0, 0) ;
	PenNormal() ;
	//	Increment the origin offset if we have just been asked to draw the Content ONLY
	if (part == kIndicatorContent)
		originOffset = (originOffset + 1) % ((((**(**thePattern).patMap).rowBytes) & 0xEFFF) * 8) ;
	DisposePixPat(thePattern) ;
}
// ---------------------------------------------------------------------------
//		� Non-Class routines
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//		� DrawBar
// ---------------------------------------------------------------------------
pascal void DrawBar(WindowPtr theWindow, short itemNumber)
{
	CommProgress					*theDialog ;
	
	theDialog = (CommProgress*)GetWRefCon(theWindow) ;
	theDialog->DrawProgressIndicator(theDialog->fCurrentPercent, kIndicatorOutline | kIndicatorContent) ;
}
// ---------------------------------------------------------------------------
//		� DrawButtonRect
// ---------------------------------------------------------------------------
pascal void DrawButtonRect(WindowPtr theWindow, short itemNumber)
{
	Rect							theRect ;
	short							itemType ;
	Handle							theItem ;
	GetDialogItem((DialogPtr)theWindow, itemNumber, &itemType, &theItem, &theRect) ;
	PenSize(3, 3) ;
	FrameRoundRect(&theRect, 16, 16) ;
	PenNormal() ;
}
