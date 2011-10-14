#ifdef _WIN32
#include <winsock2.h>
#endif
#include "sme/ClusterSmppSession.hpp"
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
#include <stdlib.h>
#include <signal.h>
#include <map>
#include <string.h>
#include <errno.h>

using namespace smsc::sms;
using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;
using namespace smsc::core::buffers;
using namespace smsc::core::synchronization;
using namespace smsc::logger;

using namespace std;


int stopped=0;
int disconnected=0;

Mutex cntMutex;

int sokcnt=0;
int serrcnt=0;
int reccnt=0;

int tempErrProb=0;
bool trackingMode=false;
FILE* sentFile=0;
FILE* receivedFile=0;

typedef map<int,string> SeqToStrMap;

SeqToStrMap seqToStr;

Mutex seqMtx;

void addSeqStr(int seq, const string& str)
{
  MutexGuard mg(seqMtx);
  seqToStr[seq]=str;
}

string getSeqStr(int seq)
{
  MutexGuard mg(seqMtx);
  SeqToStrMap::iterator it=seqToStr.find(seq);
  if(it==seqToStr.end())
  {
    return "";
  }
  string rv=it->second;
  seqToStr.erase(it);
  return rv;
}


int rnd(int mx)
{
  return mx*drand48();
}

bool chance(int prc)
{
  return rnd(100)<prc;
}

TimeSlotCounter<> sok_time_cnt(30,100);

class MyListener:public SmppPduEventListener{
public:
  virtual ~MyListener(){}
  void handleEvent(SmppHeader *pdu)
  {
    if(pdu->get_commandId()==SmppCommandSet::DELIVERY_SM)
    {
      //char buf[256];
      static Logger* log=Logger::getInstance("smpp.in");
      smsc_log_info(log,"received delivery_sm seq=%d",pdu->get_sequenceNumber());
      PduDeliverySmResp resp;
      resp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
      resp.set_messageId("");
      resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
      if(chance(tempErrProb))
      {
        resp.get_header().set_commandStatus(SmppStatusSet::ESME_RX_T_APPN);
      }else
      {
        resp.get_header().set_commandStatus(SmppStatusSet::ESME_ROK);
      }
      trans->sendDeliverySmResp(resp);
      if(trackingMode && receivedFile && resp.get_header().get_commandStatus()==SmppStatusSet::ESME_ROK)
      {
        PduDeliverySm* dlv=(PduDeliverySm*)pdu;
        const char* body;
        int len;
        if(dlv->get_optional().has_messagePayload())
        {
          body=dlv->get_optional().get_messagePayload();
          len=dlv->get_optional().size_messagePayload();
        }else
        {
          body=dlv->get_message().get_shortMessage();
          len=dlv->get_message().size_shortMessage();
        }
        fprintf(receivedFile,"%.*s\n",len,body);
      }
      {
        MutexGuard g(cntMutex);
        reccnt++;
      }
    }else
    if(pdu->get_commandId()==SmppCommandSet::SUBMIT_SM_RESP)
    {
      __trace2__("received submit sm resp:%x, seq=%d",pdu->get_commandStatus(),pdu->get_sequenceNumber());
      //printf("\nReceived async submit sm resp:%d\n",pdu->get_commandStatus());
      MutexGuard g(cntMutex);

      if(pdu->get_commandStatus()==SmppStatusSet::ESME_ROK && trackingMode && sentFile)
      {
        string msg=getSeqStr(pdu->get_sequenceNumber());
        if(!msg.empty())
        {
          //fwrite(msg.c_str(),msg.length(),1,sentFile);
          PduSubmitSmResp* resp=(PduSubmitSmResp*)pdu;
          fprintf(sentFile,"%s:%s\n",msg.c_str(),resp->get_messageId());
        }
      }

      if(pdu->get_commandStatus()==SmppStatusSet::ESME_ROK)
      {
        sokcnt++;
        sok_time_cnt.Inc();
      }else
      {
        serrcnt++;
      }
    }else
    if(pdu->get_commandId()==SmppCommandSet::DATA_SM)
    {
      PduDataSmResp resp;
      resp.get_header().set_commandId(SmppCommandSet::DATA_SM_RESP);
      resp.set_messageId("");
      resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
      if(chance(tempErrProb))
      {
        resp.get_header().set_commandStatus(SmppStatusSet::ESME_RX_T_APPN);
      }else
      {
        resp.get_header().set_commandStatus(SmppStatusSet::ESME_ROK);
      }
      trans->sendDataSmResp(resp);
      if(trackingMode && receivedFile && resp.get_header().get_commandStatus()==SmppStatusSet::ESME_ROK)
      {
        PduDataSm* dlv=(PduDataSm*)pdu;
        const char* body;
        int len;
        if(dlv->get_optional().has_messagePayload())
        {
          body=dlv->get_optional().get_messagePayload();
          len=dlv->get_optional().size_messagePayload();
        }
        fprintf(receivedFile,"%.*s\n",len,body);
      }
      {
        MutexGuard g(cntMutex);
        reccnt++;
      }
    }
    disposePdu(pdu);
  }
  void handleError(int errorCode)
  {
    printf("Error handled:%d!\n",errorCode);
    disconnected=1;
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

string host1;
int port1=9001;
string host2;
int port2=9001;
string systemId;
string password;
int timeOut=60;
int speed=10;
int maxCount=0;
int sleepOnExit=0;
int maxTime=0;
string source;
string sourcesFile;
string destination;
string destinationsFile;
string destinationsRange;
string message;
string messagesFile;
string wordsFile;

string receivedFileName;
string sentFileName;

int minLength=0;
int maxLength=0;

int minSegments=0;
int maxSegments=0;

int probIncompleteMessage=0;
int probUssdMessage=0;
int probInvalidUssdMessage=0;

int probDefault=100;
int probDatagram=0;
int probForward=0;
int probStoreAndForward=0;
int replaceIfPresent=0;
int maxSimultaneousMultipart=5;
bool autoReconnect=false;

int setDpf=-1;

bool doPartitionSms=false;

Option options[]=
{
  {"host1",'s',&host1},
  {"port1",'i',&port1},
  {"host2",'s',&host2},
  {"port2",'i',&port2},
  {"systemId",'s',&systemId},
  {"password",'s',&password},
  {"timeOut",'i',&timeOut},
  {"speed",'i',&speed},
  {"maxCount",'i',&maxCount},
  {"maxTime",'i',&maxTime},
  {"source",'s',&source},
  {"sourcesFile",'s',&sourcesFile},
  {"destination",'s',&destination},
  {"destinationsRange",'s',&destinationsRange},
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
  {"probInvalidUssdMessage",'i',&probInvalidUssdMessage},
  {"probDefault",'i',&probDefault},
  {"probDatagram",'i',&probDatagram},
  {"probForward",'i',&probForward},
  {"probStoreAndForward",'i',&probStoreAndForward},
  {"setdpf",'i',&setDpf},
  {"replaceIfPresent",'i',&replaceIfPresent},
  {"partitionSms",'b',&doPartitionSms},
  {"maxSimultaneousMultipart",'i',&maxSimultaneousMultipart},
  {"trackingMode",'b',&trackingMode},
  {"probTempErr",'i',&tempErrProb},
  {"receivedFileName",'s',&receivedFileName},
  {"sentFileName",'s',&sentFileName},
  {"autoReconnect",'b',&autoReconnect},
  {"sleepOnExit",'i',&sleepOnExit},
};

int optionsCount=sizeof(options)/sizeof(options[0]);

bool atob(const char* val)
{
  return !strcmp(val,"1") || !strcmp(val,"yes") || !strcmp(val,"on") || !strcmp(val,"true");
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
  FILE *f=fopen(filename.c_str(),"rb");
  if(!f)throw Exception("Faield to open file %s",filename.c_str());
  fseek(f,0,SEEK_END);
  int sz=(int)ftell(f);
  fseek(f,0,SEEK_SET);
  char* buf=new char[sz+1];
  fread(buf,sz,1,f);
  buf[sz]=0;
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
  if(pos==string::npos)throw Exception("Invalid option line: '%s'(%d)",line.c_str(),line.length());
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

extern "C" void onSigInt(int)
{
  stopped=1;
}

int main(int argc,char* argv[])
{
  srand((int)time(NULL));
  if(argc==1)
  {
    printf("usage: %s inifile [param=arg ...]\n",argv[0]);
    return -1;
  }
  sigset(SIGINT,onSigInt);
  sigignore(SIGPIPE);
  char var[]="SMSC_LOGGER_PROPERTIES=flooder.properties";
  putenv(var);
  Logger::Init();
  try{
    LoadConfig(argv[1]);

    for(int i=2;i<argc;i++)ParseOptionLine(argv[i]);

    std::vector<SmeConfig> cfgs;

    cfgs.resize(2);

    cfgs[0].host=host1;
    cfgs[0].port=port1;
    cfgs[0].sid=systemId;
    cfgs[0].timeOut=10;
    cfgs[0].smppTimeOut=timeOut;
    cfgs[0].password=password;
    cfgs[1]=cfgs[0];
    cfgs[1].host=host2;
    cfgs[1].port=port2;
    MyListener lst;

    StrList msgs;
    StrList words;
    StrList sources;
    StrList dests;

    Address startAddr,endAddr,curAddr;

    if(sourcesFile.length())LoadFile(sourcesFile,sources);
    else if(source.length()>0)sources.push_back(source);
    else throw Exception("Source address undefined");

    if(destinationsFile.length())
    {
      LoadFile(destinationsFile,dests);
    }
    else if (destination.length())
    {
      dests.push_back(destination);
    }
    else if(destinationsRange.length())
    {
      string::size_type pos=destinationsRange.find('-');
      if(pos==string::npos)
      {
        throw Exception("Invalid destinations range");
      }
      startAddr=destinationsRange.substr(0,pos).c_str();
      endAddr=destinationsRange.substr(pos+1).c_str();
      curAddr=startAddr;
    }else
    {
      throw Exception("Destination address undefined");
    }

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
    }else
    {
      msgs.push_back(message);
    }

    if(probDefault+probDatagram+probForward+probStoreAndForward==0)
    {
      throw Exception("probDefault+probDatagram+probForward+probStoreAndForward==0");
    }

    int delay=1000000/speed;

    time_t startTime=time(NULL);

    ClusterSmppSession ss(cfgs,&lst);
    SmppTransmitter *tr=ss.getSyncTransmitter();
    SmppTransmitter *atr=ss.getAsyncTransmitter();
    lst.setTrans(atr);
    try{
      ss.connect();
      SMS s;
      char msc[]="123";
      char imsi[]="123";
      s.setOriginatingDescriptor((uint8_t)strlen(msc),msc,(uint8_t)strlen(imsi),imsi,1);

      s.setValidTime(0);

      s.setArchivationRequested(false);
      s.setIntProperty(Tag::SMSC_DSTCODEPAGE,8);
      s.setIntProperty(Tag::SMSC_UDH_CONCAT,1);

      s.setEServiceType("XXX");

      std::list<SMS> mparts;
      std::list<SMS>::iterator current=mparts.end();
      int mpartsSize=0;
      bool lastWasPart=false;

      int cnt=0;
      time_t lasttime=time(NULL);
      time_t starttime=lasttime;
      int srcidx=0;
      int dstidx=0;
      int msgidx=0;
      int mr=0;
      string wordsTemp;

      if(trackingMode)
      {
        s.setOriginatingAddress(source.c_str());
        if(!destination.empty())
        {
          s.setDestinationAddress(destination.c_str());
        }
        if(!receivedFileName.empty())
        {
          receivedFile=fopen(receivedFileName.c_str(),"w+");
          if(!receivedFile)
          {
            printf("failed to open %s:%s\n",receivedFileName.c_str(),strerror(errno));
          }
        }
        if(!sentFileName.empty())
        {
          sentFile=fopen(sentFileName.c_str(),"w+");
          if(!sentFile)
          {
            printf("failed to open %s:%s\n",sentFileName.c_str(),strerror(errno));
          }
        }
      }

      char strbuf[32];

      int overdelay=0;
      while(!stopped)
      {
        if(disconnected)
        {
          if(!autoReconnect)
          {
            break;
          }
          sleep(1);
          ss.close();
          try{
            ss.connect();
            disconnected=0;
          }catch(...)
          {
            continue;
          }
        }
        hrtime_t msgstart=gethrtime();
        s.setIntProperty(Tag::SMPP_ESM_CLASS,0);
        s.dropProperty(Tag::SMPP_SHORT_MESSAGE);
        s.setIntProperty(Tag::SMPP_SM_LENGTH,0);

        if(trackingMode)
        {
          int len=sprintf(strbuf,"%09d",cnt);
          s.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::LATIN1);
          s.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,strbuf,(unsigned)len);
          if(destination.empty())
          {
            if(curAddr==endAddr)
            {
              curAddr=startAddr;
            }
            s.setDestinationAddress(curAddr);
            uint64_t addrVal;
            sscanf(curAddr.value,"%llu",&addrVal);
            addrVal++;
            sprintf(curAddr.value,"%0*llu",(int)strlen(curAddr.value),addrVal);
          }
        }else if(!doPartitionSms || mpartsSize<maxSimultaneousMultipart)
        {
          if(!dests.empty())
          {
            s.setDestinationAddress(dests[dstidx].c_str());
          }else
          {
            if(curAddr==endAddr)
            {
              curAddr=startAddr;
            }
            s.setDestinationAddress(curAddr);
            uint64_t addrVal;
            sscanf(curAddr.value,"%llu",&addrVal);
            addrVal++;
            sprintf(curAddr.value,"%0*llu",(int)strlen(curAddr.value),addrVal);
          }
          s.setOriginatingAddress(sources[srcidx].c_str());

          string* msgptr;
          if(messageMode==0)
          {
            msgptr=&msgs[msgidx];
          }else
          {
            msgptr=&wordsTemp;
            int len=minLength+((double)rand()/RAND_MAX)*(maxLength-minLength);
            wordsTemp=words[rand()%words.size()];
            while(wordsTemp.length()<len)
            {
              wordsTemp+=' ';
              wordsTemp+=words[rand()%words.size()];
            }
          }

          if(msgptr->length() && hasHighBit(msgptr->c_str(),msgptr->length()))
          {
            std::vector<short> tmp(msgptr->length());
            ConvertMultibyteToUCS2(msgptr->c_str(), msgptr->length(),
                &tmp[0], msgptr->length()*2,
                CONV_ENCODING_CP1251);

            s.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::UCS2);
            s.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,(char*)&tmp[0],(unsigned)msgptr->length()*2);
          }else
          {
            s.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::LATIN1);
            s.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,msgptr->c_str(),(unsigned)msgptr->length());
          }
          s.setIntProperty(Tag::SMPP_SM_LENGTH,0);
          s.setIntProperty(Tag::SMSC_DSTCODEPAGE,8);
          if(doPartitionSms)
          {
            s.setConcatSeqNum(0);
            mparts.push_back(s);
            int pres=partitionSms(&mparts.back());
            if(pres!=psSingle && pres!=psMultiple)
            {
              __warning2__("pres result:%d!!!",pres);
            }
            if(current==mparts.end())
            {
              current=mparts.begin();
            }
            mpartsSize++;
            msgidx++;
          }
        }

        {
          int mode=rand()%(probDefault+probDatagram+probForward+probStoreAndForward);
          if(mode<probDefault || doPartitionSms)
          {
            s.setIntProperty(Tag::SMPP_ESM_CLASS,s.getIntProperty(Tag::SMPP_ESM_CLASS)&(~3));
          }else if(mode<probDefault+probDatagram)
          {
            s.setIntProperty(Tag::SMPP_ESM_CLASS,(s.getIntProperty(Tag::SMPP_ESM_CLASS)&(~3))|1);
          }else if(mode<probDefault+probDatagram+probForward)
          {
            s.setIntProperty(Tag::SMPP_ESM_CLASS,(s.getIntProperty(Tag::SMPP_ESM_CLASS)&(~3))|2);
          }else
          {
            s.setIntProperty(Tag::SMPP_ESM_CLASS,(s.getIntProperty(Tag::SMPP_ESM_CLASS)&(~3))|3);
          }
        }

        {
          int ussd=rand()%100;
          if(ussd<probUssdMessage && !doPartitionSms)
          {
            s.setIntProperty(Tag::SMPP_USSD_SERVICE_OP,1); // process ussd req ind
            s.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,mr++);
          }
        }

        if(setDpf>=0 && !doPartitionSms)
        {
          s.setIntProperty(Tag::SMPP_SET_DPF,setDpf);
        }

        if(replaceIfPresent && !doPartitionSms)
        {
          s.setIntProperty(Tag::SMPP_REPLACE_IF_PRESENT_FLAG,replaceIfPresent);
        }

        if(doPartitionSms && current->hasBinProperty(Tag::SMSC_CONCATINFO))
        {
          s=*current;
          extractSmsPart(&s,s.getConcatSeqNum());
          PduSubmitSm sm;
          sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
          fillSmppPduFromSms(&sm,&s,0);
          atr->submit(sm);
          current->setConcatSeqNum(current->getConcatSeqNum()+1);
          ConcatInfo* ci=(ConcatInfo*)current->getBinProperty(Tag::SMSC_CONCATINFO,0);
          if(current->getConcatSeqNum()==ci->num)
          {
            mparts.erase(current++);
            mpartsSize--;
          }else
          {
            current++;
          }
          if(current==mparts.end())
          {
            current=mparts.begin();
          }
        }else
        {
          if(doPartitionSms)
          {
            s=*current;
            mparts.erase(current++);
            if(current==mparts.end())
            {
              current=mparts.begin();
            }
            mpartsSize--;
          }
          PduSubmitSm sm;
          sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
          sm.get_header().set_sequenceNumber(ss.getNextSeq());
          fillSmppPduFromSms(&sm,&s,0);
          atr->submit(sm);
          if(trackingMode)
          {
            addSeqStr(sm.get_header().get_sequenceNumber(),strbuf);
          }
          msgidx++;
        }

        cnt++;
        if(msgidx>=msgs.size())msgidx=0;
        srcidx++;
        if(srcidx>=sources.size())srcidx=0;
        dstidx++;
        if(dstidx>=dests.size())dstidx=0;

        //slev.Wait(delay);

        time_t now=time(NULL);
        if((cnt%500)==0 || now-lasttime>5)
        {
          double sokt;
          {
            MutexGuard g(cntMutex);
            sokt=sok_time_cnt.Get()/30.0;
          }
          printf("ut:%d sbm:%d sok:%d serr:%d recv:%d avgsp: %lf lstsp:%lf\n",
                 (int)(now-starttime),cnt,sokcnt,serrcnt,reccnt,
                 (double)sokcnt/(now-starttime),
                 sokt
                 );
          fflush(stdout);
          lasttime=time(NULL);
        }
        if(maxCount>0 && cnt>=maxCount)
        {
          break;
        }
        if(maxTime>0 && now-startTime>=maxTime)
        {
          break;
        }

        hrtime_t msgproc=gethrtime()-msgstart;
        msgproc/=1000;
        if(delay>msgproc+overdelay)
        {
          int toSleep=(int)(delay-msgproc-overdelay);
          msgstart=gethrtime();
          millisleep(toSleep/1000);
          overdelay=(int)((gethrtime()-msgstart)/1000-toSleep);
        }else
        {
          overdelay-=delay-(int)msgproc;
          //__warning2__("overdelay:%d",overdelay);
          //if(overdelay<0)overdelay=0;
        }

      }
      while(trackingMode && !stopped)
      {
        tempErrProb=0;
        sleep(10);
        if(disconnected)
        {
          if(!autoReconnect)
          {
            break;
          }
          sleep(1);
          ss.close();
          try{
            ss.connect();
            disconnected=0;
          }catch(...)
          {
            continue;
          }
        }
      }
      if(sleepOnExit)
      {
        sleep(sleepOnExit);
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
    if(trackingMode)
    {
      if(receivedFile)
      {
        fclose(receivedFile);
      }
      if(sentFile)
      {
        fclose(sentFile);
      }
    }
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
