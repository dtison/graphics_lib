// gimap.h - G42ImageMap <based on the 2.0 libraries>
#if !defined(GIMAP_H)
#define GIMAP_H                                
/*
	Image Map class - reads, writes, stores, and modifies an image map.
	There are three constructors for this class.  The first creates
	a blank class (which can later be added to).  The others create
	a map off of a file or a memory area which contains a map file.
	These files tend to be short, so the 64K limit shouldn't become
	a problem.
		G42ImageMap(const char * file_name); // map from file
		G42ImageMap(const char * map_buffer, uint buffer_length); // map from memory
		G42ImageMap(); // new map with no elements
		~G42ImageMap();
	A few other functions that would be of interest:
	Information functions include GetMapType, IsValid, IsDirty, and
	GetNumElements.  Note that you can have an Valid empty file
	(GetNumElements returns zero).  Be sure to check IsValid after
	constructing, to see if the map was valid.
		enum G42MapType // type of maps
			{ CERN, NCSA, ClientSide, Internal };
		G42MapType GetMapType(void) const; // type of map
		bool IsValid(void) const; // true if map was valid
		bool IsDirty(void) const; // true if needs saved
		uint GetNumElements(void) const; // number of elements in map
	Client Side maps have a name associated with them.  You can get and
	set this name with GetClientSideName and SetClientSideName.
		void SetClientSideName(const char * name);
		const char * GetClientSideName(void) const;
	Saving gives two options.  You can save to a file or a memory
	area.  Be sure to allocate enough memory to hold the entire map,
	as the length field is not currently checked (but will be in the
	future, so pass the correct number).  I tend to allocate 60K, to
	be extra safe.
		void Save(const char * file_name, G42MapType type); // save to disk
		void Save(char * map_buffer, uint max_buffer_length, G42MapType type);
*/
#include "gtype.h"
#include "gtools.h"
#include "math.h"
#include "ggeomtry.h"
#include "gfile.h"
const	int16 G42MaxElements = 1500;
enum G42MapType // type of maps
	{ CERN, NCSA, ClientSide, Internal };
class G42MapElement // and element of the map.
{
public:
enum G42ElementType // list of element types
	{ Rectangle, Circle, Polygon, Point, Default };
	char * URL;
	char * Comment;
	char * AltText;
	G42ElementType Type;
	G42Vector <G42Point> Points;
	int16 NumPoints;
	G42MapElement() : URL(0), Comment(0), AltText(0),
		Type(Rectangle), NumPoints(0) {}
	G42MapElement(G42ElementType type, const char * url = 0, const char * comment = 0,
		const char * alt_text = 0);
	G42MapElement(const G42MapElement & ref);
	~G42MapElement(void)
		{if (URL) delete [] URL; if (Comment) delete [] Comment;
		if (AltText) delete [] AltText;}
};
class G42ImageMap
{
public:
	G42ImageMap(const G42FilePath file_path); // map from file
	G42ImageMap(const char * map_buffer, uintf buffer_length); // map from memory
	G42ImageMap(); // new map with no elements
	~G42ImageMap();
	void AddElement(G42MapElement::G42ElementType T,
   	const G42Vector<G42Point> & PtIn, int16 nPts, const char * URL,
      const char * Comment, const char * alt_text, int16 N = -1);
	void AddElement(G42MapElement::G42ElementType T, int x, int y, int x2, int y2,
		char* URL, char* Comment, char * alt_text, int16 N = -1);
	void AddElement(const G42MapElement & element, int16 N = -1); // appends the element to the list
	void ClearSelectedElement(void);
	void DeleteSelectedElement(void);
	void DeleteElement(int16 index);
	const char * GetClientSideName(void) const
		{ return ClientSideName; }
	int16 GetNumElements(void) const // number of elements in map
		{ return NumElements; }
	const G42MapElement * GetElement(int16 index) // get the element
		{ if (index < NumElements) return Elements[index]; return 0; }
	int16 GetDefaultIndex(void) const // index of the default element
		{ return DefaultIndex; }
	G42MapType GetMapType(void) const // type of map
		{ return MapType; }
	const char * GetURLAtPoint(int x, int y); // get's the URL of the mouse
	int16 GetIndexAtPoint(int x, int y);
	int16 GetSelectedElement(void) const
		{return SelectedElement;}
	bool HasDefault(void)  const // true if the map has a default element
		{ return DefaultValid; }
	bool IsDirty(void) const // true if needs saved
		{ return Modified; }
	bool IsValid(void) const // true if map was valid
		{ return Valid; }
	bool HasSelectedElement(void) const
		{ return SelectedElementValid; }
	void MoveElement(int16 index, int dx, int dy);
	void MoveElementPos(int16 new_pos, int16 old_pos);
	void Save(const G42FilePath file_path, G42MapType type); // save to disk
	void Save(char * map_buffer, uintf max_buffer_length, G42MapType type);
	void SetClientSideName(const char * name);
	void SetImageSize(uintf width, uintf height); // set's maximum size
	void SetElement(int16 index, const G42MapElement & element); // sets element
	void SetElementURL(int16 index, const char * url); // sets element's URL
	void SetElementComment(int16 index, const char * comment); // sets comment
	void SetElementAltText(int16 index, const char * alt_text); // sets comment
	void SelectElement(int16 index);
	void SetPolygonPoint(int16 index, uintf point, int x, int y);
	void StretchElement(int16 index, uintf new_width, uintf new_height,
		int dx, int dy);
protected:
	G42Vector<G42MapElement *> Elements;
	int16 			NumElements;
	int16 			SelectedElement;
	bool	 			SelectedElementValid;
	bool 				DefaultValid;
	int16 			DefaultIndex;
	bool 				Valid;
	bool 				Modified;
	uintf 			ImageWidth;
	uintf 			ImageHeight;
	G42MapType		MapType;
	char *			Comment;
	char *			AltText;
	char *			URL;
	bool 				IsClientSide;
	bool				IsInternal;
	int 				ClientSideState;
	bool 				ClientSideDone;
	G42MapElement::G42ElementType	ClientSideShape;
	G42Vector		<G42Point> ClientSidePointArray;
	int16 			ClientSideNumPointArray;
	bool 				ClientSideHaveMap;
	bool 				ClientSideHaveShape;
	bool 				ClientSideHaveCoords;
	char *			ClientSideString;
	int 				ClientSideToken;
	char *			ClientSideName;
	int 				x1;
	int				y1;
	int 				x2;
	int 				y2;
	void				SaveNCSA(FILE *fp);
	void				SaveCERN(FILE *fp);
	void				SaveClientSide(const char * full_path, FILE *fp);
	void 				SaveInternal(FILE *fp);
	void 				SaveNCSA(char *ptr);
	void 				SaveCERN(char *ptr);
	void 				SaveClientSide(char *ptr);
	void 				SaveInternal(char *ptr);
private:
	bool 				ParseMapString(char * buffer);
	bool 				GetTwoDigits(char** ptr, int* x, int* y);
	bool 				PointInPoly(int X, int Y, int Index);
};
#endif // GIMAP_H
