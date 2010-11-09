/* ************************************************************************* *
 * BER Decoder methods: EXTERNAL type decoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_EXTERNAL
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_DECODER_EXTERNAL

#include "eyeline/asn1/ASExternal.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeINT.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeEOID.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeObjDescr.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeEmbdEnc.hpp"
#include "eyeline/asn1/BER/rtdec/DecoderProducer.hpp"


namespace eyeline {
namespace asn1 {
namespace ber {

//According to X.690 cl.8.18.1,
//EXTERNAL type is represented by SEQUENCE with following definition
//
//[UNIVERSAL 8] IMPLICIT SEQUENCE {
//           direct-reference       OBJECT IDENTIFIER OPTIONAL,
//           indirect-reference     INTEGER OPTIONAL,
//           data-value-descriptor  ObjectDescriptor OPTIONAL,
//           encoding               EmbeddedEncoding
//          }
class DecoderOfExternal : public DecoderOfSequence_T<4> {
private:
  using DecoderOfSequence_T<4>::setField;

protected:
  asn1::ASExternal *      _dVal;
  DecoderOfEmbdEncoding   _dvEnc;
  //Optional fields
  DecoderProducer_T<DecoderOfEOID>          _decDRef;
  DecoderProducer_T<DecoderOfINTEGER>       _decIRef;
  DecoderProducer_T<DecoderOfObjDescriptor> _decDescr;

  //constructs decoder of SEQUENCE representing EXTERNAL
  void construct(void);

  // ----------------------------------------
  // -- DecoderOfStructAC interface methods
  // ----------------------------------------
  //If necessary, allocates optional element and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;
  //Performs actions upon successfull optional element decoding
  virtual void markDecodedOptional(uint16_t unique_idx) /*throw() */;

public:
  // constructor for untagged EXTERNAL
  explicit DecoderOfExternal(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfSequence_T<4>(asn1::_tagsEXTERNAL, use_rule)
    , _dVal(0), _dvEnc(use_rule)
  {
    construct();
  }
  // constructor for tagged EXTERNAL
  DecoderOfExternal(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfSequence_T<4>(ASTagging(use_tag, tag_env, asn1::_tagsEXTERNAL), use_rule)
    , _dVal(0), _dvEnc(use_rule)
  {
    construct();
  }
  ~DecoderOfExternal()
  { }

  void setValue(asn1::ASExternal & use_val) { _dVal = &use_val; }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_EXTERNAL */

