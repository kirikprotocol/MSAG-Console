#ifdef _WIN32
#include <winsock2.h>
#endif
#include "sme/SmppBase.hpp"
#include <stdio.h>
#include "sms/sms.h"
#include "util/smstext.h"
#include "core/buffers/Array.hpp"
#include "core/buffers/Hash.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "util/timeslotcounter.hpp"
#include "logger/Logger.h"
#include <vector>
#include "util/sleep.h"

using namespace smsc::sms;
using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;
using namespace smsc::core::buffers;
using namespace smsc::core::synchronization;
using namespace smsc::logger;

using namespace std;

const int MAX_UNRESPONDED_HIGH=5000;
const int MAX_UNRESPONDED_LOW=2000;


int stopped=0;

Mutex cntMutex;

int sokcnt=0;
int serrcnt=0;
int reccnt=0;

TimeSlotCounter<> sok_time_cnt(30,100);

class MyListener:public SmppPduEventListener{
public:
  void handleEvent(SmppHeader *pdu)
  {
    if(pdu->get_commandId()==SmppCommandSet::DELIVERY_SM)
    {
      //char buf[256];
      PduDeliverySmResp resp;
      resp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
      resp.set_messageId("");
      resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
      resp.get_header().set_commandStatus(SmppStatusSet::ESME_ROK);
      trans->sendDeliverySmResp(resp);
      {
        MutexGuard g(cntMutex);
        reccnt++;
      }
    }else
    if(pdu->get_commandId()==SmppCommandSet::SUBMIT_SM_RESP)
    {
      __trace2__("received submit sm resp:%x, seq=%d\n",pdu->get_commandStatus(),pdu->get_sequenceNumber());
      //printf("\nReceived async submit sm resp:%d\n",pdu->get_commandStatus());
      MutexGuard g(cntMutex);

      if(pdu->get_commandStatus()==SmppStatusSet::ESME_ROK)
      {
        sokcnt++;
        sok_time_cnt.Inc();
      }else
      {
        serrcnt++;
      }
    }
    disposePdu(pdu);
  }
  void handleError(int errorCode)
  {
    printf("error!\n");
    stopped=1;
  }

  void setTrans(SmppTransmitter *t)
  {
    trans=t;
  }
protected:
  SmppTransmitter* trans;
};

struct Option{
  const char* name;
  char type;
  void* addr;
  int& asInt(){return *(int*)addr;}
  bool& asBool(){return *(bool*)addr;}
  string& asString(){return *(string*)addr;}
};

string host;
int port=9001;
string systemId;
string password;
int timeOut=60;
int speed=10;
int maxCount=10000;
string source;
string sourcesFile;
string destination;
string destinationsFile;
string message;
string messagesFile;
string wordsFile;

int minLength=0;
int maxLength=0;

int minSegments=0;
int maxSegments=0;

int probIncompleteMessage=0;
int probUssdMessage=0;
int probInvalidUssdMessage=0;

Option options[]=
{
  {"host",'s',&host},
  {"port",'i',&port},
  {"systemId",'s',&systemId},
  {"password",'s',&password},
  {"timeOut",'i',&timeOut},
  {"speed",'i',&speed},
  {"maxCount",'i',&maxCount},
  {"source",'s',&source},
  {"sourcesFile",'s',&sourcesFile},
  {"destination",'s',&destination},
  {"destinations",'s',&destinationsFile},
  {"message",'s',&message},
  {"messagesFile",'s',&messagesFile},
  {"wordsFile",'s',&wordsFile},

  {"minLength",'i',&minLength},
  {"maxLength",'i',&maxLength},
  {"minSegments",'i',&minSegments},
  {"maxSegments",'i',&maxSegments},
  {"probIncompleteMessage",'i',&probIncompleteMessage},
  {"probUssdMessage",'i',&probUssdMessage},
  {"probInvalidUssdMessage",'i',&probInvalidUssdMessage}
};

int optionsCount=sizeof(options)/sizeof(options[0]);

bool atob(const char* val)
{
  return !strcmp(val,"1") || !strcmp(val,"yes") || !strcmp(val,"on");
}

void ProcessOption(const char* name,const char* val)
{
  for(int i=0;i<optionsCount;i++)
  {
    Option& o=options[i];
    if(strcmp(o.name,name))continue;
    switch(o.type)
    {
      case 's':o.asString()=val;return;
      case 'i':o.asInt()=atoi(val);return;
      case 'b':o.asBool()=atob(val);return;
      default: throw Exception("Unknown option type %c",o.type);
    }
  }
  throw Exception("Unknown option %s",name);
}

typedef vector<string> StrList;
void LoadFile(const string& filename,StrList& sl)
{
  FILE *f=fopen(filename.c_str(),"rt");
  if(!f)throw Exception("Faield to open file %s",filename);
  fseek(f,0,SEEK_END);
  int sz=ftell(f);
  fseek(f,0,SEEK_SET);
  char* buf=new char[sz];
  fread(buf,sz,1,f);
  fclose(f);
  int strStart=0,curPos=0;
  do
  {
    while(curPos<sz && buf[curPos]!=0x0d && buf[curPos]!=0x0a)curPos++;
    if(curPos-strStart==0)
    {
      if(buf[curPos]==0x0d)curPos++;
      curPos++;
      continue;
    }
    sl.push_back(string(buf+strStart,curPos-strStart));
    if(buf[curPos]==0x0d)curPos++;
    curPos++;
    strStart=curPos;
  }while(curPos<sz);
  delete [] buf;
}

void ParseOptionLine(const string& line)
{
  string::size_type pos=line.find('=');
  if(pos==string::npos)throw Exception("Invalid option line %s",line.c_str());
  string name=line.substr(0,pos);
  string val=line.substr(pos+1);
  ProcessOption(name.c_str(),val.c_str());
}

void LoadConfig(const char* filename)
{
  StrList cfg;
  LoadFile(filename,cfg);
  for(int i=0;i<cfg.size();i++)
  {
    if(cfg[i].length()==0)continue;
    if(cfg[i][0]=='#')continue;
    ParseOptionLine(cfg[i]);
  }
}

int main(int argc,char* argv[])
{
  if(argc==1)
  {
    printf("usage: %s inifile [param=arg ...]\n",argv[0]);
    return -1;
  }
  Logger::Init();
  try{
    LoadConfig(argv[1]);

    for(int i=2;i<argc;i++)ParseOptionLine(argv[i]);

    SmeConfig cfg;

    cfg.host=host;
    cfg.port=port;
    cfg.sid=systemId;
    cfg.timeOut=10;
    cfg.smppTimeOut=timeOut;
    cfg.password=password;
    MyListener lst;

    StrList msgs;
    StrList words;
    StrList sources;
    StrList dests;

    if(sourcesFile.length())LoadFile(sourcesFile,sources);
    else if(source.length()>0)sources.push_back(source);
    else throw Exception("Source address undefined");

    if(destinationsFile.length())LoadFile(destinationsFile,dests);
    else if (destination.length())dests.push_back(destination);
    else throw Exception("Destination address undefined");

    if(messagesFile.length() && wordsFile.length())
    {
      throw Exception("Only messagesFile or wordsFile can be specified");
    }
    int messageMode=0; //messages;
    if(messagesFile.length())
    {
      if(minLength || maxLength || minSegments || maxSegments)
      {
        fprintf(stderr,"Warning: Options minLength, maxLength, minSegments, maxSegments are only used in 'words' message mode");
      }
      LoadFile(messagesFile,msgs);
    }else if(message.length())
    {
      msgs.push_back(message);
    }else if(wordsFile.length())
    {
      if(!maxLength)
      {
        throw Exception("Max length must be specified in words mode");
      }
      if(maxSegments && !minSegments)
      {
        throw Exception("minSegments cannot be 0 if maxSegments specified");
      }
      LoadFile(wordsFile,words);
      if(words.size()==0)throw Exception("Words file is empty");
      messageMode=1;//words
    }

    int delay=1000/speed;

    SmppSession ss(cfg,&lst);
    SmppTransmitter *tr=ss.getSyncTransmitter();
    SmppTransmitter *atr=ss.getAsyncTransmitter();
    lst.setTrans(tr);
    try{
      ss.connect();
      PduSubmitSm sm;
      SMS s;
      char msc[]="123";
      char imsi[]="123";
      s.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);

      s.setValidTime(0);

      s.setArchivationRequested(false);

      s.setIntProperty(Tag::SMPP_ESM_CLASS,0);

      s.setEServiceType("XXX");
      sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);

      int cnt=0;
      time_t lasttime=time(NULL);
      time_t starttime=lasttime;
      int srcidx=0;
      int dstidx=0;
      int msgidx=0;
      string wordsTemp;
      while(!stopped)
      {
        hrtime_t msgstart=gethrtime();

        s.setDestinationAddress(dests[dstidx].c_str());
        s.setOriginatingAddress(sources[srcidx].c_str());

        string* message;
        if(messageMode==0)
        {
          message=&msgs[msgidx];
        }else
        {
          message=&wordsTemp;
          int len=minLength+((double)rand()/RAND_MAX)*(maxLength-minLength);
          wordsTemp=words[rand()%words.size()];
          while(wordsTemp.length()<len)
          {
            wordsTemp+=' ';
            wordsTemp+=words[rand()%words.size()];
          }
        }

        if(hasHighBit(message->c_str(),message->length()))
        {
          std::vector<short> tmp(message->length());
          ConvertMultibyteToUCS2(message->c_str(), message->length(),
                                        &tmp[0], message->length()*2,
                                        CONV_ENCODING_CP1251);

          s.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::UCS2);
          s.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,(char*)&tmp[0],message->length()*2);
        }else
        {
          s.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::LATIN1);
          s.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,message->c_str(),message->length());
        }
        s.setIntProperty(Tag::SMPP_SM_LENGTH,0);
        fillSmppPduFromSms(&sm,&s);
        atr->submit(sm);

        cnt++;
        msgidx++;
        if(msgidx>=msgs.size())msgidx=0;
        srcidx++;
        if(srcidx>=sources.size())srcidx=0;
        dstidx++;
        if(dstidx>=dests.size())dstidx=0;

        //slev.Wait(delay);
        hrtime_t msgproc=gethrtime()-msgstart;
        millisleep(delay-msgproc/1000000);

        time_t now=time(NULL);
        if((cnt%500)==0 || now-lasttime>5)
        {
          {
            MutexGuard g(cntMutex);
            printf("ut:%d sbm:%d sok:%d serr:%d recv:%d avgsp: %lf lstsp:%lf\n",
              now-starttime,cnt,sokcnt,serrcnt,reccnt,
              (double)sokcnt/(now-starttime),
              (double)sok_time_cnt.Get()/30.0
              );
          }
          fflush(stdout);
          lasttime=time(NULL);
        }

      }
    }
    catch(std::exception& e)
    {
      printf("Exception: %s\n",e.what());
    }
    catch(...)
    {
      printf("unknown exception\n");
    }
    ss.close();
  }catch(exception& e)
  {
    printf("Exception: %s\n",e.what());
  }
  catch(...)
  {
    printf("unknown exception\n");
  }
  printf("Exiting\n");
  return 0;
}
