#ifndef AclCommandListener_dot_h
#define AclCommandListener_dot_h

#include "cluster/Interconnect.h"
#include "acls/interfaces.h"

namespace smsc {
namespace cluster {

using smsc::acls::AclAbstractMgr;

class AclCommandListener : public CommandListener
{
public:
    AclCommandListener(AclAbstractMgr   *aclmgr_);
protected:
    void aclRemove(const Command& command);
	void aclCreate(const Command& command);
	void aclUpdateInfo(const Command& command);
	void aclRemoveAddresses(const Command& command);
	void aclAddAddresses(const Command& command);
    AclAbstractMgr   *aclmgr;
	
public:
	virtual void handle(const Command& command);
};

}
}

#endif
