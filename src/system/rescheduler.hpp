#ifndef __SMSC_SYSTEM_RESCHEDULER_HPP__
#define __SMSC_SYSTEM_RESCHEDULER_HPP__

#include "core/buffers/Array.hpp"
#include "core/buffers/IntHash.hpp"
#include "util/Exception.hpp"
#include <time.h>

namespace smsc{
namespace system{

using smsc::util::Exception;
using namespace smsc::core::buffers;

class RescheduleCalculator{
public:

  static void InitDefault(const char* timestring)throw(Exception);
  static void AddToTable(const char* timeline,const char* codes);
  static time_t calcNextTryTime(time_t lasttry,int code,int attempt);
  static bool isEqualCodes(int c1,int c2)
  {
    return CodesTable.GetPtr(c1)==CodesTable.GetPtr(c2);
  }

protected:

  typedef Array<time_t> TimeArray;

  static TimeArray RescheduleTable;
  static IntHash<TimeArray*> CodesTable;

};//RescheduleCalculator

};//system
};//smsc

#endif
