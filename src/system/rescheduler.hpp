#ifndef __SMSC_SYSTEM_RESCHEDULER_HPP__
#define __SMSC_SYSTEM_RESCHEDULER_HPP__

#include "core/buffers/Array.hpp"
#include "util/Exception.hpp"
#include <time.h>

namespace smsc{
namespace system{

using smsc::util::Exception;

class RescheduleCalculator{
public:

  static void Init(const char* timestring)throw(Exception);
  static time_t calcNextTryTime(time_t lasttry,int attempt);

protected:

  static smsc::core::buffers::Array<time_t> RescheduleTable;

};//RescheduleCalculator

};//system
};//smsc

#endif
