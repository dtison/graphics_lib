// gilibman - Image Library Manager
#include "glibman.h"
G42LibManager::G42LibManager(const G42StringTable & table,
	const G42FilePath & swapper_path,
	const G42FilePath & dll_path, int32 max_entry_size, uint32 max_core_size)
:
	Heap(0), Codec(0), Swapper(0)
{
	Set(table, swapper_path, dll_path, max_entry_size,
		max_core_size);
}
G42LibManager::G42LibManager(const G42StringTable & table,
	const G42FilePath & dll_path, int32 max_entry_size, uint32 max_core_size)
:
	Heap(0), Codec(0), Swapper(0)
{
	Set(table, (char *)0, dll_path, max_entry_size, max_core_size);
}
G42LibManager::G42LibManager(const G42StringTable & table,
	int32 max_entry_size, uint32 max_core_size)
:
	Heap(0), Codec(0), Swapper(0)
{
	Set(table, (char *)0, (char *)0,
		max_entry_size, max_core_size);
}
G42LibManager::G42LibManager(const G42StringTable & table,
	const G42FilePath & swapper_path, const G42FilePath & dll_path,
	G42Notifier & notifier, int32 max_entry_size, uint32 max_core_size)
:
	Heap(0), Codec(0), Swapper(0), Notifier(notifier)
{
	Set(table, swapper_path, dll_path, max_entry_size,
		max_core_size);
}
G42LibManager::G42LibManager(const G42StringTable & table,
	const G42FilePath & dll_path, G42Notifier & notifier,
	int32 max_entry_size, uint32 max_core_size)
:
	Heap(0), Codec(0), Swapper(0), Notifier(notifier)
{
	Set(table, (char *)0, dll_path, max_entry_size, max_core_size);
}
G42LibManager::G42LibManager(const G42StringTable & table,
	G42Notifier & notifier, int32 max_entry_size, uint32 max_core_size)
:
	Heap(0), Codec(0), Swapper(0), Notifier(notifier)
{
	Set(table, (char *)0, (char *)0,
		max_entry_size, max_core_size);
}
void
G42LibManager::Set(const G42StringTable & table,
	const G42FilePath & swapper_path,
	const G42FilePath & dll_path, int32 max_entry_size, uint32 max_core_size)
{
	StringTable = table;
	Swapper = new G42SwapManager(swapper_path, Notifier);
	Heap = new G42HeapManager(*Swapper, max_entry_size, max_core_size);
	Codec = new G42CodecManager(table, dll_path);
}
G42LibManager::~G42LibManager(void)
{
	if (Heap)
	{
		delete Heap;
		Heap = 0;
	}
	if (Swapper)
	{
		delete Swapper;
		Swapper = 0;
	}
	if (Codec)
	{
		delete Codec;
		Codec = 0;
	}
}
