#ifndef __EYELINE_INFORMER_ALM_ACTIVITYLOGMINTER_HPP__
#define __EYELINE_INFORMER_ALM_ACTIVITYLOGMINTER_HPP__

#include "IActivityLogMiner.hpp"
#include "core/threads/Thread.hpp"
#include <map>
#include "core/synchronization/Mutex.hpp"

namespace eyeline{
namespace informer{
namespace alm{

namespace sync=smsc::core::synchronization;

class ActivityLogMiner:public IActivityLogMiner{
public:

  ActivityLogMiner();

  void init(const std::string& argPath,time_t argRequestTimeout);

  int createRequest(dlvid_type dlvId,const ALMRequestFilter& filter);
  bool getNext(int reqId,std::vector<ALMResult>& result,int count);

  int countRecords(dlvid_type dlvId,const ALMRequestFilter& filter);

protected:

  std::string path;
  time_t requestTimeout;

  typedef std::multimap<time_t,int> TimeMap;

  struct Request{
    dlvid_type dlvId;
    ALMRequestFilter filter;
    msgtime_type curDate;
    uint64_t offset;
    TimeMap::iterator timeIt;
  };

  typedef std::map<int,Request*> ReqMap;
  ReqMap reqMap;
  TimeMap timeMap;
  sync::Mutex mtx;
  int reqIdSeq;

  std::string mkFilePath(dlvid_type dlvId,msgtime_type date);

  int parseFiles(Request* req,std::vector<ALMResult>* result,int count,bool countOnly);

};

}
}
}

#endif
