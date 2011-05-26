#ifndef __EYELINE_INFORMER_ALM_ACTIVITYLOGMINTER_HPP__
#define __EYELINE_INFORMER_ALM_ACTIVITYLOGMINTER_HPP__

#include "IActivityLogMiner.hpp"
#include "core/threads/Thread.hpp"
#include <map>
#include "core/synchronization/Mutex.hpp"
#include "logger/Logger.h"
#include "core/buffers/File.hpp"

namespace eyeline{
namespace informer{
namespace alm{

namespace sync=smsc::core::synchronization;

class ActivityLogMiner:public IActivityLogMiner{
public:

  ActivityLogMiner();

  void init(const std::string& argPath,time_t argRequestTimeout);

  int createRequest(dlvid_type dlvId,const ALMRequestFilter& filter);
  bool getNext(int reqId, msgtime_type endTime, ALMResult* result, bool& hasMore);
  void pauseReq(int reqId);

  // int countRecords(dlvid_type dlvId,const ALMRequestFilter& filter);

protected:

  std::string path;
  time_t requestTimeout;
  smsc::logger::Logger* log;

  typedef std::multimap<time_t,int> TimeMap;

  struct Request{
    dlvid_type dlvId;
    ALMRequestFilter filter;
    msgtime_type curDate;
    uint64_t offset;
    TimeMap::iterator timeIt;
    int refCount;
    bool busy;

    Request()
    {
      dayChecked=false;
      hourChecked=false;
      day=-1;
      hour=-1;
      linesRead = 0;
      version=0;
      refCount=0;
      busy=false;
    }

    void ref()
    {
      refCount++;
    }

    void unref()
    {
      refCount--;
      if(refCount==0)
      {
        delete this;
      }
    }

    smsc::core::buffers::File f;
    unsigned version;  // the version of file format
    bool dayChecked;
    bool hourChecked;
    int day;
    int hour;
    int linesRead;
  };

  typedef std::map<int,Request*> ReqMap;
  ReqMap reqMap;
  TimeMap timeMap;
  sync::Mutex mtx;
  int reqIdSeq;

  std::string mkFilePath(dlvid_type dlvId,const ::tm& date);
  std::string mkDatePath(dlvid_type dlvId,const ::tm& date);
  std::string mkHourPath(dlvid_type dlvId,const ::tm& date);

  bool parseRecord(Request* req, msgtime_type endTime, ALMResult* result,
                   bool& hasMore );
    inline bool endTimeReached( msgtime_type endTime ) const {
        return ( currentTimeSeconds() > endTime );
    }
};

}
}
}

#endif
