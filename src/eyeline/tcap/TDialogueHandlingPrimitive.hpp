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

  ROSComponentsList & CompList(void) { return _msgTC.CompList(); }
};

} //tcap
} //eyeline

#endif

