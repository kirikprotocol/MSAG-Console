#include <stdio.h>
#include "sme/SmppBase.hpp"
#include "sms/sms.h"
#include "util/smstext.h"
#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"
#include "emailsme/statuscodes.h"
#include "util/regexp/RegExp.hpp"
#include "core/buffers/Array.hpp"
#include <signal.h>

using namespace smsc::sms;
using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;
using namespace smsc::util::regexp;
using namespace smsc::core::threads;
using namespace smsc::core::synchronization;

using smsc::core::buffers::Hash;
using smsc::core::buffers::Array;

using namespace smsc::emailsme;

namespace cfg{
 string sourceAddress;
 int protocolId;
 string serviceType;
 SmppTransmitter *tr;
 int mainId;
 bool stopSme=false;
 string smtpHost;
 int smtpPort=25;
 int retryTime=5;
};

int stopped=0;

RegExp reParseSms;

bool getField(const char* text,Hash<SMatch>& h,const char* key,string& out)
{
  if(!h.Exists(key))return false;
  SMatch& m=h[key];
  if(m.start==-1 || m.end==-1)return false;
  out.assign(text,m.start,m.end-m.start);
  return true;
}

namespace ProcessSmsCodes{
const int INVALIDSMS   =0;
const int NETERROR     =1;
const int UNABLETOSEND =2;
const int OK           =256;
};

void CheckCode(Socket& s,int code)
{
  char buf[1024];
  if(s.Gets(buf,sizeof(buf))==-1)throw Exception("");
  int retcode;
  if(sscanf(buf,"%d",&retcode)!=1)throw Exception("");;
  if(code<10)retcode/=100;
  if(retcode!=code)
  {
    fprintf(stderr,"code %d, expected %d\n",retcode,code);
    throw Exception("");
  }
}

string makeFromAddress(const char* fromaddress)
{
  return (string)fromaddress+"@smsc.novosoft.ru";
}

bool ismailchar(char c)
{
  if(isalnum(c))return true;
  return c=='.' || c=='-' || c=='_';
}

string ExtractEmail(const string& value)
{
  unsigned int pos=value.find('@');
  if(pos==string::npos)return "";
  int start=pos,end=pos;
  while(start>0 && ismailchar(value[start-1]))start--;
  while(end<value.length() && ismailchar(value[end+1]))end++;
  string res(value,start,end-start+1);
  return res;
}

void ReplaceString(string& s,const char* what,const char* to)
{
  int i=0;
  int l=strlen(what);
  int l2=strlen(to);
  while((i=s.find(what,i))!=string::npos)
  {
    s.replace(i,l,to);
    i+=l2;
  }
}

int processSms(const char* text,const char* fromaddress)
{
  Hash<SMatch> h;
  SMatch m[10];
  int n=10;
  if(!reParseSms.Match(text,m,n,&h))
  {
    return ProcessSmsCodes::INVALIDSMS;
  }
  string cf,addr,rn,subj,body,from,fromdecor;
  getField(text,h,"flag",cf);
  getField(text,h,"address",addr);
  getField(text,h,"realname",rn);
  getField(text,h,"subj",subj);
  getField(text,h,"body",body);
  Socket s;

  ReplaceString(addr,"*","@");
  ReplaceString(addr,"$","_");

  ReplaceString(body,"\n.\n","\n..\n");

  from=makeFromAddress(fromaddress);
  fromdecor=from;
  if(rn.length())
  {
    fromdecor=rn+"<"+from+">";
  }

  Array<string> to;
  int startpos=0;
  int pos=addr.find(',');
  if(pos==string::npos)
  {
    to.Push(addr);
  }else
  {
    while(pos!=string::npos)
    {
      to.Push(addr.substr(startpos,pos-startpos));
      startpos=pos+1;
      pos=addr.find(',',pos+1);
    }
    to.Push(addr.substr(startpos));
  }

  if(s.Init(cfg::smtpHost.c_str(),cfg::smtpPort,0)==-1)
    return ProcessSmsCodes::NETERROR;
  if(s.Connect()==-1)return ProcessSmsCodes::NETERROR;
  try{
    CheckCode(s,220);
    s.Printf("HELO\r\n");
    CheckCode(s,250);
    s.Printf("MAIL FROM: %s\r\n",from.c_str());
    CheckCode(s,250);
    for(int i=0;i<to.Count();i++)
    {
      s.Printf("RCPT TO: %s\r\n",ExtractEmail(to[i]).c_str());
      CheckCode(s,2);
    }
    s.Puts("DATA\r\n");
    CheckCode(s,3);
    s.Printf("From: %s\r\n",fromdecor.c_str());
    s.Printf("To: %s\r\n",addr.c_str());
    if(subj.length())s.Printf("Subject: %s\r\n",subj.c_str());
    s.Printf("\r\n");
    if(s.WriteAll(body.c_str(),body.length())==-1)throw Exception("");
    s.Puts("\r\n.\r\n");
    CheckCode(s,250);
  }catch(...)
  {
    fprintf(stderr,"SMTP session aborted\n");
    return ProcessSmsCodes::UNABLETOSEND;
  }
  s.Puts("QUIT\r\n");
  return ProcessSmsCodes::OK;
}

class MyListener:public SmppPduEventListener{
public:
  void handleEvent(SmppHeader *pdu)
  {
    if(pdu->get_commandId()==SmppCommandSet::DELIVERY_SM)
    {
      char buf[260];
      getPduText((PduXSm*)pdu,buf,sizeof(buf));
      Address addr(
      ((PduXSm*)pdu)->get_message().get_source().value.size(),
      ((PduXSm*)pdu)->get_message().get_source().get_typeOfNumber(),
      ((PduXSm*)pdu)->get_message().get_source().get_numberingPlan(),
      ((PduXSm*)pdu)->get_message().get_source().get_value());
      char addrtext[64];
      addr.getText(addrtext,sizeof(addrtext));

      int code=processSms(buf,addrtext);
      switch(code)
      {
        case ProcessSmsCodes::INVALIDSMS:code=SmppStatusSet::ESME_RX_P_APPN;
        case ProcessSmsCodes::UNABLETOSEND:
        case ProcessSmsCodes::NETERROR:code=SmppStatusSet::ESME_RX_T_APPN;
        case ProcessSmsCodes::OK:code=SmppStatusSet::ESME_ROK;
      }
      PduDeliverySmResp resp;
      resp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
      resp.get_header().set_commandStatus(code);
      resp.set_messageId("");
      resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
      trans->sendDeliverySmResp(resp);
    }
  }
  void handleError(int errorCode)
  {
    thr_kill(cfg::mainId,16);
  }

  void setTrans(SmppTransmitter *t)
  {
    trans=t;
  }
protected:
  SmppTransmitter* trans;
};

bool GetNextLine(const char* text,int maxlen,int& pos,string& line)
{
  int start=pos;
  if(pos>=maxlen)return false;
  while(pos<maxlen && text[pos]!=0x0d && text[pos]!=0x0a)pos++;
  line.assign(text+start,pos-start);
  if(text[pos]==0x0d && pos<maxlen)pos++;
  if(text[pos]==0x0a && pos<maxlen)pos++;
  return true;
}


int ProcessMessage(const char *msg,int len)
{
  string line,name,value,body,from,to,subj;
  bool inheader=true;
  int pos=0;
  for(;;)
  {
    if(!GetNextLine(msg,len,pos,line))break;
    if(inheader)
    {
      if(line.length()==0)
      {
        inheader=false;
        continue;
      }
      int pos=line.find(':');
      if(pos==string::npos)continue;
      name=line.substr(0,pos);
      pos++;
      while(pos<line.length() && line[pos]==' ')pos++;
      value=line.substr(pos);

      if(name=="From")
      {
        from=ExtractEmail(value);
        if(from.length()==0)return StatusCodes::STATUS_CODE_INVALIDMSG;
        continue;
      }
      if(name=="To")
      {
        to=ExtractEmail(value);
        if(to.length()==0)return StatusCodes::STATUS_CODE_INVALIDMSG;
        continue;
      }
      if(name=="Subject")
      {
        subj=value;
        continue;
      }
      continue;
    }
    body+=(body.length()?"\n":"")+line;
  }
  if(from.length()==0 || to.length()==0)return StatusCodes::STATUS_CODE_INVALIDMSG;
  string text=from;
  if(subj.length())
  {
    text+="##";
    text+=subj;
    text+='#';
  }
  else text+=" ";
  text+=body;
  SMS sms;
  sms.setOriginatingAddress(cfg::sourceAddress.c_str());
  string dst=to.substr(0,to.find('@'));
  sms.setDestinationAddress(dst.c_str());
  char msc[]="123";
  char imsi[]="123";
  sms.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
  sms.setDeliveryReport(0);
  sms.setArchivationRequested(false);
  sms.setEServiceType(cfg::serviceType.c_str());
  sms.setIntProperty(smsc::sms::Tag::SMPP_PROTOCOL_ID,cfg::protocolId);

  sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING,DataCoding::DEFAULT);
  sms.setBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE,text.c_str(),text.length());
  sms.setIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH,text.length());

  PduSubmitSm sm;
  sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
  fillSmppPduFromSms(&sm,&sms);
  PduSubmitSmResp *resp=cfg::tr->submit(sm);
  int rc=StatusCodes::STATUS_CODE_TEMPORARYERROR;
  if(resp)
  {
    switch(resp->get_header().get_commandStatus())
    {
      case SmppStatusSet::ESME_ROK:rc=StatusCodes::STATUS_CODE_OK;break;
      case SmppStatusSet::ESME_RINVDSTADR:rc=StatusCodes::STATUS_CODE_NOUSER;break;
      default:rc=StatusCodes::STATUS_CODE_UNKNOWNERROR;break;
    }
  }
  if(resp)disposePdu((SmppHeader*)resp);
  return rc;
}

struct XBuffer{
  char* buffer;
  int size;
  int offset;

  XBuffer(){buffer=0;size=0;offset=0;}
  ~XBuffer(){if(buffer)delete [] buffer;}

  void setSize(int newsize)
  {
    if(newsize<size)return;
    char *newbuf=new char[newsize];
    if(offset)memcpy(newbuf,buffer,offset);
    if(buffer)delete [] buffer;
    buffer=newbuf;
    size=newsize;
  }
  void append(char *mem,int count)
  {
    if(offset+count>size)setSize((offset+count)+(offset+count)/2);
    memcpy(buffer+offset,mem,count);
    offset+=count;
  }
  char* current(){return buffer+offset;}
  int freeSpace(){return size-offset;}
};

static void disp(int sig)
{
}

static void ctrlc(int sig)
{
  cfg::stopSme=1;
  thr_kill(cfg::mainId,16);
}

int main(int argc,char* argv[])
{
  sigset(16,disp);
  sigset(SIGINT,ctrlc);
  sigset(SIGTERM,ctrlc);

  cfg::mainId=thr_self();

  RegExp::InitLocale();
  reParseSms.Compile(
"/(#(?{flag}\\w)#)?"                         // optional control flag
"(?{address}(?:.*?[@*][^#\\(\\s,]+,?)+)"     // mandatory address(es)
"(#(?{realname}[^#(]+))?"                    // optional real name
"(?:##(?{subj}.*?)#|\\((?{subj}.*?)\\)|\\s)" // optinal subject
"(?{body}.+)$/xs"                            // body
);

  try{

  using namespace smsc::util;
  config::Manager::init("config.xml");
  config::Manager& cfgman= config::Manager::getInstance();

  config::ConfigView *dsConfig = new config::ConfigView(cfgman, "StartupLoader");

  cfg::smtpHost=cfgman.getString("smtp.host");
  try{
    cfg::smtpPort=cfgman.getInt("smtp.port");
  }catch(...)
  {
  }
  try{
    cfg::retryTime=cfgman.getInt("smpp.retryTime");
  }catch(...)
  {
  }


  SmeConfig cfg;
  cfg.host=cfgman.getString("smpp.host");
  cfg.port=cfgman.getInt("smpp.port");
  cfg.sid=cfgman.getString("smpp.systemId");
  cfg.timeOut=cfgman.getInt("smpp.timeout");
  cfg.origAddr=cfgman.getString("smpp.sourceAddress");

  cfg::serviceType=cfgman.getString("smpp.serviceType");
  cfg::protocolId=cfgman.getInt("smpp.protocolId");

  MyListener lst;
  SmppSession ss(cfg,&lst);
  cfg::tr=ss.getSyncTransmitter();
  cfg::sourceAddress=cfgman.getString("smpp.sourceAddress");
  lst.setTrans(cfg::tr);
  Socket srv;
  XBuffer buf;

  while(!cfg::stopSme)
  {
    if(srv.InitServer(cfgman.getString("listener.host"),cfgman.getInt("listener.port"),0)==-1)
    {
      fprintf(stderr,"emailsme: Failed to init listener\n");
      return -1;
    };
    if(srv.StartServer()==-1)
    {
      fprintf(stderr,"Failed to start listener\n");
      return -1;
    };
    for(;;)
    {
      if(cfg::stopSme)break;
      try{
        ss.connect();
      }catch(...)
      {
        sleep(cfg::retryTime);
        continue;
      }
      break;
    }
    if(cfg::stopSme)break;
    for(;;)
    {
      auto_ptr<Socket> clnt(srv.Accept());
      if(!clnt.get())break;
      int sz;
      if(clnt->ReadAll((char*)&sz,4)==-1)continue;
      sz=ntohl(sz);
      buf.setSize(sz);
      if(clnt->ReadAll(buf.buffer,sz)==-1)continue;
      int retcode=ProcessMessage(buf.buffer,sz);
      retcode=htonl(retcode);
      clnt->WriteAll(&retcode,4);
    }
    ss.close();
    srv.Close();
  }
  }catch(exception& e)
  {
    fprintf(stderr,"Top level exception:%s\n",e.what());
  }
  catch(...)
  {
    fprintf(stderr,"Top level exception:unknown\n");
  }
  return 0;
}
