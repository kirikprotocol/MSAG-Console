#ident "$Id$"
/* ************************************************************************** *
 * Abonent Providers interfaces definitions.
 * ************************************************************************** */
#ifndef SMSC_INMAN_IAPROVIDER_HPP
#define SMSC_INMAN_IAPROVIDER_HPP

#include "inman/InCacheDefs.hpp"
using smsc::inman::cache::AbonentId;
using smsc::inman::cache::AbonentCacheITF;
using smsc::inman::cache::AbonentRecord;

#include "inman/abprov/IAPErrors.hpp"
using smsc::util::RCHash;

#include "logger/Logger.h"
using smsc::logger::Logger;

namespace smsc {
namespace inman {
namespace iaprvd { //(I)NMan (A)bonent (P)roviders

typedef enum {
    iapCACHE = 0, iapIN, iapHLR, iapDB
} IAProviderType;

typedef enum {
    abNone = 0x00, abContract = 0x01, abSCF = 0x02, abContractSCF = 0x03
} IAProviderAbility_e;

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
    virtual void onIAPQueried(const AbonentId & ab_number, const AbonentSubscription & ab_info,
                                                        RCHash qry_status) = 0;
};

class IAProviderITF {
public:
    //binds AbonentCache to Provider
    virtual void bindCache(AbonentCacheITF * cache) = 0;
    //Starts query and binds listener to it. If AbonentCache is bound, the abonent info
    //will be stored in it on query completion. 
    //Returns true if query succesfully started, false otherwise
    virtual bool startQuery(const AbonentId & ab_number, IAPQueryListenerITF * pf_cb) = 0;
    //Unbinds query listener, cancels query if no listeners remain.
    virtual void cancelQuery(const AbonentId & ab_number, IAPQueryListenerITF * pf_cb) = 0;
    virtual void cancelAllQueries(void) = 0;
};

class IAProviderCreatorITF {
public:
    virtual IAProviderType      type(void) const = 0;
    virtual IAProviderAbility_e ability(void) const = 0;
    virtual const char *        ident(void) const = 0;
    virtual IAProviderITF *     create(Logger * use_log) = 0;
    virtual void                logConfig(Logger * use_log) const = 0;
};

} //iaprvd
} //inman
} //smsc

#endif /* SMSC_INMAN_IAPROVIDER_HPP */

