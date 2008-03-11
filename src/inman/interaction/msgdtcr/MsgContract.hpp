#pragma ident "$Id$"
/* ************************************************************************* *
 * INMan Abonent Contract and gsmSCFs parameters determination protocol
 * PDUs definition.
 * ************************************************************************* */
#ifndef SMSC_INMAN_ABNT_CONTRACT_MESSAGES_HPP
#define SMSC_INMAN_ABNT_CONTRACT_MESSAGES_HPP

#include "inman/interaction/messages.hpp"
#include "inman/AbntContract.hpp"
using smsc::inman::AbonentContractInfo;

namespace smsc  {
namespace inman {
namespace interaction {
// -------------------------------------------------------------------- //
// Abonent Contract detection CommandSet: 
// -------------------------------------------------------------------- //
class INPCSAbntContract : public INPCommandSetAC {
public:
    typedef enum {
        ABNT_CONTRACT_REQUEST_TAG  = 6,    // AbntContractRequest  ( SMSC --> INMAN )
        ABNT_CONTRACT_RESULT_TAG   = 7     // AbntContractResult   ( SMSC <-- INMAN )
    } CommandTag;

    typedef enum { HDR_DIALOG = 1 } HeaderFrm;

    INPCSAbntContract();

    static INPCSAbntContract * getInstance(void);

    INProtocol::CSId CsId(void) const { return INProtocol::csAbntContract; }
    const char *    CsName(void) const { return "csAbntContract"; }
    INPLoadMode loadMode(unsigned short obj_id) const { return INPCommandSetAC::lmHeader; }
};

class INPAbntContractCmd : public INPCommandAC {
public:
    INPAbntContractCmd(INPCSAbntContract::CommandTag cmd_tag)
        : INPCommandAC(cmd_tag)
    { }
    inline const INPCommandSetAC * commandSet(void) const
    {
        return INPCSAbntContract::getInstance();
    }
};

// --------------------------------------------------------- //
// Abonent Contract detection commands headers: 
// --------------------------------------------------------- // 
class CSAbntContractHdr_dlg : public INPHeaderAC {
public:
    uint32_t dlgId;

    CSAbntContractHdr_dlg() : SerializableObjectAC(INPCSAbntContract::HDR_DIALOG)
    { }

    void load(ObjectBuffer &in) throw(SerializerException)  { in >> dlgId; }
    void save(ObjectBuffer &out) const                      { out << dlgId; }
};


// --------------------------------------------------------- //
// Abonent Contract detection commands: 
// --------------------------------------------------------- // 
class AbntContractRequest : public INPAbntContractCmd {
protected:
    bool          useCache;
    std::string   subscrNum;

public:
    AbntContractRequest()
        : INPAbntContractCmd(INPCSAbntContract::ABNT_CONTRACT_REQUEST_TAG)
        , useCache(true)
    { }
    ~AbntContractRequest() { }
    //--Setters
    void setSubscrNumber(const std::string& subscr_adr) { subscrNum = subscr_adr; }
    void allowCache(bool use_cache = true)              { useCache = use_cache; }

    //--Getters
    bool cacheMode(void) const                          { return useCache; }
    const std::string & subscrAddr(void) const          { return subscrNum; }

protected:
    //SerializableObject interface
    void load(ObjectBuffer& in) throw(SerializerException)
    {
        in >> useCache;
        in >> subscrNum;
    }
    void save(ObjectBuffer& out) const
    {
        out << useCache;
        out << subscrNum;
    }
};


class AbntContractResult : public INPAbntContractCmd {
protected:
    AbonentContractInfo cntrInfo;
    std::string     nmPolicy;
    uint32_t        errCode;
    std::string     errMsg;

public:
    AbntContractResult()
        : INPAbntContractCmd(INPCSAbntContract::ABNT_CONTRACT_RESULT_TAG), errCode(0)
    { }
    ~AbntContractResult() { }

    //Setters:
    void setContractInfo(const AbonentContractInfo & cntr_info) { cntrInfo = cntr_info; }
    void setPolicy(const std::string nm_policy) { nmPolicy = nm_policy; }
    void setError(uint32_t err_code, const char * err_msg = NULL)
    { 
        cntrInfo.ab_type = AbonentContractInfo::abtUnknown;
        cntrInfo.tdpSCF.clear();
        errCode = err_code;
        if (err_msg)
            errMsg = err_msg;
        else
            errMsg.clear();
    }

    //Getters:
    inline bool  cacheUsed(void) const { return nmPolicy.empty(); }
    inline const char * policyUsed(void) const { return nmPolicy.empty() ? NULL: nmPolicy.c_str(); }
    inline uint32_t    errorCode(void) const { return errCode; }
    inline const char * errorMsg(void) const { return errMsg.c_str(); }

    const AbonentContractInfo::ContractType contractType(void) const { return cntrInfo.ab_type; }
    const AbonentContractInfo & contractInfo(void) const { return cntrInfo; }

protected:
    void load(ObjectBuffer& in) throw(SerializerException)
    {
        in >> nmPolicy;
        {
            uint8_t itmp;
            in >> itmp;
            cntrInfo.ab_type = static_cast<AbonentContractInfo::ContractType>(itmp);
        }
        {
            std::string stmp;
            in >> stmp;
            if (stmp.empty())
                cntrInfo.tdpSCF.clear();
            else {
                TonNpiAddress adr;
                if (!adr.fromText(stmp.c_str()))
                    throw SerializerException("invalid gsmSCF address",
                                              SerializerException::invObjData, stmp.c_str());
                cntrInfo.tdpSCF[TDPCategory::dpMO_SM].scfAddress = adr;
            }
        }
        in >> errCode;
        if (cntrInfo.getSCFinfo(TDPCategory::dpMO_SM)) {
            cntrInfo.tdpSCF[TDPCategory::dpMO_SM].serviceKey = errCode;
            errCode = 0;
        }
        {
            std::string stmp;
            in >> stmp;
            if (stmp.empty())
                cntrInfo.abImsi[0] = 0;
            else {
                if (stmp.length() > MAP_MAX_IMSI_AddressValueLength)
                    throw SerializerException("invalid IMSI",
                                              SerializerException::invObjData, stmp.c_str());
                cntrInfo.setImsi(stmp.c_str());
            }
        }
        in >> errMsg;
    }
    void save(ObjectBuffer& out) const
    {
        out << nmPolicy;
        out << (uint8_t)cntrInfo.ab_type;

        GsmSCFinfo          smScf;
        const GsmSCFinfo *  p_scf = cntrInfo.getSCFinfo(TDPCategory::dpMO_SM);
        if (!p_scf) { //check if SCF for MO-BC may be used
            if ((p_scf = cntrInfo.getSCFinfo(TDPCategory::dpMO_BC)) != 0)
                smScf.scfAddress = p_scf->scfAddress;
        } else 
            smScf = *p_scf;

        if (smScf.scfAddress.empty())
            out << (uint8_t)0x00;
        else
            out << smScf.scfAddress.toString();

        out << (errCode ? errCode : smScf.serviceKey);
        if (cntrInfo.getImsi()) {
            std::string si(cntrInfo.getImsi());
            out << si;
        } else
            out << (uint8_t)0x00;
        out << errMsg;
    }
};

// --------------------------------------------------------- //
// Solid instances of packets:
// --------------------------------------------------------- //
typedef INPSolidPacketT<CSAbntContractHdr_dlg, AbntContractRequest>  SPckContractRequest;
typedef INPSolidPacketT<CSAbntContractHdr_dlg, AbntContractResult>   SPckContractResult;

// --------------------------------------------------------- //
// Abonent Contract detection command handlers:
// --------------------------------------------------------- //
class AbntContractReqHandlerITF {
public:
    virtual bool onContractReq(AbntContractRequest* req, uint32_t req_id) = 0;
};

class AbntContractResHandlerITF {
public:
    virtual void onContractResult(AbntContractResult* res, uint32_t req_id) = 0;
};

} //interaction
} //inman
} //smsc

#endif /* SMSC_INMAN_ABNT_CONTRACT_MESSAGES_HPP */

