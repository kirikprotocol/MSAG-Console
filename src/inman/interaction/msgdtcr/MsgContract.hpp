/* ************************************************************************* *
 * INMan Abonent Contract and gsmSCFs parameters determination protocol
 * PDUs definition.
 * ************************************************************************* */
#ifndef SMSC_INMAN_ABNT_CONTRACT_MESSAGES_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_INMAN_ABNT_CONTRACT_MESSAGES_HPP

#include "inman/AbntContract.hpp"
#include "inman/GsmSCFInfo.hpp"

#include "inman/interaction/serializer/IMessages.hpp"
#include "inman/interaction/serializer/SerializeIntegers.hpp"
#include "inman/interaction/serializer/SerializeStdString.hpp"
#include "inman/interaction/serializer/SerializeFxdLenStringT.hpp"

#include "inman/interaction/msgdtcr/IProtoContract.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

using smsc::inman::AbonentContractInfo;
using smsc::inman::AbonentContract_e;
using smsc::inman::AbonentPolicyName_t;
using smsc::inman::GsmSCFinfo;
using smsc::util::TonNpiAddressString;

// --------------------------------------------------------- //
// Abonent Contract detection PDU's command objects:
// --------------------------------------------------------- // 
template <INPAbntContract::CommandTag_e _TagArg>
class INPAbntContractCmd_T : public SerializableObjIface {
protected:
  explicit INPAbntContractCmd_T() : SerializableObjIface(_TagArg)
  { }

public:
  static const INPAbntContract::CommandTag_e _cmdTAG = _TagArg;

  virtual ~INPAbntContractCmd_T()
  { }

  // --------------------------------------------
  // -- SerializableObjIface interface methods
  // --------------------------------------------
  //virtual void load(PacketBufferAC & in_buf) throw(SerializerException) = 0;
  //virtual void save(PacketBufferAC & out_buf) const throw(SerializerException) = 0;
};

template < INPAbntContract::CommandTag_e _TagArg >
const INPAbntContract::CommandTag_e INPAbntContractCmd_T<_TagArg>::_cmdTAG;


// --------------------------------------------------------- //
// Abonent Contract detection PDU's headers:
// --------------------------------------------------------- // 
class INPAbntContractHdr_dlg : public SerializableObjIface {
public:
  uint32_t dlgId;

  INPAbntContractHdr_dlg()
    : SerializableObjIface(INPAbntContract::HDR_DIALOG)
  { }
  virtual ~INPAbntContractHdr_dlg()
  { }

  // --------------------------------------------
  // -- SerializableObjIface interface methods
  // --------------------------------------------
  virtual void load(PacketBufferAC & in_buf) throw(SerializerException)
  {
    in_buf >> dlgId;
  }
  virtual void save(PacketBufferAC & out_buf) const throw(SerializerException)
  {
    out_buf << dlgId;
  }
};


// --------------------------------------------------------- //
// Abonent Contract detection commands: 
// --------------------------------------------------------- // 
class AbntContractRequest : public
  INPAbntContractCmd_T<INPAbntContract::ABNT_CONTRACT_REQUEST_TAG> {
protected:
  bool                useCache;
  TonNpiAddressString subscrNum;

public:
  AbntContractRequest(): INPAbntContractCmd_T<INPAbntContract::ABNT_CONTRACT_REQUEST_TAG>()
    , useCache(true)
  { }
  ~AbntContractRequest()
  { }

  //--Setters
  void setSubscrNumber(const char * subscr_adr) { subscrNum = subscr_adr; }
  void allowCache(bool use_cache = true)        { useCache = use_cache; }

  //--Getters
  bool cacheMode(void) const                    { return useCache; }
  const TonNpiAddressString & subscrAddr(void) const { return subscrNum; }

  // -----------------------------------------
  // -- SerializableObjectAC interface methods
  // -----------------------------------------
  virtual void load(PacketBufferAC & in_buf) throw(SerializerException)
  {
    in_buf >> useCache;
    in_buf >> subscrNum;
  }
  virtual void save(PacketBufferAC & out_buf) const throw(SerializerException)
  {
    out_buf << useCache;
    out_buf << subscrNum;
  }
};


class AbntContractResult : public 
  INPAbntContractCmd_T<INPAbntContract::ABNT_CONTRACT_RESULT_TAG> {
protected:
  AbonentContract_e   cntrType;
  uint32_t            errCode;
  IMSIString          abImsi;
  GsmSCFinfo          gsmSCF;
  AbonentPolicyName_t nmPolicy;
  std::string         errMsg;

public:
  AbntContractResult() : INPAbntContractCmd_T<INPAbntContract::ABNT_CONTRACT_RESULT_TAG>()
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

  // -----------------------------------------
  // -- SerializableObjectAC interface methods
  // -----------------------------------------
  virtual void load(PacketBufferAC & in_buf) throw(SerializerException);
  virtual void save(PacketBufferAC & out_buf) const throw(SerializerException);
};

// --------------------------------------------------------- //
// Template class for PDU packets:
// --------------------------------------------------------- //
template <
  class _Command /* : public INPAbntContractCmd_T<_TagArg> */
>
class SPckAbntContract_T : public INPPacket_T<INPAbntContractHdr_dlg, _Command> {
public:
  static IProtocolAC::PduId getPduId(void)
  {
    return INPAbntContract::mkPduId(_Command::_cmdTAG, INPAbntContract::HDR_DIALOG);
  }
};

// --------------------------------------------------------- //
// Solid instances of PDU packets:
// --------------------------------------------------------- //
typedef SPckAbntContract_T<AbntContractRequest>  SPckContractRequest;
typedef SPckAbntContract_T<AbntContractResult>   SPckContractResult;

// --------------------------------------------------------- //
// Abonent Contract detection command handlers:
// --------------------------------------------------------- //
class AbntContractReqHandlerITF {
protected:
  virtual ~AbntContractReqHandlerITF() //forbid interface destruction
  { }

public:
  virtual bool onContractReq(const AbntContractRequest * p_req, uint32_t req_id) = 0;
};

class AbntContractResHandlerITF {
protected:
  virtual ~AbntContractResHandlerITF() //forbid interface destruction
  { }

public:
  virtual void onContractResult(AbntContractResult * res, uint32_t req_id) = 0;
};

} //interaction
} //inman
} //smsc

#endif /* SMSC_INMAN_ABNT_CONTRACT_MESSAGES_HPP */

