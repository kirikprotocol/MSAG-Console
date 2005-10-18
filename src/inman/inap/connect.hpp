#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_CONNECT__
#define __SMSC_INMAN_INAP_CONNECT__

#include "dispatcher.hpp"
#include "inman/interaction/messages.hpp"
#include "inman/interaction/serializer.hpp"
#include "inman/common/observable.hpp"

using smsc::inman::common::ObservableT;
using smsc::inman::interaction::InmanCommand;
using smsc::inman::interaction::ObjectPipe;
using smsc::inman::interaction::SerializableObject;

namespace smsc  {
namespace inman {
namespace inap  {

class Connect;

class ConnectListener
{
    public:
//        virtual void onCommandReceived(Connect*, InmanCommand*) = 0;
        virtual void onCommandReceived(Connect*, SerializableObject*) = 0;
};

class Connect : public ObservableT< ConnectListener >
{
    public:
//        Connect(Socket* socket); //default connection with Inap serializer
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
