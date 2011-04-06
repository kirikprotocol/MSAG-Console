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

#define IAPROVIDER_DFLT_LOGGER "smsc.inman.iap"

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
  //Returns false if listener unable to handle query report right now, so
  //requests query to be rereported.
  virtual bool onIAPQueried(const AbonentId & ab_number,
                            const AbonentSubscription & ab_info,
                            RCHash qry_status) = 0;
};

class IAPQueryProcessorITF {
protected:
  virtual ~IAPQueryProcessorITF() //forbid interface destruction
  { }

public:
  // -------------------------------------------------------
  // -- IAPQueryProcessorITF interface methods
  // -------------------------------------------------------
  //Starts query and binds listener to it.
  //Returns true if query succesfully started, false otherwise
  virtual bool startQuery(const AbonentId & ab_number, IAPQueryListenerITF & pf_cb) = 0;
  //Unbinds query listener, cancels query if no listeners remain.
  //Returns false if listener is already targeted and query waits for its mutex.
  virtual bool cancelQuery(const AbonentId & ab_number, IAPQueryListenerITF & pf_cb) = 0;
  //Attempts to cancel all queries.
  //Returns false if at least one listener is already targeted and query waits for its mutex.
  virtual bool cancelAllQueries(void) = 0;
};


class IAProviderAC : public IAPQueryProcessorITF {
private:
  const IAPProperty &   _iapProp;

protected:
  explicit IAProviderAC(const IAPProperty & iap_prop) : _iapProp(iap_prop)
  { }
  //
  virtual ~IAProviderAC()
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

