#if ! defined (GIFRMS_H)
#define GIFRMS_H
#include "gifmults.h"
#if 0
class G42RMSOutsideImageException : public G42Exception
{
public:
	G42RMSOutsideImageException (int32 error_id = 0)
		: G42Exception (error_id) {}
};
#endif
class G42RMS : public G42ImageFilterMultiSender
{
public:
//	enum  G42RMSType {OverlayClip, OverlayUnion};
							G42RMS (void) : AverageRMS (0.0) {}
	virtual 				~G42RMS (void) {}
	virtual				G42ImageFilter * MakeCopy(void) const;
	double				GetAverageRMS (void) const {return AverageRMS;}
protected:
	void					HandleColor8 (G42ImageDescriptor::	
								G42ImageType overlay_type);
	void					HandleColor24 (G42ImageDescriptor::	
								G42ImageType overlay_type);
	void					HandleColor32 (G42ImageDescriptor::	
								G42ImageType overlay_type);
	void					HandleGray8 (G42ImageDescriptor::	
								G42ImageType overlay_type);
	void					HandleGray1 (G42ImageDescriptor::	
								G42ImageType overlay_type);
	virtual void 		HandleEnd (G42PathEnd * end, 
								G42PathComponent * sender);
	void					HandleFilter (void);
	/*  Variables  */
	double				AverageRMS;
};
#endif
