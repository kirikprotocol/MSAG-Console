/* ************************************************************************** *
 * Classes implementing TCAP structured dialogue PDUs according to
 * itu-t recommendation q 773 modules(2) dialoguePDUs(2) version1(1).
 * ************************************************************************** */
#ifndef __TC_STR_DIALOGUE_DEFS_HPP
#ident "@(#)$Id$"
#define __TC_STR_DIALOGUE_DEFS_HPP

#include "eyeline/tcap/proto/TCAssociateDiagnostic.hpp"
#include "eyeline/tcap/proto/TCUserInfo.hpp"
#include "eyeline/util/MaxSizeof.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

extern EncodedOID _ac_tcap_strDialogue_as;

//Base class for dialogue PDUs
class TCDlgPduAC : public ASTypeAC {
protected:
  TCUserInformation   _usrInfo;    //optional

public:
  enum ProtoVersion_e { protoVersion1 = 0 };
  enum PDUKind_e { pduNone = -1, pduAARQ = 0, pduAARE = 1, pduABRT = 4 };

  TCDlgPduAC(PDUKind_e use_pdu)
    : ASTypeAC(ASTag::tagApplication, use_pdu)
  { }

  TCUserInformation & usrInfo(void) { return _usrInfo; }

  // -- TCDlgPduAC interface methods:
  virtual const EncodedOID * ACDefined(void) const = 0;
};

//Structured TC Dialogue request APDU (AARQ)
class TCReqPDU : public TCDlgPduAC {
protected:
  unsigned    _protoVer;  //BIT STING
  EncodedOID  _acId;      //mandatory!!!

public:
  TCReqPDU() : TCDlgPduAC(TCDlgPduAC::pduAARQ)
    , _protoVer(TCDlgPduAC::protoVersion1)
  { }
  ~TCReqPDU()
  { }

  void setAppCtx(const EncodedOID & use_acid) { _acId = use_acid; }

  // -- TCDlgPduAC interface methods
  const EncodedOID * ACDefined(void) const
  {
    return _acId.length() ? &_acId : 0;
  } 

  // ---------------------------------
  // -- ASTypeAC interface methods
  // ---------------------------------
  using ASTypeAC::encode;
  using ASTypeAC::decode;
  using ASTypeAC::deferredDecode;
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
};


//Structured TC Dialogue response APDU (AARE)
class TCRespPDU : public TCDlgPduAC {
public:
  enum AssociateResult_e {
    dlgAccepted = 0, dlgRejectPermanent = 1
  };

protected:
  unsigned    _protoVer;  //BIT STING
  EncodedOID  _acId;      //mandatory!!!

  AssociateResult_e           _result;
  AssociateSourceDiagnostic   _diagnostic;

public:
  TCRespPDU() : TCDlgPduAC(TCDlgPduAC::pduAARE)
    , _protoVer(TCDlgPduAC::protoVersion1), _result(dlgAccepted)
  { }
  ~TCRespPDU()
  { }

  void acceptByUser(void)
  {
    _result = dlgAccepted; _diagnostic.setUserDiagnostic();
  }
  void acceptByPrvd(void)
  {
    _result = dlgAccepted; _diagnostic.setPrvdDiagnostic();
  }

  void rejectByUser(AssociateSourceDiagnostic::DiagnosticUser_e use_cause =
                        AssociateSourceDiagnostic::dsu_null)
  {
    _result = dlgRejectPermanent; _diagnostic.setUserDiagnostic(use_cause);
  }
  void rejectByPrvd(AssociateSourceDiagnostic::DiagnosticProvider_e use_cause =
                        AssociateSourceDiagnostic::dsp_null)
  {
    _result = dlgRejectPermanent; _diagnostic.setPrvdDiagnostic(use_cause);
  }

  void setAppCtx(const EncodedOID & use_acid) { _acId = use_acid; }

  AssociateResult_e result(void) const { return _result; }
  const AssociateSourceDiagnostic & diagnostic(void) const { return _diagnostic; }

  // -- TCDlgPduAC interface methods
  const EncodedOID * ACDefined(void) const
  {
      return _acId.length() ? &_acId : 0;
  }

  // ---------------------------------
  // -- ASTypeAC interface methods
  // ---------------------------------
  using ASTypeAC::encode;
  using ASTypeAC::decode;
  using ASTypeAC::deferredDecode;

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
};

//Structured TC Dialogue abort APDU (ABRT)
class TCAbrtPDU : public TCDlgPduAC {
public:
  enum AbortSource_e {
    dlg_srv_user = 0, dlg_srv_provider = 1 
  };

protected:
  AbortSource_e  _abrtSrc;

public:
  TCAbrtPDU(AbortSource_e abrt_src = dlg_srv_provider)
    : TCDlgPduAC(TCDlgPduAC::pduABRT), _abrtSrc(abrt_src)
  { }
  ~TCAbrtPDU()
  { }

  void setAbortSource(AbortSource_e abrt_src) { _abrtSrc = abrt_src; }

  AbortSource_e  abortSource(void) const { return _abrtSrc; }
  // -- TCDlgPduAC interface methods
  const EncodedOID * ACDefined(void) const { return 0; }

  // ---------------------------------
  // -- ASTypeAC interface methods
  // ---------------------------------
  using ASTypeAC::encode;
  using ASTypeAC::decode;
  using ASTypeAC::deferredDecode;
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
};

// TC Structured Dialogue ABSTRACT SYNTAX
class TCStrDialogueAS : public AbstractSyntax {
private:
  uint8_t pduMem[eyeline::util::MaxSizeOf3_T< TCReqPDU,
                                              TCRespPDU, TCAbrtPDU >::VALUE];
  TCDlgPduAC::PDUKind_e _pduKind;
  union {
    TCDlgPduAC *    ac;     //APDU's base class
    TCReqPDU *      req;    //[APPLICATION 0]
    TCRespPDU *     resp;   //[APPLICATION 1]
    TCAbrtPDU *     abrt;   //[APPLICATION 4]
  } _pdu;

protected:
  void resetPdu(void)
  {
    if (_pdu.ac) {
      _pdu.ac->~TCDlgPduAC();
      _pdu.ac = 0;
    }
  }

public:
  TCStrDialogueAS(TCDlgPduAC::PDUKind_e use_pdu = TCDlgPduAC::pduNone)
      : AbstractSyntax(_ac_tcap_strDialogue_as)
  {
    asTags().addOption(ASTagging(ASTag::tagApplication, TCDlgPduAC::pduAARQ));
    asTags().addOption(ASTagging(ASTag::tagApplication, TCDlgPduAC::pduAARE));
    asTags().addOption(ASTagging(ASTag::tagApplication, TCDlgPduAC::pduABRT));
    //
    _pdu.ac = 0;
    if ((_pduKind = use_pdu) != TCDlgPduAC::pduNone)
        Reset(use_pdu);
  }
  ~TCStrDialogueAS()
  {
    resetPdu();
  }

  TCDlgPduAC * Reset(TCDlgPduAC::PDUKind_e use_pdu = TCDlgPduAC::pduNone)
  {
    resetPdu();
    switch ((_pduKind = use_pdu)) {
    case TCDlgPduAC::pduAARQ: _pdu.req = new(pduMem)TCReqPDU(); break;
    case TCDlgPduAC::pduAARE: _pdu.resp = new(pduMem)TCRespPDU(); break;
    case TCDlgPduAC::pduABRT: _pdu.abrt = new(pduMem)TCAbrtPDU(); break;
    default:; //pduNone
    }
    asTags().selectOption(ASTag(ASTag::tagApplication, use_pdu));
    return _pdu.ac;
  }

  TCDlgPduAC::PDUKind_e PDUKind(void) const { return _pduKind; }
  TCDlgPduAC * Get(void)  { return _pdu.ac; }

  TCReqPDU *  Req(void)   { return _pduKind == TCDlgPduAC::pduAARQ ? _pdu.req : 0; }
  TCRespPDU * Resp(void)  { return _pduKind == TCDlgPduAC::pduAARE ? _pdu.resp : 0; }
  TCAbrtPDU * Abrt(void)  { return _pduKind == TCDlgPduAC::pduABRT ? _pdu.abrt : 0; }

  TCUserInformation * usrInfo(void)
  {
      return _pdu.ac ? &(_pdu.ac->usrInfo()) : 0;
  }

  // ---------------------------------
  // -- ASTypeAC interface methods
  // ---------------------------------
  using ASTypeAC::encode;
  using ASTypeAC::decode;
  using ASTypeAC::deferredDecode;
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
};

} //proto
} //tcap
} //eyeline

#endif /* __TC_STR_DIALOGUE_DEFS_HPP */

