// g42imap.h - image map class
#if !defined(G42IMAP_H)
#define G42IMAP_H
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
#pragma RTTI on		// KH_
#include <g42type.h>
#include <g42array.h>
#include <math.h>
class G42ImageMap
{
public:
	enum G42MapType // type of maps
		{ CERN, NCSA, ClientSide, Internal };
	enum G42ElementType // list of element types
		{ Rectangle, Circle, Polygon, Point, Default };
	class G42Coord  // coordinate system
	{
	public:
		int x;
		int y;
	};
	class G42MapElement // and element of the map.
	{
	public:
		char * URL;
		char * Comment;
		char * AltText;
		G42ElementType Type;
		G42Array<G42Coord> Coords;
		uint NumCoords;
		G42MapElement() : URL(0), Comment(0), AltText(0),
			Type(Rectangle), NumCoords(0) {}
		~G42MapElement(void)
			{if (URL) delete [] URL; if (Comment) delete [] Comment;
			if (AltText) delete [] AltText;}
	};
	G42ImageMap(const char * file_name); // map from file
	G42ImageMap(const char * map_buffer, uint buffer_length); // map from memory
	G42ImageMap(); // new map with no elements
	~G42ImageMap();
	uint GetNumElements(void) const // number of elements in map
		{ return NumElements; }
	const G42MapElement * GetElement(uint index) // get the element
		{ if (index < NumElements) return Elements[index]; return 0; }
	bool HasDefault(void)  const // true if the map has a default element
		{ return DefaultValid; }
	uint GetDefaultIndex(void) const // index of the default element
		{ return DefaultIndex; }
	G42MapType GetMapType(void) const // type of map
		{ return MapType; }
	bool IsValid(void) const // true if map was valid
		{ return Valid; }
	bool IsDirty(void) const // true if needs saved
		{ return Modified; }
	const char * GetURLAtPoint(int x, int y); // get's the URL of the mouse
	int GetIndexAtPoint(int x, int y);
	void Save(const char * file_name, G42MapType type); // save to disk
	void Save(char * map_buffer, uint max_buffer_length, G42MapType type);
	void SetImageSize(uint width, uint height); // set's maximum size
	void AddElement(G42ElementType T, G42Array<G42Coord> & PtIn, int nPts,
		char* URL, char* Comment, char * alt_text, int N = -1);
	void AddElement(G42ElementType T, int x, int y, int x2, int y2,
		char* URL, char* Comment, char * alt_text, int N = -1);
	void SetElement(uint index, G42MapElement & element); // sets element
	void SetElementURL(uint index, const char * url); // sets element's URL
	void SetElementComment(uint index, const char * comment); // sets comment
	void SetElementAltText(uint index, const char * alt_text); // sets comment
	void SelectElement(uint index);
	void MoveElement(uint index, int dx, int dy);
	void StretchElement(uint index, uint new_width, uint new_height,
		int dx, int dy);
	void SetPolygonPoint(uint index, uint point, int x, int y);
	void ClearSelectedElement(void);
	uint GetSelectedElement(void) const
		{return SelectedElement;}
	bool HasSelectedElement(void) const
		{ return SelectedElementValid; }
	void DeleteSelectedElement(void);
	void DeleteElement(uint index);
	void MoveElementPos(uint new_pos, uint old_pos);
	void SetClientSideName(const char * name);
	const char * GetClientSideName(void) const
		{ return ClientSideName; }
private:
	bool ParseMapString(char * buffer);
	bool GetTwoDigits(char** ptr, int* x, int* y);
	bool PointInPoly(int X, int Y, int Index);
protected:
	G42Array<G42MapElement *> Elements;
	uint NumElements;
	uint SelectedElement;
   bool SelectedElementValid;
	bool DefaultValid;
	uint DefaultIndex;
	bool Valid;
	bool Modified;
	uint ImageWidth;
	uint ImageHeight;
	G42MapType		MapType;
   char *			Comment;
	char *			AltText;
	char *			URL;
	bool 				IsClientSide;
	bool				IsInternal;
	int 				ClientSideState;
	bool 				ClientSideDone;
	G42ElementType	ClientSideShape;
	G42Array<G42Coord>	ClientSidePointArray;
	int 				ClientSideNumPointArray;
	bool 				ClientSideHaveMap;
	bool 				ClientSideHaveShape;
	bool 				ClientSideHaveCoords;
	char * 			ClientSideString;
	int 				ClientSideToken;
	char * 			ClientSideName;
	int 				x1;
	int				y1;
	int 				x2;
	int 				y2;
	
	void				SaveNCSA(FILE *fp);
	void				SaveCERN(FILE *fp);
	void				SaveClientSide(const char * file_name, FILE *fp);
	void 				SaveInternal(FILE *fp);
	void 				SaveNCSA(char *ptr);
	void 				SaveCERN(char *ptr);
	void 				SaveClientSide(char *ptr);
	void 				SaveInternal(char *ptr);
};
#pragma RTTI reset		// KH_
#endif // G42IMAP_H
