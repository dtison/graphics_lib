#pragma once
#include "CommCicnButton.h"
class	CommToggleButton : public CommCicnButton
{
public:
	enum {class_ID = 'Etbt'};
	static CommToggleButton*		CreateCommToggleButtonStream (LStream *inStream);
									CommToggleButton (LStream *inStream); 
	void							SetPushed (Boolean pushed, Boolean refresh = true);
protected:
	virtual void					DrawSelf();	
	virtual void					ClickSelf (const SMouseDownEvent &inMouseDown);
private:
	Boolean							IsPushed;	
};
