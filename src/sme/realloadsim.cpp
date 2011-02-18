#include <stdio.h>
#include "sme/SmppBase.hpp"
#include "sms/sms.h"
#include "util/smstext.h"
#include "core/buffers/Hash.hpp"
#include "core/buffers/File.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "logger/Logger.h"
#include <vector>
#include "util/sleep.h"
#include <signal.h>
#include "readline/readline.h"
#include "readline/history.h"
#include <map>

using namespace smsc::sms;
using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;
using namespace smsc::core::buffers;
using namespace smsc::core::synchronization;
using namespace smsc::logger;

bool running=true;
bool connected=false;

string& trimspaces(string& s)
{
  int i=0;
  while(i<s.length() && isspace(s[i]))i++;
  if(i>0)s.erase(0,i-1);
  i=(int)s.length()-1;
  while(i>=0 && isspace(s[i]))i--;
  s.erase(i+1);
  return s;
}

struct Config{
  enum ValueType{
    vtInt,vtString,vtBool
  };
  struct Value{
    Value():vt(vtInt),iValue(0)
    {
    }
    explicit Value(int i):vt(vtInt),iValue(i)
    {
    }
    explicit Value(bool b):vt(vtBool),bValue(b)
    {
    }
    explicit Value(const char* str):vt(vtString),sValue(CopyString(str))
    {
    }
    Value(const Value& rhs):vt(rhs.vt)
    {
      switch(vt)
      {
        case vtInt:iValue=rhs.iValue;break;
        case vtBool:bValue=rhs.bValue;break;
        case vtString:sValue=CopyString(rhs.sValue);break;
      }
    }
    ~Value()
    {
      if(vt==vtString)delete [] sValue;
    }
    static char* CopyString(const char* str)
    {
      int l=(int)strlen(str);
      char *rv=new char[l+1];
      memcpy(rv,str,l);
      rv[l]=0;
      return rv;
    }
    ValueType vt;
    union{
      int iValue;
      bool bValue;
      char* sValue;
    };
  };
  typedef std::multimap<std::string,Value> CfgMap;
  CfgMap cfg;
  void Read(const char* fn)
  {
    File f;
    f.ROpen(fn);
    string s;

    string section;

    while(f.ReadLine(s))
    {
      if(s.length()==0 || s[0]==';' || s[0]=='#')continue;
      if(s[0]=='[')
      {
        if(s[s.length()-1]!=']')
        {
          throw Exception("Invalid config line:%s",s.c_str());
        }
        section=s.substr(1,s.length()-2);
        section+='.';
        continue;
      }
      string::size_type eq=s.find('=');
      if(eq==string::npos)
      {
        throw Exception("Invalid config line:%s",s.c_str());
      }
      string n=s.substr(0,eq);
      string v=s.substr(eq+1);
      trimspaces(n);
      n=section+n;
      trimspaces(v);
      printf("'%s'='%s'\n",n.c_str(),v.c_str());
      if(v=="true" || v=="false")
      {
        cfg.insert(CfgMap::value_type(n,Value(v=="true")));
      }else
      {
        bool isDigit=true;
        for(int i=0;i<v.length() && isDigit;i++)
        {
          isDigit=isdigit(v[i]);
        }
        if(isDigit)
        {
          cfg.insert(CfgMap::value_type(n,Value(atoi(v.c_str()))));
        }else
        {
          cfg.insert(CfgMap::value_type(n,Value(v.c_str())));
        }
      }
    }
  }

  int getInt(const char* k)
  {
    CfgMap::iterator i=cfg.find(k);
    if(i==cfg.end())
    {
      throw Exception("Config parameter '%s' not found",k);
    }
    if(i->second.vt!=vtInt)
    {
      throw Exception("Config parameter '%s' is not int",k);
    }
    return i->second.iValue;
  }
  bool getBool(const char* k)
  {
    CfgMap::iterator i=cfg.find(k);
    if(i==cfg.end())
    {
      throw Exception("Config parameter '%s' not found",k);
    }
    if(i->second.vt!=vtBool)
    {
      throw Exception("Config parameter '%s' is not bool",k);
    }
    return i->second.bValue;
  }
  const char* getStr(const char* k)
  {
    CfgMap::iterator i=cfg.find(k);
    if(i==cfg.end())
    {
      throw Exception("Config parameter '%s' not found",k);
    }
    if(i->second.vt!=vtString)
    {
      throw Exception("Config parameter '%s' is not string",k);
    }
    return i->second.sValue;
  }
};

Config cfg;

enum EventType{
  etDeliverySm,
  etDeliverySmResp,
  etDataSm,
  etDataSmResp,
  etSubmitSm,
  etSubmitResp,
  etTask
};

struct SubmitData{
  int dc;
  std::string txt;
  Address org;
  Address dst;
};

struct Task{
};

bool validConversion(EventType et,SmppHeader* pdu)
{
  return et==etDeliverySm || et==etDataSm || et==etDataSmResp;
}

bool validConversion(EventType et,SubmitData* data)
{
  return et==etSubmitSm;
}

bool validConversion(EventType et,Task* tsk)
{
  return et==etTask;
}

struct UniversalEvent{
  EventType et;
  void* data;

  UniversalEvent(EventType argET,void* argData):et(argET),data(argData){
  }

  ~UniversalEvent()
  {
    if(!data)return;
    switch(et)
    {
      case etDeliverySm:
      case etDataSm:
      case etSubmitResp:
      case etDataSmResp:
        disposePdu(as<SmppHeader>());
        break;
      case etSubmitSm:
        delete as<SubmitData>();
        break;
      case etTask:
        delete as<Task>();
        break;
    }
  }

  template <class T>
  T* as(){
    if(!validConversion(et,(T*)data))abort();
    return (T*)data;
  }
};

struct TimeKey{
  time_t t;
  int m;
  TimeKey(time_t argT,int argM):t(argT),m(argM){

  }
  TimeKey(const TimeKey& rhs):t(rhs.t),m(rhs.m){

  }
  bool operator<(const TimeKey& tk)const
  {
    return t<tk.t || (t==tk.t && m<tk.m);
  }
};

struct MainQueue{
  typedef std::multimap<TimeKey,UniversalEvent*> QueueType;
  QueueType mainQueue;
  EventMonitor mqMon;

  void PushEvent(TimeKey tk,UniversalEvent* e)
  {
    MutexGuard mg(mqMon);
    mainQueue.insert(QueueType::value_type(tk,e));
  }

  UniversalEvent* PopEvent()
  {
    MutexGuard mg(mqMon);
    if(mainQueue.empty())
    {
      mqMon.wait(1000);
      if(mainQueue.empty())return 0;
    }
    return 0;
  }
};

MainQueue mainQueue;

class MyListener:public SmppPduEventListener{
public:
  void handleEvent(SmppHeader *pdu)
  {
    if(pdu->get_commandId()==SmppCommandSet::DELIVERY_SM)
    {
      mainQueue.PushEvent(TimeKey(0,0),new UniversalEvent(etDeliverySm,pdu));
      /*
      PduDeliverySmResp resp;
      resp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
      resp.set_messageId("");
      resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
      resp.get_header().set_commandStatus(SmppStatusSet::ESME_ROK);
      trans->sendDeliverySmResp(resp);
      */
    }else
    if(pdu->get_commandId()==SmppCommandSet::DATA_SM)
    {
      mainQueue.PushEvent(TimeKey(0,0),new UniversalEvent(etDataSm,pdu));
      /*
      PduDataSmResp resp;
      resp.get_header().set_commandId(SmppCommandSet::DATA_SM_RESP);
      resp.set_messageId("");
      resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
      resp.get_header().set_commandStatus(SmppStatusSet::ESME_ROK);
      trans->sendDataSmResp(resp);
      */
    }else
    if(pdu->get_commandId()==SmppCommandSet::SUBMIT_SM_RESP)
    {
      mainQueue.PushEvent(TimeKey(0,0),new UniversalEvent(etSubmitResp,pdu));
    }else
    if(pdu->get_commandId()==SmppCommandSet::DATA_SM_RESP)
    {
      mainQueue.PushEvent(TimeKey(0,0),new UniversalEvent(etDataSmResp,pdu));
    }
  }
  void handleError(int errorCode)
  {
    connected=false;
  }
  void setTrans(SmppTransmitter *t)
  {
    trans=t;
  }
protected:
  SmppTransmitter* trans;
};

extern "C" void CtrlCSigDisp(int)
{
  running=false;
}

class ProcessorThread:public Thread{
public:
  ProcessorThread(SmppSession& argSS):ss(argSS)
  {

  }
  int Execute()
  {
    running=true;
    while(running)
    {
      while(running && !connected)
      {
        ss.close();
        try{
          ss.connect();
          connected=true;
        }catch(...)
        {
          millisleep(5000);
        }
      }
      if(!running)break;
      UniversalEvent* e=mainQueue.PopEvent();
      if(!e)continue;
      try{
        switch(e->et)
        {
          case etDeliverySm:
          {

          }break;
          case etDeliverySmResp:
          {

          }break;
          case etDataSm:
          {

          }break;
          case etDataSmResp:
          {

          }break;
          case etSubmitSm:
          {

          }break;
          case etSubmitResp:
          {

          }break;
          case etTask:
          {

          }break;
        }
      }catch(std::exception& ex)
      {
        //
      }
      delete e;
    }
    running=false;
    return 0;
  }
  void Stop()
  {
    running=false;
  }
protected:
  SmppSession& ss;
};

int main(int argc,char* argv[])
{
  sigset(SIGINT,CtrlCSigDisp);

  smsc::logger::Logger::Init();

  using_history();

  try{
    cfg.Read("realloadsim.ini");
  }catch(std::exception& e)
  {
    printf("Failed to load config:%s",e.what());
    return 1;
  }

  try{
    SmeConfig smeCfg;

    smeCfg.host=cfg.getStr("smpp.host");
    smeCfg.port=cfg.getInt("smpp.port");
    smeCfg.sid=cfg.getStr("smpp.systemId");
    smeCfg.password=cfg.getStr("smpp.password");
    smeCfg.timeOut=cfg.getInt("smpp.socketTimeOut");
    smeCfg.smppTimeOut=cfg.getInt("smpp.timeOut");

    MyListener mlst;
    SmppSession ss(smeCfg,&mlst);
    ProcessorThread pt(ss);
    pt.Start();

    char* cmd;
    while(running)
    {
      cmd=readline(">");
      if(!cmd)
      {
        running=false;
        break;
      }
      if(!*cmd)continue;
      add_history(cmd);
    }
    pt.WaitFor();
  }catch(std::exception& e)
  {
    printf("Init exception: %s\n",e.what());
  }

  /*
  try{


    PduSubmitSm sm;
    SMS s;
    char msc[]="123";
    char imsi[]="123";
    s.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
    s.setValidTime(0);
    s.setArchivationRequested(false);
    s.setIntProperty(Tag::SMPP_ESM_CLASS,0);
    s.setEServiceType("MAP");
    sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);

    while(running)
    {
      s.setDestinationAddress("+79130000000");
      s.setOriginatingAddress("+79130000000");

      s.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::LATIN1);
      s.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,"test",4);
      fillSmppPduFromSms(&sm,&s);
      PduXSmResp* pdu=tr->submit(sm);
      if(pdu)disposePdu((SmppHeader*)pdu);
      printf("->\n");

      millisleep(100);
    }
    ss.close();
  }catch(std::exception& e)
  {
    printf("Exception:%s\n",e.what());
  }
  */
  printf("Finished\n");
  return 0;
}

