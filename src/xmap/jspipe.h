/**
 * JSCustomer.h - Example for my tutorial : Scripting C++ with JavaScript
 * (c) 2002 - Franky Braem
 * http://www.braem17.yucom.be
 */

#ifndef _JSPIPE_H
#define _JSPIPE_H


#include "pipeobj.h"
#include "xmap.h"
//#include "MapDialog.h"
 
#include <core/buffers/FastMTQueue.hpp>
#include <core/synchronization/EventMonitor.hpp>
#include <core/synchronization/Event.hpp>
//#include "sme/SmppBase.hpp"
#include <sms/sms.h>

#include <util/recoder/recode_dll.h>
#include <logger/Logger.h>

using namespace smsc::logger;
using namespace smsc::core::synchronization;
using namespace smsc::sms;
using namespace smsc::sms::Tag;
//using namespace smsc::smpp;
//using namespace smsc::sme;

extern smsc::logger::Logger *logger;

class JSPipe
{
public:
 /**
  * Constructor
  */
 JSPipe() : m_pPipe(NULL)
 {
 }
 
 /**
  * Destructor
  */
 virtual ~JSPipe()
 {
  delete m_pPipe;
  m_pPipe = NULL;
 }

 /**
  * JSGetProperty - Callback for retrieving properties
  */
 static JSBool JSGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);

 /**
  * JSSetProperty - Callback for setting properties
  */
 static JSBool JSSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);

 /**
  * JSConstructor - Callback for when a wxCustomer object is created
  */
 static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
 
 /**
  * JSDestructor - Callback for when a wxCustomer object is destroyed
  */
 static void JSDestructor(JSContext *cx, JSObject *obj);

 /**
  * JSInit - Create a prototype for wxCustomer
  */
 static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL);

    static JSBool sendSms(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
 static JSBool Register(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
 static JSBool isSmsDone(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
 static JSBool isRecieved(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
 static JSBool generateImsi(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
 static JSBool generateMsisdn(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

 static JSBool openUssdSession(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
 static JSBool sendUssdMessage(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
 static JSBool isUssdRecieved(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
 static JSBool isUssdSessionClosed(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);


 static JSClass Pipe_class;

    void setPipe(Pipe *pipe)
 {
  m_pPipe  = pipe; 
 };
    
 Pipe* getPipe() 
 {
  return m_pPipe; 
 };

 void sendOpenDialogInd(USHORT_T dlgid,bool isUSSD,ET96MAP_APP_CNTX_T cntx,std::string orastr,std::string dstastr);
 ET96MAP_SM_RP_UI_T*  mkSubmitPDU(Address destaddr,const char * text,ET96MAP_SM_RP_UI_T* pdu,bool mms);
 void SendCPMessage(MSG_T * msg);
 void forwardSMS(USHORT_T dlgid,std::string str_oa,std::string str_da,std::string str_text,std::string str_msc_addr);
 void sendDelimiterInd(USHORT_T dlgid,UCHAR_T subsistem_n);
 
 void send_PSSR_or_USSR_UssdRequestInd(UCHAR_T subsistem_number, bool isPSSR,USHORT_T dlgid,std::string str_da,std::string str_text);

/*
 PipeInfo * getPiByImsi()
 {
  std::string s_imsi = m_pPipe->GetImsi();
  int sz = qall->vPipesInform.size();
  for(int i=0;i<sz;i++)
  {
   if(qall->vPipesInform[i]->imsi)
   {
    if(s_imsi.compare(qall->vPipesInform[i]->imsi)==0)
    {
     return qall->vPipesInform[i];
    }

   }
  }

  return 0;
 };
 PipeInfo * getPiByMsisdn()
 {
  
  std::string s_msisdn = m_pPipe->GetMsisdn();
  int sz = qall->vPipesInform.size();
 

  for(int i=0;i<sz;i++)
  {
   if(qall->vPipesInform[i]->msisdn)
   {
    
    if(s_msisdn.compare(qall->vPipesInform[i]->msisdn)==0)
    {
     return qall->vPipesInform[i];
    }
   }
  }

  return 0;
 };
 */
 qMessage_t * getQall()
 {
  return qall;

 };
 void setQall(qMessage_t *qmsgt)
 {
  qall=qmsgt;
 };

 void setDialogContainer(MapDialogContainer * _mdc)
 {
  mdc= _mdc;

 };

 MapDialogContainer * getDialogConatiner()
 {
  return mdc;
 };

  /*void insertPipeInfo(PipeInfo* pnf)
 {
  qall->vPipesInform.push_back(pnf);
 }*/
public:
  USHORT_T m_dialogid;
protected:

private:

 Pipe *m_pPipe;
 qMessage_t* qall;

 MapDialogContainer * mdc;

 EventMonitor mtx;

    static JSPropertySpec Pipe_properties[];
 static JSFunctionSpec Pipe_methods[];

 enum
 {
  msisdn_prop,
  imsi_prop,
  text_prop,
  absent_prop,
  probability_prop,
  error_info_prop
 };
};

#endif  
