#ifndef __SYNCQUEUE_HPP__
#define __SYNCQUEUE_HPP__

 
#include "../src/core/buffers/Hash.hpp"
#include "../src/core/buffers/IntHash.hpp"

#include <core/buffers/FastMTQueue.hpp>
#include <core/synchronization/EventMonitor.hpp>
#include <core/synchronization/Event.hpp>
#include <logger/Logger.h>
#include "xmap.h"
#include "common.h"


using namespace smsc::core::buffers;
using namespace smsc::logger;
using namespace std;
extern smsc::logger::Logger *logger;

//using namespace smsc::core::threads;
using namespace smsc::core::synchronization;
//#include "message.h"
#include "config.h"

#define PL_FLAG_RX_FLAG 0
#define PL_FLAG_TX_FLAG 1
#define PL_FLAG_ERR_FLAG 3

#define PIPE_ACTIVE  1
#define PIPE_IS_ABSENT  2
#define HAS_NO_PIPE 0

template <class T>
class SyncQeuue{

public:
 void Push(const T& item)
 {

  MutexGuard g(mtx);
  q.Push(item);
  mtx.notify();

 };
 bool Pop(T& item,int timeout)
 {
  MutexGuard g(mtx);
  int ret=0;
  while(!q.Pop(item))
  {
   ret = mtx.wait(timeout);
   if(ret!=0)
    return false;
  }
  
  return true;
  
 };
 void Wait()
 {
  MutexGuard g(mtx);
  mtx.wait();
 };
 int Count()
 {
  MutexGuard g(mtx);
  return q.Count();
 };
protected:
 CyclicQueue<T> q;
 EventMonitor mtx;
 
};

#define PF_NOTHING 0
#define PF_SMS_SEND_CONFIRM 1
#define PF_SMS_SEND_ERROR 2
#define PF_SMS_RECIEVED 3
#define PF_SMS_SEND 4


typedef struct PipeInfo{
 char * imsi;
 char * text;
 char * msisdn;
 char * destaddr;
 char send_flag;
 char recv_flag;
 char error_flag;
 int val;
 USHORT_T dialogid;
 bool absent;
 int  probability;

};

class SyncPipesList
{
public:
 SyncPipesList(){};
 ~SyncPipesList(){};

 void _setflag(PipeInfo*pi,char  flag,int flagid)
 {
   
   switch(flagid) 
   {
   case PL_FLAG_RX_FLAG:
    pi->recv_flag = flag;
     
    break;
   case PL_FLAG_TX_FLAG:
    pi->send_flag = flag;
     
    break;

   case PL_FLAG_ERR_FLAG:
    pi->error_flag = flag;
     
    break;

   default:
    break;
   }
   
 };
 PipeInfo * getByImsi(std::string imsi)
 {
 
   
 /*
  int sz = pipes.size();

  for(int i=0;i<sz;i++)
  {
   if(pipes[i])
   {
    if(imsi.compare(pipes[i]->imsi)==0)
    {
     return pipes[i];
    }
   }
    
  }*/

  if(pipes_imsi.GetPtr((pchashstr)imsi.c_str()))
   return pipes_imsi.Get((pchashstr)imsi.c_str());
  else
   return 0;
 };
 PipeInfo * getByMsiSdn(std::string msisdn)
 {
   
 
  /*int sz = pipes.size();

  for(int i=0;i<sz;i++)
  {
   if(pipes[i])
   {
    if(msisdn.compare(pipes[i]->msisdn)==0)
    {
     return pipes[i];
    }
   }

  }
  return 0;
  */
  if(pipes_msisdn.GetPtr((pchashstr)msisdn.c_str()))
   return  pipes_msisdn.Get((pchashstr)msisdn.c_str());
  else
   return 0;

 };
 PipeInfo * getByDialogId(USHORT_T dlgid)
 {

  if(pipes_dialogid.GetPtr((int)dlgid))
   return pipes_dialogid.Get((int)dlgid);
  else
   return 0;

 };
  
 void InserPipeInfo(PipeInfo * pPi)
 {
  MutexGuard g(mtx);
  pipes_imsi.Insert((pchashstr)pPi->imsi ,pPi);
  pipes_msisdn.Insert((pchashstr)pPi->msisdn ,pPi);
 // xmap_trace(logger,"%s register MT in HLR msisdn %s, imsi %s",__func__,pPi->msisdn,pPi->imsi);
 };

 long getFlag(std::string imsi,int flagid)
 {
  MutexGuard g(mtx);
  PipeInfo * pi = getByImsi(imsi);
  
  if(pi)
  {
  
   switch(flagid) 
   {
   case PL_FLAG_RX_FLAG:
     return (long)pi->recv_flag;
    break;
   case PL_FLAG_TX_FLAG:
    return (long)pi->send_flag;
    break;

   case PL_FLAG_ERR_FLAG:
    return (long)pi->error_flag;
    break;

   default:
     break;
   };
  }
 
  return -1;
 };

 std::string getText(std::string imsi)
 {
  MutexGuard g(mtx);

  PipeInfo * pi = getByImsi(imsi);
  std::string str="";
  if(pi)
  {
   if(pi->text)
    str = pi->text;
  }

  return str;

 };

 void setDestAddr(std::string imsi,std::string dstaddr)
 {
  MutexGuard g(mtx);
  PipeInfo * pi = getByImsi(imsi);
  if(pi)
  {
   pi->destaddr = new char[dstaddr.length()+1];
   memcpy(pi->destaddr,dstaddr.c_str(),dstaddr.length()); 
  }

 }
 bool setText(std::string imsi,std::string text,char flag,int flagid)
 {
  MutexGuard g(mtx);
  PipeInfo * pi = getByImsi(imsi);
  
  if(pi)
  {
   //if (pi->text)
   // delete []pi->text;
   
   pi->text = new char[text.length()+1];
   memcpy(pi->text,text.c_str(),text.length()); 

   pi->text[text.length()]=0;

   _setflag(pi,flag,flagid);
     
   return true;

  }
  return false;
  
 };
 bool setAbsent(std::string imsi,bool _absent)
 {
  MutexGuard g(mtx);
  PipeInfo * pi = getByImsi(imsi);
  
  if(pi)
  {
   pi->absent = _absent;  
   return true;

  }
  return false;
  
 };
 bool randomevent(int prob)
 {
   int i=100*((double)rand()/(double)RAND_MAX);
   return (i>=(100-prob));

 };

 int hasPipe(const char *msisdn)
 {
  if((pipes_msisdn.GetPtr((pchashstr)msisdn)!=0))
  {

   PipeInfo *pi = pipes_msisdn.Get((pchashstr)msisdn);

  // xmap_trace(logger,"! prob=%d absent =%d!",pi->probability,pi->absent?1:0);

   if(pi->probability<100)
   {

    if(randomevent(pi->probability))
     return PIPE_ACTIVE;
    else 
     return PIPE_IS_ABSENT;
   }
   
   if(pi->absent)  
    return PIPE_IS_ABSENT;
   
  /* if(pi->absent)  
    return PIPE_ACTIVE;*/

   return PIPE_ACTIVE;

  }
  return HAS_NO_PIPE;
 };
 bool setFlag(std::string imsi,char flag,int flagid)
 {
  MutexGuard g(mtx);
  PipeInfo * pi = getByImsi(imsi);
  
  if(pi)
  {
   _setflag(pi,flag,flagid);
   return true;
  }

  return false;

 };
 std::string getImsiByMsisdn(std::string msisdn)
 {
  MutexGuard g(mtx);
  PipeInfo * pi = getByMsiSdn(msisdn);
  if(pi)
  {
   std::string str = pi->imsi;

   return str;
  }
  return (std::string)"";
 };
 bool setDialogIdByMsiSdn(std::string msisdn,USHORT_T dlgid)
 {
  MutexGuard g(mtx);
  PipeInfo * pi = getByMsiSdn(msisdn);
  if(pi)
  {
   pi->dialogid = dlgid;
   pipes_dialogid.Insert(dlgid,pi);
   
   return true;
  }
  return false;
 };

 std::string getImsiByDialogId(USHORT_T dialogid)
 {
  MutexGuard g(mtx);
  PipeInfo * pi = getByDialogId(dialogid);
  if(pi)
  {
   std::string str = pi->imsi;

   return str;
  }
  else
  {
   //xmap_trace(logger,"error *pi in null for dlg <%d> HLR size %d ",dialogid,pipes.size());
   
  }
  return (std::string)"";
 }

 

protected:

 /* two hashs for msisdn and imsi for one ptrs*/
 Hash<PipeInfo*> pipes_imsi;
 Hash<PipeInfo*> pipes_msisdn;
 IntHash<PipeInfo*> pipes_dialogid; 

 EventMonitor mtx;
};



//class MapDialog;

//typedef MapDialog* MapDialogPtr;

typedef struct qMessage_t
{
 Event event_in_queue;
 Event event_out_queue;
 Event event_outgoing_queue;
 Event event_incoming_queue;
 Event event_pipe_registered;

 Event event_rx;
 Event event_tx;

 Event command_checker_event;
 Event call_processor_event;
 Event map_statemachine_event;

 SyncQeuue<MSG_T*> CQin;
 SyncQeuue<MSG_T*> CQout;

// SyncQeuue<CMMessage* > qIncoming;
 //SyncQeuue<CMMessage* > qQoutgoing;

 SyncQeuue<std::string> qQoutgoing;

 //std::vector<PipeInfo*> vPipesInform;
 SyncPipesList PipesList;

 ConfigXml * config;
 int  bindOK;
 Event ebent_bind;

// MapDialogPtr *pDialogs;
};


typedef struct sMapDialog
{
 char * imsi;
 char * msisdn;
 UCHAR_T state;
 USHORT_T dialogid;
};

class MapDialogContainer
{
 public:
  MapDialogContainer()
  {
   dialogs.SetSize(0xffff+1);
   counter=0xffff;
  
  };
  ~MapDialogContainer(){dialogs.Empty();};

  USHORT_T searchFreeDialog()
  {
   for(USHORT_T i=0xffff/2;i<(0xffff+1);i++)
   {
    if(!dialogs.GetPtr(i))
     return i;
   }
   return 0xffff;
  }

  USHORT_T addDialog(std::string imsi,std::string msisdn)
  {
   if (counter<=0xffff/2)
   {
    smsc_log_info(logger,"dialogs container owerflow");
    return 0;
   }
   xmap_trace(logger,"%s this==0x%x imsi '%s' msisidn '%s'",__func__,this,imsi.c_str(),msisdn.c_str());
   MutexGuard g(mtx);
   sMapDialog * smd = (sMapDialog*)malloc(sizeof(sMapDialog));
   smd->state=0;
   
   smd->dialogid=searchFreeDialog();
   
   smd->imsi = new char[imsi.length()];
   smd->msisdn = new char[msisdn.length()];
   
   memcpy(smd->imsi,imsi.c_str(),imsi.length());
   memcpy(smd->msisdn,msisdn.c_str(),msisdn.length());

   smd->imsi[imsi.length()]=0; 
   smd->msisdn[msisdn.length()]=0;  

   _delete_if_exist_dlg(smd->dialogid);

   dialogs.Insert(smd->dialogid,smd);

   counter--;//from map

   xmap_trace(logger,"%s end",__func__);

   return smd->dialogid;
  };

  void _delete_if_exist_dlg(USHORT_T id)
  {
   if(dialogs.GetPtr(id))
   {
    delete dialogs.Get(id)->imsi;
    delete dialogs.Get(id)->msisdn;
    delete (sMapDialog*)(dialogs.Get(id));
    dialogs.Delete(id);
   }

  };
  void inSertAt(std::string imsi,std::string msisdn,USHORT_T id)
  {
   MutexGuard g(mtx);
   sMapDialog * smd = (sMapDialog*)malloc(sizeof(sMapDialog));
   smd->state=0;
   smd->dialogid=id;

   smd->imsi = new char[imsi.length()];
   smd->msisdn = new char[msisdn.length()];
   
   memcpy(smd->imsi,imsi.c_str(),imsi.length());
   memcpy(smd->msisdn,msisdn.c_str(),msisdn.length());

   smd->imsi[imsi.length()]=0; 
   smd->msisdn[msisdn.length()]=0;  
   
   _delete_if_exist_dlg(id);
   
   dialogs.Insert(id,smd);
   xmap_trace(logger,"%s end",__func__);
  };

  // leak tracing
  void deleteDialog(USHORT_T id)
  {
   MutexGuard g(mtx);
   
   _delete_if_exist_dlg(id);

   if (id>0xffff/2)
    counter++;
   xmap_trace(logger,"%s end",__func__);
  };

  void setState(USHORT_T id,UCHAR_T state)
  {
   MutexGuard g(mtx);
   
   if(dialogs.GetPtr(id))
   {
    dialogs.Get(id)->state = state;

    xmap_trace(logger,"%s end",__func__);
   }
   else
    smsc_log_info(logger,"error in %s try to set state for  empty dialog<%d> or bussed by JSPIPE",__func__,id);

   
  };

  UCHAR_T getState(USHORT_T id)
  {
   MutexGuard g(mtx);
   if(dialogs.GetPtr(id))
    return dialogs.Get(id)->state;
   else
    return EMPTY_DIALOG_HASH;

  };

  const char * getImsi(USHORT_T id)
  {
   MutexGuard g(mtx);
   if(dialogs.GetPtr(id))
    return dialogs.Get(id)->imsi;
   else

   return 0;

  };

  const char * getMsisdn(USHORT_T id)
  {
   MutexGuard g(mtx);
   if(dialogs.GetPtr(id))
    return dialogs.Get(id)->msisdn;
   else
   return 0;

  };

 protected:
  IntHash<sMapDialog*> dialogs;
  EventMonitor mtx;
  USHORT_T counter;
};


struct MAPMSGPAir{
UCHAR_T*msg_p;
USHORT_T size;
};
class SS7MapMsg{
public:
 SS7MapMsg(){};
 ~SS7MapMsg(){nodes.clear();};

public:
 std::vector<MAPMSGPAir>nodes;
 EventMonitor mtx;

 USHORT_T getallsize()
 {
  USHORT_T res=0;
  for(int i=0;i<nodes.size();i++)
   res+=nodes[i].size;
  ////xmap_trace(logger," get all size = %d",res);
  return res;
 };

 void insertPtr(UCHAR_T* ptr,USHORT_T size)
 {
  MutexGuard g(mtx);

  MAPMSGPAir mPair;
                
  mPair.size = size;
  mPair.msg_p= new UCHAR_T[size];//(UCHAR_T*)malloc(size);

  memcpy(mPair.msg_p,ptr,size);

  nodes.push_back(mPair);
 };
 void insertUChar(UCHAR_T ch)
 {
  MutexGuard g(mtx);
  MAPMSGPAir mPair;

  mPair.size = 1;
  mPair.msg_p= new UCHAR_T[1];//(UCHAR_T*)malloc(1);
  memcpy(mPair.msg_p,&ch,1);//=& ???
  nodes.push_back(mPair);
 };
 void insertUShort(USHORT_T us)
 {
  MutexGuard g(mtx);
  MAPMSGPAir mPair;

  mPair.size = sizeof(USHORT_T);
  mPair.msg_p= new UCHAR_T[sizeof(USHORT_T)];//(UCHAR_T*)malloc(sizeof(USHORT_T));
  
  mPair.msg_p[0]=(UCHAR_T)(us & 0x00ff);//lo byte
  mPair.msg_p[1]=(UCHAR_T)((us & 0xff00)>>8);//hi byte

  nodes.push_back(mPair);
 };

 void getAll(UCHAR_T *ptr)
 {
  MutexGuard g(mtx);
  if(ptr!=0)
  {
   for(int i=0;i<nodes.size();i++)
   {
    memcpy(ptr,nodes[i].msg_p,nodes[i].size);
    ptr+=nodes[i].size;
   }
  }
 };

 void free()
 {
  MutexGuard g(mtx);
  for(int i=0;i<nodes.size();i++)
  {
   if(nodes[i].msg_p)
    delete nodes[i].msg_p;
  }
  nodes.empty();
 };

};
#endif 