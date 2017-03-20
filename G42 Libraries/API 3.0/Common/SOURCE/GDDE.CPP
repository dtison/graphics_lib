#include "gdde.h"
extern G42DDE * GlobalDDE;
HDDEDATA CALLBACK DDE_CB(UINT type, UINT fmt, HCONV hconv, HSZ hsz1,
	HSZ hsz2, HDDEDATA hData, DWORD dwData1, DWORD dwData2)
{
	if (!GlobalDDE)
		return(0);
	switch (type)
	{
		case XTYP_POKE:
		{
		char filename[2048];
		char topic[128];
			if (hsz1)
			{
				DdeQueryString(GlobalDDE->GetInst(), hsz1, topic, 128, CP_WINANSI);
				if (!strcmp(topic, "G42_OpenFiles"))
				{
					if (hsz2)
					{
						DdeQueryString(GlobalDDE->GetInst(), hsz2, filename, 2048, CP_WINANSI);
						char * tptr = filename;
						GlobalDDE->OpenReceivedFiles(filename);
						return (HDDEDATA) DDE_FACK;
					}
				}
			}
			return (DDE_FNOTPROCESSED);
		}
		case XTYP_CONNECT:
		{
			char who[20], what[20];
			if (hsz1 && hsz2)
			{
				*who = *what = '\0';
				DdeQueryString(GlobalDDE->GetInst(), hsz2, who, 20, CP_WINANSI);
				DdeQueryString(GlobalDDE->GetInst(), hsz1, what, 20, CP_WINANSI);
				if (!strcmp(who, "G42RimShot"))
				{
					bool val = false;
					if (!GlobalDDE->IsSending() && !strcmp(what, "G42_Ping"))
						val = true;
					else if (!GlobalDDE->IsSending() && !strcmp(what, "G42_OpenFiles"))
						val = true;
					return (HDDEDATA) val;
				}
			}
			return (false);
		}
		case XTYP_REQUEST:
			return(DDE_FNOTPROCESSED);
		case XTYP_ADVSTART:
			return (false);
		case XTYP_ADVDATA:
			return (DDE_FNOTPROCESSED);
		case XTYP_EXECUTE:
			return (DDE_FNOTPROCESSED);
	}
	return (NULL);
}
G42DDE::G42DDE(void)
	: DdeInst(0L), Valid(false), Sending(false)
{
	UINT ret = DdeInitialize(&DdeInst, DDE_CB, APPCLASS_STANDARD, 0L);
	if (ret)
		return;
	ThisApp = DdeCreateStringHandle(DdeInst, "G42RimShot", CP_WINANSI);
	if (!ThisApp)
		return;
	if (!DdeNameService(DdeInst, ThisApp, (HSZ)NULL, DNS_REGISTER))
		return;
	Valid = true;
}
G42DDE::~G42DDE(void)
{
	if (ThisApp)
		DdeFreeStringHandle(DdeInst, ThisApp);
	if (DdeInst)
		DdeUninitialize(DdeInst);
}
bool
G42DDE::PingRimShot(void)
{
	HSZ topic = DdeCreateStringHandle(DdeInst, "G42_Ping", CP_WINANSI);
	if (!topic)
		return false;
	Sending = true;
	HCONV rimshot_con = DdeConnect(DdeInst, ThisApp, topic, NULL);
	if (!rimshot_con)
	{
		DdeFreeStringHandle(DdeInst, topic);
		Sending = false;
		return false;
	}
	DdeDisconnect(rimshot_con);
	DdeFreeStringHandle(DdeInst, topic);
	Sending = false;
	return true;
}
bool
G42DDE::OpenFiles(const char * buffer)
{
	HSZ topic = DdeCreateStringHandle(DdeInst, "G42_OpenFiles", CP_WINANSI);
	if (!topic)
		return false;
	Sending = true;
	HCONV rimshot_con = DdeConnect(DdeInst, ThisApp, topic, NULL);
	if (!rimshot_con)
	{
		DdeFreeStringHandle(DdeInst, topic);
		Sending = false;
		return false;
	}
	HSZ item = DdeCreateStringHandle(DdeInst, buffer, CP_WINANSI);
	HDDEDATA result = DdeClientTransaction(NULL, 0, rimshot_con, item, CF_TEXT,
			XTYP_POKE, 1000, NULL);
	bool retval;
	if (result)
		retval = true;
	else
		retval = false;
	DdeFreeStringHandle(DdeInst, item);
	DdeDisconnect(rimshot_con);
	DdeFreeStringHandle(DdeInst, topic);
	Sending = false;
	return retval;
}
