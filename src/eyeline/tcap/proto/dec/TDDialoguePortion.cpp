#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/proto/dec/TDDialoguePortion.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

/* ************************************************************************* *
 * Class TDDialoguePortion implementation:
 * ************************************************************************* */
const asn1::ASTag
  TDDialoguePortion::_typeTag(asn1::ASTag::tagApplication, 11);
//complete tagging
const asn1::ASTagging
  TDDialoguePortion::_typeTags(_typeTag, asn1::ASTagging::tagsEXPLICIT, asn1::_tagsEXTERNAL);


void TDDialoguePortion::cleanEnc(void)
{
  if (_decEnc._uni) {
    if (_decKind == proto::TCDlgPortion::asUNI)
      _decEnc._uni->~TDAPduAUDT();
    else if (_decKind == proto::TCDlgPortion::asDLG)
      _decEnc._dlg->~TDStrDialoguePdu();
    else
      _decEnc._embd->~DecoderOfEmbdEncoding();
    _decEnc._uni = NULL;
    _decKind = proto::TCDlgPortion::asNone;
  }
}

void TDDialoguePortion::cleanAll(void)
{
  cleanEnc();
  if (_decDRef) {
    _decDRef->~DecoderOfEOID();
    _decDRef = NULL;
  }
  if (_decIRef) {
    _decIRef->~DecoderOfINTEGER();
    _decIRef = NULL;
  }
  if (_decDescr) {
    _decDescr->~DecoderOfObjDescriptor();
    _decDescr = NULL;
  }
}

/*
DialoguePortion ::= [APPLICATION 11] EXPLICIT [UNIVERSAL 8] IMPLICIT SEQUENCE {
    direct-reference       OBJECT IDENTIFIER OPTIONAL,
    indirect-reference     INTEGER OPTIONAL,
    data-value-descriptor  ObjectDescriptor OPTIONAL,
    encoding               CHOICE {
        dRef-dialogue-as          StrDialogueAS,
        dRef-unidialogue-as       TCPduAUDT,
        other                     EmbeddedEncoding
    }
} */
void TDDialoguePortion::construct(void)
{
  asn1::ber::DecoderOfSequence_T<4, 2>::setField(0, asn1::_tagObjectID, asn1::ber::EDAlternative::altOPTIONAL);
  asn1::ber::DecoderOfSequence_T<4, 2>::setField(1, asn1::_tagINTEGER, asn1::ber::EDAlternative::altOPTIONAL);
  asn1::ber::DecoderOfSequence_T<4, 2>::setField(2, asn1::_tagObjDescriptor, asn1::ber::EDAlternative::altOPTIONAL);
  asn1::ber::DecoderOfSequence_T<4, 2>::setField(3, asn1::ber::DecoderOfEmbdEncoding::_tagOptions,
                                                              asn1::ber::EDAlternative::altMANDATORY);
}

// ----------------------------------------
// -- DecoderOfStructAC interface methods
// ----------------------------------------
//If necessary, allocates optional element and initializes associated TypeDecoderAC
asn1::ber::TypeDecoderAC *
  TDDialoguePortion::prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */
{
  if (!_dVal) //assertion!!!
    throw smsc::util::Exception("ber::TDDialoguePortion : value isn't set!");
  if (unique_idx > 3)
    throw smsc::util::Exception("ber::TDDialoguePortion::prepareAlternative() : undefined UId");

  if (!unique_idx) {
    _decDRef = new (_memDRef._buf) asn1::ber::DecoderOfEOID(getTSRule());
    _decDRef->setValue(_dVal->getEXT()->front()._asOid);
    return _decDRef;
  }
  if (unique_idx == 1) {
    _decIRef = new (_memIRef._buf) asn1::ber::DecoderOfINTEGER(getTSRule());
    _decIRef->setValue(_dVal->getEXT()->front()._prsCtxId);
    return _decIRef;
  }
  if (unique_idx == 2) {
    _decDescr = new (_memDescr._buf) asn1::ber::DecoderOfObjDescriptor(getTSRule());
    _decDescr->setValue(_dVal->getEXT()->front()._descr);
    return _decDescr;
  }
  //if (unique_idx == 3)
  //determine necessary structure for encoding field
  if (_dVal->getEXT()->front().hasASyntaxOID()
      && !_dVal->getEXT()->front().hasPrsContextId()) {
    if (_dVal->getEXT()->front()._asOid == _ac_tcap_strDialogue_as) {
      //structured dialogue PDU is expected
      initEncDLG(_dVal->initDLG());
      return _decEnc._dlg;
    }
    if (_dVal->getEXT()->front()._asOid == _ac_tcap_uniDialogue_as) {
      //Unidialogue PDU is expected
      initEncUNI(_dVal->initUNI());
      return _decEnc._uni;
    }
  }
  //keep going with ASExternal
  initEncEXT(_dVal->getEXT()->front()._enc);
  return _decEnc._embd;
}

//Performs actions upon successfull optional element decoding
void TDDialoguePortion::markDecodedOptional(uint16_t unique_idx) /*throw() */
{
  //NOTE: _dVal->getEXT() may be NULL only for unique_idx == 3
  if (!unique_idx)
    _dVal->getEXT()->front().setASyntaxOID();
  if (unique_idx == 1)
    _dVal->getEXT()->front().setPrsContext();
}

} //dec
} //proto
} //tcap
} //eyeline

