#ifndef SbmCommandListener_dot_h
#define SbmCommandListener_dot_h

#include "cluster/Interconnect.h"
#include "distrlist/DistrListManager.h"

namespace smsc {
namespace cluster {

using smsc::distrlist::DistrListAdmin;

class SbmCommandListener : public CommandListener
{
public:
    SbmCommandListener(DistrListAdmin* dladmin_);
protected:
    void sbmAddSubmiter(const Command& command);
	void sbmDeleteSubmiter(const Command& command);
    DistrListAdmin* dladmin;
		
public:
	virtual void handle(const Command& command);
};

}
}

#endif
