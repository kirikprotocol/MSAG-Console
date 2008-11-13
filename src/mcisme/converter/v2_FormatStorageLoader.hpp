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

struct event_v2_cell
{
  time_t          date;
  uint8_t         id;
  AbntAddrValue	  callingNum;
  uint16_t        callCount;
};

struct dat_file_v2_cell
{
  time_t        schedTime;
  uint8_t       eventCount;
  uint16_t      lastError;
  AbntAddrValue	calledNum;
  uint8_t       recordIsActive;
  event_v2_cell events[MAX_EVENTS];
};

class v2_FormatStorageLoader
{
public:

  v2_FormatStorageLoader();
  ~v2_FormatStorageLoader();

  int Init(const std::string& location);
  int MakeConvertation(smsc::mcisme::FSStorage* newFormatStorage);

private:
  smsc::logger::Logger *_logger;
  string _pathDatFile;

  File _dat_file;

  int OpenFiles(void);
  void CloseFiles(void);
};

}
}
}
#endif
