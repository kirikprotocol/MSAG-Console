/* ************************************************************************** *
 * TCAP Dialogue handling primitive(s) base class(es)
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_TDIALOGUEHANDLINGPRIMITIVE_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_TDIALOGUEHANDLINGPRIMITIVE_HPP__

# include "util/TonNpiAddress.hpp"
# include "eyeline/tcap/TDialogueId.hpp"
# include "eyeline/tcap/proto/TCAPMessage.hpp"
# include "eyeline/tcap/proto/TransactionId.hpp"

namespace eyeline {
namespace tcap {

using smsc::util::TonNpiAddress;

using eyeline::tcap::proto::TCAPMessage;
using eyeline::ros::ROSComponentsList;

//Base class for all TCAP Dialogue handling primitives
class TDialogueHandlingPrimitive {
protected:
  TDialogueId   _dlgId;
  TonNpiAddress _orgAdr;
  TonNpiAddress _dstAdr;
  proto::TCAPMessage & _msgTC;

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

typedef enum { PREARRANGED_END, BASIC_END, ABORT } end_transaction_facility_t;

} //tcap
} //eyeline

#endif

