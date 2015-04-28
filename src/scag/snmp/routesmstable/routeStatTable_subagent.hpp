#ifndef ROUTESMSTABLE_SUBAGENT_HPP__
#define ROUTESMSTABLE_SUBAGENT_HPP__

namespace scag2 {
namespace snmp {
namespace routesmstable {

class RouteStatTableSubagent{
public:
  static void Register();
  static void Unregister();
};

}}}

#endif
