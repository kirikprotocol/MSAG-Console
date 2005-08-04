#ifndef MemCommandListener_dot_h
#define MemCommandListener_dot_h

#include "cluster/Interconnect.h"
#include "distrlist/DistrListManager.h"

namespace smsc {
namespace cluster {

using smsc::distrlist::DistrListAdmin;

class MemCommandListener : public CommandListener
{
public:
    MemCommandListener(DistrListAdmin* dladmin_);
protected:
    void memAddMember(const Command& command);
	void memDeleteMember(const Command& command);
    DistrListAdmin* dladmin;
		
public:
	virtual void handle(const Command& command);
};

}
}

#endif
