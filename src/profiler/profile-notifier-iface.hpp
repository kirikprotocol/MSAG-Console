#ifndef __SMSC_PROFILER_PROFILE_NOTIFIER_IFACE_HPP__
#define __SMSC_PROFILER_PROFILE_NOTIFIER_IFACE_HPP__

#include "sms/sms.h"
#include "profiler-types.hpp"

namespace smsc{
namespace profiler{

class ProfileNotifierInterface{
public:
  virtual void AddChange(const smsc::sms::Address& addr,const Profile& prof)=0;
};

}//profiler
}//smsc

#endif
