#if !defined(GFUTIL_H)
#define GFUTIL_H
#include "gtype.h"
#include "gfile.h"
uint32 GetDiskSpace (void);
uint32 GetDiskSpace (G42FilePath & file_path);
void Unlink (const G42FilePath & file_path);
#endif // GFUTIL_H sentry
