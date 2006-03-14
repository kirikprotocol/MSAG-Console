#ifndef _SACC_EVENT_SERVER_H_
#define _SACC_EVENT_SERVER_H_
/*****************************************************************************
 *****************************************************************************
 *																			 
 * Module Name:																  
 * ============
 *
 *			SACC_EventServer.h
 *
 * Purpose:			
 * ========
 *
 *                      Header File for SACC_EventServer.cpp
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
#include "SACC_EventSender.h"

namespace scag{
namespace stat{
namespace sacc{



class EventServer : public Thread 
{

public:
	void pushEvent(void* ev);
    EventServer();
    virtual ~EventServer();
    virtual int Execute();
    void init(EventServer * eServer);
    void InitServer(std::string esHost, int esPort);
    void reinitLogger();
    void Stop();
    void Start();

protected:
    Logger *  logger;
    std::string  Host;
    int  Port;
    Socket evSocket;
    EventServer* eventServer;
    bool  bStarted;
    Multiplexer  listener;

	SyncQeuue eQueue;
	EventSender * eSender;
	 
};

  }//sacc namespace
 }//stat namespace 
}//scag namesoace

#endif