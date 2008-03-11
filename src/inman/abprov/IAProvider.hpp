#pragma ident "$Id$"
/* ************************************************************************** *
 * Abonent Providers interfaces definitions.
 * ************************************************************************** */
#ifndef SMSC_INMAN_IAPROVIDER_HPP
#define SMSC_INMAN_IAPROVIDER_HPP

#include "inman/abprov/IAPErrors.hpp"
using smsc::util::RCHash;

//#include "logger/Logger.h"
//using smsc::logger::Logger;

#include "inman/AbntContract.hpp"
using smsc::inman::AbonentId;
using smsc::inman::AbonentRecord;

#include "inman/services/ICSrvDefs.hpp"
using smsc::inman::ICSIdsSet;
using smsc::inman::ICServicesHostITF;

namespace smsc {
namespace inman {
namespace iaprvd { //(I)NMan (A)bonent (P)roviders

struct IAProvider {
    enum Type {
        iapCACHE = 0, iapIN, iapHLR, iapDB
    };
    enum Ability {
        abNone = 0x00, abContract = 0x01,
        abSCF = 0x02, abContractSCF = 0x03
    };
};

struct AbonentSubscription {
    AbonentRecord   abRec;
    TonNpiAddress   vlrNum;

    void reset(void)
        { abRec.reset(); vlrNum.clear(); }

    inline const TonNpiAddress * getVLRNum(void) const
        { return vlrNum.length ? &vlrNum : NULL; }

    std::string vlr2Str(void) const 
        { return vlrNum.length ? vlrNum.toString() : "<none>"; }
};

class IAPQueryListenerITF {
public:
    virtual void onIAPQueried(const AbonentId & ab_number,
                              const AbonentSubscription & ab_info,
                              RCHash qry_status) = 0;
};

class IAProviderITF {
public:
    //Starts query and binds listener to it.
    //Returns true if query succesfully started, false otherwise
    virtual bool startQuery(const AbonentId & ab_number, IAPQueryListenerITF * pf_cb) = 0;
    //Unbinds query listener, cancels query if no listeners remain.
    virtual void cancelQuery(const AbonentId & ab_number, IAPQueryListenerITF * pf_cb) = 0;
    virtual void cancelAllQueries(void) = 0;
};

class IAProviderCreatorITF {
public:
    virtual IAProvider::Type    type(void) const = 0;
    virtual IAProvider::Ability ability(void) const = 0;
    virtual const char *        ident(void) const = 0;
    virtual const ICSIdsSet &   ICSDeps(void) const = 0;
    virtual void                logConfig(Logger * use_log = NULL) const = 0;
    //Ensures the provider is properly initialized and returns its interface
    virtual IAProviderITF *     startProvider(const ICServicesHostITF * use_host) = 0;
    virtual void                stopProvider(bool do_wait = false) = 0;

    virtual ~IAProviderCreatorITF()
    { }
};

} //iaprvd
} //inman
} //smsc

#endif /* SMSC_INMAN_IAPROVIDER_HPP */

