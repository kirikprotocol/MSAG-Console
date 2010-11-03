/* ************************************************************************** *
 * Helper classes for operations with RPCauses (Reject Processing Cause)
 * ************************************************************************** */
#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/common/RPCList.hpp"
#include "util/csv/CSVArrayOf.hpp"

namespace smsc   {
namespace inman  {
namespace common {

//Format: "Rpc:Attr"
RPCauseATT RPCauseATT::str2val(const std::string & str_val) /*throw(std::exception)*/
{
  util::csv::CSVArrayOfStr  rpAtt(2, ':');   //separate RPCause ant attribute
  if (!rpAtt.fromStr(str_val.c_str()) || rpAtt.size() > 2)
      throw util::csv::CSValueException("RPCauseATT", str_val.c_str());

  RPCauseATT  rval;
  try {
    rval._rpc = util::csv::CSValueTraits_T<uint8_t>::str2val(rpAtt[0]);
    if (rpAtt.size() > 1)
      rval._att = util::csv::CSValueTraits_T<uint16_t>::str2val(rpAtt[1]);
  } catch (...) {
    throw util::csv::CSValueException("RPCauseATT", str_val.c_str());
  }
  return rval;
}

std::string RPCauseATT::val2str(const RPCauseATT & use_val) /*throw()*/
{
  char buf[2*sizeof(uint16_t)*3 + 2];
  snprintf(buf, sizeof(buf), "%u:%u", (unsigned)use_val._rpc, (unsigned)use_val._att);
  return std::string(buf);
}

} //common
} //inman
} //smsc

