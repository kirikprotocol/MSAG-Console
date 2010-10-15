#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtdec/DecodeExternal.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
/* ************************************************************************* *
 * Class DecoderOfExternal implementation:
 * ************************************************************************* */
//EXTERNAL type is represented by SEQUENCE with following definition
//
//[UNIVERSAL 8] IMPLICIT SEQUENCE {
//           direct-reference       OBJECT IDENTIFIER OPTIONAL,
//           indirect-reference     INTEGER OPTIONAL,
//           data-value-descriptor  ObjectDescriptor OPTIONAL,
//           encoding               EmbeddedEncoding
//          }
void DecoderOfExternal::construct(void)
{
  DecoderOfSequence_T<4>::setField(0, asn1::_tagObjectID, EDAlternative::altOPTIONAL);
  DecoderOfSequence_T<4>::setField(1, asn1::_tagINTEGER, EDAlternative::altOPTIONAL);
  DecoderOfSequence_T<4>::setField(2, asn1::_tagObjDescriptor, EDAlternative::altOPTIONAL);
  DecoderOfSequence_T<4>::setField(3, DecoderOfEmbdEncoding::_tagOptions, EDAlternative::altMANDATORY);
}

// ----------------------------------------
// -- DecoderOfStructAC interface methods
// ----------------------------------------
//If necessary, allocates optional element and initializes associated TypeDecoderAC
TypeDecoderAC * 
  DecoderOfExternal::prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */
{
  if (!_dVal) //assertion!!!
    throw smsc::util::Exception("ber::DecoderOfExternal : value isn't set!");
  if (unique_idx > 3)
    throw smsc::util::Exception("ber::DecoderOfExternal::prepareAlternative() : undefined UId");

  if (!unique_idx) {
    _decDRef.init(getTSRule()).setValue(_dVal->_asOid);
    return _decDRef.get();
  }
  if (unique_idx == 1) {
    _decIRef.init(getTSRule()).setValue(_dVal->_prsCtxId);
    return _decIRef.get();
  }
  if (unique_idx == 2) {
    _decDescr.init(getTSRule()).setValue(_dVal->_descr);
    return _decDescr.get();
  }
  //if (unique_idx == 3)
  _dvEnc.setValue(_dVal->_enc);
  return &_dvEnc;
}

//Performs actions upon successfull optional element decoding
void DecoderOfExternal::markDecodedOptional(uint16_t unique_idx) /*throw() */
{
  if (unique_idx == 1)
    _dVal->setPrsContext();
  if (unique_idx == 0)
    _dVal->setASyntaxOID();
}

} //ber
} //asn1
} //eyeline

