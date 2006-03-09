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

namespace scag{
namespace stat{
namespace sacc{

class EventSender: public Thread 
{
public:

	EventSender();
	EventSender(SyncQeuue * q,bool * bf);
	virtual ~EventSender();
	int Execute();
	bool checkQueue();
	bool processEvent(void * ev);

private:

	SyncQeuue * pQueue;
	bool * bStop;
	Socket SaccSocket;
	bool connect(string host,int port);
};

  }//sacc namespace
 }//stat namespace 
}//scag namesoace

#endif