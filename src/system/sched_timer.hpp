#ifndef __SMSC_SYSTEM_SCHE_TIMER_HPP__
#define __SMSC_SYSTEM_SCHE_TIMER_HPP__

#include "sms/sms_const.h"
#include <time.h>

namespace smsc{
namespace system{

using smsc::sms::SMSId;

class SchedTimer{
public:
  virtual void ChangeSmsSchedule(SMSId id,time_t newtime)=0;
};


};//system
};//smsc

#endif
