#ident "$Id$"
#ifndef SMSC_INMAN_ABONENT_PROVIDER_HPP
#define SMSC_INMAN_ABONENT_PROVIDER_HPP

#include "util/config/ConfigView.h"
using smsc::util::config::ConfigView;
using smsc::util::config::ConfigException;
//using smsc::util::config::CStrSet;

#include "inman/incache.hpp"
using smsc::inman::cache::AbonentId;
using smsc::inman::cache::AbonentBillType;
using smsc::inman::cache::AbonentCacheITF;


namespace smsc {
namespace inman {
namespace abprov {

class InAbonentQueryListenerITF {
public:
    virtual void onAbonentQueried(const AbonentId & ab_number, AbonentBillType ab_type) = 0;
};

class InAbonentProviderITF {
public:
    //binds AbonentCache to Provider
    virtual void bindCache(AbonentCacheITF * cache) = 0;
    //Starts query and binds listener to it. If AbonentCache is bound, the abonent info
    //will be stored in it on query completion. 
    //Returns true if query succesfully started, false otherwise
    virtual bool startQuery(const AbonentId & ab_number, InAbonentQueryListenerITF * pf_cb) = 0;
    //Unbinds query listener, cancels query if no listeners remain.
    virtual void cancelQuery(const AbonentId & ab_number, InAbonentQueryListenerITF * pf_cb) = 0;
    virtual void cancelAllQueries(void) = 0;
};

//typedef char * AbonentProviderId;

class AbonentProviderCreatorITF {
public:
    virtual const char * ident(void) const = 0;
    virtual InAbonentProviderITF *    create(Logger * use_log) = 0;
    virtual void  logConfig(Logger * use_log) const = 0;
};

//This is the Provider dynamic library entry point
extern "C" AbonentProviderCreatorITF * 
    loadupAbonentProvider(ConfigView* provCfg, Logger * use_log)
                                            throw(ConfigException);

class AbonentProviderLoader {
public:
    static const AbonentProviderCreatorITF *
        LoadDL(const char * dlname, ConfigView* provCfg) throw(ConfigException);

protected:
    void* operator new(size_t);
    AbonentProviderLoader() { }
    ~AbonentProviderLoader() { }
};

} //abprov
} //inman
} //smsc

#endif /* SMSC_INMAN_ABONENT_PROVIDER_HPP */

