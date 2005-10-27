#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_INVOKE__
#define __SMSC_INMAN_INAP_INVOKE__

#include "inman/inap/entity.hpp"

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

class Dialog;
class Invoke : public TcapEntity
{
    public:
        Invoke() {_iResHdl = NULL; }
        virtual ~Invoke() {}

        void setListener(InvokeListener * plistener) { _iResHdl = plistener; }
        const InvokeListener * getListener(void) const { return _iResHdl; }

    	virtual void send(Dialog* dialog);
        virtual void notifyResultListener(TcapEntity* resp);
        virtual void notifyResultNListener(TcapEntity* resp);
        virtual void notifyErrorListener(TcapEntity* resp);

    protected:
        InvokeListener * _iResHdl; //optional
};

} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_INVOKE__ */

