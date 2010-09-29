/* ************************************************************************** *
 * TCProvider: SCCP Service Provider requests.
 * ************************************************************************** */
#ifndef __ELC_TCAP_SCSP_REQUESTS_HPP
#ident "@(#)$Id$"
#define __ELC_TCAP_SCSP_REQUESTS_HPP

#include "core/buffers/ExtendingBuf.hpp"

#include "eyeline/sccp/SCCPAddress.hpp"
#include "eyeline/sccp/SCCPDefs.hpp"
#include "eyeline/ss7na/libsccp/MessageProperties.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

using eyeline::sccp::SCCPAddress;
using eyeline::sccp::SCCPStandard;

typedef eyeline::ss7na::libsccp::MessageProperties SCSPMsgProperties;

//Buffer capable to store LUDT_96 message at max
typedef smsc::core::buffers::ExtendingBuffer_T<uint8_t, uint16_t, 4076> UDTDataBuffer;

class SCSPUnitdataReq : public SCSPMsgProperties {
protected:
  const SCCPStandard::Kind_e  _sccpStd;
  //
  unsigned              _calledPartyLen;
  unsigned              _callingPartyLen;
  uint8_t               _calledParty[SCCPAddress::_maxOctsLen];
  uint8_t               _callingParty[SCCPAddress::_maxOctsLen];
  //
  UDTDataBuffer         _dataBuf;

public:
  SCSPUnitdataReq(SCCPStandard::Kind_e use_std = SCCPStandard::itut93)
    : _sccpStd(use_std), _calledPartyLen(0), _callingPartyLen(0)
  {
    _calledParty[0] = _callingParty[0] = 0;
  }
  ~SCSPUnitdataReq()
  { }

  SCCPStandard::Kind_e getSCCPStandard(void) const { return _sccpStd; }

  //Returns false in case of corrupted address
  bool setCalledAddr(const SCCPAddress & use_adr)
  {
    return (_calledPartyLen = use_adr.pack2Octs(_calledParty)) != 0;
  }
  //Returns false in case of corrupted address
  bool setCallingAddr(const SCCPAddress & use_adr)
  {
    return (_callingPartyLen = use_adr.pack2Octs(_callingParty)) != 0;
  }

  UDTDataBuffer & dataBuf(void) { return _dataBuf; }
  const UDTDataBuffer & dataBuf(void) const { return _dataBuf; }
  
  uint8_t calledAddrLen(void) const { return _calledPartyLen; }
  const uint8_t * calledAddr(void) const { return _calledParty; }

  uint8_t callingAddrLen(void) const { return _callingPartyLen; }
  const uint8_t * callingAddr(void) const { return _callingParty; }

  uint16_t userDataLen(void) const { return _dataBuf.getDataSize(); }
  const uint8_t * userData(void) const { return _dataBuf.get(); }
};

} //provd
} //tcap
} //eyeline

#endif /* __ELC_TCAP_SCSP_REQUESTS_HPP */

