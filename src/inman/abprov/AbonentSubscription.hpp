/* ************************************************************************* *
 * Abonent CSI (CAMEL Subscription Information) helper classes definitions
 * ************************************************************************* */
#ifndef SMSC_INMAN_ABONENT_SUBSCRIPTION_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_INMAN_ABONENT_SUBSCRIPTION_HPP

#include <string>
#include <time.h>

#include "inman/abprov/CSIRecordsMap.hpp"
#include "inman/AbntContract.hpp"
#include "inman/comp/ODBDefs.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {

using smsc::inman::comp::ODBGeneralData;

typedef smsc::util::TonNpiAddress AbonentId; //isdn international number assumed

class AbonentSubscription : public AbonentContractInfo {
private:
  using AbonentContractInfo::clear;

public:
  CSIRecordsMap   csiSCF;
  ODBGeneralData  odbGD;
  TonNpiAddress   vlrNum;
  time_t          tmQueried;

  explicit AbonentSubscription(ContractType_e abType = abtUnknown,
                               const char * p_imsi = NULL, time_t qryTm = 0)
    : AbonentContractInfo(abType, p_imsi), tmQueried(qryTm)
  { }
  ~AbonentSubscription()
  { }

  //
  const GsmSCFinfo * getSCFinfo(CSIUid_e csi_id) const
  {
    return csiSCF.getSCFinfo(csi_id);
  }

  //NOTE: tmQueried = zero, means record is ALWAYS expired!
  bool isExpired(uint32_t interval) const
  {
    return (time(NULL) >= (tmQueried + (time_t)interval));
  }

  const TonNpiAddress * getVLR(void) const
  {
    return vlrNum.empty() ? &vlrNum : NULL; 
  }
  //return size of string representation
  size_t  str_size(void) const;
  //Appends to use_str
  void toString(std::string & use_str) const;
  //
  std::string toString(void) const;

  //
  void clear(void)
  {
    AbonentContractInfo::clear(); 
    csiSCF.clear(); vlrNum.clear(); tmQueried = 0;
  }

  //Returns true if at least one parameter was updated
  bool Merge(const AbonentSubscription & use_rcd);
};

} //iaprvd
} //inman
} //smsc
#endif /* SMSC_INMAN_ABONENT_SUBSCRIPTION_HPP */

