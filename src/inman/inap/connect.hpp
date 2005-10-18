#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_CONNECT__
#define __SMSC_INMAN_INAP_CONNECT__

#include "inman/interaction/serializer.hpp"
#include "inman/common/observable.hpp"

using smsc::inman::common::ObservableT;
using smsc::inman::interaction::ObjectPipe;
using smsc::inman::interaction::SerializableObject;
using smsc::inman::interaction::SerializerITF;

namespace smsc  {
namespace inman {
namespace inap  {

class Connect;

class ConnectListener
{
    public:
        virtual void onCommandReceived(Connect*, SerializableObject*) = 0;
};

class Connect : public ObservableT< ConnectListener >
{
    public:
        Connect(Socket* sock, SerializerITF * serializer);
        virtual ~Connect();

        Socket* getSocket();
        void    send(SerializableObject* obj);

        bool    process();

    protected:
        Socket*     socket;
        ObjectPipe* pipe;
        Logger*     logger;
};

}
}
}

#endif
