#ifndef SMEUSSDTABLE_SUBAGENT_HPP__
#define SMEUSSDTABLE_SUBAGENT_HPP__

#include "scag/snmp/SnmpUtil.h"

namespace scag2 {
namespace snmp {
namespace smeussdtable {

class SmeStatTableSubagent{
public:
  static void Register();
  static void Unregister();
};

}}}

#endif
