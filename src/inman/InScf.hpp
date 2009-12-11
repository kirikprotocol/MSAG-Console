/* ************************************************************************** *
 * Types and classes describing IN-point configuration parameters.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_INSCF_HPP__
#ident "@(#)$Id$"
#define __SMSC_INMAN_INSCF_HPP__

#include <map>

#include "util/TonNpiAddress.hpp"
#include "inman/GsmSCFInfo.hpp"
#include "inman/common/RPCList.hpp"

namespace smsc  {
namespace inman {

using smsc::util::format;
using smsc::inman::common::RPCList;
using smsc::inman::common::RPCListATT;


//Defines category of trigger DPs from various State Models
struct TDPCategory {
    enum Id {
          dpUnknown = 0
        // ...
        , dpMO_BC       //Basic Call, originating trigger DPs
        , dpMO_SM       //Short Mesage, originating trigger DPs
//        , dpMT_BC       //Basic Call, terminating trigger DPs
//        , dpMT_SM       //Short Mesage, terminating trigger DPs
        // ...
        , dpRESERVED_MAX  //just a maximum cap
    };

    static const char * Name(unsigned tdp_type)
    {
        switch (tdp_type) {
        case dpMO_BC: return "MO-BC";
        case dpMO_SM: return "MO-SM";
//        case dpMT_BC: return "MT-BC";
//        case dpMT_SM: return "MT-SM";
        default:;
        }
        return "unknown";
    }
};

//Defines serving gsmSCF params for DPs of specified category
class TDPScfMap : public std::map<TDPCategory::Id, GsmSCFinfo> {
public:
    TDPScfMap() : std::map<TDPCategory::Id, GsmSCFinfo>()
    { }
    ~TDPScfMap()
    { }

    size_type Merge(const TDPScfMap & use_map)
    {
        for (TDPScfMap::const_iterator it = use_map.begin(); it != use_map.end(); ++it)
            TDPScfMap::insert(*it);
        return TDPScfMap::size();
    }

    std::string toString(TDPCategory::Id tdp_type) const
    {
        char buf[sizeof("%s{%s}") + sizeof("dpUnknown") + GsmSCFinfo::_strSZ];
        int  n = snprintf(buf, sizeof(buf)-1, "%s{", TDPCategory::Name(tdp_type));

        TDPScfMap::const_iterator it = find(tdp_type);
        if (it != end())
            n += it->second.toString(buf + n, true, (unsigned)sizeof(buf)-1-n);
        else
            n += snprintf(buf + n, (unsigned)sizeof(buf)-1-n, "<none>");
        buf[n++] = '}';
        buf[n] = 0;
        return buf;
    }

    std::string toString(void) const
    {
        std::string str("SCFs: ");
        if (empty())
            str += "<none>";
        else {
            short i = 0;
            for (TDPScfMap::const_iterator it = begin(); it != end(); ++it, ++i) {
                if (i)
                    str += ", ";
                str += TDPCategory::Name(it->first); str += "{";
                str += it->second.toString(); str += "}";
            }
        }
        return str;
    }
};


typedef std::map<uint32_t /*SKeyIdx*/, uint32_t /*SKeyVal*/> SKeyMAP;
typedef std::map<TDPCategory::Id, uint32_t /*SKeyVal*/> TDPSkeyMAP;

class SKAlgorithmAC {
protected:
    TDPCategory::Id tgtTDP;

public:
    enum Type { algSKVal = 0, algSKMap } algId;

    SKAlgorithmAC(TDPCategory::Id tgt_tdp) : tgtTDP(tgt_tdp)
    { }
    virtual ~SKAlgorithmAC()
    { }

    TDPCategory::Id TDPType(void) const { return tgtTDP; }

    virtual SKAlgorithmAC::Type Id(void) const = 0;
    virtual uint32_t getSKey(void * arg = NULL) const = 0;
    virtual std::string toString(void) const = 0;
};

class SKAlgorithm_SKVal : public SKAlgorithmAC {
protected:
    uint32_t skVal;

public:
    SKAlgorithm_SKVal(TDPCategory::Id tgt_tdp, uint32_t use_val)
        : SKAlgorithmAC(tgt_tdp), skVal(use_val)
    { }

    //SKAlgorithmAC methods implementation
    SKAlgorithmAC::Type Id(void) const { return SKAlgorithmAC::algSKVal; }
    uint32_t getSKey(void * arg) const { return skVal; }
    std::string toString(void) const
    {
        char buf[sizeof("val: %u") + sizeof(uint32_t)*3];
        snprintf(buf, sizeof(buf)-1, "val: %u", skVal);
        return buf;
    }
};

class SKAlgorithm_SKMap : public SKAlgorithmAC {
protected:
    TDPCategory::Id argTDP;
    SKeyMAP         skMap;

public:
    SKAlgorithm_SKMap(TDPCategory::Id tgt_tdp, TDPCategory::Id arg_tdp)
        : SKAlgorithmAC(tgt_tdp), argTDP(arg_tdp)
    { }

    TDPCategory::Id argType(void) const { return argTDP; }
    void insert(uint32_t skey_idx, uint32_t skey_val)
    {
        skMap.insert(SKeyMAP::value_type(skey_idx, skey_val));
    }
    SKeyMAP::size_type size(void) { return skMap.size(); }

    //SKAlgorithmAC methods implementation
    SKAlgorithmAC::Type Id(void) const { return SKAlgorithmAC::algSKMap; }
    uint32_t getSKey(void * use_arg = NULL) const //arg is type of uint32_t
    {
        if (!use_arg)
            return 0;
        uint32_t arg = *(uint32_t*)use_arg;
        SKeyMAP::const_iterator it = skMap.find(arg);
        return it != skMap.end() ? it->second : 0;
    }
    std::string toString(void) const
    {
        std::string rstr("map{");
        rstr += TDPCategory::Name(argTDP); rstr += ",";
        rstr += TDPCategory::Name(tgtTDP); rstr += "}: {";
        unsigned i = 0;
        for (SKeyMAP::const_iterator it = skMap.begin(); it != skMap.end(); ++it, ++i) {
            if (i)
                rstr += ", ";
            format(rstr, "{%u,%u}", it->first, it->second);
        }
        rstr += "}";
        return rstr;
    }
};

typedef std::map<TDPCategory::Id, SKAlgorithmAC*> SKAlgorithmMAP;

#define RP_MO_SM_transfer_rejected 21       //3GPP TS 24.011 Annex E-2
class INScfCFG {
public:         //SwitchingCenter, SMS Center, INMan
    enum IDPLocationAddr { idpLiMSC = 0, idpLiSMSC = 1, idpLiSSF = 2 } ;
    enum IDPReqMode      { idpReqMT = 0, idpReqSEQ };

    std::string     _ident;         //INPlatform ident
    TonNpiAddress   scfAdr;         //gsmSCF address always has ISDN international format
    SKAlgorithmMAP  skAlg;          //translation algoritms for various TDPs and service keys 
    //optional params:
    RPCList         rejectRPC;      //list of RP causes forcing charging denial because of low balance
    RPCListATT      retryRPC;       //list of RP causes indicating that IN point should be
                                    //interacted again a bit later
    IDPLocationAddr idpLiAddr;      //nature of initiator address to substitute into
                                    //LocationInformationMSC of InitialDP operation
                                    //while interacting this IN platfrom
    IDPReqMode      idpReqMode;     //mode of IDP requests (simultaneous or sequential)
                                    //this IN-point supports

    INScfCFG(const char * name = NULL) : idpLiAddr(idpLiMSC), idpReqMode(idpReqMT)
    { 
        if (name) _ident += name;
        rejectRPC.push_back(RP_MO_SM_transfer_rejected);
    }
    ~INScfCFG()
    {
        for (SKAlgorithmMAP::iterator it = skAlg.begin(); it != skAlg.end(); ++it) {
            delete it->second;
        }
        skAlg.clear();
    }

    const char * Ident(void) const
    {
        return _ident.size() ? _ident.c_str() : scfAdr.getSignals();
    }

    uint32_t getSKey(TDPScfMap * orgTDPs, TDPCategory::Id tgtTDP = TDPCategory::dpMO_SM) const
    {
        SKAlgorithmMAP::const_iterator acit = skAlg.find(tgtTDP);
        if (acit != skAlg.end()) {
            if (acit->second->Id() == SKAlgorithmAC::algSKVal)
                return acit->second->getSKey(NULL);

            if ((acit->second->Id() == SKAlgorithmAC::algSKMap) && orgTDPs) {
                SKAlgorithm_SKMap * alg = static_cast<SKAlgorithm_SKMap *>(acit->second);

                TDPScfMap::const_iterator tcit = orgTDPs->find(alg->argType());
                if (tcit != orgTDPs->end())
                    return alg->getSKey((void*)&(tcit->second.serviceKey));
            }
        }
        return 0;
    }
};

} //inman
} //smsc

#endif /* __SMSC_INMAN_INSCF_HPP__ */

