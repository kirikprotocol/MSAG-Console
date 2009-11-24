#ifndef __SMSC_SNMP_SMESTATTABLE_SMESTATTABLE_SUBAGENT_HPP__
#define __SMSC_SNMP_SMESTATTABLE_SMESTATTABLE_SUBAGENT_HPP__

#include "smsc/smeman/smeman.h"
#include "smsc/stat/SmeStats.hpp"

namespace smsc{
namespace snmp{
namespace smestattable{

class SmeStatTableSubagent{
public:
  static void Init(smsc::smeman::SmeManager* smeman,smsc::stat::SmeStats* smestats);
  static void Register();
  static void Unregister();
};

}//smestattable
}//snmp
}//smsc

#endif
