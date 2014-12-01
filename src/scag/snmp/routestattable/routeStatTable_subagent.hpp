#ifndef __MSAG_SNMP_ROUTESTATTABLE_ROUTESTATTABLE_SUBAGENT_HPP__
#define __MSAG_SNMP_ROUTESTATTABLE_ROUTESTATTABLE_SUBAGENT_HPP__

#include "scag/stat/impl/StatisticsManager.h"

namespace scag2 {
namespace snmp {
namespace routestattable {

class RouteStatTableSubagent{
public:
//  static void Init(scag2::stat::StatisticsManager* statman);
//  static scag2::stat::StatisticsManager* getStatMan();
  static void Register();
  static void Unregister();
};

}//routestattable
}//snmp
}//smsc

#endif
