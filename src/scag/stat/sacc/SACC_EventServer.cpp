#include "SACC_Defs.h"
#include "SACC_EventServer.h"

namespace scag{
namespace stat{
namespace sacc{

using smsc::util::Exception;
using smsc::logger::Logger;



EventServer::EventServer():logger(Logger::getInstance(EV_SERVER_LOGGER_CATHEGORY_NAME)), Host(""), Port(0), bStarted(false), eSender(0)
{
}

EventServer::~EventServer()
{
}

int EventServer::Execute()
{
    smsc::core::network::Multiplexer::SockArray ready, err;

    smsc_log_debug(logger,"EventServer: start executing...\n");

    if(!bStarted) return 1;

    if( Socket.StartServer())
        smsc_log_warn(logger, "EventServer:Failed to start socket ");

    while(bStarted)
    {
        if(listener.canRead(ready, err))
		{
         
			smsc_log_info(logger, "EventServer:There are ready sockets");

			for(int i=0; i<= ready.Count() - 1; i++)
			{
					if(ready[i] == &Socket)
					{
						smsc_log_info(logger, "EventServer:Socket ready");
						sockaddr_in addrin;
						int sz=sizeof(addrin);

						SOCKET s=accept(genSocket.getSocket(),(sockaddr*)&addrin,&sz);
						if(s != -1)
						{
							Socket * sock = new Socket(s,addrin);
							performanceServer->addGenSocket(sock);
						}
					}
				}
			}
    }

    smsc_log_debug("EventServer:Execution exit\n");

    return 1;

}

void EventServer::init(EventServer * eServer)
{
}

void EventServer::InitServer(std::string esHost, int esPort)
{
}

void EventServer::reinitLogger()
{
	    logger = Logger::getInstance(LOGGER_CATHEGORY_NAME); 
}

void EventServer::Stop()
{
	bStarted=false;
}

void EventServer::Start()
{
	bStarted=true;
	Thread::Start();
}

// note: delete events into sender thread, after sending

void EventServer::pushEvent(void *ev)
{
	uint16_t EventType;
	memcpy(&EventType,ev,2);

	switch(EventType)
	{
		case SACC_SEND_TRANSPORT_EVENT:
			{
				SACC_TRAFFIC_INFO_EVENT_t* ev = new SACC_TRAFFIC_INFO_EVENT_t;
				eQueue.Push((void*)ev);
			}
			break;

		case SACC_SEND_BILL_EVENT:
			{
				SACC_BILLING_INFO_EVENT_t* ev = new SACC_BILLING_INFO_EVENT_t;
				eQueue.Push((void*)ev);
			}
			break;

		case SACC_SEND_ALARM_EVENT:
			break;

		case SACC_SEND_OPERATOR_NOT_FOUND_ALARM:
			{
				SACC_OPERATOR_NOT_FOUND_ALARM_t* ev = new SACC_OPERATOR_NOT_FOUND_ALARM_t;
				eQueue.Push((void*)ev);
			}
			break;

		case SACC_SEND_SESSION_EXPIRATION_TIME_ALARM:
			{
				SACC_SESSION_EXPIRATION_TIME_ALARM_t * ev = new SACC_SESSION_EXPIRATION_TIME_ALARM_t;
				eQueue.Push((void*)ev);
			}
			break;

		case SACC_SEND_ALARM_MESSAGE:
			{
				SACC_ALARM_MESSAGE_t* ev = new SACC_ALARM_MESSAGE_t;
				eQueue.Push((void*)ev);
			}
			break;
			
		default:
			break;
	}
}


}
}
}
