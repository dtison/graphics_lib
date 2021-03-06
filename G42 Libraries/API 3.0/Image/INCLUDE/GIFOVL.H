#if ! defined (GIFOVL_H)
#define GIFOVL_H
#include "gifmults.h"
class G42OverlayOutsideImageException : public G42Exception
{
public:
	G42OverlayOutsideImageException (int32 error_id = 0)
		: G42Exception (error_id) {}
};
class G42Overlay : public G42ImageFilterMultiSender
{
public:
	enum  G42OverlayType {OverlayClip, OverlayUnion};
	enum  G42TransType {CopyThru};
							G42Overlay (const G42Point & point,
								G42OverlayType overlay_type = OverlayClip,
								G42TransType trans_type = CopyThru);
							G42Overlay (const G42Point & point, 
								G42Color & draw_color,
								G42OverlayType overlay_type = OverlayClip,
								G42TransType trans_type = CopyThru);
	virtual 				~G42Overlay (void) {}
	virtual				G42ImageFilter * MakeCopy(void) const;
protected:
	byte 					GetIndexFromType (G42ImageDescriptor::
								G42ImageType type);
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
	G42Color				DrawColor;
	bool					DrawColorMode;	
//	G42Vector			<G42Ptr<G42ImageData> > OverlayData;
//	G42Ptr				<G42ImageDescriptor> OverlayDescriptor;
	G42PathComponent 	* OverlaySender;
	G42OverlayType		OverlayType;
	G42Point 			Point;
	G42TransType  		TransType;
};
#endif
