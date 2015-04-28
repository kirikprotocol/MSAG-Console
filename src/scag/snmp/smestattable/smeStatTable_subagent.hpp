#ifndef SMESTATTABLE_SUBAGENT_HPP__
#define SMESTATTABLE_SUBAGENT_HPP__

#include "scag/snmp/SnmpUtil.h"

namespace scag2 {
namespace snmp {
namespace smestattable {

class SmeStatTableSubagent{
public:
  static void Register();
  static void Unregister();
};

}}}

#endif
