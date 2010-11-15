/* ************************************************************************** *
 * AbonentInfo: CAMEL subcriptions and MsISDN
 * ************************************************************************** */
#ifndef __INMAN_TEST_ABONENT_INFO_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_TEST_ABONENT_INFO_HPP

#include "inman/abprov/AbonentSubscription.hpp"

namespace smsc  {
namespace inman {
namespace test {

using smsc::util::TonNpiAddress;
using smsc::inman::iaprvd::AbonentSubscription;


class AbonentInfo : public AbonentSubscription {
private:
  using AbonentSubscription::clear;

public:
  TonNpiAddress  msIsdn;

  explicit AbonentInfo(const TonNpiAddress & p_adr,
                       AbonentContract_e cntr_type = abtUnknown,
                       const char * p_imsi = NULL)
    : AbonentSubscription(cntr_type, p_imsi), msIsdn(p_adr)
  { }

  explicit AbonentInfo(AbonentContract_e cntr_type = abtUnknown,
                       const char * p_adr = NULL,
                       const char * p_imsi = NULL)
    : AbonentSubscription(cntr_type, p_imsi)
  {
    msIsdn.fromText(p_adr);
  }

  bool Empty(void) const { return (bool)(!msIsdn.length); }

  void clear(void) { AbonentSubscription::clear(); msIsdn.clear(); }

  void setSubscription(const AbonentSubscription & ctr_inf)
  {
    *(AbonentSubscription*)this = ctr_inf;
  }
};

} //test
} //inman
} //smsc
#endif /* __INMAN_TEST_ABONENT_INFO_HPP */

