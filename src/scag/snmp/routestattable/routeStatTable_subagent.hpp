#ifndef ROUTESTATTABLE_SUBAGENT_HPP__
#define ROUTESTATTABLE_SUBAGENT_HPP__

namespace scag2 {
namespace snmp {
namespace routestattable {

class RouteStatTableSubagent{
public:
  static void Register();
  static void Unregister();
};

}}}

#endif
