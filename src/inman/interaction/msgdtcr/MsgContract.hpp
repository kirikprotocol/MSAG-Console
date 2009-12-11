/* ************************************************************************* *
 * INMan Abonent Contract and gsmSCFs parameters determination protocol
 * PDUs definition.
 * ************************************************************************* */
#ifndef SMSC_INMAN_ABNT_CONTRACT_MESSAGES_HPP
#ident "@(#)$Id$"
#define SMSC_INMAN_ABNT_CONTRACT_MESSAGES_HPP

#include "inman/interaction/messages.hpp"
#include "inman/AbntContract.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

using smsc::inman::AbonentContractInfo;
using smsc::util::TonNpiAddressString;

// -------------------------------------------------------------------- //
// Abonent Contract detection CommandSet: 
// -------------------------------------------------------------------- //
class INPCSAbntContract : public INPCommandSetAC {
public:
    enum CommandTag {
        ABNT_CONTRACT_REQUEST_TAG  = 6,    // AbntContractRequest  ( SMSC --> INMAN )
        ABNT_CONTRACT_RESULT_TAG   = 7     // AbntContractResult   ( SMSC <-- INMAN )
    };

    enum HeaderFrm { HDR_DIALOG = 1 };

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
    const INPCommandSetAC * commandSet(void) const
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
    bool                useCache;
    TonNpiAddressString subscrNum;

    // -----------------------------------------
    // -- SerializableObjectAC interface methods
    // -----------------------------------------
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

public:
    AbntContractRequest()
        : INPAbntContractCmd(INPCSAbntContract::ABNT_CONTRACT_REQUEST_TAG)
        , useCache(true)
    { }
    ~AbntContractRequest() { }
    //--Setters
    void setSubscrNumber(const char * subscr_adr) { subscrNum = subscr_adr; }
    void allowCache(bool use_cache = true)        { useCache = use_cache; }

    //--Getters
    bool cacheMode(void) const                    { return useCache; }
    const TonNpiAddressString & subscrAddr(void) const { return subscrNum; }
};


class AbntContractResult : public INPAbntContractCmd {
protected:
    AbonentContractInfo cntrInfo;
    std::string     nmPolicy;
    uint32_t        errCode;
    std::string     errMsg;

    // -----------------------------------------
    // -- SerializableObjectAC interface methods
    // -----------------------------------------
    void load(ObjectBuffer& in) throw(SerializerException);
    void save(ObjectBuffer& out) const;

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
    bool  cacheUsed(void) const { return nmPolicy.empty(); }
    const char * policyUsed(void) const { return nmPolicy.empty() ? NULL: nmPolicy.c_str(); }
    uint32_t    errorCode(void) const { return errCode; }
    const char * errorMsg(void) const { return errMsg.c_str(); }

    const AbonentContractInfo::ContractType contractType(void) const { return cntrInfo.ab_type; }
    const AbonentContractInfo & contractInfo(void) const { return cntrInfo; }
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
protected:
    virtual ~AbntContractReqHandlerITF() //forbid interface destruction
    { }

public:
    virtual bool onContractReq(AbntContractRequest* req, uint32_t req_id) = 0;
};

class AbntContractResHandlerITF {
protected:
    virtual ~AbntContractResHandlerITF() //forbid interface destruction
    { }

public:
    virtual void onContractResult(AbntContractResult* res, uint32_t req_id) = 0;
};

} //interaction
} //inman
} //smsc

#endif /* SMSC_INMAN_ABNT_CONTRACT_MESSAGES_HPP */

