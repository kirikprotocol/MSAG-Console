//------------------------------------
//  FSStorage.cpp
//  Routman Michael, 2005-2006
//------------------------------------

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <vector>
#include <list>

#include <core/buffers/File.hpp>
#include <core/synchronization/Mutex.hpp>

#include "Storage.hpp"
#include "FSStorage.hpp"
#include "AbntAddr.hpp"
#include "DeliveryQueue.hpp"

#include "MCAEventsStorage.hpp"

//extern std::string hexdmp(const uchar_t* buf, uint32_t bufSz);

namespace smsc {
namespace mcisme {

using std::vector;
using std::list;
using core::buffers;
using sms::Address;
using core::synchronization::Mutex;
using core::synchronization::MutexGuard;
using smsc::util::config::ConfigView;

extern int parseTime(const char* str);

FSStorage::FSStorage(): logger(smsc::logger::Logger::getInstance("mci.fsStor")), _first_free_cell_num(0)
{}

FSStorage::~FSStorage()
{
  CloseFiles();
  smsc_log_debug(logger, "FSStorage: Destroed.");
}

int FSStorage::Init(ConfigView* storageConfig, DeliveryQueue* pDeliveryQueue)
{
  MutexGuard lock(mut);

  smsc_log_debug(logger, "Reading Storage parameters.");
  string location;
  try {
    location = storageConfig->getString("location");
    if ( location[location.size() - 1] != '/' )
      location += '/';
  } catch (...) {
    location = "./";
    smsc_log_warn(logger, "Parameter <MCISme.Storage.location> missed. Default value is './'.");
  }

  try { maxEvents = storageConfig->getInt("maxEvents"); } catch (...){maxEvents = 20;
    smsc_log_warn(logger, "Parameter <MCISme.Storage.maxEvents> missed. Default value is 20.");}

  string sEventLifeTime;
  try { sEventLifeTime = storageConfig->getString("eventLifeTime"); } catch (...){sEventLifeTime = "24:00:00";
    smsc_log_warn(logger, "Parameter <MCISme.Storage.eventLifeTime> missed. Default value is '24:00:00'.");}
  eventLifeTime = parseTime(sEventLifeTime.c_str());

  try { bdFilesIncr = storageConfig->getInt("bdFilesIncr"); } catch (...){bdFilesIncr = DEFAULT_BDFILES_INCR;
    smsc_log_warn(logger, "Parameter <MCISme.Storage.bdFilesIncr> missed. Default value is %s.", DEFAULT_BDFILES_INCR);}

  std::string fileVersionSuffix = ".v3";
  pathDatFile = location + "dat_file" + fileVersionSuffix;

  if(maxEvents > MAX_EVENTS) maxEvents = MAX_EVENTS;
  if(bdFilesIncr > MAX_BDFILES_INCR) bdFilesIncr = MAX_BDFILES_INCR;

  if(0 != OpenFiles())
    return 1;

  smsc_log_debug(logger, "FSStorage: All files opened");
  if(0 != CompleteTransaction())
    return 2;

  if(0 != LoadEvents(pDeliveryQueue))
    return 3;

  return 0;	
}

int FSStorage::Init(const string& location, time_t _eventLifeTime, uint8_t _maxEvents, DeliveryQueue* pDeliveryQueue)
{
  MutexGuard lock(mut);

  std::string directory = location;

  if ( directory[directory.size() - 1] != '/' )
    directory += '/';

  std::string fileVersionSuffix = ".v3";
  pathDatFile = location + "dat_file" + fileVersionSuffix;

  eventLifeTime = _eventLifeTime;
  maxEvents <= MAX_EVENTS? maxEvents = _maxEvents: maxEvents = MAX_EVENTS;

  bdFilesIncr = DEFAULT_BDFILES_INCR;

  if(0 != OpenFiles())
    return 1;

  smsc_log_debug(logger, "FSStorage: All files opened");
  if(0 != CompleteTransaction())
    return 2;

  if(0 != LoadEvents(pDeliveryQueue))
    return 3;

  return 0;
}

void FSStorage::addEvent(const AbntAddr& CalledNum, const MCEvent& event, time_t schedTime)
{
  MutexGuard lock(mut);

  if(hashAbnt.Exists(CalledNum))
  {
    dat_file_cell AbntEvents={0};

    LoadAbntEvents(CalledNum, &AbntEvents);
    KillExpiredEvents(&AbntEvents);
    AddAbntEvent(&AbntEvents, event);
    SaveAbntEvents(CalledNum, &AbntEvents);
  }
  else
    CreateAbntEvents(CalledNum, event, schedTime);
}

void FSStorage::setSchedParams(const AbntAddr& CalledNum, time_t schedTime, uint16_t lastError)
{
  MutexGuard lock(mut);

  if(hashAbnt.Exists(CalledNum))
  {
    dat_file_cell AbntEvents;

    LoadAbntEvents(CalledNum, &AbntEvents);
    AbntEvents.schedTime = schedTime;
    AbntEvents.lastError = lastError;
    SaveAbntEvents(CalledNum, &AbntEvents);
  }
}

bool FSStorage::getEvents(const AbntAddr& CalledNum, vector<MCEvent>& events)
{
  MutexGuard lock(mut);

  smsc_log_debug(logger, "FSStorage::getEvents::: try get events for called abonent=[%s]", CalledNum.getText().c_str());
  if(hashAbnt.Exists(CalledNum))
  {
    dat_file_cell AbntEvents;

    LoadAbntEvents(CalledNum, &AbntEvents);

    //    smsc_log_debug(logger, "FSStorage::getEvents::: loaded data_cell: schedTime=%x,eventCount=%d,lastError=%d,calledNum=%s,recordIsActive=%d", AbntEvents.schedTime, AbntEvents.eventCount,AbntEvents.lastError,AbntAddr(&AbntEvents.calledNum).getText().c_str(), AbntEvents.recordIsActive);
    if(KillExpiredEvents(&AbntEvents))
      if(AbntEvents.eventCount == 0)
      {
        DestroyAbntEvents(CalledNum);
        return 0;
      }
      else
        SaveAbntEvents(CalledNum, &AbntEvents);

    for(int i = 0; i < AbntEvents.eventCount; i++)
    {
      MCEvent e;
      e.id = AbntEvents.events[i].id;
      e.dt = AbntEvents.events[i].date;
      e.missCallFlags = AbntEvents.events[i].flags;
      e.callCount = AbntEvents.events[i].callCount;
      memcpy((void*)&(e.caller), (void*)&(AbntEvents.events[i].callingNum), sizeof(e.caller));
      events.push_back(e);
      //      smsc_log_debug(logger, "FSStorage::getEvents::: AbntEvents.events[%d].id=%d,AbntEvents.events[%d].date=%x,AbntEvents.events[%d].callingNum=%s,AbntEvents.events[%d].callCount=%d,AbntEvents.events[%d].flags=%d",i,AbntEvents.events[i].id,i,AbntEvents.events[i].date,i,AbntAddr(&AbntEvents.events[i].callingNum).getText().c_str(),i,AbntEvents.events[i].callCount,AbntEvents.events[i].flags);
    }
    return 1;
  }
  else
    return 0;
}

void FSStorage::deleteEvents(const AbntAddr& CalledNum, const vector<MCEvent>& events)
{
  MutexGuard lock(mut);

  if(hashAbnt.Exists(CalledNum))
  {
    dat_file_cell AbntEvents;

    LoadAbntEvents(CalledNum, &AbntEvents);
    RemoveAbntEvents(&AbntEvents, events);
    KillExpiredEvents(&AbntEvents);
    if(AbntEvents.eventCount == 0)
    {
      DestroyAbntEvents(CalledNum);
      return;
    }
    SaveAbntEvents(CalledNum, &AbntEvents);
  }
  else
    return;
}

int
FSStorage::OpenFiles(void)
{
  try
  {
    if(File::Exists(pathDatFile.c_str()))
      _dat_file.RWOpen(pathDatFile.c_str());
    else
      _dat_file.RWCreate(pathDatFile.c_str());

    _dat_file.SetUnbuffered();
  }
  catch(FileException& ex)
  {
    smsc_log_debug(logger, "FSStorage: error dat_file - %s", ex.what());
    return 1;
  }

  return 0;
}

void FSStorage::CloseFiles(void)
{
  _dat_file.Close();
}

int FSStorage::LoadEvents(DeliveryQueue* pDeliveryQueue)
{
  smsc_log_info(logger, "FSStorage: Loading DataBase from files.......");

  unsigned int cell_num=0;

  try
  {
    uint32_t      rb, i, count;
    off_t         size;
    dat_file_cell abntEvent;

    while(_dat_file.Read((void*)&abntEvent, sizeof(abntEvent)))
    {
      if(!abntEvent.recordIsActive)
        freeCells.AddCell(cell_num);
      else
      {
        try {
          AbntAddr calledAbnt(&abntEvent.calledNum);
          hashAbnt.Insert(calledAbnt, cell_num);
          if ( pDeliveryQueue )
            pDeliveryQueue->Schedule(calledAbnt, false, abntEvent.schedTime, abntEvent.lastError);
        } catch (BadAddrException& ex) {}
      }
      cell_num++;
    }
  }
  catch(FileException& ex)
  {}

  _first_free_cell_num = cell_num;

  return 0;
}

int FSStorage::CompleteTransaction(void)
{
  return 0;
}

int FSStorage::CreateAbntEvents(const AbntAddr& CalledNum, const MCEvent& event, time_t schedTime)
{
  cell_t        cell;
  dat_file_cell	dat = {0};

  dat.eventCount = 1;
  dat.schedTime = schedTime;
  dat.lastError = -1;
  dat.recordIsActive = 1;
  memcpy((void*)&dat.calledNum, (void*)CalledNum.getAddrSig(), sizeof(dat.calledNum));

  dat.events[0].date = event.dt;
  dat.events[0].id = 0;
  memcpy((void*)&(dat.events[0].callingNum), (void*)&(event.caller), sizeof(dat.events[0].callingNum));
  dat.events[0].callCount = 1;
  dat.events[0].flags = event.missCallFlags;

  //  smsc_log_debug(logger, "FSStorage::CreateAbntEvents::: called addr=[%s], insert event into idx=0, event_cell.date=0x%x,event_cell.id=%d,event_cell.callingNum=%s,event_cell.callCount=%d,event_cell.flags=0x%x", AbntAddr(&dat.calledNum).getText().c_str(), dat.events[0].date, dat.events[0].id, AbntAddr(&dat.events[0].callingNum).getText().c_str(), dat.events[0].callCount, dat.events[0].flags);
  if(freeCells.Empty())
    IncrStorage(bdFilesIncr);

  cell = freeCells.GetCell();
  smsc_log_debug(logger, "FSStorage::CreateAbntEvents::: freeCells.GetCell() returned %d", cell);
  try
  {
    _dat_file.Seek(cell*sizeof(dat_file_cell), SEEK_SET);
    _dat_file.Write((void*)&dat, sizeof(dat_file_cell));
  }
  catch(FileException& ex)
  {
    smsc_log_debug(logger, "FSStorage: Error in CreateAbntEvents - %s", ex.what());
    return 1;
  }
  hashAbnt.Insert(CalledNum, cell);
  return 0;
}

int FSStorage::DestroyAbntEvents(const AbntAddr& CalledNum)
{
  cell_t cell = hashAbnt.Get(CalledNum);	

  smsc_log_debug(logger, "FSStorage::DestroyAbntEvents::: destroy event from storage for calledNum=[%s]", CalledNum.getText().c_str());
  try {
    _dat_file.Seek(cell*sizeof(dat_file_cell), SEEK_SET);

    dat_file_cell destroyedCell={0};
    _dat_file.Write((void*)&destroyedCell, sizeof(destroyedCell));
  } catch(FileException& ex) {
    smsc_log_debug(logger, "FSStorage: DestroyAbntEvents: !! Error !! - %s", ex.what());
    return 1;
  }

  hashAbnt.Remove(CalledNum);
  freeCells.AddCell(cell);

  return 0;
}

int FSStorage::SaveAbntEvents(const AbntAddr& CalledNum, const dat_file_cell* pAbntEvents)
{
  cell_t   cell = hashAbnt.Get(CalledNum);	

  try
  {
    _dat_file.Seek(cell*sizeof(dat_file_cell), SEEK_SET);
    _dat_file.Write((void*)pAbntEvents, sizeof(dat_file_cell));
  }
  catch(FileException& ex)
  {
    smsc_log_debug(logger, "FSStorage: Error in SaveAbntEvents - %s", ex.what());
    return 1;
  }
  return 0;
}

int FSStorage::LoadAbntEvents(const AbntAddr& CalledNum, dat_file_cell* pAbntEvents)
{
  cell_t cell = hashAbnt.Get(CalledNum);

  try
  {
    _dat_file.Seek(cell*sizeof(dat_file_cell), SEEK_SET);
    _dat_file.Read((void*)pAbntEvents, sizeof(dat_file_cell));
  }
  catch(FileException& ex)
  {
    smsc_log_debug(logger, "FSStorage: Error in LoadAbntEvents - %s", ex.what());
    return 1;
  }
  return 0;
}

bool
FSStorage::findAndUpdateEventDateAndCallCount(dat_file_cell* pAbntEvents,
                                              const MCEvent& event)
{
  for(int i=0; i<pAbntEvents->eventCount; ++i) {
    if ( !memcmp(&pAbntEvents->events[i].callingNum.full_addr, &event.caller.full_addr, sizeof(event.caller.full_addr)) ) {
      pAbntEvents->events[i].callCount++;
      event_cell bakCell = pAbntEvents->events[i];
      bakCell.date = event.dt;

      char dateAsString[128];
      strncpy(dateAsString, ctime(&bakCell.date), sizeof(dateAsString));
      dateAsString[strlen(dateAsString)-1]=0;

      //smsc_log_debug(logger, "FSStorage::findAndUpdateEventDateAndCallCount::: callingNum=[%s] was found in events[], found idx=%d, new time for updatable cell=%x(%s)", AbntAddr(&event.caller).getText().c_str(), i, bakCell.date, dateAsString);
      int updatedEventIdx;
      for(updatedEventIdx=i+1; updatedEventIdx < pAbntEvents->eventCount; updatedEventIdx++) {
        if ( pAbntEvents->events[updatedEventIdx].date >= bakCell.date )
          break;
      }

      memcpy(&pAbntEvents->events[i], &pAbntEvents->events[i+1], (updatedEventIdx - i - 1)*sizeof(event_cell));
      pAbntEvents->events[updatedEventIdx - 1] = bakCell;
      //smsc_log_debug(logger, "FSStorage::findAndUpdateEventDateAndCallCount::: updated event moved to array element with idx=%d: event_cell.date=%x,event_cell.id=%d,event_cell.callingNum=%s,event_cell.callCount=%d", updatedEventIdx - 1, pAbntEvents->events[updatedEventIdx - 1].date, pAbntEvents->events[updatedEventIdx - 1].id,  AbntAddr(&pAbntEvents->events[updatedEventIdx - 1].callingNum).getText().c_str(), pAbntEvents->events[updatedEventIdx - 1].callCount);

      return true;
    }
  }

  return false;
}

void
FSStorage::replaceOldestEvent(dat_file_cell* pAbntEvents,
                              const MCEvent& event)
{
  store_E_Event_in_logstore(pAbntEvents->calledNum, pAbntEvents->events[0].callingNum);
  memcpy(&pAbntEvents->events[0], &pAbntEvents->events[1], (pAbntEvents->eventCount - 1)*sizeof(event_cell));

  insertEventIntoPosition(pAbntEvents, maxEvents - 1, event);
}

void
FSStorage::store_E_Event_in_logstore(const AbntAddrValue& calledAbonent,
                                     const AbntAddrValue& callingAbonent)
{
  const std::string& calledAbonentAddr = AbntAddr(&calledAbonent).getText();
  const std::string& callingAbonentAddr = AbntAddr(&callingAbonent).getText();
  MCAEventsStorageRegister::getMCAEventsStorage().addEvent(Event_DeleteMissedCallInfo(callingAbonentAddr, calledAbonentAddr));
}

void
FSStorage::AddAbntEvent(dat_file_cell* pAbntEvents,
                        const MCEvent& event)
{
//  smsc_log_debug(logger, "FSStorage::AddAbntEvent::: try add event for callingNum=%s, data_file_cell.eventCount=%d", AbntAddr(&event.caller).getText().c_str() , pAbntEvents->eventCount);

  if ( findAndUpdateEventDateAndCallCount(pAbntEvents, event) )
    return;

  if ( pAbntEvents->eventCount == maxEvents ) {
    store_E_Event_in_logstore(pAbntEvents->calledNum, event.caller);
    replaceOldestEvent(pAbntEvents, event);
    return;
  }

  insertEventIntoPosition(pAbntEvents, pAbntEvents->eventCount++, event);

  return;
}

void
FSStorage::insertEventIntoPosition(dat_file_cell* pAbntEvents,
                                   unsigned int idx,
                                   const MCEvent& event)
{
  pAbntEvents->events[idx].date = event.dt;

  if ( idx > 0 )
    pAbntEvents->events[idx].id = pAbntEvents->events[idx-1].id + 1;
  else
    pAbntEvents->events[idx].id = 0;

  memcpy((void*)&(pAbntEvents->events[idx].callingNum), (void*)&(event.caller.full_addr), sizeof(pAbntEvents->events[idx].callingNum));
  pAbntEvents->events[idx].callCount = 1;
  pAbntEvents->events[idx].flags = event.missCallFlags;
//  smsc_log_debug(logger, "FSStorage::insertEventIntoPosition::: called addr=[%s], insert event into idx=%d, event_cell.date=0x%x,event_cell.id=%d,event_cell.callingNum=%s,event_cell.callCount=%d,event_cell.flags=0x%x", AbntAddr(&pAbntEvents->calledNum).getText().c_str(), idx, pAbntEvents->events[idx].date, pAbntEvents->events[idx].id,  AbntAddr(&pAbntEvents->events[idx].callingNum).getText().c_str(), pAbntEvents->events[idx].callCount,pAbntEvents->events[idx].flags);
}

void
FSStorage::RemoveAbntEvents(dat_file_cell* pAbntEvents,
                            const vector<MCEvent>& events)
{
  if(pAbntEvents->eventCount == 0)
    return;

  for(size_t i = events.size(); i > 0; i--)
    RemoveEvent(pAbntEvents, events[i-1]);
}

void
FSStorage::RemoveEvent(dat_file_cell* pAbntEvents,
                       const MCEvent& event)
{
  //  smsc_log_debug(logger, "FSStorage::RemoveEvent::: removable event: event.id=%d, pAbntEvents->eventCount=%d, pAbntEvents->calledNum=%s", event.id, pAbntEvents->eventCount, AbntAddr(&pAbntEvents->calledNum).getText().c_str());
  int	event_num;
  for(event_num = 0; event_num < pAbntEvents->eventCount; event_num++) {
    smsc_log_debug(logger, "FSStorage::RemoveEvent::: next bypassed event in storage: id=%d",pAbntEvents->events[event_num].id);
    if(pAbntEvents->events[event_num].id == event.id) {
      smsc_log_debug(logger, "FSStorage::RemoveEvent::: OK, deletable event was found, idx in array=%d, pAbntEvents->eventCount=%d", event_num, pAbntEvents->eventCount);
      break;
    }
  }

  if(event_num == pAbntEvents->eventCount) return;

  EraseEvent(pAbntEvents, event_num);
}

void
FSStorage::EraseEvent(dat_file_cell* pAbntEvents,
                      int event_num)
{
  if(event_num < pAbntEvents->eventCount-1)
    memcpy((void*)&(pAbntEvents->events[event_num]), (void*)&(pAbntEvents->events[event_num+1]), (pAbntEvents->eventCount-event_num-1)*sizeof(event_cell));
  memset((void*)&(pAbntEvents->events[pAbntEvents->eventCount-1]), 0, sizeof(event_cell));
  pAbntEvents->eventCount--;
  smsc_log_debug(logger, "FSStorage::EraseEvent::: decrement pAbntEvents->eventCount, now it's equal to=%d", pAbntEvents->eventCount);
}

int FSStorage::KillExpiredEvents(dat_file_cell* pAbntEvents)
{
  MCEvent e;
  int     killed = 0;

  while(pAbntEvents->eventCount)
  {
    time_t dt = pAbntEvents->events[0].date;

    if((dt + eventLifeTime) < time(0))
    {
      store_E_Event_in_logstore(pAbntEvents->calledNum, pAbntEvents->events[0].callingNum);
      EraseEvent(pAbntEvents, 0);
      killed++;
    } else
      break;
  }
  smsc_log_debug(logger, "FSStorage:KillExpiredEvents: killed: %d", killed);
  return killed;
}

int FSStorage::IncrStorage(const uint32_t& num_cells)
{
  smsc_log_debug(logger, "FSStorage::IncrStorage::: try extend to %d cells", num_cells);
  if(0 != IncrDatFile(num_cells))
    return 2;

  smsc_log_debug(logger, "FSStorage::IncrStorage::: _first_free_cell_num=%d", _first_free_cell_num);
  for(uint32_t i=0; i<num_cells; i++) freeCells.AddCell(_first_free_cell_num++);
  smsc_log_debug(logger, "FSStorage::IncrStorage::: before return _first_free_cell_num=%d", _first_free_cell_num);
  return 0;
}

int FSStorage::IncrDatFile(const uint32_t& num_cells)
{
  size_t buf_size = sizeof(dat_file_cell)*num_cells;
  uint8_t* buf = new uint8_t[buf_size];

  memset((void*)buf, 0x00, buf_size);
  try
  {
    smsc_log_debug(logger, "FSStorage::IncrDatFile::: try extend file to %d bytes", buf_size);
    _dat_file.SeekEnd(0);		
    _dat_file.Write((void*)buf, buf_size);
    smsc_log_debug(logger, "FSStorage::IncrDatFile::: extending completed");
  } catch(FileException& ex) {
    delete[] buf;
    smsc_log_debug(logger, "FSStorage: Error in IncrDatFile - %s", ex.what());
    return 2;
  } catch(...) {
    delete[] buf;
    throw;
  }

  delete[] buf;
  return 0;
}

}
}
