/* ************************************************************************** *
 * Classes implementing TCAP UNIDialogue PDUs.
 * ************************************************************************** */
#ifndef __TC_DIALOGUE_UNI_DEFS_HPP
#ident "@(#)$Id$"
#define __TC_DIALOGUE_UNI_DEFS_HPP

#include "eyeline/tcap/proto/TCUserInfo.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

extern EncodedOID _ac_tcap_uniDialogue_as;


class TCAudtPDU : public ASTypeAC {
protected:
  unsigned          _protoVer;  //BIT STING
  EncodedOID        _acId;      //mandatory!!!
  TCUserInformation _usrInfo;   //optional

public:
  enum ProtoVersion_e { protoVersion1 = 0 };

  TCAudtPDU() : ASTypeAC(ASTag::tagApplication, 0)
    , _protoVer(protoVersion1)
  { }
  ~TCAudtPDU()
  { }
  //TODO:

  void setAppCtx(const EncodedOID & use_acid) { _acId = use_acid; }
  TCUserInformation & usrInfo(void) { return _usrInfo; }
  // 
  const EncodedOID * ACDefined(void) const
  {
    return _acId.length() ? &_acId : 0;
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

class TCUniDialogueAS : public AbstractSyntax { //uniDialogue-as
protected:
  TCAudtPDU _pdu;

public:
  enum PDUKind_e { pduAUDT = 0};

  TCUniDialogueAS() : AbstractSyntax(_ac_tcap_uniDialogue_as)
  {
    asTags().addOption(*_pdu.Tagging(), true);
  }

  TCUserInformation * usrInfo(void)
  {
    return &_pdu.usrInfo();
  }
  const EncodedOID * ACDefined(void) const
  {
    return _pdu.ACDefined();
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

#endif /* __TC_DIALOGUE_UNI_DEFS_HPP */

