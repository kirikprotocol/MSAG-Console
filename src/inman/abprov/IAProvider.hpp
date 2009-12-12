/* ************************************************************************** *
 * Abonent Providers interfaces definitions.
 * ************************************************************************** */
#ifndef SMSC_INMAN_IAPROVIDER_HPP
#ident "@(#)$Id$"
#define SMSC_INMAN_IAPROVIDER_HPP

#include "logger/Logger.h"

#include "inman/abprov/IAPErrors.hpp"
#include "inman/AbntContract.hpp"

namespace smsc {
namespace inman {
namespace iaprvd { //(I)NMan (A)bonent (P)roviders

using smsc::logger::Logger;
using smsc::util::RCHash;
using smsc::inman::AbonentId;
using smsc::inman::AbonentRecord;

struct AbonentSubscription {
    AbonentRecord   abRec;
    TonNpiAddress   vlrNum;

    void reset(void)
        { abRec.reset(); vlrNum.clear(); }

    const TonNpiAddress * getVLRNum(void) const
        { return vlrNum.length ? &vlrNum : NULL; }

    TonNpiAddressString vlr2Str(void) const 
        { return vlrNum.length ? vlrNum.toString() : "<none>"; }
};

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

class IAProviderITF : public IAPQueryProcessorITF {
protected:
  virtual ~IAProviderITF()  //forbid interface destruction
  { }

public:
  enum Type_e {
    iapUnknown = 0, iapDB, iapHLR, iapATSI
  };
  enum Ability_e {
    abNone = 0x00, abContract = 0x01,
    abSCF = 0x02, abContractSCF = 0x03
  };

  static const char * nmType(Type_e val_type);
  static const char * nmAbility(Ability_e val_type);

  virtual Type_e        type(void) const = 0;
  virtual Ability_e     ability(void) const = 0;
  virtual const char *  ident(void) const = 0;
  virtual void          logConfig(Logger * use_log = NULL) const = 0;
};

} //iaprvd
} //inman
} //smsc

#endif /* SMSC_INMAN_IAPROVIDER_HPP */

