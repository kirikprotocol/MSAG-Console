#ident "$Id$"
#ifndef __SMSC_INMAN_HPP__
#define __SMSC_INMAN_HPP__

#include <map>

#include "core/synchronization/Mutex.hpp"
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

#include "inman/common/RPCList.hpp"
using smsc::inman::common::RPCList;

#include "inman/abprov/IAPLoader.hpp"   //includes cache defs
using smsc::inman::iaprvd::IAProviderCreatorITF;
using smsc::inman::iaprvd::IAProviderITF;


namespace smsc  {
namespace inman {
//_smsXSrvs bits to mask, i.e. exclude from processing logic
#define SMSX_RESERVED_MASK  0x80000000

struct XSmsService {
    std::string     name;
    uint32_t        mask;
    uint32_t        cdrCode;
    TonNpiAddress   adr;

    XSmsService(uint32_t use_mask = 0) : cdrCode(0), mask(use_mask)
    { }
    XSmsService(const std::string & use_name, uint32_t use_mask = 0)
        : name(use_name), cdrCode(0), mask(use_mask)
    { }
};
typedef std::map<uint32_t, XSmsService> SmsXServiceMap;

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
public:         //SwitchingCenter, SMS Center, INMan
    typedef enum { idpLiMSC = 0, idpLiSMSC = 1, idpLiSSF = 2 } IDPLocationAddr;

    std::string     _ident;   //INPlatform ident
    GsmSCFinfo      scf;
    RPCList         rejectRPC;      //list of RP causes forcing charging denial because of low balance
    RPCList         postpaidRPC;    //list of RP causes returned for postpaid abonents
    IDPLocationAddr idpLiAddr;      //nature of address to substitute into
                                    //LocationInformationMSC of InitialDP operation
                                    //while interacting this IN platfrom

    INScfCFG(const char * name = NULL) : idpLiAddr(idpLiMSC)
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
        idpLiAddr = idpLiMSC;
    }
    const char * ident(void) { return _ident.c_str(); }
};

typedef std::map<std::string, INScfCFG*> INScfsMAP;

class AbonentPolicy {
protected:
    friend class INManConfig;

    Mutex           _sync;
    AbonentCacheITF * cache;
    IAProviderITF * prvd;
    IAProviderCreatorITF * provAllc;
    std::string     ident;

public:
    INScfsMAP      scfMap;

    AbonentPolicy(const char * nm_pol)
        : ident(nm_pol), prvd(NULL), provAllc(NULL), cache(NULL)
    { }
    ~AbonentPolicy()
    {
        if (provAllc)
             delete provAllc;
    }

    const char * Ident(void) { return ident.c_str(); }

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
                               ident.c_str(), provAllc->ident());
            else {
                smsc_log_info(use_log, "%s: AbonentProvider %s inited",
                              ident.c_str(), provAllc->ident());
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
            return size() ? *begin() : NULL;
//        else { /* todo: search policy by matching the address pools */ }
    }
};

} //inman
} //smsc

#endif /* __SMSC_INMAN_HPP__ */

