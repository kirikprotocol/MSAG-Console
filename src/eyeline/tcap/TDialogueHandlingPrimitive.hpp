/* ************************************************************************** *
 * TCAP Dialogue handling primitive(s) base class(es)
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_TDIALOGUEHANDLINGPRIMITIVE_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_TDIALOGUEHANDLINGPRIMITIVE_HPP__

# include "eyeline/sccp/SCCPAddress.hpp"
# include "eyeline/tcap/TDialogueId.hpp"
# include "eyeline/tcap/proto/TCAPMessage.hpp"
# include "eyeline/tcap/proto/TransactionId.hpp"
# include "eyeline/asn1/EncodedOID.hpp"

namespace eyeline {
namespace tcap {

using eyeline::sccp::SCCPAddress;
using eyeline::tcap::proto::TCAPMessage;
using eyeline::ros::ROSComponentsList;


//Base class for all TCAP Dialogue handling primitives
class TDialogueHandlingPrimitive {
protected:
  TDialogueId   _dlgId;
  TCAPMessage & _msgTC;
  SCCPAddress   _orgAdr;
  SCCPAddress   _dstAdr;

public:
  enum PAbortCause_e {
    p_unrecognizedMessageType = 0
    , p_unrecognizedTransactionID = 1
    , p_badlyFormattedTransactionPortion = 2
    , p_incorrectTransactionPortion = 3
    , p_resourceLimitation = 4
    , p_dialogueTimeout = 126
    , p_genericError = 127  //That's a max value allowed by protocol
  };

  TDialogueHandlingPrimitive();

  TDialogueHandlingPrimitive(TCAPMessage & use_tmsg)
    : _dlgId(0), _msgTC(use_tmsg)
  { }
  virtual ~TDialogueHandlingPrimitive()
  { }

  void setDialogueId(TDialogueId use_id) { _dlgId = use_id; }
  TDialogueId getDialogueId(void) const { return _dlgId; }

  void setOrigAddress(const SCCPAddress & use_adr)  { _orgAdr = use_adr; }
  const SCCPAddress & getOrigAddress(void) const    { return _orgAdr; }

  void setDestAddress(const SCCPAddress & use_adr)  { _dstAdr = use_adr; }
  const SCCPAddress & getDestAddress(void) const    { return _dstAdr; }

  void setTransactionId(const proto::TransactionId & use_id)
  {
    _msgTC.setTransactionId(use_id);
  }
  const proto::TransactionId & getTransactionId(void) const
  {
    return _msgTC.getTransactionId();
  }

  const asn1::EncodedOID* getAppCtx() const;

  ROSComponentsList & CompList(void) { return _msgTC.CompList(); }
};

} //tcap
} //eyeline

#endif

