#ifndef ROUTEUSSDTABLE_SUBAGENT_HPP__
#define ROUTEUSSDTABLE_SUBAGENT_HPP__

namespace scag2 {
namespace snmp {
namespace routeussdtable {

class RouteStatTableSubagent{
public:
  static void Register();
  static void Unregister();
};

}}}

#endif
