// gilibman - Library Manager
#if !defined(GLIBMAN_H)
#define GLIBMAN_H
#include "gmem.h"
#include "gswapper.h"
#include "gcodec.h"
#include "gstrtab.h"
#include "gnotify.h"
class G42LibManager
{
public:
	G42LibManager(const G42StringTable & table, const G42FilePath & swapper_path,
		const G42FilePath & dll_path,
		int32 max_entry_size = 64512L, uint32 max_core_size = 4194304L);
	G42LibManager(const G42StringTable & table, const G42FilePath & dll_path,
		int32 max_entry_size = 64512L, uint32 max_core_size = 4194304L);
	G42LibManager(const G42StringTable & table,
		int32 max_entry_size = 64512L, uint32 max_core_size = 4194304L);
	G42LibManager(const G42StringTable & table, const G42FilePath & swapper_path,
		const G42FilePath & dll_path, G42Notifier & notifier,
		int32 max_entry_size = 64512L, uint32 max_core_size = 4194304L);
	G42LibManager(const G42StringTable & table, const G42FilePath & dll_path,
		 G42Notifier & notifier,
		int32 max_entry_size = 64512L, uint32 max_core_size = 4194304L);
	G42LibManager(const G42StringTable & table, G42Notifier & notifier,
		int32 max_entry_size = 64512L, uint32 max_core_size = 4194304L);
	~G42LibManager(void);
	G42HeapManager & GetHeap(void) const
		{ return *Heap; }
	G42SwapManager & GetSwapper(void) const
		{ return *Swapper; }
	G42CodecManager & GetCodec(void) const
		{ return *Codec; }
	const G42StringTable & GetStringTable(void) const
		{ return StringTable; }
	G42ResourceIdentifier GetResourceIdentifier(void) const
		{ return StringTable.GetResourceIdentifier(); }
protected:
	G42CodecManager *	Codec;
	G42HeapManager *	Heap;
	G42NotifierVerifier
							Notifier;
	G42StringTable		StringTable;
	G42SwapManager *	Swapper;
	void Set(const G42StringTable & table, const G42FilePath & swapper_path,
		const G42FilePath & dll_path, int32 max_entry_size, uint32 max_core_size);
	G42LibManager(G42LibManager &) {}    // Hidden Copy Constructor.
	G42LibManager & operator = (G42LibManager &)
		{return *this;}											// Hidden Operator=
};
#endif // GILIBMAN_H sentry
