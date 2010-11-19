/* ************************************************************************** *
 * Abonent Providers interfaces definitions.
 * ************************************************************************** */
#ifndef SMSC_INMAN_IAPROVIDER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_INMAN_IAPROVIDER_HPP

#include "logger/Logger.h"

#include "inman/abprov/IAPDefs.hpp"
#include "inman/abprov/IAPErrors.hpp"
#include "inman/abprov/AbonentSubscription.hpp"

namespace smsc {
namespace inman {
namespace iaprvd { //(I)NMan (A)bonent (P)roviders

using smsc::logger::Logger;
using smsc::util::RCHash;

class IAPQueryListenerITF {
protected:
  virtual ~IAPQueryListenerITF() //forbid interface destruction
  { }

public:
  virtual void onIAPQueried(const AbonentId & ab_number,
                            const AbonentSubscription & ab_info,
                            RCHash qry_status) = 0;
};

class IAPQueryProcessorITF {
protected:
  virtual ~IAPQueryProcessorITF() //forbid interface destruction
  { }

public:
  //Starts query and binds listener to it.
  //Returns true if query succesfully started, false otherwise
  virtual bool startQuery(const AbonentId & ab_number, IAPQueryListenerITF * pf_cb) = 0;
  //Unbinds query listener, cancels query if no listeners remain.
  virtual void cancelQuery(const AbonentId & ab_number, IAPQueryListenerITF * pf_cb) = 0;
  virtual void cancelAllQueries(void) = 0;
};


class IAProviderAC : public IAPQueryProcessorITF {
private:
  const IAPProperty &   _iapProp;

protected:
  explicit IAProviderAC(IAPType_e iap_type)
    : _iapProp((iap_type < IAPProperty::iapReserved) ?
                _knownIAPType[iap_type] : _knownIAPType[IAPProperty::iapUnknown])
  { }
  //
  virtual ~IAProviderAC()  //forbid interface destruction
  { }

public:
  IAPType_e           getType(void) const { return _iapProp._iapKind; }
  const IAPProperty & getProperty(void) const { return _iapProp; }
  const char *        getIdent(void) const { return _iapProp._iapIdent.c_str(); }
  const IAPAbility &  getAbility(void) const { return _iapProp._iapAbility; }

  // ------------------------------------------
  // -- IAProviderAC interface methods
  // ------------------------------------------
  virtual void        logConfig(Logger * use_log = NULL) const = 0;
};

} //iaprvd
} //inman
} //smsc

#endif /* SMSC_INMAN_IAPROVIDER_HPP */

