#if !defined(GUNDO_H)
#define GUNDO_H

#include "gtype.h"
#include "gtools.h"


template <class T>
class G42MultiUndo
{
	class G42Undo
	{
	public:
		G42Undo (const char * undo_text) : UndoText(0)
			{SetUndoText(undo_text);}
		~G42Undo (void) {}
		const char * GetUndoText (void) const
			{return UndoText ? (const char *)UndoText : (const char *)0;}
		void SetUndoText (const char * undo_text);

	private:
		G42APtr<char> UndoText;
	};

	class G42UndoElement : public G42Undo
	{
	public:
		G42UndoElement (const T & undo_sound, const char * undo_text = 0) :
			G42Undo(undo_text)
			{SetUndoElement(undo_sound);}
		~G42UndoElement (void) {}
		T & GetUndoElement (void)
			{return UndoElement;}
		void SetUndoElement (const T & undo_sound)
			{UndoElement = (T&)undo_sound;}

	private:
		T	UndoElement;
	};

public:
	G42MultiUndo (int max_entries) : MaxEntries(max_entries), UndoEntries(0),
		UndoArray(0)
		{UndoArray = new G42UndoElement * [MaxEntries];}
	~G42MultiUndo (void);
	int GetUndoEntries (void) const
		{return UndoEntries;}
	int GetMaxEntries (void) const
		{return MaxEntries;}
	void SetMaxEntries (int max_entries);
	bool HasUndoElement (void) const
		{return UndoEntries > 0;}
	T & GetUndoElement (void);
	const char * GetUndoText (void) const;
	void Insert (const T & undo_sound, const char * undo_text = 0);
	void Remove (void);
	void Clear (void);
	T & operator * (void)
		{return GetUndoElement();}

private:
	G42UndoElement **	UndoArray;
	int					MaxEntries;
	int 					UndoEntries;
};

template <class T>
void
G42MultiUndo<T>::SetMaxEntries (int max_entries)
{
	if (max_entries == MaxEntries)
		return;

	if (max_entries < MaxEntries)
	{
		if (UndoEntries > max_entries)
		{
			for (int n = 0; n < UndoEntries - max_entries; n++)
				delete UndoArray[n];

			UndoEntries--;

			for (int n = 0; n < UndoEntries; n++)
				UndoArray[n] = UndoArray[n+1];
		}
	}
	else
	{
		G42UndoElement ** new_sound = new G42UndoElement * [max_entries];
		for (int n = 0; n < UndoEntries; n++)
			new_sound[n] = UndoArray[n];

		delete [] UndoArray;
		UndoArray = new_sound;
	}

	MaxEntries = max_entries;
}

template <class T>
G42MultiUndo<T>::~G42MultiUndo (void)
{
	for (int n = 0; n < UndoEntries; n++)
		delete UndoArray[n];

	delete [] UndoArray;
}

template <class T>
void
G42MultiUndo<T>::Insert (const T & undo_sound, const char * undo_text)
{
	if (UndoEntries == MaxEntries)
	{
		delete UndoArray[0];
		for (int n = 0; n < MaxEntries - 1; n++)
		{
			UndoArray[n] = UndoArray[n + 1];
		}

		UndoEntries--;
	}

	UndoArray[UndoEntries] = new G42UndoElement(undo_sound, undo_text);
	UndoEntries++;
}

template <class T>
void
G42MultiUndo<T>::Remove (void)
{
	if (!UndoEntries)
		throw G42Exception(0);

	delete UndoArray[UndoEntries - 1];
	UndoEntries--;
}

template <class T>
T &
G42MultiUndo<T>::GetUndoElement (void)
{
	if (!UndoEntries)
		throw G42Exception(0);

	return UndoArray[UndoEntries - 1]->GetUndoElement();
}

template <class T>
const char *
G42MultiUndo<T>::GetUndoText (void) const
{
	if (!UndoEntries)
		throw G42Exception(0);

	return UndoArray[UndoEntries - 1]->GetUndoText();
}

template <class T>
void
G42MultiUndo<T>::Clear (void)
{
	if (!UndoEntries)
		return;

	for (int n = 0; n < UndoEntries; n++)
		delete UndoArray[n];

	UndoEntries = 0;
}

template <class T> void
G42MultiUndo<T>::G42Undo::SetUndoText (const char * undo_text)
{
	if (!undo_text || *undo_text == 0)
		return;

	UndoText = new char[strlen(undo_text) + 6];
	sprintf(UndoText, "Undo %s", undo_text);
}

#endif	//GUNDO_H Sentry - do not type after this line
