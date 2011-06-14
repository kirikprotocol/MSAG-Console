/* ************************************************************************** *
 * USS Gateway protocol PDUs definition.
 * ************************************************************************** */
#ifndef __SMSC_USS_GATEWAY_PDUS_DEFS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_USS_GATEWAY_PDUS_DEFS

#include "ussman/comp/USSOperationData.hpp"

#include "ussman/interaction/IProtoUSSG.hpp"
#include "ussman/interaction/USSGMessages.hpp"

namespace smsc  {
namespace ussman {
namespace interaction {

using smsc::util::TonNpiAddress;
using smsc::util::IMSIString;

using smsc::ussman::comp::USSDataString;
using smsc::ussman::comp::USSOperationData;

// --------------------------------------------------------- //
// Base class for USSman commands
// --------------------------------------------------------- // 
class USSGatewayCmdAC : public SerializableObjIface
                      , public USSOperationData {
protected:
  static const unsigned   _ussOpData_strSZ =
    sizeof("msAdr=%s, ussData={%s}") + TonNpiAddress::_strSZ + USSDataString::_max_StrForm_sz;

  explicit USSGatewayCmdAC(INPUSSGateway::CommandTag_e cmd_tag)
    : SerializableObjIface(cmd_tag), USSOperationData()
  { }

  //
  unsigned log2str(char * use_buf, unsigned buf_len = _ussOpData_strSZ) const;

public:
  virtual ~USSGatewayCmdAC()
  { }

  void setMsIsdn(const char * adrStr) throw (std::exception);

  const USSOperationData & getOpData(void) const
  {
    return *(static_cast<const USSOperationData*>(this));
  }

  // --------------------------------------------
  // -- SerializableObjIface interface methods
  // --------------------------------------------
  virtual void load(PacketBufferAC & in_buf) throw(SerializerException);
  virtual void save(PacketBufferAC & out_buf) const throw(SerializerException);
};


template <INPUSSGateway::CommandTag_e _TagArg>
class USSGatewayCmd_T : public USSGatewayCmdAC {
protected:
  USSGatewayCmd_T() : USSGatewayCmdAC(_TagArg)
  { }

public:
  static const INPUSSGateway::CommandTag_e _cmdTAG = _TagArg;

  virtual ~USSGatewayCmd_T()
  { }
};


template < INPUSSGateway::CommandTag_e _TagArg >
const INPUSSGateway::CommandTag_e USSGatewayCmd_T<_TagArg>::_cmdTAG;

// --------------------------------------------------------- //
// USSMan PDU's command objects:
// --------------------------------------------------------- // 
class USSRequestMessage : public USSGatewayCmd_T<INPUSSGateway::PROCESS_USS_REQUEST_TAG> {
public:
  uint8_t       _inSSN;   //supplementary service provider SSN
  TonNpiAddress _inAddr;  //supplementary service provider ISDN address
  IMSIString    _imsi;    //subscriber IMSI, optional

  static const unsigned _ussReq_strSZ = sizeof("IN={%s:%u}, IMSI=%s, %s")
                                        + TonNpiAddress::_strSZ + IMSIString::MAX_SZ
                                        + _ussOpData_strSZ;

  USSRequestMessage() : USSGatewayCmd_T<INPUSSGateway::PROCESS_USS_REQUEST_TAG>()
    , _inSSN(0)
  { }
  virtual ~USSRequestMessage()
  { }

  unsigned log2str(char * use_buf, unsigned buf_len = _ussReq_strSZ) const;

  void setIN_Isdn(const char * addr_str) throw (std::exception);

  const IMSIString * getIMSI(void) const { return _imsi.empty() ? NULL : &_imsi; }

  // --------------------------------------------
  // -- SerializableObjIface interface methods
  // --------------------------------------------
  virtual void load(PacketBufferAC & in_buf) throw(SerializerException);
  virtual void save(PacketBufferAC & out_buf) const throw(SerializerException);
};


class USSResultMessage : public USSGatewayCmd_T<INPUSSGateway::PROCESS_USS_RESULT_TAG> {
public:
  enum Status_e {
    reqOK = 0           //request successfully processed
    , reqDUPLICATE = 1  //same request is already in process
    , reqFAILED = 2     //request failed because of an error is occured while processing.
    , reqDENIED = 3     //request cann't be processed due to some limitations, further attempt is allowed.
    , reqUNKNOWN = 0xFF //
  };

  Status_e  _status;

  static const unsigned _ussRes_strSZ = sizeof("status = %u(%s), %s") + sizeof("reqDUPLICATE") +_ussOpData_strSZ;

  static Status_e uint2Status(uint16_t ui_val)
  {
    return (ui_val <= reqFAILED) ? static_cast<Status_e>(ui_val) : reqUNKNOWN;
  }

  static const char * status2str(Status_e st_val);

  USSResultMessage() : USSGatewayCmd_T<INPUSSGateway::PROCESS_USS_RESULT_TAG>()
    , _status(reqOK)
  { }
  virtual ~USSResultMessage()
  { }

  bool      isStatusOk(void) const { return _status == reqOK; }

  unsigned  log2str(char * use_buf, unsigned buf_len = _ussRes_strSZ) const;

  // --------------------------------------------
  // -- SerializableObjIface interface methods
  // --------------------------------------------
  virtual void load(PacketBufferAC & in_buf) throw(SerializerException);
  virtual void save(PacketBufferAC & out_buf) const throw(SerializerException);
};

// --------------------------------------------------------- //
// Solid instances of USSMan packets:
// --------------------------------------------------------- //
typedef USSGPacket_T<USSRequestMessage>  SPckUSSRequest;
typedef USSGPacket_T<USSResultMessage>   SPckUSSResult;

// --------------------------------------------------------- //
// USSman command handler interface:
// --------------------------------------------------------- //
class USSCommandHandlerITF {
protected:
  virtual ~USSCommandHandlerITF() //forbid interface destruction
  { }

public:
  virtual void onProcessUSSRequest(USSRequestMessage * req_msg) = 0;
  virtual void onDenyUSSRequest(USSRequestMessage * req_msg) = 0;
};

} //interaction
} //ussman
} //smsc

#endif /* __SMSC_USS_GATEWAY_PDUS_DEFS */
