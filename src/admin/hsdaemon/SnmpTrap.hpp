#ifndef __SMSC_ADMIN_HSDAEMON_SNMPTRAP_HPP__
#define __SMSC_ADMIN_HSDAEMON_SNMPTRAP_HPP__

#ifdef SNMP
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/buffers/File.hpp"
#include "logger/Logger.h"
#include "util/config/Manager.h"
#endif

namespace smsc{
namespace admin{
namespace hsdaemon{

#ifdef SNMP
class CsvFileHandle{
public:
  CsvFileHandle(const char* loc):location(loc)
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
      snprintf(fn,sizeof(fn),"%s/%04d%02d%02d-%02d%02d%02d.hs.lst",location.c_str(),
        t2.tm_year+1900,t2.tm_mon+1,t2.tm_mday,t2.tm_hour,t2.tm_min,t2.tm_sec);
      f.WOpen(fn);
      const char* header="SUBMIT_TIME,ALARM_ID,ALARMCATEGORY,SEVERITY,TEXT\n";
      f.Write(header,strlen(header));
      opened=true;
    }
    f.Write(str,strlen(str));
    f.Flush();
  }

  void Close()
  {
    smsc::core::synchronization::MutexGuard mg(mtx);
    std::string fn=f.getFileName();
    fn.erase(fn.length()-3);
    fn+="csv";
    f.Rename(fn.c_str());
    f.Close();
    opened=false;
  }

protected:
  smsc::core::buffers::File f;
  bool opened;
  smsc::core::synchronization::Mutex mtx;
  std::string location;
};

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
      Roll();
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

inline void SnmpTrap(const char* obj,const char* alarmId,int svrt,const char* msg)
{
  static smsc::logger::Logger *log=smsc::logger::Logger::getInstance("snmp");
  try{
    static CsvFileHandle fh(smsc::util::config::Manager::getInstance().getString("snmp.logsDirectory"));
    static CsvRollingThread crt(fh);
    static bool crtStarted=false;
    if(!crtStarted)
    {
      crt.Start();
      crtStarted=true;
    }
    time_t t=time(NULL);
    tm t2;
    gmtime_r(&t,&t2);
    char buf[1024];
    snprintf(buf,sizeof(buf),
      "%02d.%02d.%04d %02d:%02d:%02d,"
      "\"%s\",\"%s\",%d,\"%s\"\n",
      t2.tm_year+1900,t2.tm_mon+1,t2.tm_mday,t2.tm_hour,t2.tm_min,t2.tm_sec,
      alarmId,obj,svrt,msg);
    fh.Write(buf);
    char cmd[2048];
    snprintf(cmd,sizeof(cmd),
      "snmptrap -v 2c -c ussdc  traphost '' SIBINCO-SMSC-MIB::smscAlertFFMR "
      "SIBINCO-SMSC-MIB::alertSeverity i %d "
      "SIBINCO-SMSC-MIB::alertMessage s '%s' "
      "SIBINCO-SMSC-MIB::alertObjCategory s '%s' "
      "SIBINCO-SMSC-MIB::alertId s '%s'",
      svrt,msg,obj,alarmId);
    system(cmd);
    smsc_log_info(log,"exec:%s",cmd);
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"SnmpTrap::Exception:%s",e.what());
  }
}

#else //SNMP
inline void SnmpTrap(const char* obj,const char* alarmId,int svrt,const char* msg)
{
}
#endif

}//hsdaemon
}//admin
}//smsc

#endif
