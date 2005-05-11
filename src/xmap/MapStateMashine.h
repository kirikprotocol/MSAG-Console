#ifndef __MAP_STATE_MASHINE_H__GREEN__
#define __MAP_STATE_MASHINE_H__GREEN__

#include "xmap.h"
//#include <map>
//#include <vector>
#include <sms/sms.h>
//#include "MapDialog.h"
//#include "queue.h"
#include "syncque.hpp"

#include "../src/core/threads/Thread.hpp"

#include <core/buffers/FastMTQueue.hpp>
#include <core/synchronization/EventMonitor.hpp>
#include <core/synchronization/Event.hpp>

using namespace smsc::core::threads;
using namespace smsc::core::synchronization;

using namespace smsc::sms;
using namespace smsc::sms::Tag;
//#include "mmqueue.h"
//#include "dinfo.h"


//typedef std::map<USHORT_T,MapDialog*> cDIALOGS;

//typedef std::vector<MapDialog*> cDIALOGS;

class CMapStateMashine: public Thread
{
//protected:
 ///static std::auto_ptr<Manager> manager;

 
public:
 UCHAR_T parseUssdReqResp(MSG_T*message);
 CMapStateMashine(void);
 //CMapStateMashine(cqueue * pinqu,cqueue * poutqu,CMMQueue* pOutGQue,CMMQueue* pIncomQue, int * pflag);
 //CMapStateMashine(SyncQeuue<TEMPLATE_MSG> * pinqu,SyncQeuue<TEMPLATE_MSG> * poutqu,CMMQueue* pOutGQue,CMMQueue* pIncomQue,Event*  ievent,Event * oevent);
 CMapStateMashine(qMessage_t*msg_store);
 ~CMapStateMashine(void);

// int run();
 int Execute();
 int RecievePrimitive();
 int connect();
 void SetDialogContainer(MapDialogContainer* _mdc);

private:
 int ProcessPrimitive(MSG_T * message);
 void sendBindConf(MSG_T* message);

 //int sendOpenDialogInd(USHORT_T dialogid);
 //USHORT_T getMsgDialogId(MSG_T * message);
 
 USHORT_T getDialogId(MSG_T* message);

 // dlgs
 void onOpenRequest(MSG_T* message);
 void onDelimiterRequest(MSG_T* message);//close_ind
 void onOpenResponce(MSG_T* message);
 void onCloseReq(MSG_T* message);//statePosition = DIALOG_FINISH;

 //
 void onSendRinfoRequest(MSG_T* message);
 void onForwardSMSRequest(MSG_T* message);
 void onForwardMOSMSResp(MSG_T* message);
 

 // Ussd primitives //
 void onUssdRequest(MSG_T* message);
 void onUssdResponce(MSG_T* message);
 void onUssdNotifyRequest(MSG_T* message);
 void onUssdNotifyResponce(MSG_T* message);
 void onUssdClosing(MSG_T*message);
 //
 void onUserAbortRequest(MSG_T* message);
 //
 void dumpmsg(const char *fmtstr,MSG_T* message);
 void SendCPMessage(MSG_T * msg);

private:
  
// cqueue * pInputQueue;
// cqueue * pOutQueue;
 int * processFlag; 

 //cDIALOGS pDialogs;

 
 
 //pthread_mutex_t mutex;

 
// SyncQeuue<CMMessage *>* pIncomingMSGQueue;
// SyncQeuue<std::string>* pOutgoingMSGQueue;
// USHORT_T dialogindex;

// SyncQeuue<TEMPLATE_MSG> * pInputQueue2;
// SyncQeuue<TEMPLATE_MSG> * pOutQueue2;
 
// Event * event_in_q ;
// Event * event_out_q;
// Event * event_outgoing_queue;
// Event * event_incoming_queue;
// Event* globa_levent;
 
 qMessage_t* quall;
 bool connected;
 //CMMessage prmsgAt(int index);
// std::vector<cdinfo> dinfos;
 //std::list<CMMessage> processed_messages;
 
 MapDialogContainer* mdc;

 EventMonitor mtx;
};

#endif
