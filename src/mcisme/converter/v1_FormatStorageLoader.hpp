//------------------------------------
//  FSStorage.hpp
//  Routman Michael, 2005-2006
//------------------------------------
//
//	Файл содержит описание класса FSStorage 
//

#ifndef __MCISME_CONVERTER_OLDFORMATSTORAGELOADER_HPP__
#define __MCISME_CONVERTER_OLDFORMATSTORAGELOADER_HPP__

#include <logger/Logger.h>
#include <list>
#include <queue>
#include <vector>
#include <string>
#include <string.h>

#include <core/buffers/Array.hpp>

#include <core/synchronization/Mutex.hpp>

#include <sms/sms.h>
#include <core/buffers/Hash.hpp>
#include <core/buffers/File.hpp>
#include <mcisme/Storage.hpp>
#include <mcisme/AbntAddr.hpp>
#include <mcisme/DeliveryQueue.hpp>

#include <mcisme/FSStorage.hpp>

namespace smsc {
namespace mcisme {
namespace converter {

struct event_cell
{
  time_t        date;
  uint8_t       id;
  AbntAddrValue	calling_num;
  uint8_t       reserved[16 - sizeof(uint8_t) - sizeof(AbntAddrValue)];
};

struct dat_file_cell
{
  time_t        schedTime;
  uint8_t       event_count;
  uint16_t      last_error;
  AbntAddrValue	inaccessible_num;
  uint8_t       reserved[16 - sizeof(uint8_t) - sizeof(AbntAddrValue) - sizeof(uint16_t)];
  event_cell    events[MAX_EVENTS];
};

struct idx_file_cell
{
  AbntAddrValue	inaccessible_num;
};

typedef uint32_t cell_t;
class HashAbnt
{
  Hash<cell_t> num_cell;

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

class v1_FormatStorageLoader
{
  smsc::logger::Logger *logger;
public:

  v1_FormatStorageLoader();
  ~v1_FormatStorageLoader();

  int Init(const std::string& location, const std::string& sEventLifeTime);
  int v1_FormatStorageLoader::MakeConvertation(smsc::mcisme::FSStorage* newFormatStorage);

private:

  HashAbnt	hashAbnt;

  string   pathDatFile;
  string   pathIdxFile;

  File     dat_file;
  File     idx_file;

  uint8_t  maxEvents;
  time_t   eventLifeTime;

  uint8_t* zero_dat_cell;
  uint8_t* zero_idx_cell;

  int OpenFiles(void);
  void CloseFiles(void);

  int LoadAbntEvents(const smsc::mcisme::AbntAddr& CalledNum, dat_file_cell* pAbntEvents);
};

}
}
}
#endif
