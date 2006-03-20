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
 *			(format described on "WHIOSD ����������⢨� ���㫥�.doc" )
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
#include "SACC_Events.h"

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
	void Put(const SACC_TRAFFIC_INFO_EVENT_t& ev);
	void Put(const SACC_BILLING_INFO_EVENT_t& ev);
	void Put(const SACC_ALARM_MESSAGE_t & ev);
	void Put(const SACC_SESSION_EXPIRATION_TIME_ALARM_t& ev);
	void Put(const SACC_OPERATOR_NOT_FOUND_ALARM_t& ev);

	void Start();
	void init(std::string& host,int port,int timeout,bool * bf,smsc::logger::Logger * lg);
private:

	SyncQueue<void *> eventsQueue;
	bool * bStarted;
	bool bConnected;
	Socket SaccSocket;
	std::string Host;
	int Port;
	bool checkQueue();
	bool retrieveConnect();
	bool connect(std::string host,int port,int timeout);
	bool processEvent(void * ev);
	smsc::logger::Logger * logger;
};

  }//sacc namespace
 }//stat namespace 
}//scag namesoace

#endif