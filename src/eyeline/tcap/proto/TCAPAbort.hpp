/* ************************************************************************** *
 * TCAP message: Abort (according to
 * itu-t(0) recommendation(0) q(17) 773 modules(2) messages(1) version3(3)).
 * ************************************************************************** */
#ifndef __TCAP_MESSAGE_ABORT_HPP
#ident "@(#)$Id$"
#define __TCAP_MESSAGE_ABORT_HPP

#include "eyeline/tcap/proto/TCStrDialogue.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

//TCAP Abort message
//May be the one of following alternatives:
//  1) provider abort indicated by P_AbortCause
//  2) user abort indicated by some user defined pdu
//  3) structured dialog (_ac_tcap_strDialogue_as) ABRT-apdu, that may
//     indicate either user or provider abort
class TCMsgAbort : public ASTypeAC {
public:
  enum AbortForm_e {
    abrtFrmProvider = 0   //provider abort indicated by P_AbortCause
    , abrtFrmUser         //user abort indicated by some user defined pdu
    , abrtFrmDialog       //structured dialog ABRT-apdu, that may
                          //indicate either user or provider abort
  };
  enum PAbortCause_e {
    p_unrecognizedMessageType = 0
    , p_unrecognizedTransactionID = 1
    , p_badlyFormattedTransactionPortion = 2
    , p_incorrectTransactionPortion = 3
    , p_resourceLimitation = 4
    , p_maxCause = 127  //That's a max value allowed by protocol
    , p_noReason = 0xFF
  };

private:
  uint8_t pduMem[eyeline::util::MaxSizeOf2_T<TCAbrtPDU, TCExternal>::VALUE];
  union {
    ASTypeAC *    ac;    //PDU's base class
    TCAbrtPDU *   dlg;
    TCExternal *  usr;
  } _pdu;
  AbortForm_e     _abrtForm;
  PAbortCause_e   _pabrtCause; //equal to p_noReason if 
                               //TCAbrtPDU.AbortSource() == dlg_srv_provider
protected:
  void resetPdu(void)
  {
    if (_pdu.ac) {
      _pdu.ac->~ASTypeAC();
      _pdu.ac = 0;
    }
  }

public:
  TCMsgAbort(AbortForm_e use_kind = abrtFrmProvider)
      : ASTypeAC(ASTag::tagApplication, 7)
  {
    _pdu.ac = 0;
    Reset(use_kind);
  }
  ~TCMsgAbort()
  {
    resetPdu();
  }

  AbortForm_e AbortForm(void) const { return _abrtForm; }

  void Reset(AbortForm_e use_kind = abrtFrmProvider)
  {
    resetPdu();
    switch ((_abrtForm = use_kind)) {
    case abrtFrmDialog:   _pdu.ac = new(pduMem)TCAbrtPDU(); break;
    case abrtFrmUser:     _pdu.ac = new(pduMem)TCExternal(); break;
    default: //abrtFrmProvider
      _pabrtCause = p_noReason;
    }
    return;
  }

  TCAbrtPDU::AbortSource_e  AbortSource(void) const
  {
    switch (_abrtForm) {
    case abrtFrmDialog:
      return _pdu.dlg->abortSource();
    case abrtFrmUser:
      return TCAbrtPDU::dlg_srv_user;
    default:; //abrtFrmProvider
    }
    return TCAbrtPDU::dlg_srv_provider;
  }

  bool PAbortCause(PAbortCause_e & pabrt_cause) const
  {
    if (_abrtForm == abrtFrmProvider) {
      pabrt_cause = _pabrtCause;
      return true;
    }
    if ((_abrtForm == abrtFrmDialog)
        && (_pdu.dlg->abortSource() == TCAbrtPDU::dlg_srv_provider)) {
      pabrt_cause = p_noReason;
      return true;
    }
    return false;
  }

  const TCExternal *  UAbortCause(void) const
  {
    if (_abrtForm == abrtFrmDialog)
      return _pdu.dlg->usrInfo().first();
    return _abrtForm == abrtFrmUser ? _pdu.usr : 0;
  }

  TCUserInformation * usrInfo(void)
  {
    return _abrtForm == abrtFrmDialog ? &(_pdu.dlg->usrInfo()) : 0;
  }

  // ---------------------------------
  // -- ASTypeAC interface methods
  // ---------------------------------
  //REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
  ENCResult Encode(BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valDecoded,
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult Decode(const BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valMixed | valDecoded
  //NOTE: in case of valMixed keeps references to BITBuffer !!!
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult DeferredDecode(const BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    /*throw ASN1CodecError*/;
};


} //proto
} //tcap
} //eyeline

#endif /* __TCAP_MESSAGE_ABORT_HPP */

