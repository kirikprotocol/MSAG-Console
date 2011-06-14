/* ************************************************************************* *
 * Abonent CSI (CAMEL Subscription Information) helper classes definitions
 * ************************************************************************* */
#ifndef SMSC_INMAN_SUBSCR_CONTRACT_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_INMAN_SUBSCR_CONTRACT_HPP

#include "util/TonNpiAddress.hpp"

namespace smsc {
namespace inman {

using smsc::util::IMSIString;

struct AbonentContractInfo {
  enum ContractType_e { abtUnknown = 0x00, abtPostpaid = 0x01, abtPrepaid = 0x02 };

  ContractType_e  abType;
  IMSIString      abImsi;

  explicit AbonentContractInfo(ContractType_e cntr_type = abtUnknown,
                               const char * p_imsi = NULL)
    : abType(cntr_type), abImsi(p_imsi)
  { }
  ~AbonentContractInfo()
  { }

  static const char * type2Str(ContractType_e cntr_type)
  {
    if (cntr_type == abtPostpaid)
      return "Postpaid";
    if (cntr_type == abtPrepaid)
      return "Prepaid";
    return "Unknown";
  }

  const char * type2Str(void) const { return type2Str(abType); }

  //Returns cstr containing IMSI signals if latter is defined
  const char * getImsi(void) const { return !abImsi.empty() ? abImsi.c_str() : NULL; }

  void setImsi(const char * p_imsi)
  {
    if (p_imsi && p_imsi[0])
      abImsi = p_imsi;
    else
      abImsi.clear();
  }

  //Returns cstr containing either IMSI signals or string "none"
  const char * imsiCStr(void) const { return !abImsi.empty() ? abImsi.c_str() : "<none>"; }

  void clear(void)
  {
    abType = abtUnknown; abImsi.clear();
  }

  bool isUnknown(void) const { return (bool)(abType == abtUnknown); }
  bool isPostpaid(void) const { return (bool)(abType == abtPostpaid); }
  bool isPrepaid(void) const { return (bool)(abType == abtPrepaid); }
};

typedef AbonentContractInfo::ContractType_e AbonentContract_e;

//
typedef smsc::core::buffers::FixedLengthString<64 + 1> AbonentPolicyName_t;


} //inman
} //smsc
#endif /* SMSC_INMAN_SUBSCR_CONTRACT_HPP */

