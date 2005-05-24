#ifndef __SMSC_PROFILER_PROFILE_NOTIFIER_HPP__
#define __SMSC_PROFILER_PROFILE_NOTIFIER_HPP__

#include "core/threads/ThreadedTask.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/buffers/CyclicQueue.hpp"
#include "core/network/Socket.hpp"
#include "sms/sms.h"
#include "profiler.hpp"
#include <string>
#include "core/buffers/File.hpp"
#include "profiler-types.hpp"
#include "profile-notifier-iface.hpp"

namespace smsc{
namespace profiler{

using smsc::core::synchronization::EventMonitor;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

class ProfileNotifier:public smsc::core::threads::ThreadedTask, public ProfileNotifierInterface{
public:
  ProfileNotifier(const char* argHost,int argPort,const char* argDir):
    host(argHost),port(argPort),dir(argDir)
  {
    reconnectTimeout=10;
    log=smsc::logger::Logger::getInstance("prof.not");
  }
  const char* taskName()
  {
    return "ProfileNotifier";
  }
  int Execute()
  {
    MutexGuard mg(mon);
    smsc::core::network::Socket s;
    bool socketInit=false;
    bool socketOk=false;
    time_t lastReconnect=time(NULL);

    std::string filePath=dir;
    if(*filePath.rbegin()!='/')
    {
      filePath+='/';
    }
    filePath+="profile_changes.lst";

    if(s.Init(host.c_str(),port,0)==-1)
    {
      __warning2__("PN: Failed to init socket for %s:%d",host.c_str(),port);
    }else
    {
      socketInit=true;
    }
    if(socketInit && s.Connect()==-1)
    {
      __warning2__("PN: Failed to connect to %s:%d",host.c_str(),port);
    }else
    {
      debug2(log,"PN: connected ok to %s:%d",host.c_str(),port);
      socketOk=true;
    }

    debug2(log,"filepath=%s",filePath.c_str());

    File fTemp;
    bool isFileOpen=false;

    while(!isStopping)
    {
      while(!isStopping && queue.Count()==0)
      {
        time_t now=time(NULL);
        if(socketInit && !socketOk && now-lastReconnect>reconnectTimeout)
        {
          debug2(log,"PN: reconnecting at %d",now);
          lastReconnect=now;
          mon.Unlock();
          socketOk=s.Connect()!=-1;
          mon.Lock();
          if(socketOk)
          {
            debug1(log,"PN: Connect ok");
            if(File::Exists(filePath.c_str()))
            {
              if(isFileOpen)
              {
                fTemp.Close();
                isFileOpen=false;
              }
              File f;
              try{
                f.ROpen(filePath.c_str());
                std::string str;
                while(f.ReadLine(str))
                {
                  debug2(log,"sending %s",str.c_str());
                  str+='\n';
                  if(s.WriteAll(str.c_str(),str.length())==-1)
                  {
                    debug1(log,"write failed");
                    socketOk=false;
                    break;
                  }
                }
                if(socketOk)
                {
                  debug2(log,"unlink %s",filePath.c_str());
                  File::Unlink(filePath.c_str());
                }
              }catch(std::exception& e)
              {
                warn2(log,"Delayed records resend failed:%s",e.what());
              }
            }
          }else
          {
            debug1(log,"PN: Failed to reconnect (again).");
          }
        }
        mon.wait(1000);
      }
      if(!isStopping && queue.Count())
      {
        //
        std::string str;
        queue.Pop(str);
        str+='\n';
        if(socketOk)
        {
          debug2(log,"PN: writing to socket:%s",str.c_str());
          mon.Unlock();
          try{
            if(s.WriteAll(str.c_str(),str.length())==-1)
            {
              debug1(log,"PN: write to socket failed");
              socketOk=false;
            }
          }catch(...)
          {
          }
          mon.Lock();
        }
        if(!socketOk)
        {
          if(!isFileOpen)
          {
            try{
              fTemp.WOpen(filePath.c_str());
              isFileOpen=true;
            }catch(std::exception& e)
            {
              warn2(log,"PN: failed to open file for writing: %s, record lost: %s",e.what(),str.c_str());
            }
          }

          if(isFileOpen)
          {
            try{
              fTemp.Write(str.c_str(),str.length());
              fTemp.Flush();
            }catch(std::exception& e)
            {
              warn2(log,"PN: Failed to write to file %s, record lost: %s",filePath.c_str(),str.c_str());
              isFileOpen=false;
            }
          }
        }
      }
    }
    return 0;
  }

  virtual void AddChange(const smsc::sms::Address& addr,const Profile& prof)
  {
    std::string res=addr.toString();
    res+=':';
    res+="DC=";
    if(prof.codepage&DataCoding::UCS2)
      res+="UCS2";
    else
      res+="DEFAULT";
    res+=';';
    if(prof.codepage&ProfileCharsetOptions::UssdIn7Bit)
    {
      res+="USSDIN7BIT;";
    }
    switch(prof.reportoptions)
    {
      case ProfileReportOptions::ReportNone:res+="REPORT=NONE";break;
      case ProfileReportOptions::ReportFull:res+="REPORT=FULL";break;
      case ProfileReportOptions::ReportFinal:res+="REPORT=FINAL";break;
    }
    res+=';';

    res+="TRANSLIT=";
    res+=prof.translit?"YES":"NO";

    __trace2__("PN: received %s",res.c_str());

    MutexGuard mg(mon);
    queue.Push(res);
  }

protected:
  smsc::core::buffers::CyclicQueue<std::string> queue;
  EventMonitor mon;
  std::string host;
  int port;
  int reconnectTimeout;
  std::string dir;
  smsc::logger::Logger* log;
};

}//profiler
}//smsc

#endif
