#ifndef _SCAG_UTIL_STORAGE_DATAFILECREATOR_H_
#define _SCAG_UTIL_STORAGE_DATAFILECREATOR_H_

#include <string>
#include <memory>

#include "logger/Logger.h"
#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/buffers/File.hpp"
#include "util/Uint64Converter.h"
#include "util/timeslotcounter.hpp"
#include "DataFileManager.h"


namespace scag    {
namespace util    {
namespace storage {

using std::string;

using smsc::core::threads::Thread;
using smsc::core::buffers::File;
using smsc::core::buffers::FileException;
using smsc::core::synchronization::EventMonitor;
using smsc::core::synchronization::MutexGuard;
using smsc::logger::Logger;

class DataFileCreator {
public:
  enum State {
    OK,
    CREATING,
    CREATED,
    ERROR
  };
public:
  DataFileCreator(DataFileManager& manager, Logger* log):manager_(manager), file_(0), fileSize_(0), blockSize_(0), filesCount_(0), writeBlockSize_(8),
                                            fileCreated_(false), logger_(log), state_(OK) {};

  void init(int32_t fileSize, int32_t blockSize, int32_t filesCount, const string& dbPath, const string& dbName) {
    dbPath_ = dbPath;
    dbName_ = dbName;
    fileSize_ = fileSize;
    blockSize_ = blockSize;
    filesCount_ = filesCount;
    ffbThreshold_ = (fileSize_ / 2);
    sleepTime_ = getSleepTime();
    if (logger_) smsc_log_info(logger_, "init creator: fileSize:%d blockSize:%d filesCount:%d reserver:%d sleepTime:%d ms",
                                          fileSize_, blockSize_, filesCount_, fileSize_ - ffbThreshold_, sleepTime_);
  }

  void openPreallocatedFile(int64_t ffb) {
    name_ = makeDataFileName(filesCount_);
    State state = getState();
    if (state == CREATED || state == CREATING) {
      return;
    }
    if (getThreshold(ffb) >= ffbThreshold_) {
      if (File::Exists(name_.c_str())) {
        std::auto_ptr<File> f(new File);
        if (logger_) smsc_log_debug(logger_, "open preallocated file: %s", name_.c_str());
        f->RWOpen(name_.c_str());
        f->SetUnbuffered();
        setState(CREATED);
        file_ = f.release();
      } else {
        setState(CREATING);
        sleepTime_ = 0;
        createDataFile();
        sleepTime_ = getSleepTime();
      }
      return;
    }
    if (getThreshold(ffb) < ffbThreshold_) {
      checkExistedFile();
      return;
    }
  }

  void create(int64_t ffb, int32_t filesCount) {
    filesCount_ = filesCount;
    if (ffb == 0 && filesCount_ == 0) {
      State state = getState();
      if (state == CREATED || state == CREATING) {
        return;
      }
      name_ = makeDataFileName(filesCount_);
      if (logger_) smsc_log_debug(logger_, "allocate first data file");
      setState(CREATING);
      sleepTime_ = 0;
      createDataFile();
      sleepTime_ = getSleepTime();
      return;
    }
    if (getThreshold(ffb) == ffbThreshold_) {
      State state = getState();
      if (state == CREATED || state == CREATING) {
        return;
      }
      name_ = makeDataFileName(filesCount_);
      if (logger_) smsc_log_debug(logger_, "ffb:%d == ffbThreshold_:%d", ffb - ((filesCount_ - 1) * fileSize_), ffbThreshold_);
      setState(CREATING);
      manager_.createDataFile(*this);
      return;
    }
  }

  const char* getFileName() const {
    return name_.c_str();
  }

  File* getFile() {
    while (getState() == CREATING) {
      if (logger_) smsc_log_warn(logger_, "wait while data file: '%s' allocated", name_.c_str());
      MutexGuard mg(monitor_);
      monitor_.wait();
    }
    if (getState() == CREATED) {
      setState(OK);
      return file_;
    } else {
      return 0;
    }
  }

  int64_t prepareBlock(char* writeBlock, int blocksCount, int64_t startIndex, int lastBlock) {
    lastBlock = lastBlock > 0 ? 1 : 0;
    for (int i = 0; i < blocksCount - lastBlock; ++i) {
      uint64_t ni = Uint64Converter::toNetworkOrder(startIndex++);
      memcpy(writeBlock + (i * blockSize_), &ni, sizeof(ni));
    }
    if (lastBlock > 0) {
      uint64_t ni = Uint64Converter::toNetworkOrder(-1);
      memcpy(writeBlock + ((blocksCount - 1) * blockSize_), &ni, sizeof(ni));
    }
    return startIndex;
  }

  unsigned getSleepTime() {
    uint32_t reserve = fileSize_ - ffbThreshold_;
    uint32_t totalTime = reserve / manager_.getExpectedSpeed() * 1000;
    if (totalTime <= 0) {
      return 0;
    }
    uint32_t writeCount = fileSize_ % writeBlockSize_ > 0 ? fileSize_ / writeBlockSize_ + 1 : fileSize_ / writeBlockSize_;
    totalTime = (totalTime / 4) * 3;
    return totalTime / writeCount;
  }

  void createDataFile() {
    name_ = makeDataFileName(filesCount_);
    if (logger_) smsc_log_info(logger_, "preallocate new data file: '%s'", name_.c_str());
    time_t startTime = time(NULL);
    file_ = 0;
    fileCreated_ = false;
    //setState(CREATING);
    char *emptyBlock = 0;
    checkExistedFile();
    try {
      std::auto_ptr<File> f(new File);
      f->RWCreate(name_.c_str());
      f->SetUnbuffered();
  
      int64_t startBlock = filesCount_ * fileSize_;
      int64_t endBlock = (filesCount_ + 1) * fileSize_;
      int blocksCount = writeBlockSize_;
      int lastWriteSize = fileSize_ % blocksCount;
      int writeCount = lastWriteSize > 0 ? fileSize_ / blocksCount : fileSize_ / blocksCount - 1;

      emptyBlock = new char[blockSize_ * blocksCount];
      memset(emptyBlock, 0x00, blockSize_ * blocksCount);
      int64_t index = startBlock + 1;
      for (int i = 0; i < writeCount; ++i) {
        index = prepareBlock(emptyBlock, blocksCount, index, 0);
        hrtime_t startWrite = gethrtime();
        f->Write(emptyBlock, blockSize_ * blocksCount);
        int writeTime = static_cast<int>((gethrtime() - startWrite) / 1000000);
        int sleepTime = sleepTime_ - writeTime;
        if (sleepTime > 0) {
          MutexGuard mg(sleepMonitor_);
          sleepMonitor_.wait(sleepTime);
        }
      }
      blocksCount = lastWriteSize > 0 ? lastWriteSize : blocksCount;
      index = prepareBlock(emptyBlock, blocksCount, index, 1);
      f->Write(emptyBlock, blockSize_ * blocksCount);

      file_ = f.release();
      fileCreated_ = true;
      setState(CREATED);
      time_t preallTime = time(NULL) - startTime;
      if (logger_) smsc_log_info(logger_, "new data file: '%s' allocated %d sec", name_.c_str(), preallTime);

      if (emptyBlock) delete[] emptyBlock;
      MutexGuard mg(monitor_);
      monitor_.notify();
    } catch (const std::exception& ex) {
      if (logger_) smsc_log_error(logger_, "Error create data file. std::exception: '%s'", ex.what());
      file_ = 0;
      setState(ERROR);
      if (emptyBlock) delete[] emptyBlock;
      MutexGuard mg(monitor_);
      monitor_.notify();
      //throw;
    }
  };

private:
  int64_t getThreshold(int64_t ffb) {
    return ffb - ((filesCount_ - 1) * fileSize_);
  }

  void checkExistedFile() {
    if (!File::Exists(name_.c_str())) {
      return;
    }
    for ( int backnum = 0;; ++backnum ) {
        const std::string backname = makeDataFileName( filesCount_, backnum );
        if ( File::Exists(backname.c_str()) ) {
            if ( backnum < 10 ) continue;
            if (logger_) smsc_log_error(logger_, "error create data file: file '%s' already exists", name_.c_str());
            throw FileException(FileException::errOpenFailed, name_.c_str());
        }
        if (logger_) smsc_log_warn( logger_, "Renaming unregistered data file %s into %s", name_.c_str(), backname.c_str() );
        File::Rename( name_.c_str(), backname.c_str() );
        if (logger_) smsc_log_warn( logger_, "unregisterd data file %s is renamed into %s", name_.c_str(), backname.c_str() );
        break;
    }
  }

  string makeDataFileName( int num, int backnum = -1 ) const
  {
      char	buff[60];
      int pos = snprintf(buff, sizeof(buff), "-%.7d", num );
      if ( backnum >= 0 ) {
          snprintf( buff+pos, sizeof(buff)-pos, ".%07d.backup", backnum );
      }
      return dbPath_ + '/' + dbName_ + buff;
  }

  void setState(State state) {
    MutexGuard mg(monitor_);
    state_ = state;
  }

  State getState() {
    MutexGuard mg(monitor_);
    return state_;
  }

private:
  DataFileManager& manager_;
  File *file_;
  int32_t fileSize_; 
  int32_t blockSize_;
  int32_t filesCount_;
  uint32_t writeBlockSize_;
  string name_;
  string dbPath_;
  string dbName_;
  bool fileCreated_;
  int32_t ffbThreshold_;
  Logger* logger_;
  EventMonitor monitor_;
  EventMonitor sleepMonitor_;
  State state_;
  unsigned sleepTime_;
};


}//storage
}//util
}//scag


#endif


