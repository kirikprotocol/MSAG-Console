#ifndef DlCommandListener_dot_h
#define DlCommandListener_dot_h

#include "cluster/Interconnect.h"
#include "distrlist/DistrListManager.h"

namespace smsc {
namespace cluster {

using smsc::distrlist::DistrListAdmin;

class DlCommandListener : public CommandListener
{
public:
    DlCommandListener(DistrListAdmin* dladmin_);
protected:
    void dlAdd(const Command& command);
	void dlDelete(const Command& command);
	void dlAlter(const Command& command);
    DistrListAdmin* dladmin;
		
public:
	virtual void handle(const Command& command);
};

}
}

#endif
