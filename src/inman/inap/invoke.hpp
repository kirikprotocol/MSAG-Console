#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_INVOKE__
#define __SMSC_INMAN_INAP_INVOKE__

#include <map>

#include "entity.hpp"

#include "inman/common/types.hpp"
#include "inman/common/observable.hpp"
using smsc::inman::common::ObservableT;

namespace smsc  {
namespace inman {
namespace inap  {

class InvokeListener
{
    public:
        //NOTE: below methods should not take ownership of TcapEntity::param
        virtual void result(TcapEntity*) = 0;
        virtual void error(TcapEntity*) = 0;
        virtual void resultNL(TcapEntity*) = 0;
};


class Invoke : public TcapEntity, public ObservableT< InvokeListener >
{
    public:
    	virtual void send(Dialog* dialog);
        virtual void notifyResultListeners(TcapEntity* resp);
        virtual void notifyResultNListeners(TcapEntity* resp);
        virtual void notifyErrorListeners(TcapEntity* resp);
};

}
}
}

#endif
