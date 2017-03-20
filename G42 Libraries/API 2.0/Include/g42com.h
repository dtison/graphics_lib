// g42com.h - Comment class
#if !defined(G42COM_H)
#define G42COM_H
class G42Comment
{
public:
	G42Comment() : Text(0), Key(0), Seperator(0) {}
	G42Comment(G42Comment & comment);
	G42Comment & operator = (G42Comment & comment);
	~G42Comment();
	const char * GetComment(void);
	const char * GetKey(void);
	const char * GetText(void)
		{ return Text; }
	void SetComment(const char * comment);
	void SetComment(const char * key, const char * text);
private:
	void Clear(void);
	char * Text;
	char * Key;
	char * Seperator;
};
inline void
G42Comment::Clear(void)
{
	if (Key)
		delete[] Key;
	else if (Text)
		delete[] Text;
	Key = 0;
	Text = 0;
	Seperator = 0;
}
inline const char *
G42Comment::GetComment(void)
{
	if (Seperator)
		*Seperator = ' ';
	if (Key)
		return Key;
	return Text;
}
inline const char *
G42Comment::GetKey(void)
{
	if (Seperator)
		*Seperator = '\0';
	return Key;
}
inline void
G42Comment::SetComment(const char * comment)
{
	Clear();
	if (comment && *comment)
	{
		int length = strlen(comment);
		Text = new char [length + 1];
		if (Text)
			strcpy(Text, comment);
	}
}
inline void
G42Comment::SetComment(const char * key, const char * text)
{
	Clear();
	int key_length;
	if (key && *key)
		key_length = strlen(key);
	else
		key_length = 0;
	int text_length;
	if (text && *text)
		text_length = strlen(text);
	else
   	text_length = 0;
	if (key_length && text_length)
	{
		Key = new char [key_length + text_length + 4];
		if (Key)
		{
			strcpy(Key, key);
			Seperator = Key + key_length;
			strcpy(Seperator, " - ");
			Text = Seperator + 3;
			strcpy(Text, text);
		}
	}
	else if (key_length)
	{
		Key = new char [key_length + 1];
		if (Key)
			strcpy(Key, text);
	}
	else if (text_length)
	{
		Text = new char [text_length + 1];
		if (Text)
			strcpy(Text, text);
	}
}
inline G42Comment &
G42Comment::operator = (G42Comment & comment)
{
	if (&comment != this)
	{
		Clear();
		SetComment(comment.Key, comment.Text);
	}
   return *this;
}
inline
G42Comment::G42Comment(G42Comment & comment) : Text(0), Key(0), Seperator(0)
{
	SetComment(comment.Key, comment.Text);
}
inline
G42Comment::~G42Comment()
{
	Clear();
}
#endif // G42COM_H
