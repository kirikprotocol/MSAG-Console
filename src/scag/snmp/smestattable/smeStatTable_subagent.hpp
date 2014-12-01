#ifndef __MSAG_SNMP_SMESTATTABLE_SMESTATTABLE_SUBAGENT_HPP__
#define __MSAG_SNMP_SMESTATTABLE_SMESTATTABLE_SUBAGENT_HPP__

#include "scag/stat/impl/StatisticsManager.h"

namespace scag2 {
namespace snmp {
namespace smestattable {

class SmeStatTableSubagent{
public:
  static void Init(scag2::stat::StatisticsManager* statman);
  static scag2::stat::StatisticsManager* getStatMan();
  static void Register();
  static void Unregister();
};

}//smestattable
}//snmp
}//smsc

#endif
