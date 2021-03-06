#pragma once
#include <LApplication.h>
#include "G42memry.h"	// For G42 Memory Manager
#include "G42image.h"	// G42Image Central Class
#include <string.h>
const short Num_Images = 33; // 56;	// Read & display all images in a window
class	CPPStarterApp : public LApplication 
{
public:
						CPPStarterApp();		
	virtual 			~CPPStarterApp();		
protected:
	virtual void		StartUp();	
private:
	/*  G42 Memory Manager Things */
	G42Data					*DataObject;
	G42SwapManager 			*Swap;
	G42CoreTable 			*Core;
	G42PageTable 			*Page;
	/*  G42Image  */
	G42Image				*Images [Num_Images];
	Point					DrawOrigins [Num_Images];
	void					DrawSomeImages (void);
	G42Image				*ReadImage (StringPtr filename);
};