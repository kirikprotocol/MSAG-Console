#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_SERVER__
#define __SMSC_INMAN_INAP_SERVER__

#include <list>

#include "logger/Logger.h"
#include "core/threads/Thread.hpp"
#include "core/synchronization/Event.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/network/Socket.hpp"
#include "inman/common/observable.hpp"
#include "connect.hpp"
#include "synch.hpp"

using smsc::logger::Logger;
using smsc::core::network::Socket;
using smsc::core::threads::Thread;
using smsc::core::synchronization::Event;
using smsc::core::synchronization::Mutex;
using smsc::inman::common::ObservableT;


namespace smsc  {
namespace inman {
namespace inap  {

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
    	Server(const char* szHost, int nPort);
    	virtual ~Server();

		void openConnect(Connect* connect);
		void closeConnect(Connect* connect);

		int  Execute();
		void Stop();
		void Run();

	protected:
		Event			   	started;
		Event			   	stopped;
		volatile bool	   	running;
    	Socket 				serverSocket;
		Connects 			connects;
        Logger*				logger;
};

}
}
}

#endif
