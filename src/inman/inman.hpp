#ident "$Id$"
#ifndef __SMSC_INMAN_HPP__
#define __SMSC_INMAN_HPP__

#include <map>

#include "core/synchronization/Mutex.hpp"
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

#include "inman/common/RPCList.hpp"
using smsc::inman::common::RPCList;

#include "inman/abprov/IAPLoader.hpp"
using smsc::inman::iaprvd::IAProviderCreatorITF;
using smsc::inman::iaprvd::IAProviderITF;

#include "inman/incache.hpp"
using smsc::inman::cache::AbonentCacheITF;


namespace smsc  {
namespace inman {

class INManConfig;

struct SS7_CFG {    //SS7 stack interaction:
    unsigned char   userId;         //PortSS7 user id [1..20]
    unsigned short  capTimeout;     //optional timeout for operations with IN platform
    TonNpiAddress   ssf_addr;       //local ISDN address
    int             own_ssn;        //local SSN
    unsigned short  maxDlgId;       //maximum number of simultaneous TC dialogs, max: 65530
};

#define RP_MO_SM_transfer_rejected 21       //3GPP TS 24.011 Annex E-2
class INScfCFG {
public:
    std::string     _ident;   //INPlatform ident
    MAPSCFinfo      scf;
    RPCList         rejectRPC;      //list of RP causes forcing charging denial because of low balance
    RPCList         postpaidRPC;    //list of RP causes returned for postpaid abonents

    INScfCFG(const char * name = NULL)
    { 
        if (name) _ident += name;
        rejectRPC.push_back(RP_MO_SM_transfer_rejected);
    }
    void reset(void)
    {
        scf.serviceKey = 0;
        scf.scfAddress.clear();
        rejectRPC.clear();
        rejectRPC.push_back(RP_MO_SM_transfer_rejected);
        postpaidRPC.clear();
    }
    const char * ident(void)
    {
        return _ident.c_str();
    }
};

typedef std::map<std::string, INScfCFG*> INScfsMAP;

class AbonentPolicy {
protected:
    friend class INManConfig;

    Mutex           _sync;
    AbonentCacheITF * cache;
    IAProviderITF * prvd;
    IAProviderCreatorITF * provAllc;

public:
    const char *    ident;   //policy ident
    INScfsMAP      scfMap;

    AbonentPolicy(const char * nm_pol = NULL)
        : ident(nm_pol), prvd(NULL), provAllc(NULL), cache(NULL)
    { }
    ~AbonentPolicy()
    {
        if (provAllc)
             delete provAllc;
    }

    bool getSCFparms(INScfCFG* scf)
    {
        INScfsMAP::const_iterator it = scfMap.find(scf->scf.scfAddress.toString());
        if (it != scfMap.end()) {
            *scf = *((*it).second);
            return true;
        }
        return false;
    }

    IAProviderITF * getIAProvider(Logger * use_log)
    {
        MutexGuard grd(_sync);
        if (!prvd && provAllc) {
            if (!(prvd = provAllc->create(use_log)))
                smsc_log_error(use_log, "%s: AbonentProvider %s initialization failed!",
                               ident, provAllc->ident());
            else {
                smsc_log_info(use_log, "%s: AbonentProvider %s inited", ident,
                              provAllc->ident());
                provAllc->logConfig(use_log);
                if (cache)
                    prvd->bindCache(cache);
            }
        }
        return prvd;
    }

    void bindCache(AbonentCacheITF * use_cache)
    {
        MutexGuard grd(_sync);
        cache = use_cache;
        if (prvd)
            prvd->bindCache(cache);
    }
};

//NOTE: the first policy is a preferred one !!!
class AbonentPolicies : public std::list<AbonentPolicy*> {
public:
    AbonentPolicies()
    { }
    ~AbonentPolicies()
    { 
        for (AbonentPolicies::iterator it = begin(); it != end(); it++)
            delete (*it);
    }

    void setPreferred(AbonentPolicy* def_pol)
    { 
        if (def_pol) push_front(def_pol); 
    }
    void addPolicy(AbonentPolicy* use_pol)
    {
        if (use_pol) push_back(use_pol); 
    }

    AbonentPolicy* getPolicy(const TonNpiAddress * an_number = NULL) const
    {
//        if (!ab_number)
            return *begin();
//        else { /* todo: search policy by matching the address pools */ }
    }
};

} //inman
} //smsc

#endif /* __SMSC_INMAN_HPP__ */

