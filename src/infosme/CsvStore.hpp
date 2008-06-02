#ifndef __SMSC_INFOSME_CSVSTORE_HPP__
#define __SMSC_INFOSME_CSVSTORE_HPP__

#include <string>
#include <list>
#include <map>
#include <set>
#include "core/buffers/File.hpp"
#include "core/synchronization/Mutex.hpp"
#include "TaskTypes.hpp"
#include "logger/Logger.h"

namespace smsc{
namespace infosme{

namespace buf=smsc::core::buffers;
namespace sync=smsc::core::synchronization;

//recordid structure
//0x08041712XXXXXXXX
//0xYYmmddhhFILEOFFS
class CsvStore{
public:
  CsvStore(const std::string& argLoc):location(argLoc)
  {
    if(location.length())
    {
      if(location[location.length()-1]!='/')location+='/';
    }else
    {
      location="./";
    }
    log=smsc::logger::Logger::getInstance("csvstore");
    unknownDirs=false;
    currentReadFile=0;
  }
  ~CsvStore();
  void Init();
  uint32_t Delete(bool onlynew);

  bool getNextMessage(Message& message);
  void setMsgState(uint64_t msgId,uint8_t state);
  void createMessage(time_t date,const Message& message,uint8_t state=NEW);
  bool enrouteMessage(uint64_t msgId);
  void loadMessage(uint64_t msgId,Message& message,uint8_t& state);
  void finalizeMsg(uint64_t msgId,time_t date,uint8_t state);

  typedef std::list<std::string> StrList;

  struct Directory{
    Directory()
    {
      unknownFiles=false;
    }
    int date;
    std::string dirPath;
    StrList files;
    StrList opened;
    StrList arcFiles;
    bool unknownFiles;
  };

protected:
  sync::Mutex mtx;
  std::string location;
  bool unknownDirs;

  smsc::logger::Logger* log;

  typedef std::map<std::string,Directory*> DirMap;
  DirMap dirs,pdirs;

  struct CsvFile{
    CsvFile():readAll(false)
    {
    }
    int date;//0xYYmmdd00
    int hour;//0xhh
    buf::File f;
    uint64_t fileSize;
    uint64_t readOff;
    std::string fileName;

    typedef std::set<uint64_t> OffSet;
    OffSet openMessages;
    bool readAll;

    bool Open(const char* argFileName,bool cancreate=true);
    uint64_t ReadRecord(uint8_t& state,Message& message);
    void AppendRecord(uint8_t state,time_t date,const Message& message);
    bool setState(uint64_t off,uint8_t state);
    bool setStateAndDate(uint64_t off,uint8_t state,time_t date);
    uint8_t getState(uint64_t off);
  };


  uint32_t mkFileKey(CsvFile* f)
  {
    uint32_t rv=f->date;
    rv<<=8;
    rv|=f->hour;
    return rv;
  }

  void closeFile(uint32_t fk);

  typedef std::map<uint32_t,CsvFile*> OpenedFilesMap;
  OpenedFilesMap ofMap;

  CsvFile* currentReadFile;
public:

  friend struct FullScan;
  class FullScan{
  protected:
    CsvStore& store;
    int didx;
    int fidx;
    CsvStore::DirMap::iterator dit;
    CsvStore::StrList::iterator fit;
    CsvFile f;
    bool first;
  public:
    FullScan(CsvStore& argStore):store(argStore)
    {
      didx=0;
      fidx=0;
      first=true;
    }
    bool Next(uint8_t& state,Message& msg);
  };


};

}//infosme
}//smsc

#endif

