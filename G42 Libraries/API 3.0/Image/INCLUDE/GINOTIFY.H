// ginotify.h
#if !defined (GINOTIFY_H)
#define GINOTIFY_H
/*  Classes and things for callback messages for images */
#include "gnotify.h"
class G42MousePos : public G42NotifyMessage
{
public:
				G42MousePos (int16 x, int16 y) : G42NotifyMessage (), X (x), Y (y) {}
	int16		X;
	int16		Y;
};
class G42QueryURL : public G42NotifyMessage
{
public:
					G42QueryURL (void) : URL (0), Comment (0), AltText (0) {}
					G42QueryURL (const char * url, const char * comment = 0,
					const char * alt_text = 0); 
	void			SetURL (const char * url);
	void			SetComment (const char * comment);
	void			SetAltText (const char * text);
	const char  * GetURL (void) const {return URL;}
	const char  * GetComment (void) const {return Comment;}
	const char  * GetAltText (void) const {return AltText;}
private:
	char 			* URL;
	char 			* Comment;
	char 			* AltText;
};
/*  Reports index of map element that the mouse has moved over  */
class G42HotElement : public G42NotifyMessage
{
public:
				G42HotElement (int16 index) : index (index) {}
	int16		index;			
};
/*  Reports index of map element that has been selected  */
class G42SelectedElement : public G42NotifyMessage
{
public:
				G42SelectedElement (int16 index) : index (index) {}
	int16		index;			
};
/*  Reports index of map element that has been added  */
class G42AddElement : public G42NotifyMessage
{
public:
				G42AddElement (int16 index) : index (index) {}
	int16		index;			
};
/*  Reports image map is about to be clobbered (overwritten)  */
class G42ClobberMap : public G42NotifyMessage
{
public:
				G42ClobberMap (void) {}
};
// For area notify messages.
class G42AreaController;
// Sent when the image has changed in an area.  Useful for status bars.
class G42ImageInfoChanged : public G42NotifyMessage
{
public:
	G42ImageInfoChanged(G42AreaController * sender = 0) : Sender(sender) {}
	G42AreaController * Sender;
};
// For area notify messages.
class G42SurfaceController;
// sent when the window needs invalidating
class G42CanvasNeedsRedrawn : public G42NotifyMessage
{
public:
	G42CanvasNeedsRedrawn(G42SurfaceController * sender = 0) : Sender(sender) {}
	G42SurfaceController * Sender;
};
#endif // GINOTIFY_H
