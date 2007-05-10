#ifndef __SMSC_SNMP_SMESTATTABLE_SMESTATTABLE_SUBAGENT_HPP__
#define __SMSC_SNMP_SMESTATTABLE_SMESTATTABLE_SUBAGENT_HPP__

#include "core/threads/Thread.hpp"
#include "smeman/smeman.h"
#include "stat/SmeStats.hpp"

namespace smsc{
namespace snmp{
namespace smestattable{

class SmeStatTableSubagentThread:public smsc::core::threads::Thread{
public:
  void Init(smsc::smeman::SmeManager* smeman,smsc::stat::SmeStats* smestats);
  int Execute();
  void Stop();
protected:
};

}//smestattable
}//snmp
}//smsc

#endif
