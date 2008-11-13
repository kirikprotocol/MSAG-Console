#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#include "v2_FormatStorageLoader.hpp"

int errno;

#include <stdio.h>

namespace smsc {
namespace mcisme {
namespace converter {

using std::vector;
using std::list;
using core::buffers;
using sms::Address;
using core::synchronization::Mutex;
using core::synchronization::MutexGuard;
using smsc::util::config::ConfigView;

v2_FormatStorageLoader::v2_FormatStorageLoader(): _logger(smsc::logger::Logger::getInstance("mci.fsStor"))
{}

v2_FormatStorageLoader::~v2_FormatStorageLoader()
{
  CloseFiles();
  smsc_log_debug(_logger, "OldFormatStorageLoader: Destroed.\n");
}

int v2_FormatStorageLoader::Init(const std::string& location)
{
  std::string dirName(location);
  if ( dirName[dirName.size() - 1] != '/' )
    dirName += '/';

  std::string fileVersionSuffix=".v2";

  _pathDatFile = location + "dat_file" + fileVersionSuffix;

  if(0 != OpenFiles())
    return 1;

  smsc_log_debug(_logger, "OldFormatStorageLoader: All files opened");

  return 0;	
}

int
v2_FormatStorageLoader::MakeConvertation(smsc::mcisme::FSStorage* newFormatStorage)
{
  smsc_log_info(_logger, "OldFormatStorageLoader: Loading DataBase from files.......");

  try
  {
    dat_file_v2_cell abntEvents;
    while(_dat_file.Read((void*)&abntEvents, sizeof(abntEvents)))
    {
      AbntAddr calledNum(&abntEvents.calledNum);
      for (uint8_t i=0; i<abntEvents.eventCount; ++i) {
        MCEvent event;
        event.id = abntEvents.events[i].id;
        event.dt = abntEvents.events[i].date;
        memcpy(&event.caller, &abntEvents.events[i].callingNum, sizeof(event.caller));
        event.callCount = abntEvents.events[i].callCount;
        event.missCallFlags = 0;
        newFormatStorage->addEvent(calledNum, event, time(0));
      }
    }
  } catch(FileException ex) {}

  return 0;
}

int
v2_FormatStorageLoader::OpenFiles(void)
{
  try {
    if(File::Exists(_pathDatFile.c_str()))
      _dat_file.ROpen(_pathDatFile.c_str());
    else {
      smsc_log_error(_logger, "v2_FormatStorageLoader::OpenFiles::: file [%s] is absent", _pathDatFile.c_str());
      return 1;
    }
    _dat_file.SetUnbuffered();
  } catch(FileException ex) {
    smsc_log_debug(_logger, "v2_FormatStorageLoader: error dat_file - %s\n", ex.what());
    return 1;
  }

  return 0;
}

void v2_FormatStorageLoader::CloseFiles(void)
{
  _dat_file.Close();
}

}
}
}
