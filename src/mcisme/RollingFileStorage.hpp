#ifndef __MCISME_ROLLINGFILESTORAGE_HPP__
# define __MCISME_ROLLINGFILESTORAGE_HPP__

# include <string.h>
# include <inttypes.h>
# include <sys/time.h>
# include <string>
# include <list>
# include <deque>

# include "mcisme/AbntAddr.hpp"
# include "mcisme/util.hpp"
# include "core/buffers/Hash.hpp"
# include "core/threads/Thread.hpp"
# include "core/synchronization/Event.hpp"
# include "core/synchronization/Mutex.hpp"
# include "logger/Logger.h"

namespace smsc {
namespace mcisme {

struct variable_data {
  variable_data() : length(0), value(NULL), currentPos(0), bufferOnHeap(false) {}
  ~variable_data() {
    if (bufferOnHeap)
      delete [] valueBuffer.bufferOnHeap;
  }

  void allocateSpace(uint32_t size) {
    if (size < sizeof(valueBuffer.bufferOnStack))
      value = valueBuffer.bufferOnStack;
    else {
      value = valueBuffer.bufferOnHeap = new uint8_t [size];
      bufferOnHeap = true;
    }
  }

  uint32_t length;
  union {
    uint8_t bufferOnStack[1024];
    uint8_t* bufferOnHeap;
  } valueBuffer;
  bool bufferOnHeap;
  uint8_t* value;
  uint32_t currentPos;
};

class RecordData {
public:
  RecordData()
  : _schedTime(0), _eventCount(0), _actualEventCount(0), _lastError(0), _calculatedCrc(0), _nextId(0)
  {}
  RecordData(uint64_t sched_time, uint16_t last_error, const AbntAddrValue& called_num)
  : _schedTime(sched_time), _eventCount(0), _actualEventCount(0), _lastError(last_error),
    _calculatedCrc(0), _nextId(0)
  {
    _calledNum.addr_content = called_num.addr_content;
  }

  // return total length of data record
  uint32_t getSize() const;
  // serialize record's data including record key
  void serialize(variable_data* record_for_write);
  // fill up record's fields from supplied data
  void deserialize(variable_data* record_data);

  uint64_t getSchedTime() const { return _schedTime; }
  void setSchedTime(uint64_t sched_time) { _schedTime = sched_time; }

  uint8_t getEventCount() const { return _actualEventCount; }

  uint16_t getLastError() const { return _lastError; }
  void setLastError(uint16_t last_error) { _lastError = last_error; }

  const AbntAddrValue& getCalledNum() const { return _calledNum; }

  uint32_t calculateCrc() const { return _calculatedCrc; }

  struct event_data
  {
    event_data()
    : date(0), id(0), callCount(0), flags(0), eventDeleted(false)
    {}

    event_data(uint64_t date_, const AbntAddrValue& calling_num, uint16_t call_count, uint8_t flags_)
    : date(date_), id(0), callCount(call_count), flags(flags_), eventDeleted(false)
    {
      memcpy(callingNum.full_addr, calling_num.full_addr, sizeof(callingNum.full_addr));
    }

    uint64_t        date;
    uint8_t         id;
    AbntAddrValue   callingNum;
    uint16_t        callCount;
    uint8_t         flags;
    bool            eventDeleted;
  };

  class Iterator {
  public:
    Iterator(event_data* events, uint8_t* event_count, uint8_t* actual_event_count)
    : _events(events), _eventCount(event_count),
      _actualEventCount(actual_event_count), _currentIdx(0)
    {}

    event_data* next();
    void remove(); // remove element referenced by iterator

  private:
    event_data* _events;
    uint8_t *_eventCount, *_actualEventCount;
    uint8_t _currentIdx;
  };

  Iterator getIterator() {
    return Iterator(_events, &_eventCount, &_actualEventCount);
  }

  void addEventData(const event_data& data);
  void removeEventData(uint8_t id);
  void addOrReplaceEventData(const event_data& data);

  std::string toString() const;

private:
  static const unsigned MAX_EVENTS = 50;

  uint64_t      _schedTime;
  uint8_t       _eventCount; // number of events that have been ever added to eventData
  uint8_t       _actualEventCount;// number of active (not deleted) events
  uint16_t      _lastError;
  AbntAddrValue _calledNum;

  event_data    _events[MAX_EVENTS];

  uint32_t _calculatedCrc;
  uint8_t _nextId;
};

class RecordHeader {
public:
  RecordHeader()
  : _totalRecordLen(0), _recordDeleted(0)
  {}

  void setRecordId(const std::string& id) { _id = id; }
  const std::string& getRecordId() const { return _id; }

  // get length of record's header
  uint32_t getSize() const
  {
    return
        static_cast<uint32_t>(sizeof(_totalRecordLen) + sizeof(uint16_t) /*length part of _id*/+
                              _id.length() + sizeof(_recordDeleted));
  }

  uint32_t getRecordLen() const { return _totalRecordLen; }
  void setRecordLen(uint32_t record_len) { _totalRecordLen = record_len; }

  void markAsDeleted() { _recordDeleted = true; }
  bool isDeleted() const { return _recordDeleted; }

  // write record's header as sequence of bytes
  void serialize(variable_data* record_for_write) const;
  // fill up record's header from supplied data
  void deserialize(variable_data* record_data);

private:
  uint32_t _totalRecordLen;// in order to skip deleted records w/o deserialization
  std::string _id;
  uint8_t _recordDeleted;
};

class RecordTail {
public:
  uint32_t getCrc() const { return _crc; }
  void setCrc(uint32_t crc) { _crc = crc; }

  uint32_t getSize() const { return static_cast<uint32_t>(sizeof(_crc)); }

  void serialize(variable_data* record_for_write) const;
  void deserialize(variable_data* record_data);

private:
  uint32_t _crc;
};

class FileRecord {
public:
  FileRecord()
  : _recordData(NULL)
  {}

  // The argument RecordData* MUST be allocated ON THE HEAP.
  FileRecord(RecordData* record_data)
  : _recordData(record_data)
  {
    _recordHeader.setRecordId(AbntAddr(_recordData->getCalledNum().full_addr).getText());
  }

  virtual ~FileRecord() {
    delete _recordData;
  }

  virtual void serialize(variable_data* file_record);
  void deserialize(variable_data* record_data);

  RecordData* getRecordData() { return _recordData; }
  const RecordData* getRecordData() const { return _recordData; }

  const char* getId() const { return _recordHeader.getRecordId().c_str(); }
  void markAsDeleted() { _recordHeader.markAsDeleted(); }
  bool isDeleted() const { return _recordHeader.isDeleted(); }

protected:
  RecordHeader _recordHeader;
  RecordData* _recordData;
  RecordTail _recordTail;
};

class DeletedFileRecord : public FileRecord {
public:
  DeletedFileRecord(const std::string& id)
  {
    _recordHeader.setRecordId(id);
    markAsDeleted();
  }

  virtual void serialize(variable_data* file_record) {
    uint32_t totalRecordLen = _recordHeader.getSize();

    file_record->allocateSpace(totalRecordLen);
    file_record->length = totalRecordLen;
    _recordHeader.setRecordLen(totalRecordLen);
    _recordHeader.serialize(file_record);
  }
};

class FileHeader {
public:
  void serialize(variable_data* file_record);
  // return false if file header was deserialized successfully, else return true.
  bool deserialize(variable_data* record_data);
private:
  static const char* MAGIC_STAMP;
  static const uint16_t STORAGE_VERSION = 1;
  static const unsigned MAGIC_STAMP_LEN = 12; // mci_storage + terminated zero
  static const unsigned HEADER_SIZE = MAGIC_STAMP_LEN + sizeof(STORAGE_VERSION);
};

class RollingFileStorage;

class RollingThread : public core::threads::Thread
{
public:
  RollingThread(RollingFileStorage& rolling_storage)
  : _rollingStorage(rolling_storage)
  {}

  virtual int Execute();
  void stop();
  void start();

private:
  bool isRunning() { return _isRunning; }

  bool _isRunning;
  RollingFileStorage& _rollingStorage;
  core::synchronization::Event _threadHasActivated, _threadHasStopped;
};

class RollingFileStorage
{
public:
  RollingFileStorage(const string& location, const std::string& file_prefix, size_t max_file_size, unsigned max_number_of_files);
  ~RollingFileStorage() { close(); }

  void start();
  void stop();

  // append record to the end of file and save one in memory
  // make file rolling if current file size hits to max file size limit
  void saveRecord(FileRecord* record);
  // return record from memory by key identifier value.
  // this method operates only with records in memory
  // returns pointer to record if it was found, else returns NULL.
  FileRecord* getRecord(const char* record_id);
  void deleteRecord(const char* record_id);

  void close();

  void printDebug();

private:
  friend class Iterator;

  // read all rolling files and save records in memory
  void loadRecords();

  // load records from file previously opened by openFile() method
  void loadFile(const std::string& file_name);

  void openFile(const std::string& full_qualified_file_name);
  // close file descriptor (_fd) if it is open.
  void closeCurrentFile();
  std::string formFileName() const;
  // create new file specified by file_prefx (that set in constructor)
  // and file_suffix that correspond to current time in format DDMMYYYYHH24MISS
  // File created in directory specified by the _location data member that set in constructor
  void createFile();
  void initializeFile(const std::string& file_name);

  void updateRecordInCache(FileRecord* new_record);
  void makeFileRolling();
  void waitForRollingActivation();
  void writeRecordToCurrentActiveFile(FileRecord* record, bool do_switch);
  bool isRollingInProgress() const { return _rollingInProgress; }

  struct FileRecordInfo {
    std::string recordId;
    /*
      time (in msecs) when record was added into file.
      last modification time for record stored in struct FileRecordWrapper.
      insertTime is taken into account when makes file rolling:
        if insertTime < FileRecordWrapper.modificationTime ==> a newer record stored at
        some subsequent file and so it isn't needed to move this record during the rolling
    */
    unsigned long long insertTime;
    FileRecordInfo()
    : insertTime(0)
    {}
    explicit FileRecordInfo(const char* record_id, unsigned long long insert_time)
    : recordId(record_id), insertTime(insert_time)
    {}
  };
  typedef std::list<FileRecordInfo> file_records_info_t;
  struct FileRecordWrapper {
    explicit FileRecordWrapper(FileRecord* file_record)
    : actualFileRecord(file_record),
      modificationTime(getTimeInMsec())
    {}

    FileRecord* actualFileRecord;
    unsigned long long modificationTime;
  };
  struct file_info {
    file_info()
    : fileRecords(NULL)
    {}

    explicit file_info(std::string file_name)
    : fileName(file_name), fileRecords(new file_records_info_t())
    {}

    std::string fileName;
    file_records_info_t* fileRecords; // list of descriptors for records inserted into file
  };

  std::deque<file_info> _activeFilesArray;

  core::buffers::Hash<FileRecordWrapper*> _recordsCache;
  std::string _baseFileName, _fileNamePrefix, _location;
  size_t _maxFileSize, _currentFileSize;
  int _fd;
  unsigned _activeFileSeqNum;
  unsigned _maxNumberOfFiles;
  bool _rollingInProgress;
  friend class RollingThread;
  RollingThread _rollingThread;
  core::synchronization::Event _makeRollingEvent, _rollingInProgressEvent;
  mutable core::synchronization::Mutex _internalStructureLock; // sync point used in public methods

  logger::Logger* _logger;

  void moveRecord(FileRecordInfo& record, const std::string& orig_fname);

public:
  class Iterator {
  public:
    Iterator(RollingFileStorage& storage)
    : _storage(storage), _iter(&storage._recordsCache)
    {}

    // return NULL if there isn't more elements
    FileRecord* next()
    {
      core::synchronization::MutexGuard synchronize(_storage._internalStructureLock);
      FileRecordWrapper* record;
      char* calledAddr;
      if (_iter.Next(calledAddr, record))
        return record->actualFileRecord;
      else
        return NULL;
    }

    void rewind()
    {
      _iter.First();
    }

  private:
    RollingFileStorage& _storage;
    core::buffers::Hash<FileRecordWrapper*>::Iterator _iter;
  };

  Iterator getIterator() {
    return Iterator(*this);
  }
};

}}

#endif
