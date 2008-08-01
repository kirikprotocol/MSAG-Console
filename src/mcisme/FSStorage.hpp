//------------------------------------
//  FSStorage.hpp
//  Routman Michael, 2005-2006
//------------------------------------
//
//	‘айл содержит описание класса FSStorage 
//

#ifndef ___FSSTOREGE_H
#define ___FSSTOREGE_H

#include <logger/Logger.h>
#include <list>
#include <queue>
#include <vector>
#include <string>
#include <string.h>

#include <core/buffers/Array.hpp>

//#include <core/threads/Thread.hpp>
//#include <core/threads/ThreadPool.hpp>

#include <core/synchronization/Mutex.hpp>
//#include <core/synchronization/Event.hpp>
//#include <core/synchronization/EventMonitor.hpp>

#include <sms/sms.h>
#include <core/buffers/Hash.hpp>
#include <core/buffers/File.hpp>
#include <mcisme/Storage.hpp>
#include <mcisme/AbntAddr.hpp>
#include <mcisme/DeliveryQueue.hpp>

namespace smsc { namespace mcisme
{

using std::vector;
using std::list;
using std::queue;
using std::string;
using core::buffers::Array;
using sms::Address;
using core::synchronization::Mutex;

const int MAX_EVENTS = 50;
const int MAX_BDFILES_INCR = 10000;
const int DEFAULT_BDFILES_INCR = 1000;

//	ќписани€ сопутствующих структур данных.

//	структура "событие дл€ абонента" 
struct event_cell
{
  time_t          date;
  uint8_t         id;
  AbntAddrValue	  callingNum;
  uint16_t        callCount;
};

//	структура файла событий дл€ абонентов
struct dat_file_cell
{
  time_t        schedTime;
  uint8_t       eventCount;
  uint16_t      lastError;
  AbntAddrValue	calledNum;
  uint8_t       recordIsActive;
  event_cell    events[MAX_EVENTS];
};

//	очередь свободных €чеек(позици€ в файле) дл€ файла индексов и файла событий дл€ абонентов.
class FreeCells
{
  list<uint32_t>	free_cells;

public:
  FreeCells(){}
  ~FreeCells(){Erase();}

  void AddCell(const uint32_t& free_cell)
  {
    free_cells.push_back(free_cell);
  }
  uint32_t GetCell(void)
  {
    uint32_t free_cell = free_cells.front();
    free_cells.pop_front();
    return free_cell;
  }
  bool Empty(void){return free_cells.empty();} 
  void Erase(void){free_cells.clear();}
};

typedef uint32_t	cell_t;
class HashAbnt
{
  Hash<cell_t>	num_cell;	//	таблица местоположиний записей в файлах данных и индексов об абонентне (ключ - Address)

public:
  HashAbnt(){}
  ~HashAbnt(){Erase();}

  void Insert(const AbntAddr& abnt, const cell_t& cell)
  {
    num_cell.Insert(abnt.toString().c_str(), cell);
  }
  cell_t Get(const AbntAddr& abnt)
  {
    return num_cell.Get(abnt.toString().c_str());
  }
  void Remove(const AbntAddr& abnt)
  {
    num_cell.Delete(abnt.toString().c_str());
  }

  int Exists(const AbntAddr& abnt)
  {
    return num_cell.Exists(abnt.toString().c_str());
  }

  void Erase(void)
  {
    num_cell.Empty();			
  }
};

class FSStorage: public Storage
{
public:

  FSStorage();
  ~FSStorage();

  int Init(smsc::util::config::ConfigView* storageConfig, DeliveryQueue* pDeliveryQueue);
  int Init(const string& location, time_t eventLifeTime, uint8_t maxEvents, DeliveryQueue* pDeliveryQueue);
  virtual void addEvent(const AbntAddr& CalledNum, const MCEvent& event, time_t schedTime);
  virtual void setSchedParams(const AbntAddr& CalledNum, time_t schedTime, uint16_t lastError = -1);
  bool getEvents(const AbntAddr& CalledNum, vector<MCEvent>& events);
  void deleteEvents(const AbntAddr& CalledNum, const vector<MCEvent>& events);

private:
  smsc::logger::Logger *logger;
  FreeCells	freeCells;			//	список свободных €чеек(номер €чейки).

  //	хэш-таблица абонентов, дл€ которых существуют событи€.
  //	 люч - абонент, значение - индекс в файле событий дл€ абонентов
  //				(номер записи. позици€ в файле определ€етс€ индекс*размер_записи).
  //
  HashAbnt  hashAbnt;

  string    pathDatFile;
  uint32_t  bdFilesIncr;

  File      _dat_file;

  uint8_t   maxEvents;
  time_t    eventLifeTime;

  unsigned int _first_free_cell_num;

  Mutex	    mut;

  int  OpenFiles(void);
  void CloseFiles(void);
  int CompleteTransaction(void);
  int LoadEvents(DeliveryQueue* pDeliveryQueue);

  int CreateAbntEvents(const AbntAddr& CalledNum, const MCEvent& event, time_t schedTime);
  int DestroyAbntEvents(const AbntAddr& CalledNum);
  int SaveAbntEvents(const AbntAddr& CalledNum, const dat_file_cell* pAbntEvents);
  int LoadAbntEvents(const AbntAddr& CalledNum, dat_file_cell* pAbntEvents);
  void AddAbntEvent(dat_file_cell* pAbntEvents, const MCEvent& event);
  void RemoveAbntEvents(dat_file_cell* pAbntEvents, const vector<MCEvent>& events);
  void RemoveEvent(dat_file_cell* pAbntEvents, const MCEvent& event);
  int KillExpiredEvents(dat_file_cell* pAbntEvents);		// убирает событи€ у которых истек срок доставки и возвращает кол-во уничтоженны событий.

  int IncrDatFile(const uint32_t& num_cells);
  int IncrStorage(const uint32_t& num_cells);

  void replaceOldestEvent(dat_file_cell* pAbntEvents,
                          const MCEvent& event);
  void EraseEvent(dat_file_cell* pAbntEvents, int eventNum);
  bool findAndUpdateEventDateAndCallCount(dat_file_cell* pAbntEvents,
                                          const MCEvent& event);
  void insertEventIntoPosition(dat_file_cell* pAbntEvents,
                               unsigned int idx,
                               const MCEvent& event);

  void store_E_Event_in_logstore(const AbntAddrValue& calledAbonent,
                                 const AbntAddrValue& callingAbonent);
};

};	//  namespace mcisme
};	//  namespace smsc
#endif
