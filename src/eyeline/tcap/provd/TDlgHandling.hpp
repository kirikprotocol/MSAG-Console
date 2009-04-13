/* ************************************************************************** *
 * TCAP Dialogue handling primitive(s) base class(es)
 * ************************************************************************** */
#ifndef __ELC_TCAP_DIALOGUE_PRIMITIVES_DEFS_HPP
#ident "@(#)$Id$"
#define __ELC_TCAP_DIALOGUE_PRIMITIVES_DEFS_HPP

#include "util/TonNpiAddress.hpp"
#include "eyeline/tcap/proto/TCAPMessage.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

using smsc::util::TonNpiAddress;

using eyeline::tcap::proto::TCAPMessage;
using eyeline::tcap::proto::TransactionId;
using eyeline::ros::ROSComponentsList;


typedef uint32_t TDialogueId;

//Base class for all TCAP Dialogue handling primitives
class TDialogueHandlingPrimitive {
protected:
  TDialogueId   _dlgId;
  TonNpiAddress _orgAdr;
  TonNpiAddress _dstAdr;
  TCAPMessage & _msgTC;

public:
  TDialogueHandlingPrimitive(TCAPMessage & use_tmsg)
    : _dlgId(0), _msgTC(use_tmsg)
  { }
  virtual ~TDialogueHandlingPrimitive()
  { }

  void setDialogueId(TDialogueId use_id) { _dlgId = use_id; }
  TDialogueId getDialogueId(void) const { return _dlgId; }

  void setOrigAddress(const TonNpiAddress & use_adr)  { _orgAdr = use_adr; }
  const TonNpiAddress & getOrigAddress(void) const    { return _orgAdr; }

  void setDestAddress(const TonNpiAddress & use_adr)  { _dstAdr = use_adr; }
  const TonNpiAddress & getDestAddress(void) const    { return _dstAdr; }

  void setTransactionId(const TransactionId & use_id)
  {
    _msgTC.setTransactionId(use_id);
  }
  const TransactionId & getTransactionId(void) const
  {
    return _msgTC.getTransactionId();
  }

  ROSComponentsList & CompList(void) { return _msgTC.CompList(); }
};

} //provd
} //tcap
} //eyeline

#endif /* __ELC_TCAP_DIALOGUE_PRIMITIVES_DEFS_HPP */

