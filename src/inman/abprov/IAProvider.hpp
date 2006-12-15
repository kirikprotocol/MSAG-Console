#ident "$Id$"
/* ************************************************************************** *
 * Abonent Providers definitions, Threaded Provider facility
 * ************************************************************************** */
#ifndef SMSC_INMAN_IAPROVIDER_HPP
#define SMSC_INMAN_IAPROVIDER_HPP

#include "inman/InCacheDefs.hpp"
using smsc::inman::cache::AbonentId;
//using smsc::inman::cache::AbonentBillType;
using smsc::inman::cache::AbonentCacheITF;
using smsc::inman::cache::AbonentRecord;



namespace smsc {
namespace inman {
namespace iaprvd { //(I)NMan (A)bonent (P)roviders

class IAPQueryListenerITF {
public:
    virtual void onIAPQueried(const AbonentId & ab_number, const AbonentRecord & ab_rec) = 0;
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

} //iaprvd
} //inman
} //smsc

#endif /* SMSC_INMAN_IAPROVIDER_HPP */

