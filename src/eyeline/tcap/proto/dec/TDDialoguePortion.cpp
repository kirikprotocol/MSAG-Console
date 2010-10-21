#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

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


void TDDialoguePortion::clearAll(void)
{
  _decAS.clear();
  _decDRef.clear();
  _decIRef.clear();
  _decDescr.clear();
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
  setField(0, asn1::_tagObjectID, asn1::ber::EDAlternative::altOPTIONAL);
  setField(1, asn1::_tagINTEGER, asn1::ber::EDAlternative::altOPTIONAL);
  setField(2, asn1::_tagObjDescriptor, asn1::ber::EDAlternative::altOPTIONAL);
  setField(3, asn1::ber::DecoderOfEmbdEncoding::_tagOptions, asn1::ber::EDAlternative::altMANDATORY);
}

// ----------------------------------------
// -- DecoderOfStructAC interface methods
// ----------------------------------------
//NOTE: input encoding is decoded first to intermediate ASExternal value,
//      if contained encoding belongs to one of defined TCAP AbstractSyntaxes
//      then decoded value is reinitialized accordingly.
asn1::ber::TypeDecoderAC *
  TDDialoguePortion::prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */
{
  if (!_dVal) //assertion!!!
    throw smsc::util::Exception("ber::TDDialoguePortion : value isn't set!");
  if (unique_idx > 3)
    throw smsc::util::Exception("ber::TDDialoguePortion::prepareAlternative() : undefined UId");

  if (!unique_idx) {
    _decDRef.init(getTSRule()).setValue(_dVal->ext().get()->front()._asOid);
    return _decDRef.get();
  }
  if (unique_idx == 1) {
    _decIRef.init(getTSRule()).setValue(_dVal->ext().get()->front()._prsCtxId);
    return _decIRef.get();
  }
  if (unique_idx == 2) {
    _decDescr.init(getTSRule()).setValue(_dVal->ext().get()->front()._descr);
    return _decDescr.get();
  }
  //if (unique_idx == 3)
  //determine necessary structure for encoding field
  if (_dVal->ext().get()->front().hasASyntaxOID()
      && !_dVal->ext().get()->front().hasPrsContextId()) {
    if (_dVal->ext().get()->front()._asOid == _ac_tcap_strDialogue_as) {
      //structured dialogue PDU is expected, reinit _dVal
      _decAS.dlg().init(getTSRule()).setValue(_dVal->dlg().init());
      return _decAS.get();
    }
    if (_dVal->ext().get()->front()._asOid == _ac_tcap_uniDialogue_as) {
      //Unidialogue PDU is expected, reinit _dVal
      _decAS.uni().init(getTSRule()).setValue(_dVal->uni().init());
      return _decAS.get();
    }
  } 
  //keep going with ASExternal
  _decAS.embd().init(getTSRule()).setValue(_dVal->ext().get()->front()._enc);
  return _decAS.get();
}

//Performs actions upon successfull optional element decoding
void TDDialoguePortion::markDecodedOptional(uint16_t unique_idx) /*throw() */
{
  //NOTE: _dVal->ext().get() may be NULL only for unique_idx == 3
  if (!unique_idx)
    _dVal->ext().get()->front().setASyntaxOID();
  if (unique_idx == 1)
    _dVal->ext().get()->front().setPrsContext();
}

} //dec
} //proto
} //tcap
} //eyeline

