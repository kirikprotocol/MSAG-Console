#ident "$Id$"
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

    INPCommandSetId CsId(void) const { return csAbntContract; }
    INPLoadMode loadMode(unsigned short obj_id) const { return INPCommandSetAC::lmHeader; }
};

class INPAbntContractCmd : public INPCommandAC {
public:
    INPAbntContractCmd(INPCSAbntContract::CommandTag cmd_tag) : INPCommandAC(cmd_tag)
    { }
    INPCommandSetAC * commandSet(void) const { return INPCSAbntContract::getInstance(); }
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
    std::string   nmPolicy;

public:
    AbntContractResult()
        : INPAbntContractCmd(INPCSAbntContract::ABNT_CONTRACT_RESULT_TAG)
    { }
    ~AbntContractResult() { }

    //Setters:
    void setContractInfo(const AbonentContractInfo & cntr_info) { cntrInfo = cntr_info; }
    void setPolicy(const std::string nm_policy) { nmPolicy = nm_policy; }

    //Getters:
    bool  cacheUsed(void) const { return nmPolicy.empty(); }
    const char * policyUsed(void) const { return nmPolicy.empty() ? NULL: nmPolicy.c_str(); }

    const AbonentContractInfo::ContractType contractType(void) const { return cntrInfo.ab_type; }
    const AbonentContractInfo & contractInfo(void) const { return cntrInfo; }

protected:
    void load(ObjectBuffer& in) throw(SerializerException)
    {
        in >> nmPolicy;
        uint8_t itmp;
        in >> itmp;
        cntrInfo.ab_type = static_cast<AbonentContractInfo::ContractType>(itmp);
        std::string stmp;
        in >> stmp;
        if (!cntrInfo.gsmSCF.scfAddress.fromText(stmp.c_str()))
            throw SerializerException("invalid gsmSCF address",
                                      SerializerException::invObjData, stmp.c_str());
        if (cntrInfo.gsmSCF.scfAddress.length)
            in >> cntrInfo.gsmSCF.serviceKey;

    }
    void save(ObjectBuffer& out) const
    {
        out << nmPolicy;
        out << (uint8_t)cntrInfo.ab_type;
        out << cntrInfo.gsmSCF.scfAddress.toString();
        if (cntrInfo.gsmSCF.scfAddress.length)
            out << cntrInfo.gsmSCF.serviceKey;
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

