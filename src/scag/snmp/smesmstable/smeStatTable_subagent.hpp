#ifndef SMESMSTABLE_SUBAGENT_HPP__
#define SMESMSTABLE_SUBAGENT_HPP__

#include "scag/snmp/SnmpUtil.h"

namespace scag2 {
namespace snmp {
namespace smesmstable {

class SmeStatTableSubagent{
public:
  static void Register();
  static void Unregister();
};

}}}

#endif
