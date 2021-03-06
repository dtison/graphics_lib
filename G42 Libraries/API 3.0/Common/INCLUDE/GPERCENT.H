/*-----------------------------------------------------------------------------
	file:			gpercent.h
	author:		John Pazdernik
	classes:		G42PercentDoneMessage
	date:			02/26/97
	revision:	3.0
	G42PercentDoneMessage is a notification message used to carry infor-
	mation about the percent of an operation that is completed to a mon-
	itoring process.  This process can return a cancel operation action.
-----------------------------------------------------------------------------*/
#if !defined (GPERCENT_H)
#define GPERCENT_H
#include "gnotify.h"
/*-----------------------------------------------------------------------------
	class G42PercentDoneMessage
-----------------------------------------------------------------------------*/
class G42PercentDoneMessage : public G42NotifyMessage
{
public:
	// constructor/destructor
	G42PercentDoneMessage(intf text_id)
		: G42NotifyMessage(), PercentDone(0), Cancel(false),
		TextId(text_id), Finished(false) {}
	~G42PercentDoneMessage(void) {}
	// member functions
	bool GetCancel(void) const
		{return Cancel;}
	intf GetPercentDone(void) const
		{return PercentDone;}
	intf GetTextId(void) const
		{return TextId;}
	bool IsFinished(void) const
		{return Finished;}
	void SetCancel(bool cancel = true)
		{Cancel = cancel;}
	void SetPercentDone(intf percent)
		{PercentDone = percent;}
	void SetTextId(intf text_id)
		{TextId = text_id;}
	void SetFinished(bool finished = true)
		{Finished = finished;}
protected:
	// data members
	bool	Cancel;			// Set true to cancel operation
	intf	PercentDone;	// Percent of operation completed
	intf	TextId;
	bool	Finished;
	// Hidden operator= and copy constructor
	G42PercentDoneMessage(G42PercentDoneMessage &) {}
	G42PercentDoneMessage & operator=(G42PercentDoneMessage &)
		{return *this;}
};
#endif // GPERCENT_H sentry
