#include "RollingFileStorageAdapter.hpp"
#include "MCAEventsStorage.hpp"

namespace smsc {
namespace mcisme {

int
RollingFileStorageAdapter::Init(smsc::util::config::ConfigView* storage_cfg_section, DeliveryQueue* delivery_queue)
{
  std::string location;
  try {
    location = storage_cfg_section->getString("location");
    if ( location[location.size() - 1] != '/' )
      location += '/';
  } catch (...) {
    location = "store/";
    smsc_log_warn(_logger, "Parameter <MCISme.Storage.location> missed. Default value is './'.");
  }

  string sEventLifeTime;
  try {
    sEventLifeTime = storage_cfg_section->getString("eventLifeTime");
  } catch (...) {
    sEventLifeTime = "24:00:00";
    smsc_log_warn(_logger, "Parameter <MCISme.Storage.eventLifeTime> missed. Default value is '24:00:00'.");
  }

  unsigned maxFileSize;
  try {
    maxFileSize = storage_cfg_section->getInt("maxFileSize");
  } catch (...) {
    maxFileSize = MAX_FILE_SIZE;
    smsc_log_warn(_logger, "Parameter <MCISme.Storage.maxFileSize> missed. Default value is %d.", MAX_FILE_SIZE);
  }

  unsigned maxNumberOfFiles;
  try {
    maxNumberOfFiles = storage_cfg_section->getInt("maxNumberOfFiles");
  } catch (...) {
    maxNumberOfFiles = MAX_NUMBER_OF_FILES;
    smsc_log_warn(_logger, "Parameter <MCISme.Storage.maxNumberOfFiles> missed. Default value is %d.", MAX_NUMBER_OF_FILES);
  }

  int eventLifeTime = parseTime(sEventLifeTime.c_str());
  if (eventLifeTime < 0)
    throw util::Exception("RollingFileStorageAdapter::Init: invalid value of eventLifeTime parameter");

  init(location, eventLifeTime, maxFileSize, maxNumberOfFiles, delivery_queue);
  return 0;
}

void
RollingFileStorageAdapter::init(const string& location, unsigned event_life_time, unsigned max_file_size,
                                unsigned max_number_of_files, DeliveryQueue* delivery_queue)
{
  _eventLifeTime = event_life_time;

  _rollingFileStorage = new RollingFileStorage(location, "mci_storage", max_file_size, max_number_of_files);
  _rollingFileStorage->start();

  if (delivery_queue)
    loadEvents(delivery_queue);
}

int
RollingFileStorageAdapter::Init(const string& location, time_t event_life_time, uint8_t max_events, DeliveryQueue* delivery_queue)
{
  return -1;
}

RollingFileStorageAdapter::~RollingFileStorageAdapter()
{
  _rollingFileStorage->stop();
  delete _rollingFileStorage;
}

void
RollingFileStorageAdapter::loadEvents(DeliveryQueue* delivery_queue)
{
  smsc_log_info(_logger, "RollingFileStorageAdapter: Loading events from file storage...");

  RollingFileStorage::Iterator iter = _rollingFileStorage->getIterator();
  FileRecord* fileRecord;
  while((fileRecord = iter.next()))
  {
    const RecordData* recordData = fileRecord->getRecordData();
    const AbntAddrValue& calledAddr = recordData->getCalledNum();
    AbntAddr calledAbnt(&calledAddr);
    delivery_queue->Schedule(calledAbnt, false, static_cast<time_t>(recordData->getSchedTime()), recordData->getLastError());
  }
}

void
RollingFileStorageAdapter::addEvent(const AbntAddr& called_num, const MCEvent& event, time_t sched_time)
{
  core::synchronization::MutexGuard synchronize(_lock);

  FileRecord* record = _rollingFileStorage->getRecord(called_num.getText().c_str());
  if (record)
  {
    RecordData* recordData = record->getRecordData();
    killExpiredData(recordData);
    smsc_log_debug(_logger, "RollingFileStorageAdapter::addEvent: addOrReplace event={dt=%lld,caller=%s,missCallFlags=%d}",
                   event.dt, smsc::mcisme::AbntAddr(&event.caller).getText().c_str(), event.missCallFlags);
    recordData->addOrReplaceEventData(RecordData::event_data(event.dt, event.caller, 1, event.missCallFlags));
    _rollingFileStorage->saveRecord(record);
  } else
  {
    AbntAddrValue calledAddress;
    memcpy(calledAddress.full_addr, called_num.getAddrSig(), sizeof(calledAddress.full_addr));
    RecordData* recordData = new RecordData(sched_time, -1, calledAddress);

    smsc_log_debug(_logger, "RollingFileStorageAdapter::addEvent: add new event={dt=%lld,caller=%s,missCallFlags=%d}",
                   event.dt, smsc::mcisme::AbntAddr(&event.caller).getText().c_str(), event.missCallFlags);
    recordData->addEventData(RecordData::event_data(event.dt, event.caller, 1, event.missCallFlags));
    FileRecord* newRecord = new FileRecord(recordData);
    _rollingFileStorage->saveRecord(newRecord);
  }
}

void
RollingFileStorageAdapter::setSchedParams(const AbntAddr& called_num, time_t sched_time, uint16_t last_error)
{
  core::synchronization::MutexGuard synchronize(_lock);

  FileRecord* record = _rollingFileStorage->getRecord(called_num.getText().c_str());
  if (record)
  {
    RecordData* recordData = record->getRecordData();
    recordData->setSchedTime(sched_time);
    recordData->setLastError(last_error);
    _rollingFileStorage->saveRecord(record);
  }
}

bool
RollingFileStorageAdapter::getEvents(const AbntAddr& called_num, vector<MCEvent>& events)
{
  core::synchronization::MutexGuard synchronize(_lock);

  FileRecord* record = _rollingFileStorage->getRecord(called_num.getText().c_str());
  if (record)
  {
    RecordData* recordData = record->getRecordData();
    smsc_log_debug(_logger, "RollingFileStorageAdapter::getEvents: found record=%s", recordData->toString().c_str());
    if (killExpiredData(recordData))
    {
      if (recordData->getEventCount() == 0)
      {
        _rollingFileStorage->deleteRecord(record->getId());
        return false;
      } else
        _rollingFileStorage->saveRecord(record);
    }

    RecordData::Iterator iter = recordData->getIterator();
    RecordData::event_data* eventData;
    while ((eventData = iter.next()))
    {
      MCEvent e;
      e.id = eventData->id;
      e.dt = eventData->date;
      e.missCallFlags = eventData->flags;
      e.callCount = eventData->callCount;
      memcpy(e.caller.full_addr, eventData->callingNum.full_addr, sizeof(e.caller.full_addr));
      events.push_back(e);
      smsc_log_debug(_logger, "RollingFileStorageAdapter::getEvents::: push MCEvent={id=%d,dt=%lld,callingNum=%s,callCount=%d,missCallFlags=%d",
                     e.id, e.dt, AbntAddr(&e.caller).getText().c_str(), e.callCount, e.missCallFlags);
    }
    return true;
  } else
    return false;
}

void
RollingFileStorageAdapter::deleteEvents(const AbntAddr& called_num, const vector<MCEvent>& events)
{
  core::synchronization::MutexGuard synchronize(_lock);

  FileRecord* record = _rollingFileStorage->getRecord(called_num.getText().c_str());
  if (record)
  {
    RecordData* recordData = record->getRecordData();
    if(recordData->getEventCount() == 0)
    {
      _rollingFileStorage->deleteRecord(called_num.getText().c_str());
      return;
    }

    for(size_t i = events.size(); i > 0; i--)
      recordData->removeEventData(events[i-1].id);

    killExpiredData(recordData);

    if (recordData->getEventCount() == 0)
      _rollingFileStorage->deleteRecord(called_num.getText().c_str());
    else
      _rollingFileStorage->saveRecord(record);
  }
}

bool
RollingFileStorageAdapter::killExpiredData(RecordData* record_data)
{
  RecordData::Iterator iter = record_data->getIterator();
  RecordData::event_data* eventData;
  int count=0;
  time_t curTime = time(0);
  while ((eventData = iter.next()))
  {
    if (eventData->date + _eventLifeTime > curTime)
      break;
    smsc_log_debug(_logger, "RollingFileStorageAdapter::killExpiredData: delete expired event={id=%d,callingNum=%s,date=%lu,flags=%d,callCount=%d}",
                   eventData->id, smsc::mcisme::AbntAddr(&eventData->callingNum).getText().c_str(),
                   eventData->date, eventData->flags, eventData->callCount);
    store_E_Event_in_logstore(record_data->getCalledNum(), eventData->callingNum);
    iter.remove();
    ++count;
  }
  return bool(count);
}

void
RollingFileStorageAdapter::store_E_Event_in_logstore(const AbntAddrValue& called_abonent,
                                                     const AbntAddrValue& calling_abonent)
{
  const std::string& calledAbonentAddr = AbntAddr(&called_abonent).getText();
  const std::string& callingAbonentAddr = AbntAddr(&calling_abonent).getText();
  MCAEventsStorageRegister::getMCAEventsStorage().addEvent(Event_DeleteMissedCallInfo(callingAbonentAddr, calledAbonentAddr));
}

}}
