#ifndef PrcCommandListener_dot_h
#define PrcCommandListener_dot_h

#include "cluster/Interconnect.h"
#include "distrlist/DistrListManager.h"

namespace smsc {
namespace cluster {

using smsc::distrlist::DistrListAdmin;

class PrcCommandListener : public CommandListener
{
public:
    PrcCommandListener(DistrListAdmin* dladmin_);
protected:
    void prcAddPrincipal(const Command& command);
	void prcDeletePrincipal(const Command& command);
	void prcAlterPrincipal(const Command& command);
    DistrListAdmin* dladmin;
		
public:
	virtual void handle(const Command& command);
};

}
}

#endif
