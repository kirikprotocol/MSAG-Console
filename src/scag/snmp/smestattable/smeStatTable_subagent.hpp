#ifndef __SMSC_SNMP_SMESTATTABLE_SMESTATTABLE_SUBAGENT_HPP__
#define __SMSC_SNMP_SMESTATTABLE_SMESTATTABLE_SUBAGENT_HPP__

//#include "smsc/smeman50/smeman.h"
//#include "smsc/stat/SmeStats.hpp"

namespace scag2{
namespace snmp{
namespace smestattable{

class SmeStatTableSubagent{
public:
//  static void Init(smsc::smeman50::SmeManager* smeman, smsc::stat::SmeStats* smestats);
  static void Init(int* smeman, int* smestats);
  static void Register();
  static void Unregister();
};

}//smestattable
}//snmp
}//smsc

#endif
