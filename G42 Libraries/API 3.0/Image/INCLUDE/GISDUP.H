// gisdup.h
#if !defined(GISDUP_H)
#define GISDUP_H
#include "gpcomp.h"
#include "gtools.h"
class G42ImageStreamDuplicator : public G42PathComponent
{
public:
	G42ImageStreamDuplicator();
	virtual ~G42ImageStreamDuplicator();
	virtual void Process(G42PathObject * object, G42PathComponent * sender);
	virtual void ReplaceReceiver(G42PathComponent * old_receiver,
		G42PathComponent * new_receiver);
	virtual void SetReceiver(G42PathComponent * receiver);
	virtual void SetHead(void);
	virtual void ReceiverDestructing(G42PathComponent * receiver);
	virtual void ClearPath(bool path_excepted = false);
protected:
	G42ExpandableArray<G42PathComponent *> Receivers;
	intf NumReceivers;
};
#endif // GISDUP_H
