#ident "$Id$"

#ifndef __SMSC_INMAN_TCP_SERVER__
#define __SMSC_INMAN_TCP_SERVER__

#include <list>

#include "logger/Logger.h"
#include "core/threads/Thread.hpp"
#include "core/synchronization/Event.hpp"
#include "core/network/Socket.hpp"
#include "inman/common/observable.hpp"
#include "inman/interaction/connect.hpp"

using smsc::logger::Logger;
using smsc::core::network::Socket;
using smsc::inman::inap::Connect;
using smsc::core::threads::Thread;
using smsc::core::synchronization::Event;
using smsc::inman::common::ObservableT;


namespace smsc  {
namespace inman {
namespace interaction  {

class Server;

class ServerListener
{
    public:
        virtual void onConnectOpened(Server*, Connect*) = 0;
        virtual void onConnectClosed(Server*, Connect*) = 0;
};

class Server : public Thread, public ObservableT< ServerListener >
{
    typedef std::list<Connect*> Connects;

    public:
        Server(const char* szHost, int nPort, SerializerITF * serializer);
        virtual ~Server();

        void openConnect(Connect* connect);
        void closeConnect(Connect* connect);

        int  Execute();
        void Stop();
        void Run();

    protected:
        Event           started;
        Event           stopped;
        volatile bool   running;

        SerializerITF * ipSerializer;
        Socket      serverSocket;
        Connects    connects;
        Logger*     logger;
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_TCP_SERVER__ */

