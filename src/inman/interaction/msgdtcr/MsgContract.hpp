/* ************************************************************************* *
 * INMan Abonent Contract and gsmSCFs parameters determination protocol
 * PDUs definition.
 * ************************************************************************* */
#ifndef SMSC_INMAN_ABNT_CONTRACT_MESSAGES_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_INMAN_ABNT_CONTRACT_MESSAGES_HPP

#include "inman/interaction/messages.hpp"
#include "inman/AbntContract.hpp"
#include "inman/GsmSCFInfo.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

using smsc::inman::AbonentContractInfo;
using smsc::inman::AbonentContract_e;
using smsc::inman::GsmSCFinfo;
using smsc::util::TonNpiAddressString;

// -------------------------------------------------------------------- //
// Abonent Contract detection CommandSet: 
// -------------------------------------------------------------------- //
class INPCSAbntContract : public INPCommandSetAC { //singleton
protected:
  INPCSAbntContract();
  ~INPCSAbntContract()
  { }

public:
  enum CommandTag {
    ABNT_CONTRACT_REQUEST_TAG  = 6,    // AbntContractRequest  ( SMSC --> INMAN )
    ABNT_CONTRACT_RESULT_TAG   = 7     // AbntContractResult   ( SMSC <-- INMAN )
  };

  enum HeaderFrm { HDR_DIALOG = 1 };

  static INPCSAbntContract * getInstance(void);

  // ----------------------------------------
  // -- INPCommandSetAC interface methods
  // ----------------------------------------
  virtual INPLoadMode loadMode(unsigned short obj_id) const
  {
    return INPCommandSetAC::lmHeader;
  }
};

class INPAbntContractCmd : public INPCommandAC {
protected:
  INPAbntContractCmd(INPCSAbntContract::CommandTag cmd_tag)
    : INPCommandAC(cmd_tag)
  { }

public:
  virtual ~INPAbntContractCmd()
  { }

  // ----------------------------------
  // -- INPCommandAC interface methods
  // ----------------------------------
  virtual const INPCommandSetAC * commandSet(void) const
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
  virtual void load(ObjectBuffer& in) throw(SerializerException)
  {
      in >> useCache;
      in >> subscrNum;
  }
  virtual void save(ObjectBuffer& out) const
  {
      out << useCache;
      out << subscrNum;
  }

public:
  static const INPCSAbntContract::CommandTag
    _cmdTAG = INPCSAbntContract::ABNT_CONTRACT_REQUEST_TAG;

  AbntContractRequest() : INPAbntContractCmd(_cmdTAG), useCache(true)
  { }
  ~AbntContractRequest()
  { }

  //--Setters
  void setSubscrNumber(const char * subscr_adr) { subscrNum = subscr_adr; }
  void allowCache(bool use_cache = true)        { useCache = use_cache; }

  //--Getters
  bool cacheMode(void) const                    { return useCache; }
  const TonNpiAddressString & subscrAddr(void) const { return subscrNum; }
};


class AbntContractResult : public INPAbntContractCmd {
protected:
  AbonentContract_e cntrType;
  uint32_t        errCode;
  IMSIString      abImsi;
  GsmSCFinfo      gsmSCF;
  std::string     nmPolicy;
  std::string     errMsg;

  // -----------------------------------------
  // -- SerializableObjectAC interface methods
  // -----------------------------------------
  virtual void load(ObjectBuffer& in) throw(SerializerException);
  virtual void save(ObjectBuffer& out) const;

public:
  static const INPCSAbntContract::CommandTag
    _cmdTAG = INPCSAbntContract::ABNT_CONTRACT_RESULT_TAG;

  AbntContractResult() : INPAbntContractCmd(_cmdTAG)
    , cntrType(AbonentContractInfo::abtUnknown), errCode(0)
  { }
  ~AbntContractResult()
  { }

  //Setters:
  void setContractInfo(AbonentContract_e cntr_type, const char * ab_imsi = NULL);
  void setGsmSCF(const GsmSCFinfo & gsm_scf) { gsmSCF = gsm_scf; }
  void setPolicy(const std::string & nm_policy) { nmPolicy = nm_policy; }
  void setError(uint32_t err_code, const char * err_msg = NULL);

  //Getters:
  bool  cacheUsed(void) const { return nmPolicy.empty(); }
  const char * policyUsed(void) const { return nmPolicy.empty() ? NULL : nmPolicy.c_str(); }
  uint32_t    errorCode(void) const { return errCode; }
  const char * errorMsg(void) const { return errMsg.c_str(); }

  AbonentContract_e contractType(void) const { return cntrType; }
  const char * getSubscrImsi(void) const { return abImsi.empty() ? NULL : abImsi.c_str(); }
  const GsmSCFinfo * getGsmSCF(void) const { return gsmSCF.empty() ? NULL : &gsmSCF; }
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

