#pragma once
#include <LCicnButton.h>
class	CommCicnButton : public LCicnButton 
{
public:
	enum {class_ID = 'Eicn'};
	static CommCicnButton*	
				CreateFromStream (LStream *inStream);
				CommCicnButton (const CommCicnButton &original) :
					LCicnButton (original) {};
				CommCicnButton (LStream *inStream) : LCicnButton (inStream) {};
	virtual	~CommCicnButton() {};
protected:
	virtual void	DrawSelf();	
};
