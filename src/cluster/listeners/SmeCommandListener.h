#ifndef SmeCommandListener_dot_h
#define SmeCommandListener_dot_h

#include "cluster/Interconnect.h"
#include "smeman/smeman.h"

namespace smsc {
namespace cluster {

class SmeCommandListener : public CommandListener
{
public:
    SmeCommandListener(smsc::smeman::SmeManager *smeman_);
protected:
    void smeAdd(const Command& command);
	void smeRemove(const Command& command);
	void smeUpdate(const Command& command);
    smsc::smeman::SmeManager *smeman;
		
public:
	virtual void handle(const Command& command);
};

}
}

#endif
