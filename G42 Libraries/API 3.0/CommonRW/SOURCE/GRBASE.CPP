#include "grbase.h"
G42BaseReader::G42BaseReader(void)
	: G42PathComponent(), File(0), Heap(0), BytesUsed(0), Options(0)
{}
G42BaseReader::G42BaseReader(G42HeapManager & heap, G42Notifier & notifier,
	G42File * file, G42BaseOptions * options)
	: G42PathComponent(), File(file), Heap(&heap), BytesUsed(0),
	Options(0)
{
	Notifier = &notifier;
	if (options)
		Options = options->MakeCopy();
}
G42BaseReader::~G42BaseReader(void)
{
	if (Options)
		delete Options;
	Options = 0;
}
