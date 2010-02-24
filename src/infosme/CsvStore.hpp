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
  CsvStore( const std::string& argLoc ) : location(argLoc)
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
    curDir=dirs.end();
  }
  ~CsvStore();
  void Init();
  uint32_t Delete(bool onlynew);

  bool getNextMessage(Message& message);
  uint64_t createMessage(time_t date,const Message& message,uint8_t state=NEW);
  void enrouteMessage(uint64_t msgId);
  void loadMessage(uint64_t msgId,Message& message,uint8_t& state);
  // if msg is passed it is filled with fields of actual message
  void setMsgState(uint64_t msgId,uint8_t state, Message* msg = 0);
  void finalizeMsg(uint64_t msgId,time_t date,uint8_t state, Message* msg = 0);
    
  void closeAllFiles();
    // if all messages have been read and their states are final
    bool isProcessed();

  struct Directory;
  struct CsvFile{
      CsvFile(smsc::logger::Logger* log, int argDate,int argHour,Directory* argDir ) :
      log_(log),
      readAll(false),processed(false),date(argDate),hour(argHour),dir(argDir)
    {
      openMessages=0;
      curMsg=timeMap.end();
    }
    smsc::logger::Logger* log_;
    bool readAll;
    bool processed;
    int date;//0xYYmmdd00
    int hour;//0xhh
    Directory* dir;
    buf::File f;

    struct Record{
      Message msg;
      uint8_t state;
    };

    typedef std::multimap<time_t,Record> TimeMap;
    typedef std::map<uint64_t,TimeMap::iterator> MessageMap;

    TimeMap timeMap;
    MessageMap msgMap;
    TimeMap::iterator curMsg;
    int openMessages;

    bool isOpened()
    {
      return f.isOpened();
    }

    bool haveMessages()
    {
      return curMsg!=timeMap.end();
    }

    std::string fileName()
    {
      char buf[32];
      sprintf(buf,"%02x%s.csv",hour,processed?"processed":"");
      return buf;
    }

    std::string fullPath()
    {
      return dir->dirPath+'/'+fileName();
    }

    enum GetRecordResult{
      grrNoMoreMessages,
      grrRecordOk,
      grrRecordNotReady
    };

    bool Open(bool cancreate=false,bool readFinal=false);
    void Close(bool processed=true);
    GetRecordResult getNextRecord(Record& rec,time_t date,bool onlyNew=true);
    void ReadRecord(Record& rec);
    static void ReadRecord(buf::File& f,Record& rec);
    MessageMap::iterator findRecord(uint64_t msgId,bool loadFromDisk=false);
    uint64_t AppendRecord(uint8_t state,time_t date,const Message& message);
    void setState(uint64_t msgId,uint8_t state, Message* msg = 0);
    void setStateAndDate(uint64_t msgId,uint8_t state,time_t date, Message* msg = 0);
    // uint8_t getState(uint64_t msgId);
  };

  typedef std::map<int,CsvFile*> FileMap;
  struct Directory{
    Directory()
    {
      unknownFiles=false;
    }
    ~Directory()
    {
      FileMap::iterator end=files.end();
      for(FileMap::iterator it=files.begin();it!=end;it++)
      {
        delete it->second;
      }
    }
    int date;
    std::string dirPath;
    FileMap files;
    bool unknownFiles;
  };

protected:
  sync::Mutex mtx;
  std::string location;
  bool unknownDirs;

  smsc::logger::Logger* log;

  typedef std::map<int,Directory*> DirMap;
  DirMap dirs;
  DirMap::iterator curDir;
  FileMap::iterator curFile;


  uint32_t mkFileKey(CsvFile* f)
  {
    uint32_t rv=f->date;
    rv<<=8;
    rv|=f->hour;
    return rv;
  }

  typedef std::set<uint32_t> CloseSet;
  CloseSet closeSet;

  CsvFile& findFile(const char* func,uint64_t msgId,uint64_t& off);

  void canClose(CsvFile& file);
  void removeCanClose(CsvFile& file);

  // static uint32_t tm2xdate( struct tm& t );

public:

  friend struct FullScan;
  class FullScan{
  protected:
    CsvStore& store;
    CsvStore::DirMap::iterator dit;
    CsvStore::FileMap::iterator fit;
    CsvFile f;
    FullScan(const FullScan&);
  public:
    FullScan(CsvStore& argStore):store(argStore),f(smsc::logger::Logger::getInstance("fullscan"),0,0,0)
    {
      dit=store.dirs.begin();
      if(dit!=store.dirs.end())
      {
        fit=dit->second->files.begin();
      }
    }
    bool Next(uint8_t& state,Message& msg);
  };


};

}//infosme
}//smsc

#endif

