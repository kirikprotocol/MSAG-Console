/* ************************************************************************** *
 * TCAP messages according to
 * itu-t(0) recommendation(0) q(17) 773 modules(2) messages(1) version3(3).
 * ************************************************************************** */
#ifndef __TCAP_MESSAGE_DEFS_HPP
#ident "@(#)$Id$"
#define __TCAP_MESSAGE_DEFS_HPP

#include "eyeline/tcap/proto/TransactionId.hpp"
#include "eyeline/ros/ROSCompList.hpp"
//#include "eyeline/tcap/proto/TCStrDialogue.hpp"
#include "eyeline/tcap/proto/TCUniDialogue.hpp"
#include "eyeline/tcap/proto/TCAPAbort.hpp"


namespace eyeline {
namespace tcap {
namespace proto {

using eyeline::ros::ROSComponentPrimitive;
using eyeline::ros::ROSComponentRfp;
using eyeline::ros::ROSComponentsList;


//TCAP unidirection dialogue portion.
//May be the one of APDUs defined in _ac_tcap_strDialogue_as.
typedef TCUniDialogueAS TCUniDlgPortion;

//TCAP Begin/Continue/End messages structured dialogue portion.
//May be the one of APDUs defined in _ac_tcap_strDialogue_as.
typedef TCStrDialogueAS TCMsgDlgPortion;

//TCAP Abort dialogue portion.
typedef TCMsgAbort TCMsgAbortPortion;

//TCAP message: aggregates {TransactionId, DialoguePortion, ComponentPortion}.
//DialoguePortion may be one of two alternatives:
// 1) normal dialogue portion (T_Begin/T_Cont/T_End)
// 2) abort dialogue portion (T_Abort)
// 3) unidirection dialogue portion
class TCAPMessage : public ASTypeAC {
public:
  enum TKind_e {
    t_none = -1
    //uniDialogue-a messages:
    , t_unidirection = 1    //[APPLICATION 1]
    //dialogue-as messages:
    , t_begin = 2           //[APPLICATION 2]
    , t_end = 4             //[APPLICATION 4]
    , t_continue = 5        //[APPLICATION 5]
    , t_abort = 7           //[APPLICATION 7]
  };

private:
  uint8_t pduMem[eyeline::util::MaxSizeOf3_T< TCMsgAbortPortion, 
                                            TCMsgDlgPortion, TCUniDlgPortion >::VALUE];
  union {
    ASTypeAC *          ac; //base class
    TCMsgDlgPortion *   dlg;
    TCMsgAbortPortion *  abrt;
    TCUniDlgPortion *   uni;
  }       _pdu;
  TKind_e _msgKind;

protected:
  TransactionId       _trId;
  ROSComponentsList   _compPart;

  void resetPdu(void)
  {
    if (_pdu.ac) {
      _pdu.ac->~ASTypeAC();
      _pdu.ac = 0;
    }
    _compPart.clear();
  }

public:
  TCAPMessage(TKind_e use_kind = t_none)
  {
    asTags().addOption(ASTagging(ASTag::tagApplication, TCAPMessage::t_unidirection));
    asTags().addOption(ASTagging(ASTag::tagApplication, TCAPMessage::t_begin));
    asTags().addOption(ASTagging(ASTag::tagApplication, TCAPMessage::t_end));
    asTags().addOption(ASTagging(ASTag::tagApplication, TCAPMessage::t_continue));
    asTags().addOption(ASTagging(ASTag::tagApplication, TCAPMessage::t_abort));
    //
    _pdu.ac = 0;
    if ((_msgKind = use_kind) != use_kind)
      Reset(use_kind);
  }
  ~TCAPMessage()
  {
    resetPdu();
    _msgKind = t_none;
  }

  TKind_e MsgKind(void) const { return _msgKind; }
  //
  void Reset(TKind_e use_kind = t_none, bool is_dlg_resp = false)
  {
    resetPdu();
    switch ((_msgKind = use_kind)) {
    case t_begin: {
      _pdu.ac = new(pduMem)TCMsgDlgPortion(TCDlgPduAC::pduAARQ);
    } break;
    case t_continue:
    case t_end: {
      if (is_dlg_resp)
        _pdu.ac = new(pduMem)TCMsgDlgPortion(TCDlgPduAC::pduAARE);
    } break;
    case t_abort:
      _pdu.ac = new(pduMem)TCMsgAbortPortion();
    break;
    case t_unidirection:
      _pdu.ac = new(pduMem)TCUniDlgPortion();
    break;

    default:; //t_none
    }
    //NOTE: if use_kind is not one of specified by constructor options,
    //currently selected tag is set to undefined
    asTags().selectOption(ASTag(ASTag::tagApplication, use_kind));
  }

  void setTransactionId(const TransactionId & use_id) { _trId = use_id; }
  const TransactionId & getTransactionId(void) const { return _trId; }
  //
  TCMsgDlgPortion * DlgPortion(void) const
  {
    return ((_msgKind == t_begin) || (_msgKind == t_continue)
            || (_msgKind == t_end)) ? _pdu.dlg : 0;
  }
  //
  TCMsgAbortPortion * AbortPortion(void) const
  {
    return (_msgKind == t_abort) ? _pdu.abrt : 0;
  }
  //
  TCUniDlgPortion * UniPortion(void) const
  {
    return (_msgKind == t_unidirection) ? _pdu.uni : 0;
  }
  //
  const EncodedOID * ACDefined(void) const
  {
    if (UniPortion())
      return _pdu.uni->ACDefined();
    TCMsgDlgPortion * dlg = DlgPortion();
    return (dlg && dlg->Get()) ? dlg->Get()->ACDefined() : 0;
  }

  TCUserInformation * usrInfo(void)
  {
    if (UniPortion())
      return _pdu.uni->usrInfo();
    if (DlgPortion())
      return _pdu.dlg->usrInfo();
    return AbortPortion() ? _pdu.abrt->usrInfo() : 0;
  }

  ROSComponentsList & CompList(void) { return _compPart; }

  using ASTypeAC::encode;
  using ASTypeAC::decode;
  using ASTypeAC::deferredDecode;
  // ---------------------------------
  // -- ASTypeAC interface methods
  // ---------------------------------
  //REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
  ENCResult encode(OCTBuffer & use_buf, EncodingRule use_rule = TransferSyntax::ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valDecoded,
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult decode(const OCTBuffer & use_buf, EncodingRule use_rule = TransferSyntax::ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valMixed | valDecoded
  //NOTE: in case of valMixed keeps references to BITBuffer !!!
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult deferredDecode(const OCTBuffer & use_buf, EncodingRule use_rule = TransferSyntax::ruleDER)
    /*throw ASN1CodecError*/;


  // ***********************************************
  // * Message construction helper methods
  // ***********************************************

  //creates TCAPMessage for initiation of transaction.
  //Desired AC may be specified.
  //TCMsgDlgPortion * TBegin(const EncodedOID * app_ctx = 0);
  //creates TCAPMessage for continuing just requested transaction (T_Begin
  //just got with AC). Desired AC must be specified (either the same as
  //requested or another one).
  //TCMsgDlgPortion * TContAC(const EncodedOID & app_ctx);
  //creates TCAPMessage for continuing already established transaction
  //TCMsgDlgPortion * TCont(void);
  //creates TCAPMessage for T_End(basic)
  //TCMsgDlgPortion * TEnd(void);
  //creates TCAPMessage for aborting just requested transaction
  //(T_Begin just got with AC) because of requested AC is unsupported.
  //Suggested AC may be specified (see ITU Q.771 clause 3.1.2.2.2)
  //TCMsgAbortPortion * UAbortAC(const EncodedOID * sugg_ctx = 0);
  //
  //TCMsgAbortPortion * UAbort(/*TODO: add abort reason*/); 
};

} //proto
} //tcap
} //eyeline

#endif /* __TCAP_MESSAGE_DEFS_HPP */

