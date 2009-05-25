/* ************************************************************************** *
 * TCAP Dialogue handling primitive(s) base class(es)
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_TDIALOGUEHANDLINGPRIMITIVE_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_TDIALOGUEHANDLINGPRIMITIVE_HPP__

# include "eyeline/asn1/EncodedOID.hpp"
# include "eyeline/ros/ROSCompList.hpp"
# include "eyeline/tcap/TDialogueId.hpp"
# include "eyeline/tcap/TDlgUserInfo.hpp"
# include "eyeline/tcap/proto/TransactionId.hpp"

namespace eyeline {
namespace tcap {

using eyeline::asn1::EncodedOID;
using eyeline::ros::ROSComponentsList;
using eyeline::tcap::proto::TransactionId;

//Base class for all TCAP Dialogue handling primitives
class TDialogueHandlingPrimitive {
protected:
  TDialogueId   _dlgId;
  TransactionId _trId;
  TDlgUserInfo  _usrInfo;

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

  TDialogueHandlingPrimitive() : _dlgId(0)
  { }
  virtual ~TDialogueHandlingPrimitive()
  { }

  void setDialogueId(TDialogueId use_id) { _dlgId = use_id; }
  TDialogueId getDialogueId(void) const { return _dlgId; }

  void setTransactionId(const TransactionId & use_id) { _trId = use_id; }
  const TransactionId & getTransactionId(void) const  { return _trId; }

  TDlgUserInfo & UserInfo(void) { return _usrInfo; }

  // ------------------------------------------------
  // -- TDialogueHandlingPrimitive interface methods
  // ------------------------------------------------
  virtual const EncodedOID * getAppCtx(void) const = 0;
  virtual ROSComponentsList * CompList(void) = 0;
};

} //tcap
} //eyeline

#endif /* __EYELINE_TCAP_TDIALOGUEHANDLINGPRIMITIVE_HPP__ */

