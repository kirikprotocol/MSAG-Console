#include <stdio.h>
#include "sme/SmppBase.hpp"
#include "sms/sms.h"
#include "util/smstext.h"
#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"
#include "emailsme/statuscodes.h"
#define NAMEDBRACKETS
#include "util/regexp/RegExp.cpp"
#include "core/buffers/Array.hpp"
#include <signal.h>
#include "db/DataSource.h"
#include "db/DataSourceLoader.h"
#include "util/templates/Formatters.h"
#include "util/xml/init.h"

#include "emailsme/util/PipedChild.hpp"

using namespace smsc::sms;
using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;
using namespace smsc::util::regexp;
using namespace smsc::core::threads;
using namespace smsc::core::synchronization;
using namespace smsc::util::templates;

using smsc::core::buffers::Hash;
using smsc::core::buffers::Array;

using namespace smsc::emailsme;

using namespace std;

FILE *emlIn;
FILE *emlOut;

class EmptyGetAdapter:public GetAdapter{
public:

  virtual bool isNull(const char* key)
      throw(AdapterException)
  {
    return false;
  }

  virtual const char* getString(const char* key)
      throw(AdapterException)
  {
    return "";
  }

  virtual int8_t getInt8(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

  virtual int16_t getInt16(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

  virtual int32_t getInt32(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

  virtual int64_t getInt64(const char* key)
      throw(AdapterException)
  {
    return 0;
  }


  virtual uint8_t getUint8(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

  virtual uint16_t getUint16(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

  virtual uint32_t getUint32(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

  virtual uint64_t getUint64(const char* key)
      throw(AdapterException)
  {
    return 0;
  }


  virtual float getFloat(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

  virtual double getDouble(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

  virtual long double getLongDouble(const char* key)
      throw(AdapterException)
  {
    return 0;
  }


  virtual time_t getDateTime(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

};


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
 int defaultDailyLimit;
 int annotationSize;
 string maildomain;
 string mailstripper;
 smsc::db::DataSource *dataSource;
 OutputFormatter *msgFormat;
};

class ConnectionGuard{
  smsc::db::DataSource *ds;
  smsc::db::Connection *conn;
public:
  ConnectionGuard(smsc::db::DataSource *_ds):ds(_ds)
  {
    conn=ds->getConnection();
  }
  ~ConnectionGuard()
  {
    ds->freeConnection(conn);
  }
  smsc::db::Connection* operator->()
  {
    return conn;
  }
  smsc::db::Connection* get()
  {
    return conn;
  }
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
const int NOPROFILE    =3;
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
    __warning2__("code %d, expected %d",retcode,code);
    throw Exception("code %d, expected %d",retcode,code);
  }
}

string makeFromAddress(const char* fromaddress)
{
  return (string)fromaddress+"@"+cfg::maildomain;
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

int getUsageCount(int id)
{
  using namespace smsc::db;
  const char* sql="select count(*) from EMLSME_HISTORY where user_id=:id";
  ConnectionGuard connection(cfg::dataSource);
  if(!connection.get())throw Exception("Failed to get db connection");
  auto_ptr<Statement> statement(connection->createStatement(sql));
  if(!statement.get())throw Exception("Failed to prepare statement");
  statement->setInt32(1,id);
  auto_ptr<ResultSet> rs(statement->executeQuery());
  if(!rs.get())throw Exception("Failed to make a query to DB");
  return rs->getInt32(1);
}


string MapEmailToAddress(const string& username,string& fwd)
{
  using namespace smsc::db;
  const char* sql="select address,forward,daily_limit,(select count(*) from EMLSME_HISTORY where MSG_DATE<=:1 and EMLSME_HISTORY.address=EMLSME_PROFILES.address) from EMLSME_PROFILES where username=:2";
  ConnectionGuard connection(cfg::dataSource);
  if(!connection.get())throw Exception("Failed to get db connection");
  auto_ptr<Statement> statement(connection->createStatement(sql));
  if(!statement.get())throw Exception("Failed to prepare statement");
  __trace__("MapEmailToAddress statement created");
  time_t now=time(NULL);
  statement->setDateTime(1,now+24*60*60);
  statement->setString(2,username.c_str());
  auto_ptr<ResultSet> rs(statement->executeQuery());
  __trace__("Execute ok");
  if(!rs.get())throw Exception("Failed to make a query to DB");
  if(!rs->fetchNext())throw Exception("Mapping username->address for %s not found",username.c_str());
  int dailyLimit=rs->isNull(3)?cfg::defaultDailyLimit:rs->getInt32(3);
  int currentCount=rs->getInt32(4);
  __trace2__("%s:currentCount=%d, dailyLimit=%d",username.c_str(),currentCount,dailyLimit);
  if(currentCount>=dailyLimit)
  {
    throw Exception("daily limit reached for %s",username.c_str());
  }
  fwd=rs->isNull(2)?"":rs->getString(2);
  __trace2__("Map %s->%s",username.c_str(),rs->getString(1));
  return rs->getString(1);
}

string MapAddressToEmail(const string& address)
{
  using namespace smsc::db;
  const char* sql="select username from EMLSME_PROFILES where address=:1";
  ConnectionGuard connection(cfg::dataSource);
  if(!connection.get())throw Exception("Failed to get db connection");
  auto_ptr<Statement> statement(connection->createStatement(sql));
  if(!statement.get())throw Exception("Failed to prepare statement");
  statement->setString(1,address.c_str());
  auto_ptr<ResultSet> rs(statement->executeQuery());
  if(!rs.get())throw Exception("Failed to make a query to DB");
  if(!rs->fetchNext())throw Exception("Mapping address->email for %s not found",address.c_str());
  __trace2__("Map %s->%s",address.c_str(),rs->getString(1));
  return rs->getString(1);
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

int SendEMail(const string& from,const Array<string>& to,const string& subj,const string& body,bool rawMsg=false)
{
  Socket s;
  if(s.Init(cfg::smtpHost.c_str(),cfg::smtpPort,0)==-1)
  {
    __warning__("smtp host resolve failed");
    return ProcessSmsCodes::NETERROR;
  }
  if(s.Connect()==-1)
  {
    __warning__("smtp connect failed");
    return ProcessSmsCodes::NETERROR;
  }
  CheckCode(s,220);
  s.Printf("HELO\r\n");
  CheckCode(s,250);
  s.Printf("MAIL FROM: %s\r\n",ExtractEmail(from).c_str());
  CheckCode(s,250);
  for(int i=0;i<to.Count();i++)
  {
    s.Printf("RCPT TO: %s\r\n",ExtractEmail(to[i]).c_str());
    CheckCode(s,2);
  }
  s.Puts("DATA\r\n");
  CheckCode(s,3);
  if(!rawMsg)
  {
    s.Printf("From: %s\r\n",from.c_str());
    string addr;
    for(int i=0;i<to.Count();i++)
    {
      addr+=to[i];
      if(i!=to.Count()-1)addr+=',';
    }
    s.Printf("To: %s\r\n",addr.c_str());
    if(subj.length())s.Printf("Subject: %s\r\n",subj.c_str());
    s.Printf("\r\n");
  }
  if(s.WriteAll(body.c_str(),body.length())==-1)throw Exception("Failed to write body");
  s.Puts("\r\n.\r\n");
  CheckCode(s,250);
  s.Puts("QUIT\r\n");
  return ProcessSmsCodes::OK;
}

int processSms(const char* text,const char* fromaddress)
{
  Hash<SMatch> h;
  SMatch m[10];
  int n=10;
  if(!reParseSms.Match(text,m,n,&h))
  {
    __trace2__("RegExp match failed:%d",reParseSms.LastError());
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

  try{
    from=makeFromAddress(MapAddressToEmail(fromaddress).c_str());
  }catch(exception& e)
  {
    __warning2__("failed to map address to mail:%s",e.what());
    return ProcessSmsCodes::NOPROFILE;
  }
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

  try{
    int rv=SendEMail(fromdecor,to,subj,body);
    if(rv!=ProcessSmsCodes::OK)return rv;
  }catch(...)
  {
    __warning__("SMTP session aborted");
    return ProcessSmsCodes::UNABLETOSEND;
  }
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
      addr.toString(addrtext,sizeof(addrtext));

      int code=processSms(buf,addrtext);
      __trace2__("processSms: code=%d",code);
      switch(code)
      {
        case ProcessSmsCodes::NOPROFILE:
        case ProcessSmsCodes::INVALIDSMS:code=SmppStatusSet::ESME_RX_P_APPN;break;
        case ProcessSmsCodes::UNABLETOSEND:
        case ProcessSmsCodes::NETERROR:code=SmppStatusSet::ESME_RX_T_APPN;break;
        case ProcessSmsCodes::OK:code=SmppStatusSet::ESME_ROK;break;
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

void IncUsageCounter(const string& address)
{
  using namespace smsc::db;
  __trace2__("inc counter for %s",address.c_str());
  const char* sql="insert into EMLSME_HISTORY (address,MSG_DATE) values (:1,:2)";
  ConnectionGuard connection(cfg::dataSource);
  if(!connection.get())throw Exception("Failed to get db connection");
  auto_ptr<Statement> statement(connection->createStatement(sql));
  if(!statement.get())throw Exception("Failed to prepare statement");
  statement->setString(1,address.c_str());
  statement->setDateTime(2,time(NULL));
  statement->executeUpdate();
  connection->commit();
}

int ProcessMessage(const char *msg,int len)
{
  string line,name,value,from,to;
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
      continue;
    }
    break;
  }

  if(!util::childRunning)
  {
    if(emlIn)fclose(emlIn);
    if(emlOut)fclose(emlOut);
    if(util::ForkPipedCmd(cfg::mailstripper.c_str(),emlIn,emlOut)<=0)
    {
      return StatusCodes::STATUS_CODE_UNKNOWNERROR;
    }
  };
  __trace2__("write msg size:%d",len);
  fprintf(emlOut,"%d\n",len);fflush(emlOut);
  __trace__("write msg");
  int sz=0;
  while(sz<len)
  {
    int wr=fwrite(msg,1,len,emlOut);fflush(emlOut);
    if(wr<=0)
    {
      return StatusCodes::STATUS_CODE_UNKNOWNERROR;
    }
    sz+=wr;
  }
  __trace__("read resp len");
  char buf[16];
  if(!fgets(buf,sizeof(buf),emlIn))
  {
    return StatusCodes::STATUS_CODE_UNKNOWNERROR;
  }
  __trace2__("resp len=%d",len);
  len=atoi(buf);
  auto_ptr<char> newmsg(new char[len+1]);

  sz=0;
  while(sz<len)
  {
    int rv=fread(newmsg.get(),1,len-sz,emlIn);
    if(rv==0 || rv==-1)return StatusCodes::STATUS_CODE_UNKNOWNERROR;
    sz+=rv;
  }
  newmsg.get()[len]=0;

  __trace2__("newmsg:%s",newmsg.get());

  //cfg::annotationSize


  SMS sms;
  sms.setOriginatingAddress(cfg::sourceAddress.c_str());

  string fwd;
  string dst=MapEmailToAddress(to.substr(0,to.find('@')),fwd);
  if(fwd.length())
  {
    try{
      Array<string> to;
      to.Push(fwd);
      string body(msg,len);
      SendEMail(from,to,"",body,true);
    }catch(exception& e)
    {
      __warning2__("Failed to forward msg to %s:%s",fwd.c_str(),e.what());
    }
  }

  pos=0;
  string subj;
  GetNextLine(newmsg.get(),len,pos,from);
  GetNextLine(newmsg.get(),len,pos,subj);
  string text=newmsg.get()+pos;

  ContextEnvironment ce;
  EmptyGetAdapter ga;

  int fldlen=subj.length()+from.length()+to.length();
  if(text.length()>cfg::annotationSize-fldlen)
  {
    text=text.substr(0,cfg::annotationSize-fldlen);
  }

  pos=0;
  while((pos=text.find("\"",pos))!=string::npos)
  {
    text.at(pos)='\'';
  }
  pos=0;
  while((pos=subj.find("\"",pos))!=string::npos)
  {
    subj.at(pos)='\'';
  }

  ce.exportStr("from",ExtractEmail(from).c_str());
  ce.exportStr("to",to.c_str());
  ce.exportStr("subj",subj.c_str());
  ce.exportStr("body",text.c_str());

  text="";
  cfg::msgFormat->format(text,ga,ce);

  __trace2__("result:%s",text.c_str());



  sms.setDestinationAddress(dst.c_str());
  char msc[]="";
  char imsi[]="";
  sms.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
  sms.setDeliveryReport(0);
  sms.setArchivationRequested(false);
  sms.setEServiceType(cfg::serviceType.c_str());
  sms.setIntProperty(smsc::sms::Tag::SMPP_PROTOCOL_ID,cfg::protocolId);

  sms.setIntProperty(smsc::sms::Tag::SMPP_ESM_CLASS,0);
  if(hasHighBit(text.c_str(),text.length()))
  {
    auto_ptr<short> ucs2(new short[text.length()+1]);
    ConvertMultibyteToUCS2(text.c_str(),text.length(),ucs2.get(),(text.length()+1)*2,CONV_ENCODING_CP1251);
    sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING,DataCoding::UCS2);
    sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD,(char*)ucs2.get(),text.length()*2);
  }else
  {
    sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING,DataCoding::LATIN1);
    sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD,text.c_str(),text.length());
  }
  //sms.setIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH,text.length());

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
    if(rc==StatusCodes::STATUS_CODE_OK)
    {
      try{
        IncUsageCounter(dst);
      }catch(exception& e)
      {
        __warning2__("failed to inc counter:%s",e.what());
      }
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


void atExitHandler(void)
{
    smsc::util::xml::TerminateXerces();
	smsc::logger::Logger::Shutdown();
}

int main(int argc,char* argv[])
{
  smsc::logger::Logger::Init();
  atexit(atExitHandler);

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
  using namespace smsc::db;
  config::Manager::init("config.xml");
  config::Manager& cfgman= config::Manager::getInstance();

  config::ConfigView *dsConfig = new config::ConfigView(cfgman, "StartupLoader");

  const char* OCI_DS_FACTORY_IDENTITY = "OCI";
  DataSourceLoader::loadup(dsConfig);

  cfg::dataSource = DataSourceFactory::getDataSource(OCI_DS_FACTORY_IDENTITY);
  if (!cfg::dataSource) throw Exception("Failed to get DataSource");
  auto_ptr<ConfigView> config(new ConfigView(cfgman,"DataSource"));

  cfg::dataSource->init(config.get());


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
  cfg.password=cfgman.getString("smpp.password");
  cfg.origAddr=cfgman.getString("smpp.sourceAddress");

  cfg::serviceType=cfgman.getString("smpp.serviceType");
  cfg::protocolId=cfgman.getInt("smpp.protocolId");

  cfg::maildomain=cfgman.getString("mail.domain");

  Socket srv;

  if(srv.InitServer(cfgman.getString("listener.host"),cfgman.getInt("listener.port"),0,0)==-1)
  {
    __warning2__("emailsme: Failed to init listener at %s:%d",cfgman.getString("listener.host"),cfgman.getInt("listener.port"));
    return -1;
  };
  if(srv.StartServer()==-1)
  {
    __warning__("Failed to start listener");
    return -1;
  };


  cfg::mailstripper=cfgman.getString("mail.stripper");
  if(util::ForkPipedCmd(cfg::mailstripper.c_str(),emlIn,emlOut)<=0)
  {
    __warning2__("Failed to execute mail stripper:%s",strerror(errno));
    fprintf(stderr,"Failed to execute mail stripper:%s",strerror(errno));
    return -1;
  }

  cfg::msgFormat=new OutputFormatter(cfgman.getString("mail.format"));

  cfg::defaultDailyLimit=cfgman.getInt("defaults.dailyLimit");
  cfg::annotationSize=cfgman.getInt("defaults.annotationSize");

  __trace2__("defaults.annotationSize:%d",cfg::annotationSize);

  MyListener lst;
  SmppSession ss(cfg,&lst);
  cfg::tr=ss.getSyncTransmitter();
  cfg::sourceAddress=cfgman.getString("smpp.sourceAddress");
  lst.setTrans(cfg::tr);
  XBuffer buf;

  while(!cfg::stopSme)
  {
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
      __trace__("Waiting for connection");
      {
        auto_ptr<Socket> clnt(srv.Accept());
        if(!clnt.get())break;
        int sz;
        clnt->setTimeOut(10);
        __trace__("Got connection");
        if(clnt->ReadAll((char*)&sz,4)==-1)continue;
        sz=ntohl(sz);
        __trace2__("Message size:%d",sz);
        buf.setSize(sz+1);
        if(clnt->ReadAll(buf.buffer,sz)==-1)continue;
        __trace__("Processing message");
        int retcode;
        try{
          buf.buffer[sz]=0;
          retcode=ProcessMessage(buf.buffer,sz);
        }catch(exception& e)
        {
          __warning2__("process message failed:%s",e.what());
          retcode=StatusCodes::STATUS_CODE_UNKNOWNERROR;
        }
        __trace2__("Processing finished, code=%d",retcode);
        retcode=htonl(retcode);
        clnt->WriteAll(&retcode,4);
      }
    }
    ss.close();
  }
  __trace__("exiting");
  //srv.Close();
  srv.Abort();
  }catch(exception& e)
  {
    __warning2__("Top level exception:%s",e.what());
  }
  catch(...)
  {
    __warning__("Top level exception:unknown");
  }
  if(emlIn)fclose(emlIn);
  if(emlOut)fclose(emlOut);
  return 0;
}
