#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include "core/threads/Thread.cpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/buffers/File.hpp"
//#include "logger/Logger.h"
#include <syslog.h>
#include <stdarg.h>

namespace smsc{
namespace snmp{
namespace smscsnmp{

class CsvFileHandle{
public:
  CsvFileHandle(const char* argLocation,const char* argPrefix):location(argLocation),prefix(argPrefix)
  {
    opened=false;
  }
  void Write(const char* str)
  {
    smsc::core::synchronization::MutexGuard mg(mtx);
    if(!opened)
    {
      time_t t=time(NULL);
      tm t2;
      gmtime_r(&t,&t2);
      char fn[1024];
      snprintf(fn,sizeof(fn),"%s/%04d%02d%02d_%02d%02d%02d.%s.lst",location.c_str(),
        t2.tm_year+1900,t2.tm_mon+1,t2.tm_mday,t2.tm_hour,t2.tm_min,t2.tm_sec,prefix.c_str());
      f.WOpen(fn);
      const char* header="SUBMIT_TIME,ALARM_ID,ALARMCATEGORY,SEVERITY,TEXT\n";
      f.Write(header,strlen(header));
      opened=true;
    }
    f.Write(str,strlen(str));
    f.Flush();
    Close();
  }

  void Close()
  {
    smsc::core::synchronization::MutexGuard mg(mtx);
    if(opened)
    {
      std::string fn=f.getFileName();
      fn.erase(fn.length()-3);
      fn+="csv";
      f.Rename(fn.c_str());
      f.Close();
    }
    opened=false;
  }

protected:
  smsc::core::buffers::File f;
  bool opened;
  smsc::core::synchronization::Mutex mtx;
  std::string location;
  std::string prefix;
};

/*
class CsvRollingThread:public smsc::core::threads::Thread{
public:
  CsvRollingThread(CsvFileHandle& fh):csvFile(fh)
  {
  }
  ~CsvRollingThread()
  {
    Stop();
  }
  int Execute()
  {
    while(!isStopping)
    {
      mon.wait(5*60*1000);
      try{
        Roll();
      }catch(std::exception& e)
      {
        syslog(LOG_ERR,"Failed to roll csv file:%s",e.what());
      }
    }
    return 0;
  }
  void Stop()
  {
    smsc::core::synchronization::MutexGuard mg(mon);
    isStopping=true;
    mon.notify();
  }
protected:
  smsc::core::synchronization::EventMonitor mon;
  bool isStopping;
  CsvFileHandle& csvFile;
  void Roll()
  {
    csvFile.Close();
  }
};
*/

const char* getLogLocation()
{
  if(!smsc::core::buffers::File::Exists("/etc/smscsnmp.conf"))return ".";
  smsc::core::buffers::File f;
  try{
    f.ROpen("/etc/smscsnmp.conf");
  }catch(...)
  {
    return ".";
  }
  static std::string logLoc;
  if(f.ReadLine(logLoc))
  {
    return logLoc.c_str();
  }else
  {
    return ".";
  }
}

struct SysLogTracker
{
  SysLogTracker()
  {
    openlog("smscsnmp:",0,LOG_USER);
  }
  ~SysLogTracker()
  {
    closelog();
  }
}sli;

extern "C" void SnmpTrap(const char* prefix,const char* obj,const char* alarmId,int svrt,const char* fmt,...)
{
  try{
    static CsvFileHandle fh(getLogLocation(),prefix);
    /*
    static CsvRollingThread crt(fh);
    static bool crtStarted=false;
    if(!crtStarted)
    {
      crt.Start();
      crtStarted=true;
    }
    */
    time_t t=time(NULL);
    tm t2;
    gmtime_r(&t,&t2);
    char msg[1024]={0,};
    va_list args;
    va_start (args, fmt);
    vsnprintf (msg,sizeof(msg),fmt, args);
    va_end (args);

    char buf[2048];
    snprintf(buf,sizeof(buf),
      "%02d.%02d.%04d %02d:%02d:%02d,"
      "\"%s\",\"%s\",%d,\"%s\"\n",
      t2.tm_mday,t2.tm_mon+1,t2.tm_year+1900,t2.tm_hour,t2.tm_min,t2.tm_sec,
      alarmId,obj,svrt,msg);
    fh.Write(buf);
    char cmd[4096];
    snprintf(cmd,sizeof(cmd),
      "snmptrap -v 2c -c ussdc  traphost '' SIBINCO-SMSC-MIB::smscAlertFFMR "
      "SIBINCO-SMSC-MIB::alertSeverity i %d "
      "SIBINCO-SMSC-MIB::alertMessage s '%s' "
      "SIBINCO-SMSC-MIB::alertObjCategory s '%s' "
      "SIBINCO-SMSC-MIB::alertId s '%s'",
      svrt,msg,obj,alarmId);
    syslog(LOG_INFO,"exec:%s",cmd);
    system(cmd);
  }catch(std::exception& e)
  {
    syslog(LOG_ERR,"SnmpTrap::Exception:%s",e.what());
  }
}

}//smscsnmp
}//snmp
}//smsc