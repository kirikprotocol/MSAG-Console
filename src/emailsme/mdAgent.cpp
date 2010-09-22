#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <string.h>

#include "core/network/Socket.hpp"
#include "core/buffers/String.hpp"
#include "emailsme/statuscodes.h"

#define DEBUG

#ifdef DEBUG
#include <stdarg.h>
#define LOG(x) logger.logit x
#else
#define LOG(x)
#endif

using smsc::core::network::Socket;
using smsc::core::buffers::String;

String logFile="logs/mdAgent.log";
String host="";
int port=0;
int maxsize=1024*1024;
int maxlogsize=512*1024*1024;

#ifdef DEBUG
struct LogFile{
FILE *f;
LogFile()
{
  f=fopen(logFile,"at");
}
void reopen()
{
  fclose(f);
  f=fopen(logFile,"at");
}
void logit(const char* fmt,...)
{
  if(!f)return;
  va_list arglist;
  va_start(arglist,fmt);
  vfprintf(f,fmt,arglist);
  fflush(f);
  if(ftell(f)>maxlogsize)
  {
    fclose(f);
    String logFileBack=logFile+".1";
    rename(logFile.Str(),logFileBack.Str());
    f=fopen(logFile,"at");
  }
  va_end(arglist);
}
~LogFile()
{
  if(f)fclose(f);
}
}logger;
#endif

bool ReadConfig(const char* cfgfile)
{
  FILE *f=fopen(cfgfile,"rt");
  if(!f)return false;
  char strbuf[1024];
  while(fgets(strbuf,(int)sizeof(strbuf),f))
  {
    if(strbuf[0]=='#')continue;
    size_t l=strlen(strbuf);
    while(l>0 && (strbuf[l-1]==0x0a || strbuf[l-1]==0x0d))
    {
      strbuf[l-1]=0;
      l--;
    }
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
    if(!strcmp(strbuf,"logfile"))
    {
      logFile=val;
      logger.reopen();
      continue;
    }
    if(!strcmp(strbuf,"maxlogsize"))
    {
      maxlogsize=atoi(val);
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
  if(argc==1)
  {
    LOG(("Usage: mdAgent path_to_config_file\n"));
    return(EX_USAGE);
  }
  if(!ReadConfig(argv[1]))
  {
    LOG(("Failed to read config\n"));
    return(EX_USAGE);
  }
  String msg;
  char buf[1024];
  ssize_t rd;
  time_t t=time(0);
  LOG(("Start reading(%s) %s",argc==3?argv[2]:"",ctime(&t)));
  while((rd=read(0,buf,(int)sizeof(buf)))>0)
  {
    LOG(("Read %d bytes\n",rd));
    msg.Concat(buf,0,(int)rd);
    if(msg.Length()>maxsize)
    {
      LOG(("Too long message\n"));
      return(EX_CANTCREAT);
    }
    LOG(("Total read:%d\n",msg.Length()));
  }
  LOG(("Finished reading\n"));
  if(rd==-1)
  {
    LOG(("Read error\n"));
    return(EX_IOERR);
  }
  if(argc==3)
  {
    String xto;
    xto="X-To: ";
    xto+=argv[2];
    xto+="\n";
    msg.Insert(0,xto);
  }
  LOG(("Connecting to %s:%d\n",host.Str(),port));
  Socket s;
  if(s.Init(host,port,0)==-1 || s.Connect()==-1)
  {
    LOG(("Failed to connect to %s:%d\n",host.Str(),port));
    return(EX_UNAVAILABLE);
  };
  int len=htonl(msg.Length());
  if(s.WriteAll(&len,4)==-1)
  {
    LOG(("Network error\n"));
    return(EX_IOERR);
  }
  if(s.WriteAll(msg.Str(),msg.Length())==-1)
  {
    LOG(("Network error (write failed)\n"));
    return(EX_IOERR);
  };
  int code=0;
  if(s.ReadAll((char*)&code,4)==-1)
  {
    LOG(("Network error (status read failed)\n"));
    return(EX_IOERR);
  };
  code=ntohl(code);
  LOG(("Processed, code=%d\n",code));
  using namespace smsc::emailsme;
  switch(code)
  {
    case StatusCodes::STATUS_CODE_UNKNOWNERROR:
    case StatusCodes::STATUS_CODE_INVALIDMSG://return(EX_IOERR);
    case StatusCodes::STATUS_CODE_NOUSER:return(EX_NOUSER);
    case StatusCodes::STATUS_CODE_TEMPORARYERROR:return(EX_TEMPFAIL);
    case StatusCodes::STATUS_CODE_LIMIEXCEEDED:return(EX_UNAVAILABLE);
  }
  s.Abort();
  return(EX_OK); // message sent successfully
}
