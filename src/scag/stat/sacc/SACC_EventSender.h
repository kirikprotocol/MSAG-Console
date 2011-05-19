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
 *   (format described on "WHIOSD ����������⢨� ���㫥�.doc" )
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

//#include "SACC_SyncQueue.h"
#include <logger/Logger.h>
//#include "SACC_Events.h"
#include "core/threads/Thread.hpp"
#include "core/network/Multiplexer.hpp"
#include "core/network/Socket.hpp"
#include "logger/Logger.h"
#include "scag/stat/Statistics.h"
//#include "SACC_Events.h"
#include <core/synchronization/EventMonitor.hpp>
#include <util/BufferSerialization.hpp>

#include <core/buffers/FastMTQueue.hpp>
#include <core/synchronization/EventMonitor.hpp>
#include <core/synchronization/Event.hpp>
#include <core/buffers/CyclicQueue.hpp>


#include <string>

namespace scag{ namespace stat{ namespace sacc{

using namespace smsc::core::threads;
using smsc::core::network::Socket;
using smsc::core::network::Multiplexer;
using namespace smsc::logger;
using smsc::util::Exception;
using smsc::util::SerializationBuffer;
using namespace scag::stat;
using namespace smsc::core::synchronization;
using namespace smsc::core::buffers;

class EventSender: public Thread
{
private:
    typedef CyclicQueue< SaccEvent* > QueueType;


public:
    bool isActive() { return bConnected; };

    EventSender();
    virtual ~EventSender();
    int Execute();
    void PushEvent(SaccEvent* item);

    void Start();
    void Stop();
    void init(std::string& host,int port,int timeout,int queuelen/*,bool * bf,*/,smsc::logger::Logger * lg);

private:
    bool checkQueue();
    bool connect(std::string host,int port,int timeout);
    bool processEvent(SaccEvent* ev);
    void sendPing();
    smsc::logger::Logger * logger;

    // to make compiler happy
    void Start(int);
    QueueType* switchQueue(); // and return freed queue

private:
    time_t lastOverflowNotify, lastConnectTry;
    EventMonitor mtx;
    QueueType eventsQueue[2];
    unsigned queueIdx; // 0,1
    bool bStarted;
    bool bConnected;
    int  Timeout;
    Socket SaccSocket;
    std::string Host;
    int QueueLength;
    int Port;
    SaccSerialBuffer pdubuffer;
};

}}}

#endif
