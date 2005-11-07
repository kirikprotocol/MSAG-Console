#ident "$Id$"

#ifndef __SMSC_INMAN_TCP_CONNECT__
#define __SMSC_INMAN_TCP_CONNECT__

#include "inman/interaction/serializer.hpp"
#include "inman/common/observable.hpp"

using smsc::inman::common::ObservableT;
using smsc::inman::interaction::ObjectPipe;
using smsc::inman::interaction::SerializableObject;
using smsc::inman::interaction::SerializerITF;

namespace smsc  {
namespace inman {
namespace interaction {

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
        void    setPipeFormat(ObjectPipe::PipeFormat frm);
        bool    process();

    protected:
        Socket*     socket;
        ObjectPipe* pipe;
        Logger*     logger;
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_TCP_CONNECT__ */

