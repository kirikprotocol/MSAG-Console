#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <string.h>

#include "core/network/Socket.hpp"
#include "core/buffers/String.hpp"
#include "emailsme/statuscodes.h"

#define DEBUG

#ifdef DEBUG
#define LOG(x) printf x
#else
#define LOG(x)
#endif

using smsc::core::network::Socket;
using smsc::core::buffers::String;

String host="";
int port=0;
int maxsize=1024*1024;

bool ReadConfig(const char* cfgfile)
{
  FILE *f=fopen(cfgfile,"rt");
  if(!f)return false;
  char strbuf[1024];
  while(fgets(strbuf,sizeof(strbuf),f))
  {
    if(strbuf[0]=='#')continue;
    if(strbuf[strlen(strbuf)-1]==0x0a)strbuf[strlen(strbuf)-1]=0;
    if(!strbuf[0])continue;
    char *val=strchr(strbuf,'=');
    if(!val)
    {
      LOG(("Invalid config string %s\n",strbuf));
      continue;
    }
    *val=0;
    val++;
    if(!strcmp(strbuf,"host"))
    {
      host=val;
      continue;
    }
    if(!strcmp(strbuf,"port"))
    {
      port=atoi(val);
      continue;
    }
    if(!strcmp(strbuf,"maxsize"))
    {
      maxsize=atoi(val);
      continue;
    }
    LOG(("Unknown config parameter %s\n",strbuf));
  }
  fclose(f);
  if(host=="" || port==0)return false;
  return true;
}

int main(int argc,char* argv[])
{
  if(argc==0)
  {
    LOG(("Usage: mdAgent path_to_config_file\n"));
    exit(EX_USAGE);
  }
  if(!ReadConfig(argv[1]))
  {
    LOG(("Failed to read config"));
    exit(EX_USAGE);
  }
  Socket s;
  if(s.Init(host,port,0)==-1 || s.Connect()==-1)
  {
    LOG(("Failed to connect to %s:%d\n",host.Str(),port));
    exit(EX_UNAVAILABLE);
  };
  String msg;
  char buf[1024];
  int rd;
  while((rd=read(0,buf,sizeof(buf)))>0)
  {
    msg.Concat(buf,0,rd);
    if(msg.Length()>maxsize)
    {
      exit(EX_CANTCREAT);
    }
  }
  if(rd==-1)
  {
    LOG(("Read error"));
    exit(EX_IOERR);
  }
  int len=htonl(msg.Length());
  if(s.WriteAll(&len,4)==-1)
  {
    LOG(("Network error"));
    exit(EX_IOERR);
  }
  if(s.WriteAll(msg.Str(),msg.Length())==-1)
  {
    LOG(("Network error"));
    exit(EX_IOERR);
  };
  int code;
  if(s.Read((char*)&code,4)==-1)
  {
    LOG(("Network error"));
    exit(EX_IOERR);
  };
  code=ntohl(code);
  using namespace smsc::emailsme;
  switch(code)
  {
    case StatusCodes::STATUS_CODE_UNKNOWNERROR:
    case StatusCodes::STATUS_CODE_INVALIDMSG:exit(EX_IOERR);
    case StatusCodes::STATUS_CODE_NOUSER:exit(EX_NOUSER);
    case StatusCodes::STATUS_CODE_TEMPORARYERROR:exit(EX_TEMPFAIL);
  }
  exit(EX_OK); // message sent successfully
}
