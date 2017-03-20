#if ! defined (GIFSHARP_H)
#define GIFSHARP_H
#include "gifilter.h"
class G42Transparency : public G42ImageFilter
{
public:
							G42Transparency (const G42Color & color);
							G42Transparency (void);
	virtual 				~G42Transparency (void);
	virtual void 		HandleData (G42ImageData * data, 
								G42PathComponent *);
	virtual void 		HandleDescriptor (G42ImageDescriptor * descriptor, 
								G42PathComponent *);
	virtual void 		HandleEnd (G42PathEnd * end, 
								G42PathComponent *);
	virtual 				G42ImageFilter * MakeCopy(void) const;
protected:
	void					HandleFilter (void);
	void					ProcessFrame (void);
	G42Color				TransparentColor;
	bool					Transparent;
};
#endif
