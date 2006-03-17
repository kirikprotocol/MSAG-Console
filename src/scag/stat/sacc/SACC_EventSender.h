#ifndef _SACC_EVENT_SENDER_H_
#define _SACC_EVENT_SENDER_H_
/*****************************************************************************
 *****************************************************************************
 *																			 
 * Module Name:																  
 * ============
 *
 *			SACC_EventSender.h
 *
 * Purpose:			
 * ========
 *
 *                      Header File for SACC_EventSender.cpp
 * 			Perform and send messages to WHOISD SACC part 
 *			(format described on "WHIOSD взаимодействие модулей.doc" )
 *			
 * Author(s) & Creation Date:	
 * ==========================
 *
 *			Gregory Panin, 2006/03/03
 *  Copyright (c): 
 *  =============
 * 			EyeLine Communications
 * 			All rights reserved.
 */

#include "SACC_SyncQueue.h"
#include <logger/Logger.h>
#include "SACC_Events.h"
#include "core/threads/Thread.hpp"
#include "core/network/Multiplexer.hpp"
#include "core/network/Socket.hpp"
#include "logger/Logger.h"
#include <scag/stat/Statistics.h>

#include <string>
using namespace smsc::core::threads;
using smsc::core::network::Socket;
using smsc::core::network::Multiplexer;
using namespace smsc::logger;
using smsc::util::Exception;
using namespace scag::stat;
using namespace scag::stat::Counters;

namespace scag{
namespace stat{
namespace sacc{


class EventSender: public Thread 
{
public:
	bool isActive();

	EventSender();
	virtual ~EventSender();
	int Execute();
	void Put(SaccStatistics& ev);

	void Start();
	void init(std::string& host,int port,int timeout,bool * bf,smsc::logger::Logger * lg);
private:

	SyncQueue<SaccStatistics *> eventsQueue;
	bool * bStarted;
	bool bConnected;
	Socket SaccSocket;
	std::string Host;
	int Port;
	bool checkQueue();
	bool connect(std::string host,int port,int timeout);
	bool processEvent(SaccStatistics * ev);
	void makeTransportEvent(SaccStatistics * st,SACC_TRAFFIC_INFO_EVENT_t * ev);
	smsc::logger::Logger * logger;
};

  }//sacc namespace
 }//stat namespace 
}//scag namesoace

#endif