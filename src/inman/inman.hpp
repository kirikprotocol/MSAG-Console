#pragma ident "$Id$"
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
using smsc::inman::iaprvd::IAProvider;


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


typedef std::map<std::string, const INScfCFG *> INScfsMAP;

class AbonentPolicy {
protected:
    friend class INManConfig;

    mutable Mutex   _sync;
    std::string     ident;
    IAProviderCreatorITF * provAllc;

public:
    INScfsMAP      scfMap;

    AbonentPolicy(const char * nm_pol)
        : ident(nm_pol), provAllc(NULL)
    { }
    ~AbonentPolicy()
    {
        MutexGuard grd(_sync);
        if (provAllc)
             delete provAllc;
    }

    inline const char * Ident(void) const { return ident.c_str(); }
    bool useSS7(void) const
    {
        IAProvider::Type provType = !provAllc ? IAProvider::iapCACHE : 
                                                provAllc->type();
        return (!scfMap.empty()
                || (provType == IAProvider::iapIN)
                || (provType == IAProvider::iapHLR));
    }

    const INScfCFG * getSCFparms(const TonNpiAddress* scf) const
    {
        INScfsMAP::const_iterator it = scfMap.find(scf->toString());
        return (it != scfMap.end()) ? it->second : NULL;
    }

    inline IAProvider::Ability getIAPAbilities(void) const
    {
        return provAllc ? provAllc->ability() : IAProvider::abNone;
    }

    IAProviderITF * getIAProvider(void)
    {
        MutexGuard grd(_sync);
        return provAllc->getProvider();
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

