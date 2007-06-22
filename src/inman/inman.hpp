#ident "$Id$"
#ifndef __SMSC_INMAN_HPP__
#define __SMSC_INMAN_HPP__

#include <map>

#include "core/synchronization/Mutex.hpp"
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

#include "inman/common/RPCList.hpp"
using smsc::inman::common::RPCList;

#include "inman/abprov/IAProvider.hpp"   //includes cache defs
using smsc::inman::iaprvd::IAProviderCreatorITF;
using smsc::inman::iaprvd::IAProviderITF;
using smsc::inman::iaprvd::IAProviderType;
using smsc::inman::iaprvd::IAProviderAbility_e;


namespace smsc  {
namespace inman {
//_smsXSrvs bits to mask, i.e. exclude from processing logic
#define SMSX_NOCHARGE_SRV  0x80000000   //SMSX Extra service free of charge
#define SMSX_INCHARGE_SRV  0x40000000   //Charge SMS via IN point despite of
                                        //billMode setting

#define SMSX_RESERVED_MASK  (SMSX_NOCHARGE_SRV|SMSX_INCHARGE_SRV)

class ChargeObj {
public:
    enum MSG_TYPE  { msgUnknown = 0, msgSMS = 1, msgUSSD = 2, msgXSMS = 3 };
    enum BILL_MODE { billOFF = 0, bill2CDR = 1, bill2IN = 2 };
};


struct XSmsService {
    std::string     name;
    uint32_t        mask;
    uint32_t        cdrCode; //actually it's a serviceId
    TonNpiAddress   adr;
    bool            chargeBearer;

    XSmsService(uint32_t use_mask = 0)
        : cdrCode(0), mask(use_mask), chargeBearer(false)
    { }
    XSmsService(const std::string & use_name, uint32_t use_mask = 0)
        : name(use_name), cdrCode(0), mask(use_mask), chargeBearer(false)
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

    SS7_CFG()
    { own_ssn = maxDlgId = capTimeout = userId = 0; }
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
    INScfCFG(const INScfCFG & use_cfg)
    {
        _ident = use_cfg._ident;
        rejectRPC = use_cfg.rejectRPC;
        postpaidRPC = use_cfg.postpaidRPC;
        idpLiAddr = use_cfg.idpLiAddr;

        uint32_t orgKey = scf.serviceKey;
        scf = use_cfg.scf;
        if (!scf.serviceKey)
            scf.serviceKey = orgKey;
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

    const char * Ident(void) const { return ident.c_str(); }
    bool useSS7(void) const
    {
        IAProviderType provType = !provAllc ? smsc::inman::iaprvd::iapCACHE : 
                                                provAllc->type();
        return (!scfMap.empty()
                || (provType == smsc::inman::iaprvd::iapIN)
                || (provType == smsc::inman::iaprvd::iapHLR));
    }

    bool getSCFparms(INScfCFG* scf) const
    {
        INScfsMAP::const_iterator it = scfMap.find(scf->scf.scfAddress.toString());
        if (it != scfMap.end()) {
            *scf = *(it->second);
            return true;
        }
        return false;
    }

    IAProviderAbility_e getIAPAbilities(void) const
    {
        return provAllc ? provAllc->ability() : smsc::inman::iaprvd::abNone;
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

    bool useSS7(void) const
    {
        for (AbonentPolicies::const_iterator it = begin(); it != end(); it++) {
            if ((*it)->useSS7())
                return true;
        }
        return false;
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
            return !empty() ? *begin() : NULL;
//        else { /* todo: search policy by matching the address pools */ }
    }
};

} //inman
} //smsc

#endif /* __SMSC_INMAN_HPP__ */

