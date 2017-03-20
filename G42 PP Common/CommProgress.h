#pragma once
#ifndef __DIALOGS__
#include <dialogs.h>
#endif
typedef enum {
	kDialogContinues							= 0,
	kDialogOKHit,
	kDialogCancelHit,
	kDialogExceeding100pc,
	kStateSpaceWithinLimit,
	kStateSpaceExceedingLimit
} tDialogProcessStates ;
typedef enum {
	kDefaultButtonIndex							= 1,
	kCancelButtonIndex,
	kDefaultButtonOutlineIndex
} tDialogItemIndicies ;
#define	kIndicatorOutline						0x0001
#define	kIndicatorContent						0x0002
#define	kDefaultInfiniteDrawDelay				1
#define	kInfinitePaternResID					6000
class CommProgress
{
	private:
		float					fStartStatePercent ;
		float					fCurrentStateSpace ;
		float					fCurrentStateValue ;
		float					fCurrentStateSpacePercent ;
		long					fInfiniteDrawTime ;
		Boolean					fPercentText ;
		Boolean					fProgressBar ;
		Boolean					fInfiniteBar ;
		short					fPercentTextItem ;
		short					fProgressBarItem ;
		short					fInfiniteBarItem ;
		short					fPercentTextDeltaLimit ;
		UserItemUPP				fButtonOutlineDrawProc ;
		UserItemUPP				fProgressBarDrawProc ;
		Boolean			FlashButton(DialogPtr theDialog, short buttonIndex) ;
		void			UpdateProgressIndicator(float newPercent) ;
	protected:
		virtual Boolean	ProcessEvent(EventRecord *theEvent, short *result) ;
		virtual	void	DrawPercentIndicator(float percent, short deltaLimit, short itemIndex, short part) ;
		virtual	void	DrawStdBarIndicator(float percent, short itemIndex, short part) ;
		virtual	void	DrawInfiniteBarIndicator(float percent, short itemIndex, short part) ;
		virtual	short	CreateTheDialog(short dlgResId) ;
	public:
		DialogPtr				fDialog ;
		float					fCurrentPercent ;
		long					fInfiniteDrawDelay ;
		CommProgress() ;							// Constructor
		~CommProgress() ;							// Destructor
		virtual void	SetParamText(void) ;
		virtual	short	SetupDialog(short dlgResId) ;
		virtual	short	SetupDialog(short dlgResId, ConstStr255Param param0, ConstStr255Param param1, ConstStr255Param param2, ConstStr255Param param3) ;
		virtual	void	SetProgressBar(short dlgItem) ;
		virtual	void	SetInfiniteBar(short dlgItem) ;
		virtual	void	SetPercentText(short dlgItem, short deltaLimit) ;
		
		virtual	void	SetCurrentState(float statePercent) ;
		virtual	void	SetStateSpace(float space) ;
		virtual short	SetCurrentStateValue(float value) ;
		virtual	void	DrawProgressIndicator(float percent, short part) ;
		
		virtual	void	BeginModal(void) ;
		virtual short	ProcessIdle(void) ;
		virtual	short	ProcessModal(void) ;
		virtual Boolean	CanProcessEvent(EventRecord *theEvent, short *result) ;
		virtual	void	EndModal(void) ;
} ;
