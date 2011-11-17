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

  time_t requestTimeout;
  static std::string path;
  static smsc::logger::Logger* log;

  typedef std::multimap<time_t,int> TimeMap;

  struct Request {

    Request()
    {
      day=-1;
      hour=-1;
      linesRead = 0;
      zipVersion=0;
      version=0;
      refCount=0;
      offset=0;
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

      void show( const char* where );

      bool parseRecord( msgtime_type endTime,
                        ALMResult* result,
                        bool& hasMore );

      inline void closeFile() { f.Close(); }

  private:
      /// opens the next file for given record
      /// return true if the file is opened.
      /// if the endTime is reached but file is not opened,
      ///  then return false and set hasMore.
      bool openNextFile( msgtime_type endTime,
                         bool& hasMore );

      bool readZipVersion();
      bool scanZipToDate();
      msgtime_type readZipChunkHead();
      bool readChunkVersion();

      inline static bool endTimeReached( msgtime_type endTime ) {
          return ( currentTimeSeconds() > endTime );
      }

      void advance( bool bigjump ) {
          if (bigjump) {
              if (hour!=-1) {
                  curDate -= curDate % (60*60);
                  curDate += 60*60;
              } else {
                  curDate -= curDate % (24*60*60);
                  curDate += 24*60*60;
              }
          } else {
              curDate -= curDate % 60;
              curDate += 60;
          }
      }

  public:
    dlvid_type dlvId;
    ALMRequestFilter filter;
    msgtime_type curDate;
    TimeMap::iterator timeIt;
    bool busy;
  private:
    smsc::core::buffers::File f;
    unsigned zipVersion;   // 0 for nonzip, >0 for zip
    uint64_t nextzipchunk; // offset to the next zip chunk (=0 if not zipped)
    uint64_t offset;       // offset in the current file (=0 if next is needed)
    int linesRead;
    unsigned version;      // the version of file format
    int refCount;
    int day;               // -1 if not checked; taken from zip
    int hour;              // -1 if not checked; taken from zip
  };

  typedef std::map<int,Request*> ReqMap;
  ReqMap reqMap;
  TimeMap timeMap;
  sync::Mutex mtx;
  int reqIdSeq;

  static std::string mkFilePath(dlvid_type dlvId,const ::tm& date);
  static std::string mkDatePath(dlvid_type dlvId,const ::tm& date);
  static std::string mkHourPath(dlvid_type dlvId,const ::tm& date);

};

}
}
}

#endif
