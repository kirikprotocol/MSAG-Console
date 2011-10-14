#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <algorithm>
#include <assert.h>

#include "RollingFileStorage.hpp"
#include "util.hpp"
#include "util/crc32.h"
#include "util/BufferSerialization.hpp"
#include "util/Exception.hpp"

namespace smsc {
namespace mcisme {

void
RecordData::addEventData(const event_data& data)
{
  if (_eventCount >= MAX_EVENTS)
    throw util::Exception("RecordData::addEventData::: max number of events exceeded");
  _events[_eventCount] = data;
  _events[_eventCount].id = _nextId++;
  _eventCount++, _actualEventCount++;
}

void
RecordData::removeEventData(uint8_t id)
{
  for(int event_num = 0; event_num < _eventCount; event_num++)
  {
    if(_events[event_num].id == id)
    {
      _actualEventCount--;
      _events[event_num].eventDeleted = true;
      break;
    }
  }
}

void
RecordData::addOrReplaceEventData(const event_data& data)
{
  int16_t insertIdx=-1;
  for(uint16_t i = 0; i < _eventCount; i++)
  {
    if (!memcmp(_events[i].callingNum.full_addr, data.callingNum.full_addr, sizeof(data.callingNum.full_addr)))
    {
      event_data updatedEvent = _events[i];
      updatedEvent.callCount++;
      updatedEvent.date = data.date;
      updatedEvent.flags = data.flags;
      updatedEvent.eventDeleted = false;

      if ( _events[i].date == data.date)
        _events[i] = updatedEvent;
      else if ( _events[i].date < data.date)
      {
        uint16_t updatedEventIdx = i+1;
        while (updatedEventIdx < _eventCount && _events[updatedEventIdx].date < data.date)
          updatedEventIdx++;
        if (updatedEventIdx == _eventCount)
          updatedEventIdx--;
        memmove(&_events[i], &_events[i+1], (updatedEventIdx - i) * sizeof(event_data));
        _events[updatedEventIdx] = updatedEvent;
      } else
      {
        uint16_t idx = i;
        if (i > 0)
        {
          while (idx>0 && _events[idx - 1].date > data.date)
            --idx;
          if (i > idx)
            memmove(&_events[idx+1], &_events[idx], (i - idx) * sizeof(event_data));
        }
        _events[idx] = updatedEvent;
      }
      return;
    }
    if (_events[i].date > data.date)
    {
      if (insertIdx == -1 || _events[i].date == _events[insertIdx].date)
        insertIdx = i;
    }
  }

  if (insertIdx == -1)
  {
    _events[_eventCount] = data;
    _events[_eventCount].id = _nextId++;
  }
  else
  {
    memmove(&_events[insertIdx+1], &_events[insertIdx], (_eventCount - insertIdx) * sizeof(event_data));
    _events[insertIdx] = data;
    _events[insertIdx].id = _nextId++;
  }
  _eventCount++; _actualEventCount++;
}

RecordData::event_data*
RecordData::Iterator::next()
{
  if (_currentIdx == *_eventCount)
    return NULL;

  while (_events[_currentIdx].eventDeleted &&  ++_currentIdx < *_eventCount);

  if (_currentIdx == *_eventCount)
    return NULL;
  return &_events[_currentIdx++];
}

void
RecordData::Iterator::remove()
{
  if (_currentIdx == 0 || (_currentIdx - 1 == *_eventCount))
    return;

  _events[_currentIdx - 1].eventDeleted = true;
  *_actualEventCount--;
}

uint32_t
RecordData::getSize() const
{
  static uint32_t eventDataLen =
      static_cast<uint32_t>(sizeof(uint64_t) + sizeof(uint8_t) + sizeof(AbntAddrValue) +
                            sizeof(uint16_t) + sizeof(uint8_t));
  static uint32_t recordFixedLen =
      static_cast<uint32_t>(sizeof(_schedTime) + sizeof(_eventCount) +
                            sizeof(_lastError) + sizeof(_calledNum));

  return recordFixedLen + eventDataLen * _actualEventCount;
}

void
RecordData::serialize(variable_data* record_for_write)
{
  util::SerializationBuffer buffer;

  buffer.setExternalBuffer(record_for_write->value + record_for_write->currentPos,
                           record_for_write->length - record_for_write->currentPos);

  buffer.WriteNetInt64(_schedTime);
  buffer.WriteByte(_actualEventCount);
  buffer.WriteNetInt16(_lastError);
  buffer.Write(_calledNum.full_addr, sizeof(_calledNum.full_addr));

  for (uint16_t i = 0; i < _eventCount; ++i)
  {
    if ( !_events[i].eventDeleted )
    {
      buffer.WriteNetInt64(_events[i].date);
      buffer.WriteByte(_events[i].id);
      buffer.Write(_events[i].callingNum.full_addr, sizeof(_events[i].callingNum.full_addr));
      buffer.WriteNetInt16(_events[i].callCount);
      buffer.WriteByte(_events[i].flags);
    }
  }

  _calculatedCrc = util::crc32(0, buffer.getBuffer(), buffer.getPos());
  record_for_write->currentPos += buffer.getPos();
}

void
RecordData::deserialize(variable_data* record_data)
{
  util::SerializationBuffer buffer;
  buffer.setExternalBuffer(record_data->value + record_data->currentPos,
                           record_data->length - record_data->currentPos);

  _schedTime = buffer.ReadNetInt64();
  _eventCount = _actualEventCount = buffer.ReadByte();
  _lastError = buffer.ReadNetInt16();
  buffer.Read(_calledNum.full_addr, sizeof(_calledNum.full_addr));

  for (uint16_t i = 0; i < _eventCount; i++)
  {
    _events[i].date = buffer.ReadNetInt64();
    _events[i].id = buffer.ReadByte();
    buffer.Read(_events[i].callingNum.full_addr, sizeof(_events[i].callingNum.full_addr));
    _events[i].callCount = buffer.ReadNetInt16();
    _events[i].flags = buffer.ReadByte();
  }

  _calculatedCrc = util::crc32(0, buffer.getBuffer(), buffer.getPos());
  record_data->currentPos += buffer.getPos();
}

std::string
RecordData::toString() const
{
  char buf[1024];
  AbntAddr addr;
  int offset= snprintf(buf, sizeof(buf), "schedTime=%llu,eventCount=%u,actualEventCount=%d,lastError=%u,calledNum=%s",
                       _schedTime, _eventCount, _actualEventCount, _lastError, AbntAddr(_calledNum.full_addr).getText().c_str());
  for (uint16_t i = 0; i < _eventCount; i++)
  {
    if ( !_events[i].eventDeleted )
      offset += snprintf(buf+offset, sizeof(buf)-offset, ",event={date=%llu,id=%u,callingNum=%s,callCount=%u,flags=%u}",
                         _events[i].date, _events[i].id, AbntAddr(_events[i].callingNum.full_addr).getText().c_str(),
                         _events[i].callCount, _events[i].flags);
  }
  return buf;
}

void
RecordHeader::serialize(variable_data* record_for_write) const
{
  util::SerializationBuffer buffer;
  buffer.setExternalBuffer(record_for_write->value, record_for_write->length);

  buffer.WriteNetInt32(_totalRecordLen);
  buffer.WriteNetInt16(static_cast<uint16_t>(_id.length()));
  buffer.Write(_id.data(), _id.length());
  buffer.WriteByte(_recordDeleted);

  record_for_write->currentPos = buffer.getPos();
}

void
RecordHeader::deserialize(variable_data* record_data)
{
  util::SerializationBuffer buffer;
  buffer.setExternalBuffer(record_data->value + record_data->currentPos,
                           record_data->length - record_data->currentPos);

  _totalRecordLen = buffer.ReadNetInt32();
  uint16_t idLen = buffer.ReadNetInt16();
  variable_data buf;
  buf.allocateSpace(idLen);
  buffer.Read(buf.value, idLen);
  _id.assign(buf.value, buf.value + idLen);
  _recordDeleted = buffer.ReadByte();

  record_data->currentPos += buffer.getPos();
}

void
RecordTail::serialize(variable_data* record_for_write) const
{
  util::SerializationBuffer buffer;

  buffer.setExternalBuffer(record_for_write->value + record_for_write->currentPos,
                           record_for_write->length - record_for_write->currentPos);

  buffer.WriteNetInt32(_crc);
  record_for_write->currentPos += buffer.getPos();
}

void
RecordTail::deserialize(variable_data* record_data)
{
  util::SerializationBuffer buffer;
  buffer.setExternalBuffer(record_data->value + record_data->currentPos,
                           record_data->length - record_data->currentPos);

  _crc = buffer.ReadNetInt32();
  record_data->currentPos += buffer.getPos();
}

void
FileRecord::serialize(variable_data* file_record)
{
  uint32_t totalRecordLen = _recordHeader.getSize() + _recordData->getSize() + _recordTail.getSize();

  file_record->allocateSpace(totalRecordLen);
  file_record->length = totalRecordLen;
  _recordHeader.setRecordLen(totalRecordLen);
  _recordHeader.serialize(file_record);
  _recordData->serialize(file_record);
  _recordTail.setCrc(_recordData->calculateCrc());
  _recordTail.serialize(file_record);
}

void
FileRecord::deserialize(variable_data* record_data)
{
  _recordHeader.deserialize(record_data);
  if (_recordData)
    delete _recordData;
  if (_recordHeader.isDeleted())
    _recordData = NULL;
  else {
    _recordData = new RecordData();

    _recordData->deserialize(record_data);
    _recordTail.deserialize(record_data);
    if ( _recordTail.getCrc() != _recordData->calculateCrc() )
      throw util::Exception("FileRecord::deserialize::: corrupted file record: expected crc=0x%08X, calculated crc=0x%08X",
                            _recordTail.getCrc(), _recordData->calculateCrc());
  }
}

const char* FileHeader::MAGIC_STAMP = "mci_storage";

void
FileHeader::serialize(variable_data* file_header)
{
  util::SerializationBuffer buffer;
  file_header->allocateSpace(HEADER_SIZE);
  file_header->length = HEADER_SIZE;
  buffer.setExternalBuffer(file_header->value + file_header->currentPos,
                           file_header->length - file_header->currentPos);

  buffer.Write(MAGIC_STAMP, MAGIC_STAMP_LEN);
  buffer.WriteNetInt16(STORAGE_VERSION);

  file_header->currentPos += buffer.getPos();
}

bool
FileHeader::deserialize(variable_data* file_data_buf)
{
  util::SerializationBuffer buffer;
  buffer.setExternalBuffer(file_data_buf->value + file_data_buf->currentPos,
                           file_data_buf->length - file_data_buf->currentPos);

  char magicStamp[12]; // null-terminated string
  buffer.Read(magicStamp, MAGIC_STAMP_LEN);

  if (strncmp(magicStamp, MAGIC_STAMP, MAGIC_STAMP_LEN))
    return true;

  uint16_t storageVersion = buffer.ReadNetInt16();
  if (storageVersion != STORAGE_VERSION)
    return true;

  file_data_buf->currentPos += buffer.getPos();

  return false;
}

int RollingThread::Execute()
{
  _isRunning = true;
  _threadHasActivated.Signal();

  logger::Logger* logger = logger::Logger::getInstance("rfstorage");
  smsc_log_debug(logger, "RollingThread::Execute::: started thread");
  while(isRunning()) {
    smsc_log_debug(logger, "RollingThread::Execute::: waiting for rolling signal");
    _rollingStorage.waitForRollingActivation();
    if (!isRunning())
      break;
    smsc_log_debug(logger, "RollingThread::Execute::: start do rolling");
    _rollingStorage.makeFileRolling();
  }
  _threadHasStopped.Signal();

  return 0;
}

void
RollingThread::start()
{
  Start();
  _threadHasActivated.Wait();
}

void
RollingThread::stop()
{
  if (_isRunning) {
    _isRunning = false;
    _rollingStorage._makeRollingEvent.Signal(); // to wake up RollingThread that possible is sleeping on a makeRollingEvent object
    _threadHasStopped.Wait();
  }
}

RollingFileStorage::RollingFileStorage(const string& location,
                                       const std::string& file_prefix,
                                       size_t max_file_size,
                                       unsigned max_number_of_files)
: _fileNamePrefix(file_prefix), _location(location),
  _maxFileSize(max_file_size), _fd(-1), _currentFileSize(0), _activeFileSeqNum(-1),
  _maxNumberOfFiles(max_number_of_files), _rollingInProgress(false),
  _rollingThread(*this), _logger(logger::Logger::getInstance("mci.store"))
{
  if (_location[_location.length() - 1] != '/')
    _location += '/';
}

void
RollingFileStorage::start()
{
  loadRecords();
  // create new storage file if no files matched to the pattern were found
  if (_fd < 0)
    createFile();

  _rollingThread.start();
}

void
RollingFileStorage::stop()
{
  _rollingThread.stop();
}

std::string
RollingFileStorage::formFileName() const
{
  char fileNameSuffix[128];
  tm lcltm;
  time_t curTime = time(0);
  strftime(fileNameSuffix, sizeof(fileNameSuffix), ".%Y%m%d%H%M%S", localtime_r(&curTime, &lcltm));
  std::string fileName = _location + _fileNamePrefix + fileNameSuffix;

  if (!_activeFilesArray.empty()) {
    const std::string& lastActiveFileName = _activeFilesArray.back().fileName;

    // file format is base_name.timestampe.uniq_idx, e.g. test_rolling.20110830202111.005
    // uniq_idx presents only if there are several files with the same prefix base_name.timestampe
    std::string::size_type fNameStartIdx = lastActiveFileName.rfind('/') + 1;
    std::string::size_type fNameDotIdx = lastActiveFileName.find('.', fNameStartIdx);
    const std::string& lastActiveFileNameSuffix = lastActiveFileName.substr(fNameDotIdx);

    if (!strncmp(lastActiveFileNameSuffix.c_str(), fileNameSuffix, strlen(fileNameSuffix))) {
      unsigned i = 0;

      std::string::size_type uniq_id_pos = lastActiveFileNameSuffix.find('.', 1); // start search from next char after the first '.'
      if (uniq_id_pos != std::string::npos) {
        sscanf(lastActiveFileNameSuffix.substr(uniq_id_pos + 1).c_str(), "%d", &i);
        ++i;
      }
      char idx_str[32];
      sprintf(idx_str, ".%03u", i);

      fileName += idx_str;
    }
  }

  return fileName;
}

void
RollingFileStorage::createFile()
{
  std::string fileName = formFileName();

  initializeFile(fileName);

  // access to _activeFilesArray protected by mutex locked by deleteRecord, saveRecord
  _activeFilesArray.push_back(file_info(fileName));
  smsc_log_debug(_logger, "RollingFileStorage::createFile::: _activeFileSeqNum=%d, _maxNumberOfFiles=%d, created filename='%s'",
                 _activeFileSeqNum, _maxNumberOfFiles, fileName.c_str());
  if (++_activeFileSeqNum >= _maxNumberOfFiles) {
    smsc_log_debug(_logger, "RollingFileStorage::createFile::: activate makeFileRolling");
    _makeRollingEvent.Signal();  // activate makeFileRolling()
    _rollingInProgressEvent.Wait(); // waiting for while rolling will started
  }
}

void
RollingFileStorage::initializeFile(const std::string& file_name)
{
  _fd = open(file_name.c_str(), O_RDWR | O_APPEND | O_CREAT | O_EXCL, 0600);
  if (_fd < 0)
    throw util::SystemError("RollingFileStorage::initializeFile::: can't create file '%s'", file_name.c_str());

  variable_data fileHeaderBuf;
  FileHeader fileHeader;
  fileHeader.serialize(&fileHeaderBuf);

  if (write(_fd, fileHeaderBuf.value, fileHeaderBuf.length) != fileHeaderBuf.length)
    throw util::SystemError("RollingFileStorage::initializeFile::: can't write data to file");

  smsc_log_debug(_logger, "initializeFile: new file %s created", file_name.c_str());
  _currentFileSize = fileHeaderBuf.length;
}

void
RollingFileStorage::openFile(const std::string& full_qualified_file_name)
{
  _fd = open(full_qualified_file_name.c_str(), O_RDWR | O_APPEND, 0600);
  smsc_log_debug(_logger, "RollingFileStorage::openFile::: open(%s) return %d, errno=%d", full_qualified_file_name.c_str(), _fd, errno);
  if ( _fd < 0 )
    throw util::SystemError("RollingFileStorage::openFile::: can't open file '%s'", full_qualified_file_name.c_str());
}

void
RollingFileStorage::closeCurrentFile()
{
  if (_fd > -1) {
    ::close(_fd);
    _fd = -1;
    _currentFileSize = 0;
  }
}

void
RollingFileStorage::loadRecords()
{
  DIR* dir = opendir(_location.c_str());
  if (!dir)
    throw util::SystemError("RollingFileStorage::loadRecords: can't open directory '%s'", _location.c_str());

  std::vector<std::string> listOfFiles;
  struct dirent* nextDirEntry;
  errno = 0;
  while (nextDirEntry=readdir(dir)) {
    if (!strncmp(nextDirEntry->d_name, _fileNamePrefix.c_str(), _fileNamePrefix.length()))
      listOfFiles.push_back(nextDirEntry->d_name);
  }
  if (errno) {
    int save_errno = errno;
    closedir(dir);
    throw util::SystemError(save_errno, "RollingFileStorage::loadRecords: can't read from directory '%s'", _location.c_str());
  }
  closedir(dir);
  std::sort(listOfFiles.begin(), listOfFiles.end());

  for(std::vector<std::string>::const_iterator iter=listOfFiles.begin(); iter != listOfFiles.end();++iter) {
    closeCurrentFile();
    smsc_log_debug(_logger, "RollingFileStorage::loadRecords: processing of file %s", (*iter).c_str());
    openFile(*iter);
    loadFile(*iter);
  }
}

void
RollingFileStorage::loadFile(const std::string& file_name)
{
  // Since loadFile() called during startup only (i.e. from one thread)
  // we don't need protect access to internal structures (_fd, _activeFilesArray, etc)
  struct stat statbuf;
  variable_data readBuffer;

  if (fstat(_fd, &statbuf))
    throw util::SystemError("RollingFileStorage::loadFile::: fstat failed");

  readBuffer.allocateSpace(static_cast<uint32_t>(statbuf.st_size));
  if (read(_fd, readBuffer.value, statbuf.st_size) != statbuf.st_size)
    throw util::SystemError("RollingFileStorage::loadFile::: can't read from file");

  readBuffer.length = static_cast<uint32_t>(statbuf.st_size);

  FileHeader fileHeader;
  if (fileHeader.deserialize(&readBuffer))
  {
    smsc_log_error(_logger, "RollingFileStorage::loadFile: wrong format of file '%s'", file_name.c_str());
    return;
  }

  _activeFilesArray.push_back(file_info(file_name));
  _activeFileSeqNum++;
  do {
    FileRecord* newRecord = new FileRecord();
    try {
      newRecord->deserialize(&readBuffer);

      updateRecordInCache(newRecord);
    } catch (std::exception& ex) {
      delete newRecord;
      smsc_log_error(_logger, "RollingFileStorage::loadFile: caught exception '%s'", ex.what());
    }
  } while(readBuffer.currentPos < statbuf.st_size);

  _currentFileSize = statbuf.st_size;
}

void
RollingFileStorage::updateRecordInCache(FileRecord* new_record)
{
  FileRecordWrapper** foundRecord = _recordsCache.GetPtr(new_record->getId());
  if (foundRecord)
  {
    FileRecordWrapper* recordInCache = *foundRecord;

    if (new_record->isDeleted())
    {
      smsc_log_debug(_logger, "RollingFileStorage::updateRecordInCache: updatedRecord->getId()='%s', DELETE RECORD: new_record=%p, recordInCache=%p",
                     new_record->getId(), new_record, recordInCache);

      _recordsCache.Delete(new_record->getId());
      delete new_record;

      delete recordInCache->actualFileRecord;
      delete recordInCache;
    } else {
      unsigned long long modificationTime = getTimeInMsec();

      if (recordInCache->actualFileRecord != new_record)
      {
        delete recordInCache->actualFileRecord;
        recordInCache->actualFileRecord = NULL;
        recordInCache->actualFileRecord = new_record;
      }

      _activeFilesArray.back().fileRecords->push_back(FileRecordInfo(new_record->getId(), modificationTime));
      recordInCache->modificationTime = modificationTime;
    }
  } else if (!new_record->isDeleted())
  {
    FileRecordWrapper* new_record_wrap = new FileRecordWrapper(new_record);
    // DEBUG
    smsc_log_debug(_logger, "updateRecordInCache: _activeFileSeqNum=%d,"
                            " push_back newRecord::recordData=%s into _activeFilesArray.back().actualFileRecords=%p, new_record_wrap=0x%p",
                   _activeFileSeqNum, new_record->getRecordData()->toString().c_str(),
                   _activeFilesArray.back().fileRecords, new_record_wrap);
    // END DEBUG
    _activeFilesArray.back().fileRecords->push_back(FileRecordInfo(new_record->getId(), new_record_wrap->modificationTime));
    _recordsCache.Insert(new_record->getId(), new_record_wrap);
  }
}

void
RollingFileStorage::makeFileRolling()
{
  _rollingInProgress = true;
  _rollingInProgressEvent.Signal();
  file_info firstFInfo;

  {
    core::synchronization::MutexGuard synchronize(_internalStructureLock);

    firstFInfo = _activeFilesArray.front();

    _activeFilesArray.pop_front();
  }

  smsc_log_debug(_logger, "makeFileRolling: processing records from file %s", firstFInfo.fileName.c_str());
  while (!firstFInfo.fileRecords->empty()) {
    FileRecordInfo actualFileRec = firstFInfo.fileRecords->front();

    moveRecord(actualFileRec, firstFInfo.fileName);
    firstFInfo.fileRecords->pop_front();
  }

  smsc_log_debug(_logger, "makeFileRolling: remove file %s", firstFInfo.fileName.c_str());
  unlink(firstFInfo.fileName.c_str());

  delete firstFInfo.fileRecords;

  _rollingInProgress = false;
}

void
RollingFileStorage::moveRecord(FileRecordInfo& record, const std::string& orig_fname)
{
  core::synchronization::MutexGuard synchronize(_internalStructureLock);

  FileRecordWrapper** foundRecord = _recordsCache.GetPtr(record.recordId.c_str());
  if (!foundRecord)
  {
    smsc_log_debug(_logger, "makeFileRolling: file %s: record with id=%s was removed", orig_fname.c_str(), record.recordId.c_str());
    return;
  }
  if (record.insertTime == (*foundRecord)->modificationTime)
  {
    writeRecordToCurrentActiveFile((*foundRecord)->actualFileRecord, false);
    _activeFilesArray.back().fileRecords->push_back(FileRecordInfo((*foundRecord)->actualFileRecord->getId(),
                                                                   record.insertTime));

    smsc_log_debug(_logger, "makeFileRolling: move record %s from file %s to file %s",
                   (*foundRecord)->actualFileRecord->getRecordData()->toString().c_str(), orig_fname.c_str(),
                   _activeFilesArray.back().fileName.c_str());
  }
}

void
RollingFileStorage::waitForRollingActivation()
{
  smsc_log_debug(_logger, "RollingFileStorage::waitForRollingActivation::: enter in wait state");
  _makeRollingEvent.Wait();
  smsc_log_debug(_logger, "RollingFileStorage::waitForRollingActivation::: leave wait state");
}

void
RollingFileStorage::saveRecord(FileRecord* record)
{
  core::synchronization::MutexGuard synchronize(_internalStructureLock);
  writeRecordToCurrentActiveFile(record, true);
  updateRecordInCache(record);
}

void
RollingFileStorage::writeRecordToCurrentActiveFile(FileRecord* record, bool do_switch)
{
  variable_data preparedData;

  record->serialize(&preparedData);

  if (record->getRecordData())
    smsc_log_debug(_logger, "writeRecord:: _maxFileSize=%lu,_currentFileSize=%lu,preparedData.length=%u,recordData->getEventCount()=%d,isRollingInProgress()=%d,do_switch=%d",
                   _maxFileSize, _currentFileSize, preparedData.length, record->getRecordData()->getEventCount(), isRollingInProgress(), do_switch);

  if (do_switch && (_currentFileSize + preparedData.length >= _maxFileSize) && !isRollingInProgress())
  {
    closeCurrentFile();
    createFile();
  }

  if (write(_fd, preparedData.value, preparedData.length) != preparedData.length)
    throw util::SystemError("RollingFileStorage::saveRecord::: can't write data to file");

  _currentFileSize += preparedData.length;
  if (record->getRecordData())
    smsc_log_debug(_logger, "writeRecord:: write record='%s', _activeFileSeqNum=%d, _currentFileSize=%lu", record->getRecordData()->toString().c_str(), _activeFileSeqNum, _currentFileSize);
}

FileRecord*
RollingFileStorage::getRecord(const char* record_id)
{
  core::synchronization::MutexGuard synchronize(_internalStructureLock);
  FileRecordWrapper** foundRecord = _recordsCache.GetPtr(record_id);
  if (foundRecord)
    return (*foundRecord)->actualFileRecord;
  else
    return NULL;
}

void
RollingFileStorage::deleteRecord(const char* record_id)
{
  core::synchronization::MutexGuard synchronize(_internalStructureLock);
  FileRecordWrapper** foundRecord = _recordsCache.GetPtr(record_id);
  if (foundRecord) {
    FileRecordWrapper* record = *foundRecord;
    _recordsCache.Delete(record_id);

    AbntAddr recordId(record->actualFileRecord->getRecordData()->getCalledNum().full_addr);
    DeletedFileRecord erasedRecord(recordId.getText());
    writeRecordToCurrentActiveFile(&erasedRecord, true);

    smsc_log_debug(_logger, "RollingFileStorage::deleteRecord: delete record=%p, record->actualFileRecord=%p for record_id=%s", record, record->actualFileRecord, record_id);
    delete record->actualFileRecord;
    delete record;
  }
}

void
RollingFileStorage::close()
{
  core::synchronization::MutexGuard synchronize(_internalStructureLock);

  if (_fd < 0)
    return;

  ::close(_fd);
  _fd = -1;

  core::buffers::Hash<FileRecordWrapper*>::Iterator iter = _recordsCache.getIterator();
  iter.First();
  char* key;
  FileRecordWrapper* record;
  while(iter.Next(key, record)) {
    smsc_log_debug(_logger, "RollingFileStorage::close: delete record=%p, delete record->actualFileRecord=%p", record, record->actualFileRecord);
    delete record->actualFileRecord;
    delete record;
  }
}

void
RollingFileStorage::printDebug()
{
  core::synchronization::MutexGuard synchronize(_internalStructureLock);
  for(std::deque<file_info>::const_iterator iter = _activeFilesArray.begin(), end_iter = _activeFilesArray.end();
      iter != end_iter; ++iter) {
    const file_info& fInfo = *iter;
    smsc_log_debug(_logger, "\n\n%s\n", fInfo.fileName.c_str());

    for (file_records_info_t::const_iterator af_iter = fInfo.fileRecords->begin(), af_end_iter = fInfo.fileRecords->end();
        af_iter != af_end_iter; ++af_iter) {
      FileRecordWrapper** foundRecord = _recordsCache.GetPtr((*af_iter).recordId.c_str());
      FileRecord* actualFileRecord = (*foundRecord)->actualFileRecord;
      if (actualFileRecord && actualFileRecord->getRecordData())
        smsc_log_debug(_logger, "\t%s\n", actualFileRecord->getRecordData()->toString().c_str());
    }
  }
}

}}
