#ifndef MscCommandListener_dot_h
#define MscCommandListener_dot_h

#include "cluster/Interconnect.h"

namespace smsc {
namespace cluster {

class MscCommandListener : public CommandListener
{
protected:
    void mscRegistrate(const Command& command);
	void mscUnregistrate(const Command& command);
	void mscBlock(const Command& command);
	void mscClear(const Command& command);
		
public:
	virtual void handle(const Command& command);
};

}
}

#endif
