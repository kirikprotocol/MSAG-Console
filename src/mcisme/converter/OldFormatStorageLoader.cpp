#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#include "OldFormatStorageLoader.hpp"

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

static int parseTime(const char* str)
{
  int hour, minute, second;
  if (!str || str[0] == '\0' ||
      sscanf(str, "%02d:%02d:%02d", 
             &hour, &minute, &second) != 3) return -1;
    
  return hour*3600+minute*60+second;
}

static const std::string
time2string(time_t t)
{
  tm lcltm;
  char timeStr[64];
  strftime(timeStr, sizeof(timeStr), "%Y%m%d %H:%M:%S", localtime_r(&t, &lcltm));

  return timeStr;
}

OldFormatStorageLoader::OldFormatStorageLoader(): logger(smsc::logger::Logger::getInstance("mci.fsStor"))
{
  zero_idx_cell = new uint8_t[sizeof(idx_file_cell)];
  zero_dat_cell = new uint8_t[sizeof(dat_file_cell)];

  memset((void*)zero_idx_cell, 0x00, sizeof(idx_file_cell));
  memset((void*)zero_dat_cell, 0x00, sizeof(dat_file_cell));
}

OldFormatStorageLoader::~OldFormatStorageLoader()
{
  delete[] zero_dat_cell;
  delete[] zero_idx_cell;

  CloseFiles();
  smsc_log_debug(logger, "OldFormatStorageLoader: Destroed.\n");
}

int OldFormatStorageLoader::Init(const std::string& location, const std::string& sEventLifeTime)
{
  eventLifeTime = parseTime(sEventLifeTime.c_str());

  pathDatFile = location + "dat_file";
  pathIdxFile = location + "idx_file";

  if(0 != OpenFiles())
    return 1;

  smsc_log_debug(logger, "OldFormatStorageLoader: All files opened");

  return 0;	
}

int
OldFormatStorageLoader::MakeConvertation(smsc::mcisme::FSStorage* newFormatStorage)
{
  uint8_t buf[sizeof(idx_file_cell)];

  smsc_log_info(logger, "OldFormatStorageLoader: Loading DataBase from files.......\n");

  try
  {
    dat_file_cell abntEvents;
    uint32_t      cell_num=0;

    while(idx_file.Read((void*)buf, sizeof(buf)))
    {
      if(memcmp(buf, zero_idx_cell, sizeof(idx_file_cell)))
      {
        AbntAddr abnt(buf);
        hashAbnt.Insert(abnt, cell_num);
        LoadAbntEvents(abnt, &abntEvents);
        AbntAddr calledNum(&abntEvents.inaccessible_num);
        if(abntEvents.event_count == 0)
        {
          hashAbnt.Remove(calledNum);
          continue;
        }

        for(uint8_t i =0; i<abntEvents.event_count; ++i) {
          smsc::mcisme::MCEvent eventToStore;
          eventToStore.dt = abntEvents.events[i].date;
          eventToStore.caller = abntEvents.events[i].calling_num;
          eventToStore.callCount = 1;

          AbntAddr callingNum(&abntEvents.events[i].calling_num);
          if((eventToStore.dt + eventLifeTime) < time(0))
            smsc_log_debug(logger, "load event: skip expired event: calledNum=[%s],date=[%s],callingNum=[%s]", calledNum.getText().c_str(), time2string(abntEvents.events[i].date).c_str(), callingNum.getText().c_str());
          else {
            smsc_log_debug(logger, "load event: calledNum=[%s],date=[%s],callingNum=[%s]", calledNum.getText().c_str(), time2string(abntEvents.events[i].date).c_str(), callingNum.getText().c_str());
            time_t schedTime = time(0);
            newFormatStorage->addEvent(calledNum, eventToStore, schedTime);
          }
        }
      }
      cell_num++;
    }
  } catch(FileException ex) {}

  return 0;
}

int
OldFormatStorageLoader::OpenFiles(void)
{
  try
  {
    if(File::Exists(pathDatFile.c_str()))
      dat_file.ROpen(pathDatFile.c_str());
    else {
      smsc_log_error(logger, "OldFormatStorageLoader::OpenFiles::: file [%s] is absent", pathDatFile.c_str());
      return 1;
    }
    dat_file.SetUnbuffered();
  } catch(FileException ex) {
    smsc_log_debug(logger, "OldFormatStorageLoader: error dat_file - %s\n", ex.what());
    return 1;
  }

  try
  {
    if(File::Exists(pathIdxFile.c_str()))
      idx_file.ROpen(pathIdxFile.c_str());
    else {
      smsc_log_error(logger, "OldFormatStorageLoader::OpenFiles::: file [%s] is absent", pathIdxFile.c_str());
      return 1;
    }

    idx_file.SetUnbuffered();
  } catch(FileException ex) {
    smsc_log_debug(logger, "OldFormatStorageLoader: error idx_file - %s\n", ex.what());
    dat_file.Close();
    return 2;
  }

  return 0;
}

void OldFormatStorageLoader::CloseFiles(void)
{
  dat_file.Close();
  idx_file.Close();
}

int OldFormatStorageLoader::LoadAbntEvents(const AbntAddr& CalledNum, dat_file_cell* pAbntEvents)
{
  cell_t	cell = 	hashAbnt.Get(CalledNum);

  try
  {
    dat_file.Seek(cell*sizeof(dat_file_cell), SEEK_SET);
    dat_file.Read((void*)pAbntEvents, sizeof(dat_file_cell));
  }
  catch(FileException ex)
  {
    smsc_log_debug(logger, "OldFormatStorageLoader: Error in LoadAbntEvents - %s\n", ex.what());
    return 1;
  }
  return 0;
}

}
}
}
