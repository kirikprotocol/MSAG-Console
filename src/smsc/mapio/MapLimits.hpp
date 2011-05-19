#ifndef __SMSC_SYSTEM_MAPIO_MAPLIMITS__
#define __SMSC_SYSTEM_MAPIO_MAPLIMITS__

#include <string>
#include <set>
#include <map>
#include "core/synchronization/Mutex.hpp"
#include "logger/Logger.h"

namespace smsc{
namespace mapio{

namespace sync=smsc::core::synchronization;

enum UssdParsingMode{
  upmAlways,
  upmOnlyStar,
  upmNever
};

class MapLimits{
public:
  MapLimits()
  {
    limitIn=0;
    limitInSRI=0;
    limitUSSD=0;
    limitOutSRI=0;
    limitNIUSSD=0;
    smsOpenRespRealAddr=false;
    log=smsc::logger::Logger::getInstance("maplimits");
  }


  int getLimitIn()
  {
    return limitIn;
  }
  int getLimitInSRI()
  {
    return limitInSRI;
  }
  int getLimitUSSD()
  {
    return limitUSSD;
  }
  int getLimitOutSRI()
  {
    return limitOutSRI;
  }
  int getLimitNIUSSD()
  {
    return limitNIUSSD;
  }

  bool isNoSRIUssd(const std::string& ussd)
  {
    sync::MutexGuard mg(mtx);
    std::string::size_type pos=ussd.find(':');
    if(pos!=std::string::npos)
    {
      return noSriUssd.find(ussd.substr(pos+1))!=noSriUssd.end();
    }
    return noSriUssd.find(ussd)!=noSriUssd.end();
  }

  bool isCondSRIUssd(const std::string& ussd)
  {
    sync::MutexGuard mg(mtx);
    std::string::size_type pos=ussd.find(':');
    if(pos!=std::string::npos)
    {
      return condSriUssd.find(ussd.substr(pos+1))!=condSriUssd.end();
    }
    return condSriUssd.find(ussd)!=condSriUssd.end();
  }

  bool isATIUssd(const std::string& ussd)
  {
    sync::MutexGuard mg(mtx);
    std::string::size_type pos=ussd.find(':');
    if(pos!=std::string::npos)
    {
      return atiUssd.find(ussd.substr(pos+1))!=atiUssd.end();
    }
    return atiUssd.find(ussd)!=atiUssd.end();
  }

  bool isOpenRespRealAddr(const std::string& ussd)
  {
    sync::MutexGuard mg(mtxOpenResp);
    std::string::size_type pos=ussd.find(':');
    if(pos!=std::string::npos)
    {
      return openRespRealAddr.find(ussd.substr(pos+1))!=openRespRealAddr.end();
    }
    return openRespRealAddr.find(ussd)!=openRespRealAddr.end();
  }

  bool isSmsOpenRespRealAddr()
  {
    return smsOpenRespRealAddr;
  }

  UssdParsingMode getUssdParsing(const std::string& ussd)
  {
    sync::MutexGuard mg(mtx);
    std::string::size_type pos=ussd.find(':');
    StrModeMap::iterator it;
    if(pos!=std::string::npos)
    {
      it=ussdParseMode.find(ussd.substr(pos+1));
    }else
    {
      it=ussdParseMode.find(ussd);
    }
    if(it==ussdParseMode.end())
    {
      return defaultUPM;
    }
    return it->second;
  }


  static void Init(const char* fn);
  static void Shutdown()
  {
    delete instance;
    instance=0;
  }
  static MapLimits& getInstance()
  {
    return *instance;
  }

  void Reinit();

  void reportMscFailure(const char* msc)
  {
    //sync::MutexGuard mg(mtxMsc);
    MSCInfo* mscPtr=mscInfo.GetPtr(msc);
    if(!mscPtr)
    {
      MSCInfo mi;
      mi.failCount=1;
      mscInfo.Insert(msc,mi);
      return;
    }
    if(mscPtr->clevel<maxCLevels-1 || mscPtr->failCount<limitsOut[mscPtr->clevel].failUpperLimit)
    {
      mscPtr->failCount++;
      smsc_log_info(log,"msc %s inc fail count:%d",msc,mscPtr->failCount);
    }
    if(mscPtr->clevel<maxCLevels-1 && mscPtr->failCount>=limitsOut[mscPtr->clevel].failUpperLimit)
    {
      mscPtr->clevel++;
      smsc_log_warn(log,"msc %s congestion level increased to %d",msc,mscPtr->clevel+1);
    }
  }
  void reportMscOk(const char* msc)
  {
    //sync::MutexGuard mg(mtxMsc);
    MSCInfo* mscPtr=mscInfo.GetPtr(msc);
    if(!mscPtr)
    {
      MSCInfo mi;
      mscInfo.Insert(msc,mi);
      return;
    }
    if(mscPtr->failCount==0)return;
    mscPtr->okCount++;
    smsc_log_debug(log,"msc %s inc ok count:%d",msc,mscPtr->okCount);
    if(mscPtr->okCount>=limitsOut[mscPtr->clevel].okToLower)
    {
      mscPtr->failCount--;
      smsc_log_info(log,"msc %s dec fail count:%d",msc,mscPtr->failCount);
      if(mscPtr->clevel>0 && mscPtr->failCount<limitsOut[mscPtr->clevel-1].failLowerLimit)
      {
        mscPtr->clevel--;
        smsc_log_warn(log,"msc %s congestion level decreased to %d",msc,mscPtr->clevel+1);
      }
      mscPtr->okCount=0;
    }
  }

  // return congestion level or -1 if limit reached
  int incDlgCounter(const char* msc)
  {
    //sync::MutexGuard mg(mtxMsc);
    MSCInfo* mscPtr=mscInfo.GetPtr(msc);
    if(!mscPtr)
    {
      mscPtr=mscInfo.SetItem(msc,MSCInfo());
    }
    CLevelInfo& li=limitsOut[mscPtr->clevel];
    if(li.dlgCount>=li.dlgLimit)
    {
      return -1;
    }
    li.dlgCount++;
    smsc_log_debug(log,"inc dlg count by msc %s[cl=%d]:%d",msc,mscPtr->clevel,li.dlgCount);
    return mscPtr->clevel;
  }

  void decDlgCounter(int clevel)
  {
    //sync::MutexGuard mg(mtxMsc);
    if(clevel>=0 && clevel<maxCLevels)
    {
      limitsOut[clevel].dlgCount--;
      smsc_log_debug(log,"dec dlg count[cl=%d]:%d",clevel,limitsOut[clevel].dlgCount);
    }else
    {
      smsc_log_warn(log,"attempt to decrement dlg count for incorect clevel=%d",clevel);
    }
  }

protected:
  static MapLimits* instance;

  enum{
    maxCLevels=8
  };


  smsc::logger::Logger* log;

  std::string configFilename;


  typedef std::set<std::string> StringSet;
  StringSet noSriUssd;
  StringSet condSriUssd;
  StringSet atiUssd;
  sync::Mutex mtx;
  void parseUssdCodes(const char* name,const std::string& str,StringSet& codes);

  StringSet openRespRealAddr;
  sync::Mutex mtxOpenResp;

  typedef std::map<std::string,UssdParsingMode> StrModeMap;
  StrModeMap ussdParseMode;
  UssdParsingMode defaultUPM;
  sync::Mutex mtxUpm;

  bool smsOpenRespRealAddr;


  int limitIn;
  int limitInSRI;
  int limitUSSD;
  int limitOutSRI;
  int limitNIUSSD;

  struct CLevelInfo{
    int dlgCount;
    int dlgLimit;
    int failUpperLimit;
    int failLowerLimit;
    int okToLower;
  };

  CLevelInfo limitsOut[maxCLevels];

  struct MSCInfo{
    MSCInfo():clevel(0),failCount(0),okCount(0)
    {
    }
    int clevel;
    int failCount;
    int okCount;
  };

  sync::Mutex mtxMsc;
  smsc::core::buffers::Hash<MSCInfo> mscInfo;
};

}
}

#endif

