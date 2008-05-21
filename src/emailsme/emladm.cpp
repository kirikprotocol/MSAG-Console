#include <stdio.h>
#include "emailsme/AbonentProfile.hpp"
#include <string>
#include <map>
#include "util/Exception.hpp"
#include "util/BufferSerialization.hpp"
#include "core/network/Socket.hpp"
#include "sms/sms_util.h"

using namespace smsc::sms;
using namespace smsc::emailsme;
using namespace smsc::util;
using namespace smsc::core::network;

typedef std::map<std::string,std::string> ParamsMap;

int getParamAsInt(const ParamsMap& m,const char* name)
{
  ParamsMap::const_iterator it=m.find(name);
  if(it==m.end())
  {
    throw smsc::util::Exception("Parameter %s not found",name);
  }
  return atoi(it->second.c_str());
}

int getOptParamAsInt(const ParamsMap& m,const char* name,int defVal=0)
{
  try{
    return getParamAsInt(m,name);
  }catch(...)
  {
  }
  return defVal;
}

std::string getParamAsString(const ParamsMap& m,const char* name)
{
  ParamsMap::const_iterator it=m.find(name);
  if(it==m.end())
  {
    throw smsc::util::Exception("Parameter %s not found",name);
  }
  return it->second;
}

std::string getOptParamAsString(const ParamsMap& m,const char* name,const char* defVal="")
{
  try{
    return getParamAsString(m,name);
  }catch(...){}
  return defVal;
}

void ParseParams(ParamsMap& m,const char* str)
{
  const char* ptr1=str;
  for(;;)
  {
    const char* ptr2=strchr(ptr1,',');
    const char* eq=strchr(ptr1,'=');
    if(!eq)break;
    std::string nm;
    nm.assign(ptr1,eq-ptr1);
    std::string vl;
    while(ptr2 && ptr2<eq)ptr2=strchr(ptr2+1,',');
    if(ptr2)
    {
      vl.assign(eq+1,ptr2-(eq+1));
    }else
    {
      vl=eq+1;
    }
    m.insert(ParamsMap::value_type(nm,vl));
    if(!ptr2)break;
    ptr1=ptr2+1;
  }
}

char host[256];
int port;
Socket s;
bool ConnectSocket()
{
  if(s.Init(host,port,0)==-1)
  {
    printf("Socket init failed at %s:%d\n",host,port);
    return false;
  }
  if(s.Connect()==-1)
  {
    printf("Failed to connect to %s:%d\n",host,port);
    return false;
  }
  return true;
}

void DumpProfile(const AbonentProfile& p)
{
  printf("addr=%s\n",p.addr.toString().c_str());
  printf("user=%s\n",p.user.c_str());
  if(p.forwardEmail.length())
  {
    printf("forward email=%s\n",p.forwardEmail.c_str());
  }
  if(p.realName.length())
  {
    printf("real name=%s\n",p.realName.c_str());
  }
  printf("numbermap:%s\n",p.numberMap?"on":"off");
  printf("limit:%d%c\n",p.limitValue,p.ltype==ltDay?'D':p.ltype==ltWeek?'W':'M');
  printf("limitCountGsm2Eml:%d\n",p.limitCountGsm2Eml);
  printf("limitCountEml2Gsm:%d\n",p.limitCountEml2Gsm);
}


int main(int argc,char* argv[])
{
  if(argc!=4)
  {
    printf("Usage: %s host:port command args\n",argv[0]);
    printf("Commands:\n"
    "update addr={address},user={username},limit={number}{d|w|m}[,forwardEmail={email}][,realName={realname}]\n"
    "delete addr={address}\n"
    "getbyaddr addr={address}\n"
    "getbyuser user={username}\n"
    );
    return -1;
  }
  if(sscanf(argv[1],"%[^:]:%d",host,&port)!=2)
  {
    printf("Invalid host:port entry:%s\n",argv[1]);
    return -1;
  }
  std::string command=argv[2];
  ParamsMap m;
  ParseParams(m,argv[3]);
  try{
    if(command=="update")
    {
      smsc::emailsme::AbonentProfile p;
      p.addr=getParamAsString(m,"addr").c_str();
      p.user=getParamAsString(m,"user");
      p.forwardEmail=getOptParamAsString(m,"fwdEmail");
      p.realName=getOptParamAsString(m,"realName");
      std::string limit=getParamAsString(m,"limit");
      char lt;
      if(!sscanf(limit.c_str(),"%d%c",&p.limitValue,&lt))
      {
        printf("Invalid limit value:%s\n",limit.c_str());
        return -1;
      }
      switch(lt)
      {
        case 'd':p.ltype=ltDay;break;
        case 'w':p.ltype=ltWeek;break;
        case 'm':p.ltype=ltMonth;break;
        default:
        {
          printf("Unknown limit type:%c\n",lt);
          return -1;
        }
      }
      p.numberMap=true;
      p.limitDate=0;
      p.limitCountGsm2Eml=0;
      p.limitCountEml2Gsm=0;
      SerializationBuffer sb;
      sb.WriteNetInt32(1);
      p.Write(sb);
      if(!ConnectSocket())return -1;
      int pktSz=htonl(sb.getPos());
      s.WriteAll((char*)&pktSz,4);
      s.WriteAll(sb.getBuffer(),sb.getPos());
      int rv;
      s.ReadAll((char*)&rv,4);
      rv=ntohl(rv);
      printf("Result:%d\n",rv);
    }else if(command=="delete")
    {
      SerializationBuffer sb;
      sb.WriteNetInt32(2);
      WriteAddress(sb,getParamAsString(m,"addr").c_str());
      if(!ConnectSocket())return -1;
      int pktSz=htonl(sb.getPos());
      s.WriteAll((char*)&pktSz,4);
      s.WriteAll(sb.getBuffer(),sb.getPos());
      int rv;
      s.ReadAll((char*)&rv,4);
      rv=ntohl(rv);
      printf("Result:%d\n",rv);
    }else if(command=="getbyaddr")
    {
      SerializationBuffer sb;
      sb.WriteNetInt32(3);
      WriteAddress(sb,getParamAsString(m,"addr").c_str());
      if(!ConnectSocket())return -1;
      int pktSz=htonl(sb.getPos());
      s.WriteAll((char*)&pktSz,4);
      s.WriteAll(sb.getBuffer(),sb.getPos());
      int rv;
      s.ReadAll((char*)&rv,4);
      rv=ntohl(rv);
      if(rv>0)
      {
        char* buf=new char[rv];
        s.ReadAll(buf,rv);
        SerializationBuffer sb2;
        sb2.setExternalBuffer(buf,rv);
        AbonentProfile p;
        p.Read(sb2);
        DumpProfile(p);
      }else
      {
        printf("Profile not found\n");
      }
    }else if(command=="getbyuser")
    {
      SerializationBuffer sb;
      sb.WriteNetInt32(4);
      WriteString(sb,getParamAsString(m,"user").c_str());
      if(!ConnectSocket())return -1;
      int pktSz=htonl(sb.getPos());
      s.WriteAll((char*)&pktSz,4);
      s.WriteAll(sb.getBuffer(),sb.getPos());
      int rv;
      s.ReadAll((char*)&rv,4);
      rv=ntohl(rv);
      if(rv>0)
      {
        char* buf=new char[rv];
        s.ReadAll(buf,rv);
        SerializationBuffer sb2;
        sb2.setExternalBuffer(buf,rv);
        AbonentProfile p;
        p.Read(sb2);
        DumpProfile(p);
      }else
      {
        printf("Profile not found\n");
      }
    }else
    {
      printf("Unknown command:%s\n",command.c_str());
    }
  }catch(std::exception& e)
  {
    printf("Exception:%s\n",e.what());
  }
  return 0;
}
