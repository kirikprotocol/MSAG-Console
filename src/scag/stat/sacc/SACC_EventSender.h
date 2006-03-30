#ifndef _SACC_EVENT_SENDER_H_
#define _SACC_EVENT_SENDER_H_
/*****************************************************************************
 *****************************************************************************
 *                    
 * Module Name:                  
 * ============
 *
 *   SACC_EventSender.h
 *
 * Purpose:   
 * ========
 *
 *                      Header File for SACC_EventSender.cpp
 *    Perform and send messages to WHOISD SACC part 
 *   (format described on "WHIOSD взаимодействие модулей.doc" )
 *   
 * Author(s) & Creation Date: 
 * ==========================
 *
 *   Gregory Panin, 2006/03/03
 *  Copyright (c): 
 *  =============
 *    EyeLine Communications
 *    All rights reserved.
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
#include <core/synchronization/EventMonitor.hpp>

#include <string>
using namespace smsc::core::threads;
using smsc::core::network::Socket;
using smsc::core::network::Multiplexer;
using namespace smsc::logger;
using smsc::util::Exception;
using namespace scag::stat;
using namespace scag::stat::Counters;
using namespace smsc::core::synchronization;

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
// void Put(const SACC_SESSION_EXPIRATION_TIME_ALARM_t& ev);
// void Put(const SACC_OPERATOR_NOT_FOUND_ALARM_t& ev);
 void Put(const SACC_ALARM_t & ev);

 void Start();
 void Stop();
 void init(std::string& host,int port,int timeout,int queuelen,bool * bf,smsc::logger::Logger * lg);
private:

 //SyncQueue<void *> eventsQueue;
 EventMonitor mtx;
 CyclicQueue<void *> eventsQueue;
 bool * bStarted;
 bool bConnected;
 int Timeout;
 Socket SaccSocket;
 std::string Host;
 int QueueLength;
 int Port;
 
 EventMonitor evReconnect;
 EventMonitor evQueue;

 bool checkQueue();
 bool PushEvent(void* item);
 bool connect(std::string host,int port,int timeout);
 bool processEvent(void * ev);
 
 void performTransportEvent(const SACC_TRAFFIC_INFO_EVENT_t& e);
 void performBillingEvent(const SACC_BILLING_INFO_EVENT_t& e);
 void performAlarmMessageEvent(const SACC_ALARM_MESSAGE_t& e);
 void performAlarmEvent(const SACC_ALARM_t& e);
// void performSessionExpiredEvent(const SACC_SESSION_EXPIRATION_TIME_ALARM_t& e);
// void performOperatorNotFoundEvent(const SACC_OPERATOR_NOT_FOUND_ALARM_t& e);

 //void makeAlarmEvent(uint32_t evtype);
 smsc::logger::Logger * logger;
};

  }//sacc namespace
 }//stat namespace 
}//scag namesoace

#endif
