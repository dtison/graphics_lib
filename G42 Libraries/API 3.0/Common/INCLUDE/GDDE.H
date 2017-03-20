#if !defined(G42DDE_H)
#define G42DDE_H
#include "gtype.h"
#define STRICT
#include <windows.h>
#include <ddeml.h>
class G42DDE
{
private:
	uint32	DdeInst;
	HSZ 		ThisApp;
	bool		Valid;
	bool 		Sending;
	// Hidden Copy Constructor and operator=
	G42DDE(G42DDE & ref) {}
	G42DDE & operator= (G42DDE & ref)
		{return *this;}
public:
	G42DDE(void);
	virtual ~G42DDE(void);
	bool IsValid(void) const
		{return Valid;}
	bool IsSending(void) const
		{return Sending;}
	int32 GetInst(void) const
		{return DdeInst;}
	bool PingRimShot(void);
	bool OpenFiles(const char * buffer);
	virtual void OpenReceivedFiles(const char * buffer) = 0;
};
#endif // G42DDE_H
